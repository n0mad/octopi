#pragma once
#include <fstream>
#include "./../model_abstract.h"
#include "./../defs.h"
#include <vector>

using namespace std;
class Decoder
{
public:
    Decoder(TModel* model, std::vector<bool>* input, int size);
    uint64 DecodeTarget(const uint64 total);
    void DecodeSequence(std::vector<uint8> &output);
    void Decode(uint64 low_count, uint64 high_count);
protected:
    uint8 GetBit();
    uint8 mBitBuffer;
    uint8 mBitCount;
    uint64 mLow;
    uint64 mHigh;
    uint64 mStep;
    uint64 mScale;
    uint64 mBuffer;
    TModel* Model;
    std::vector<bool>* Input;
    int Size;
    int Pointer;
};
