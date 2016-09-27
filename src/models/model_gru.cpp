#include "model_gru.h"
#include "json_util.h"
#include "./../picojson/picojson.h"
#include <fstream>
#include <math.h>

using namespace std;
using namespace Eigen;
using namespace OctopiJsonHelper;

TModelGRU::TModelGRU(const string &fileName)
    : Observed(0)
{
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cerr << "cannot read input file for the gru model " << fileName << endl;
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
        cerr << "Incorrect JSON, too few fields" << endl;
        exit(-4);
    };

    string model_type = list[0].get<string>();
    if (model_type != "gru") {
        cerr << "gru type model expected, got " << model_type << endl;
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

        GateMatrixes.push_back(MatrixXd(2 * layer_size, 2 * layer_size));
        ParseMatrix(GateMatrixes.back(), layer[0]);

        GateBiases.push_back(MatrixXd(2 * layer_size, 1));
        ParseVector(GateBiases.back(), layer[1]);

        CandidateMatrixes.push_back(MatrixXd(2 * layer_size, layer_size));
        ParseMatrix(CandidateMatrixes.back(), layer[2]);

        CandidateBiases.push_back(MatrixXd(layer_size, 1));
        ParseVector(CandidateBiases.back(), layer[3]);

        States.push_back(VectorXd(layer_size));
        States.back() *= 0;
    };

    UpdateSpace();
};

void TModelGRU::Reset()
{
    for (int i = 0; i < States.size(); ++i)
        States[i] *= 0;

    UpdateSpace();
};


void TModelGRU::Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &normalizer)
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
    //cout << "encoding char: " << Chars[i] << " " << lower_bound << " " << upper_bound << endl;
    //DumpSpace();

    low_count   = (uint32) (NORMALIZER * lower_bound);
    upper_count = (uint32) (NORMALIZER * upper_bound);
    normalizer = NORMALIZER;
};

uint8 TModelGRU::Decode(uint32 value, uint32 &lower_count, uint32 &upper_count)
{
    double value_d = value;
    value_d /= NORMALIZER;
    if (value_d > 1.) {
        DumpSpace();
        cerr << "value_d above 1: [ " << value_d << " ]" << endl;
        abort();
    };

    double lower_bound = 0;
    int i = 0;
    for (i = 0; i < Chars.size() && lower_bound + Space[i] < value_d; ++i)
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

    //cout << Chars[i] << " " << lower_bound << " " << upper_bound << endl;
    //DumpSpace();
    return Chars[i];
};

void TModelGRU::UpdateSpace() {
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

void TModelGRU::DumpSpace() {
    double max = 0;
    int argmax = 0;
    for(int i = 0; i < Chars.size(); ++i) {
        cout << Space[i] << " " << Chars[i] << "\t";
        if(Space[i] > max) {
            max = Space[i];
            argmax = i;
        };
    };
    cout << "argmax: " << Chars[argmax] << " max: " << max << endl;
};

void TModelGRU::DumpState() {
    //cout << State_0 << endl;
    //cout << State_1 << endl;
};


void TModelGRU::Observe(uint8 symbol) {
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
    int layer_size = input.size();

    for (int i = 0; i < States.size(); ++i) {
        Eigen::VectorXd input_state(2 * States[i].rows());
        input_state << input, States[i];

        Eigen::VectorXd gates = GateMatrixes[i].transpose() * input_state + GateBiases[i];
        gates = gates.unaryExpr([](double elem) { return 1.0/ (1.0 + std::exp(-elem)); });


        Eigen::VectorXd r = gates.head(layer_size);
        Eigen::VectorXd u = gates.tail(layer_size);

        r.array() = r.array() * States[i].array(); //r coordinate-wise s

        Eigen::VectorXd input_reset_state(2 * States[i].rows());
        input_reset_state << input, r;

        Eigen::VectorXd candidate = CandidateMatrixes[i].transpose() * input_reset_state + CandidateBiases[i];
        candidate = candidate.unaryExpr([](double elem) { return std::tanh(elem); });

        States[i].array() = u.array() * States[i].array() - (u.array() - 1.0) * candidate.array();
        input = States[i];
    };

    Observed += 1;
    UpdateSpace();

};
