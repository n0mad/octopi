#include <iostream>
#include "decoder.h"
#include <vector>

// constants to split the number space of 32 bit integers
// most significant bit kept free to prevent overflows
const unsigned int g_FirstQuarter = 0x20000000;
const unsigned int g_ThirdQuarter = 0x60000000;
const unsigned int g_Half = 0x40000000;

using namespace std;

Decoder::Decoder(TModel *model, vector<bool>* input, uint32 size) {
    mBitCount = 0;
    mBitBuffer = 0;
    mLow = 0;
    mHigh = 0x7FFFFFFF; // just work with least significant 31 bits
    //mScale = 0;
    mBuffer = 0;
    mStep = 0;

    Model = model;
    Input = input;
    Size = size;
    Pointer = 0;
}

uint8 Decoder::GetBit() {
    if (Pointer < Input->size())
        return (*Input)[Pointer++];
    else
        return 0;
}

uint32 Decoder::DecodeTarget(uint32 total)
{
    mStep = (mHigh - mLow + 1) / total;
    return (mBuffer - mLow) / mStep;
}
void Decoder::Decode(uint32 low_count, uint32 high_count)
{
    // update upper bound
    mHigh = mLow + mStep * high_count - 1; // interval open at the top => -1
    // update lower bound
    mLow = mLow + mStep * low_count;
    // e1/e2 scaling
    while((mHigh < g_Half) || (mLow >= g_Half)) {
        if(mHigh < g_Half)
        {
            mLow = mLow * 2;
            mHigh = mHigh * 2 + 1;
            mBuffer = 2 * mBuffer + GetBit();
        }
        else if(mLow >= g_Half)
        {
            mLow = 2 * (mLow - g_Half);
            mHigh = 2 * (mHigh - g_Half) + 1;
            mBuffer = 2 * (mBuffer - g_Half) + GetBit();
        }
        mScale = 0;
    }
    // e3 scaling
    while((g_FirstQuarter <= mLow) && (mHigh < g_ThirdQuarter))
    {
        mScale++;
        mLow = 2 * (mLow - g_FirstQuarter);
        mHigh = 2 * (mHigh - g_FirstQuarter) + 1;
        mBuffer = 2 * (mBuffer - g_FirstQuarter) + GetBit();
    }
}


void Decoder::DecodeSequence(vector<uint8> &output) {
    // Fill buffer with bits from the input stream
    for(int i = 0; i < 31; i++) // just use the 31 least significant bits
        mBuffer = (mBuffer << 1) | GetBit();
    while(output.size() < Size) {
        uint32 low_count = 0, upper_count = 0;
        uint32 value = DecodeTarget(Model->GetNormalizer());
        uint8 symbol = Model->Decode(value, low_count, upper_count);
        output.push_back(symbol);
        Model->Observe(symbol);
        Decode(low_count, upper_count);
    };
};
