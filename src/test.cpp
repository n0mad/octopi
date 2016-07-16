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

using namespace std;

template <typename T>
void testModelText(int length) {
    //std::ifstream t("./rnn/data/tinyshakespeare/input.txt");
    std::ifstream t("data/input.txt.decode");
    std::string str_data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());


    unique_ptr<T> model(new T());
    cout << "testing " << model->GetName() << " on a text dataset length " << str_data.size() << endl;

    vector<uint8> data;
    for(auto c: str_data) {
        if (data.size() == length)
            break;
        data.push_back(c);
    };


    clock_t begin = clock();

    Encoder enc(model.get());
    enc.Encode(data);

    vector<bool> enc_output;
    enc.GetEncodedData(enc_output);

    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "bits used " << enc_output.size() << " for " << data.size() << " chars. Bit/char: " << 1. * enc_output.size() / data.size() << endl;
    cout << "encoding time: " << elapsed_secs << "sec" << endl;


    model.reset(new T());
    vector<uint8> dec_output;
    begin = clock();
    Decoder den(model.get(), &enc_output, data.size());
    den.DecodeSequence(dec_output);
    end = clock();
    elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    cout << "decoding time: " << elapsed_secs << "sec" << endl;


    if (dec_output.size() != data.size())
        cerr << "!!! different lengths " << dec_output.size() << " and " << data.size() << endl;
    for (int i = 0; i < data.size(); ++i) {
        if (dec_output[i] != data[i])
            cerr << "!!! different seq " << dec_output[i] << " and " << data[i] << " for i = " << i << endl;
    };

};


template <typename T>
void testModelUniform(int lower, int upper, int dataSize) {
    unique_ptr<T> model(new T());
    cout << "testing " << model->GetName() << " on a dataset length " << dataSize << " uniform over [" << lower << ", " << upper << "]" << endl;

    vector<uint8> data;
    default_random_engine generator;
    std::uniform_int_distribution<int> distribution(lower, upper);

    for(int i = 0; i < dataSize; ++i) {
        int symbol = distribution(generator);
        data.push_back(symbol);
    };


    Encoder enc(model.get());
    enc.Encode(data);

    vector<bool> enc_output;
    enc.GetEncodedData(enc_output);

    cout << "bits used " << enc_output.size() << " for " << data.size() << " chars. Bit/char: " << 1. * enc_output.size() / data.size() << endl;

    model.reset(new T());
    vector<uint8> dec_output;
    Decoder den(model.get(), &enc_output, data.size());
    den.DecodeSequence(dec_output);


    if (dec_output.size() != data.size())
        cerr << "!!! different lengths " << dec_output.size() << " and " << data.size() << endl;
    for (int i = 0; i < data.size(); ++i) {
        if (dec_output[i] != data[i])
            cerr << "!!! different seq " << dec_output[i] << " and " << data[i] << " for i = " << i << endl;
    };

};

void testBytesBits() {

    {
        TBytesBits bb;
        vector<bool> input = {1, 1, 1, 1, 1, 1, 1, 1};
        vector<uint8> output;

        bb.Bits2Bytes(input, output);
        assert(output[0] == 255);

        input = {0, 0, 0, 0, 0, 0, 0, 0};
        bb.Bits2Bytes(input, output);
        assert(output[0] == 0);
     }

    {
        TBytesBits bb;
        for(int i = 0; i < 256; ++i) {
            vector<uint8> input;
            vector<bool> output;

            input.push_back(i);

            bb.Bytes2Bits(input, output);

            assert(output.size() == 8);
            for(int j = 1; j < 8; ++j) {
                bool bit = (0 != (i & (1 << (j - 1))));
                assert(output[8 - j] == bit);
            };
        };
    };

    {
        //aligned for 1 byte so far
        vector<bool> data = {1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1};
        TBytesBits bb;
        vector<uint8> bytes;

        bb.Bits2Bytes(data, bytes);

        vector<bool> bits;
        bb.Bytes2Bits(bytes, bits);
        for(int i = 0; i < data.size(); ++i)
            assert(bits == data);

    };
};

int main(int argc, char *argv[])
{
    testModelUniform<TModelConst>('a', 'z', 1000);
    testModelUniform<TModelAdaptive>('a', 'z', 1000);
    //testModelUniform<TModelRNN2>('a', 'z', 1000);

    testModelUniform<TModelConst>('a', 'b', 1000);
    testModelUniform<TModelAdaptive>('a', 'b', 1000);
    //testModelUniform<TModelRNN2>('a', 'b', 1000);

    testModelText<TModelConst>(10000);
    testModelText<TModelAdaptive>(10000);
    testModelText<TModelRNN2>(10000);

    testBytesBits();

    return 0;
}
