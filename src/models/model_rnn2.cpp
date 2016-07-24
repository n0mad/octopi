#include "model_rnn2.h"
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

TModelRNN2::TModelRNN2(const string &fileName)
    : Observed(0)
{
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cerr << "cannot read input file for the rnnmodel " << fileName << endl;
        exit(-2);
    };

    picojson::value v;
    inputFile >> v;
    string err = picojson::get_last_error();

    if (!err.empty()) {
        cerr << err << endl;
        exit(-3);
    };

    if (!v.is<picojson::array>()) {
        cerr << "JSON is not an array" << endl;
        exit(-4);
    };

    const picojson::value::array &list = v.get<picojson::array>();
    if (list.size() < 7) {
        cerr << "Incorrect JSON" << endl;
        exit(-4);
    };

    string model_type = list[0].get<string>();
    if (model_type != "rnn") {
        cerr << "rnn type model expected, got " << model_type << endl;
        exit(-5);
    };

    int layer_size = list[1].get<double>();

    const picojson::value::array &chars = list[2].get<picojson::array>();
    int n_chars = chars.size();
    for(int i = 0; i < n_chars; ++i) {
        Chars.push_back(chars[i].get<double>());
    };

    Embedding = MatrixXd(n_chars, layer_size);
    ParseMatrix(Embedding, list[3]);

    Softmax_W = MatrixXd(layer_size, n_chars);
    ParseMatrix(Softmax_W, list[4]);

    Softmax_B = MatrixXd(n_chars, 1);
    ParseVector(Softmax_B, list[5]);

    for (int i = 6; i < list.size(); ++i) {
        //layers
        const picojson::value::array &layer = list[i].get<picojson::array>();

        Matrixes.push_back(MatrixXd(2 * layer_size, layer_size));
        ParseMatrix(Matrixes.back(), layer[0]);

        Biases.push_back(MatrixXd(layer_size, 1));
        ParseVector(Biases.back(), layer[1]);

        States.push_back(VectorXd(layer_size));
        States.back() *= 0;
    };

    UpdateSpace();
};

void TModelRNN2::Reset()
{
    for (int i = 0; i < States.size(); ++i)
        States[i] *= 0;

    UpdateSpace();
};


void TModelRNN2::Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &normalizer)
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

uint8 TModelRNN2::Decode(uint32 value, uint32 &lower_count, uint32 &upper_count)
{
    double value_d = value;
    value_d /= NORMALIZER;
    if (value_d > 1.) {
        cerr << "value_d above 1" << endl;
        abort();
    };

    double lower_bound = 0;
    int i = 0;
    for (i = 0; i < Chars.size() && lower_bound + Space[i] <= value_d; ++i)
        lower_bound += Space[i];
    if (i >= Space.size()) {
        cerr << "i too big" << value_d << "space norm" << (Space.array().sum() < 1 ? "smaller" : "not smaller") << endl;
        DumpSpace();
        abort();
        i = Space.size() - 1;
    };

    double upper_bound = lower_bound + Space[i];
    if (upper_bound > 1.) {
        cerr << "unnormalized: " << upper_bound << " " << i << " " << Space[i] << endl;
        upper_bound = 1.;
    };
    lower_count   = (uint32) (NORMALIZER * lower_bound);
    upper_count = (uint32) (NORMALIZER * upper_bound);

    return Chars[i];
};

void TModelRNN2::UpdateSpace() {
    const VectorXd &top_layer_state = States.back();
    VectorXd output = Softmax_W.transpose() * top_layer_state;
    output += Softmax_B;

    double d = output.maxCoeff();
    output.array() -= d;
    auto output2 = output.unaryExpr<double(*)(double)>(&std::exp);

    Space = output2.array() / output2.sum();
    //Space.array() /= Space.sum();
    Space = output2.array() / output2.sum() * LAMBDA + (1. - LAMBDA) / Space.size();
};

void TModelRNN2::DumpSpace() {
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

void TModelRNN2::DumpState() {
    //cout << State_0 << endl;
    //cout << State_1 << endl;
};


void TModelRNN2::Observe(uint8 symbol) {
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

    VectorXd input = Embedding.row(i);

    /*if (0 == Observed % 100) {
        Observed = 0;
    };*/

    for (int i = 0; i < States.size(); ++i) {
        Eigen::VectorXd concat(2 * States[i].rows());
        concat << input, States[i];

        States[i] = Matrixes[i].transpose() * concat + Biases[i];
        States[i] = States[i].unaryExpr([](double elem) { return std::tanh(elem); });

        //TODO fix
        input = States[i];
    };

    Observed += 1;
    UpdateSpace();
};
