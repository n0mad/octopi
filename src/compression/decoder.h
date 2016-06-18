#pragma once
#include <fstream>
#include "./../model_abstract.h"
#include "./../defs.h"
#include <vector>

using namespace std;
class Decoder
{
public:
    Decoder(TModel* model, std::vector<bool>* input, uint32 size);
    uint32 DecodeTarget(const uint32 total);
    void DecodeSequence(std::vector<uint8> &output);
    void Decode(uint32 low_count, uint32 high_count);
protected:
    uint8 GetBit();
    uint8 mBitBuffer;
    uint8 mBitCount;
    uint32 mLow;
    uint32 mHigh;
    uint32 mStep;
    uint32 mScale;
    uint32 mBuffer;
    TModel* Model;
    std::vector<bool>* Input;
    uint32 Size;
    int Pointer;
};
