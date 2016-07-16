#include <iostream>
#include <fstream>
#include "model_abstract.h"
#include "models/model_const.h"
#include "models/model_adaptive.h"
#include "models/model_rnn.h"
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

TModel* getModel(int argc, char *argv[]) {
    if (string(argv[2]) == "adaptive"s) {
        return new TModelAdaptive();
    };
    if (string(argv[2]) == "const"s) {
        return new TModelConst();
    };
    abort();
};

int main(int argc, char *argv[])
{
    if (argc < 4) {
        cout << "too few arguments" << endl;
        printUsage();
    };
    //load model
    unique_ptr<TModel> model(getModel(argc, argv));

    if (string(argv[1]) == "c"s) {
        //compress
        string inputFileName = argv[3];
        ifstream inputStream(inputFileName, std::ios::in | std::ifstream::binary);
        noskipws(inputStream);
        vector<uint8> data;
        data.insert(data.begin(), std::istream_iterator<uint8>(inputStream), std::istream_iterator<uint8>() );

        Encoder enc(model.get());
        enc.Encode(data);

        vector<bool> enc_output;
        enc.GetEncodedData(enc_output);
        cout << "first 160 bits\n";
        for(int i = 0; i < 160; ++i)
            cout << (int) enc_output[i];
        cout << endl;

        vector<uint8> output;
        TBytesBits bb;
        bb.Bits2Bytes(enc_output, output);

        string outputFileName = argv[4];
        ofstream outputStream(outputFileName, std::ios_base::binary);
        outputStream << (long) data.size();
        std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));

        /*
        for(int i = 0; i < 20; ++i)
            cout << "byte output " << (int) output[i];
        cout << endl;
        model->Reset();
        Decoder den(model.get(), &enc_output, 3339155);
        vector<uint8> dec_output;
        den.DecodeSequence(dec_output);

        for(int i = 0; i < 500; ++i)
            cout << (char) dec_output[i];
        cout << endl;*/

        for(int i = 0; i < 20; ++i) {
            cout << (int) output[i] << " ";
            for(int j = i * 8; j < i * 8 + 8; ++j)
                cout << (int) enc_output[j];
            cout << endl;
        };


    } else if (string(argv[1]) == "d"s) {
        //decompress
        string inputFileName = argv[3];
        ifstream inputStream(inputFileName, std::ios::in | std::ifstream::binary);
        vector<uint8> data;
        noskipws(inputStream);
        long size;
        inputStream >> size;
        cout << "unarch size " << size << endl;
        while(!inputStream.eof()) {
            uint8 b;
            inputStream >> b;
            data.push_back(b);
        };
        //for(int k = 0; k < data.size(); ++k)

        //data.insert(data.begin(), std::istream_iterator<uint8>(inputStream), std::istream_iterator<uint8>() );


        TBytesBits bb;
        vector<bool> bits;

        bb.Bytes2Bits(data, bits);
        for(int i = 0; i < 20; ++i) {
            cout << (int) data[i] << " ";
            for(int j = i * 8; j < i * 8 + 8; ++j)
                cout << (int) bits[j];
            cout << endl;
        };

        //cout << "first  bits\n";
        //for(int i = 0; i < 160; ++i)
        //    cout << (int) bits[i];
        //cout << endl;

        vector<uint8> output;
        Decoder den(model.get(), &bits, size);
        den.DecodeSequence(output);

        string outputFileName = argv[4];
        for(int i = 0; i < 20; ++i)
            cout << (char) output[i];
        cout << endl;
        ofstream outputStream(outputFileName, std::ios_base::binary);
        std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(outputStream));

     };
    //remove rnn.h
    return 0;
}
