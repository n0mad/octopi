#include <iostream>
#include <fstream>
#include "model_abstract.h"
#include "models/model_const.h"
#include "models/model_adaptive.h"
#include "models/model_gru.h"
#include "models/model_rnn2.h"
#include "compression/encoder.h"
#include "compression/decoder.h"
#include "compression/bytesbits.h"
#include <assert.h>
#include <random>
#include <string>

using namespace std;

void printUsage() {
    cout << "usage: octopi -(c|d) <model> <input file> <output file>";
};

TModel* LoadModel(int argc, char *argv[]) {
    if (string(argv[2]) == "adaptive"s) {
        return new TModelAdaptive();
    } else if (string(argv[2]) == "const"s) {
        return new TModelConst();
    } else {
        cout << "gru only" << endl;
        //return new TModelRNN2(argv[2]);
        return new TModelGRU(argv[2]);
    };
     abort();
};

void Decompress(const string& from, const string& to, TModel* model) {
    ifstream inputStream(from, std::ios::in | std::ifstream::binary);
    vector<uint8> data;
    noskipws(inputStream);
    uint32 size;
    inputStream >> size;

    data.insert(data.begin(), std::istream_iterator<uint8>(inputStream), std::istream_iterator<uint8>() );
    /*while(!inputStream.eof()) {
        uint8 b;
        inputStream >> b;
        data.push_back(b);
    };*/

    TBytesBits bb;
    vector<bool> bits;

    bb.Bytes2Bits(data, bits);
    vector<uint8> output;
    Decoder den(model, &bits, size);
    den.DecodeSequence(output);

    ofstream outputStream(to, std::ios_base::binary);
    std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));
}

void Compress(const string& from, const string& to, TModel* model) {
    ifstream inputStream(from, std::ios::in | std::ifstream::binary);
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

    ofstream outputStream(to, std::ios::out | std::ios_base::binary);
    outputStream << (uint32) data.size();
    std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));
}

int main(int argc, char *argv[])
{

    //TModelGRU _model(argv[2]);
    //exit(0);
    if (argc < 4) {
        cout << "too few arguments" << endl;
        printUsage();
        return 1;
    };
    //load model
    unique_ptr<TModel> model(LoadModel(argc, argv));
    string from = argv[3];
    string to = argv[4];

    if (string(argv[1]) == "c"s) {
        Compress(from, to, model.get());
    } else if (string(argv[1]) == "d"s) {
        Decompress(from, to, model.get());
    };
    return 0;
}
