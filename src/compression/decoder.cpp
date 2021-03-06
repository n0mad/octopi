#include <iostream>
#include "decoder.h"
#include <vector>

using namespace std;

Decoder::Decoder(TModel *model, vector<bool>* input, int size) {
    mBitCount = 0;
    mBitBuffer = 0;
    mLow = 0;
    mHigh = g_Max;
    mScale = 0;
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

uint64 Decoder::DecodeTarget(uint64 total)
{
    mStep = (mHigh - mLow + 1) / total;
    return (mBuffer - mLow) / mStep;
}
void Decoder::Decode(uint64 low_count, uint64 high_count)
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
    for(int i = 0; i < g_BitsUsed; i++) // just use the g_BitsUsed least significant bits
        mBuffer = (mBuffer << 1) | GetBit();
    while(output.size() < Size) {
        uint64 low_count = 0, upper_count = 0;
        uint64 value = DecodeTarget(Model->GetNormalizer());
        uint8 symbol = Model->Decode(value, low_count, upper_count);
        output.push_back(symbol);
        Model->Observe(symbol);
        Decode(low_count, upper_count);
    };
};
