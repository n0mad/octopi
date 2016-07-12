#include "model_rnn.h"
#include "./../picojson/picojson.h"
#include <fstream>
#include <math.h>

using namespace std;
using namespace Eigen;

static void ParseVector(MatrixXd &v, const picojson::value &o) {
    const picojson::value::array &b = o.get<picojson::value::array>();
    for(int j = 0; j < b.size(); ++j) {
        double d = b[j].get<double>();
        v(j) = d;
    };
};

static void ParseMatrix(MatrixXd &m, const picojson::value &o) {
    const picojson::value::array &b = o.get<picojson::value::array>();
    for(int i = 0; i < b.size(); ++i) {
        const picojson::value::array &row = b[i].get<picojson::value::array>();
        for(int j = 0; j < row.size(); ++j) {
            double d = row[j].get<double>();
            m(i, j) = d;
        };
    };
};

TModelRNN::TModelRNN(const string &fileName)
    : Softmax_B(207, 1)
    , RnnBias(256, 1)
    , State(256)
    , Space(207)
    , Embedding(207, 256)
    , RnnW(256, 512)
    , Softmax_W(256, 207)
    , Concat(512)
{
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cerr << "cannot reading input file for the rnnmodel";
        exit(-2);
    };

    picojson::value v;
    inputFile >> v;
    string err = picojson::get_last_error();

    if (!err.empty()) {
        cerr << err << endl;
        exit(-3);
    };

    if (!v.is<picojson::object>()) {
        cerr << "JSON is not an object" << endl;
        exit(-4);
    };

    const picojson::value::object &root = v.get<picojson::object>();
    for (picojson::value::object::const_iterator i = root.begin(); i != root.end(); ++i) {
        if (i->first == "softmax_b") {
            const picojson::value &b_v = i->second;
            ParseVector(Softmax_B, b_v);
            continue;
        };
        if (i->first == "rnn_b") {
            const picojson::value &b_v = i->second;
            ParseVector(RnnBias, b_v);
            continue;
        };
        if (i->first == "embeddings") {
            const picojson::value &b_v = i->second;
            ParseMatrix(Embedding, b_v);
            continue;
        };
        if (i->first == "softmax_w") {
            const picojson::value &b_v = i->second;
            ParseMatrix(Softmax_W, b_v);
            continue;
        };
        if (i->first == "rnn_w") {
            const picojson::value &b_v = i->second;
            ParseMatrix(RnnW, b_v);
            continue;
        };
        if (i->first == "chars") {
            const picojson::value::array &b = i->second.get<picojson::value::array>();
            //TODO: use the picojson int
            for(int i = 0; i < b.size(); ++i) {
                Chars.push_back(b[i].get<double>());
            };
            continue;
        };
    };
    State *= 0.;
    UpdateSpace();
};

void TModelRNN::Reset()
{
    State *= 0.;
    UpdateSpace();
};


void TModelRNN::Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &normalizer)
{
    low_count = 0;
    upper_count = 0;
    normalizer = 0;

    double lower_bound = 0;
    int i = 0;
    for (; i < Chars.size() && Chars[i] != symbol; ++i) {
        lower_bound += Space[i];
    };

    double upper_bound = lower_bound + Space[i];

    low_count   = (uint32) (NORMALIZER * lower_bound);
    upper_count = (uint32) (NORMALIZER * upper_bound);
    normalizer = NORMALIZER;
};

uint8 TModelRNN::Decode(uint32 value, uint32 &lower_count, uint32 &upper_count)
{
    double value_d = value;
    value_d /= NORMALIZER;
    value_d = value_d > 1 ? 1. : value_d;

    double lower_bound = 0;
    int i = 0;
    for (i = 0; i < Chars.size() && lower_bound + Space[i] <= value_d; ++i)
        lower_bound += Space[i];
    if (i >= Space.size()) {
        cerr << "i too big" << value_d << endl;
        abort();
    };

    double upper_bound = lower_bound + Space[i];
    if (upper_bound > 1.) {
        cerr << "unnormalized: " << upper_bound << " " << i << " " << Space[i] << endl;
        //abort();
        upper_bound = 1.;
    };
    lower_count   = (uint32) (NORMALIZER * lower_bound);
    upper_count = (uint32) (NORMALIZER * upper_bound);

    return Chars[i];
};

void TModelRNN::UpdateSpace() {
    VectorXd output = Softmax_W.transpose() * State;
    output += Softmax_B;

    auto output2 = output.unaryExpr<double(*)(double)>(&std::exp);
    Space = output2.array() / output2.sum() * LAMBDA + (1. - LAMBDA) / Space.size();
};

void TModelRNN::DumpSpace() {
    double max = 0;
    int argmax = 0;
    for(int i = 0; i < Chars.size(); ++i) {
        cout << Space[i] << " " << Chars[i] << endl;
        if(Space[i] > max) {
            max = Space[i];
            argmax = i;
        };
    };
    cout << "argmax: " << Chars[argmax] << " max: " << max << endl;
};

void TModelRNN::DumpState() {
    cout << State << endl;
};


void TModelRNN::Observe(uint8 symbol) {
    //TODO: binary search or sort
    int i = 0;
    bool found = false;
    for (i = 0; i < Chars.size(); ++i) {
        if (symbol == Chars[i]) {
            found = true;
            break;
        };
    };

    if(!found) {
        cerr << "char not found " << (int) symbol << " or " << symbol << endl;
        abort();
    };
    const VectorXd &emb = Embedding.row(i);

    Concat << emb, State;

    VectorXd result = RnnW * Concat + RnnBias;
    result = result.unaryExpr([](double elem) { return std::tanh(elem); });
    State = result;

    UpdateSpace();
};
