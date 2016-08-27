#include "model_abstract.h"
#include "models/model_const.h"
#include "models/model_adaptive.h"
#include "models/model_gru.h"
#include "models/model_rnn.h"
#include "compression/encoder.h"
#include "compression/decoder.h"
#include "compression/bytesbits.h"
#include "picojson/picojson.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>

using namespace std;

void printUsage() {
    cout << "usage: octopi (-c|-d) <model> <input file> <output file>" << endl;
};

TModel* LoadModel(int argc, char *argv[]) {
    if (string(argv[2]) == "adaptive"s) {
        return new TModelAdaptive();
    } else if (string(argv[2]) == "const"s) {
        return new TModelConst();
    } else {
        string fileName(argv[2]);
        ifstream inputFile(fileName);
        if (!inputFile.is_open()) {
            cerr << "Cannot read the model file" << fileName << endl;
            abort();
        };

        picojson::value v;
        inputFile >> v;
        string err = picojson::get_last_error();

        if (!err.empty()) {
            cerr << "Error while reading json" << err << endl;
            abort();
        };

        if (!v.is<picojson::array>()) {
            cerr << "Bad JSON format: the top level element is not an array" << endl;
            abort();
        };

        const picojson::value::array &list = v.get<picojson::array>();

        if (!list[0].is<string>()) {
            cerr << "Bad JSON format: model type is not a string" << endl;
            abort();
        };

        string model_type = list[0].get<string>();
        if (model_type == "gru") {
            return new TModelGRU(argv[2]);
        } else if (model_type == "rnn") {
            return new TModelRNN(argv[2]);
        } else {
            cerr << "Unsupported model type:" << model_type << ", only gru or rnn are supported so far" << endl;
            abort();
        };
        //unreachable
        return nullptr;
    };
};

void Decompress(const string& from, const string& to, TModel* model) {
    ifstream inputStream(from, std::ios::in | std::ifstream::binary);
    vector<uint8> data;
    noskipws(inputStream);
    int dataSize;
    inputStream.read((char *) &dataSize, sizeof(int)); //not too compatible

    data.insert(data.begin(), std::istream_iterator<uint8>(inputStream), std::istream_iterator<uint8>() );

    TBytesBits bb;
    vector<bool> bits;

    bb.Bytes2Bits(data, bits);
    vector<uint8> output;
    Decoder den(model, &bits, dataSize);
    den.DecodeSequence(output);

    ofstream outputStream(to, std::ios_base::binary);
    std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));
}

void Compress(const string& from, const string& to, TModel* model) {
    ifstream inputStream(from, std::ios::in | std::ios::binary);
    noskipws(inputStream);
    vector<uint8> data;
    data.insert(data.begin(), std::istream_iterator<uint8>(inputStream), std::istream_iterator<uint8>() );

    Encoder enc(model);
    enc.Encode(data);

    vector<bool> enc_output;
    enc.GetEncodedData(enc_output);

    vector<uint8> output;
    TBytesBits bb;
    bb.Bits2Bytes(enc_output, output);

    ofstream outputStream(to, std::ios::out | std::ios::binary);
    int dataSize = data.size();
    outputStream.write( (char *) &dataSize, sizeof(int)); //not too compatible
    //outputStream << (uint32) data.size();
    std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        cout << "too few arguments" << endl;
        printUsage();
        return 1;
    };
    unique_ptr<TModel> model(LoadModel(argc, argv));
    string from = argv[3];
    string to = argv[4];

    if (string(argv[1]) == "-c"s) {
        Compress(from, to, model.get());
    } else if (string(argv[1]) == "-d"s) {
        Decompress(from, to, model.get());
    } else {
        printUsage();
    };

    return 0;
}
