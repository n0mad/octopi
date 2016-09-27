#include <iostream>
#include "encoder.h"

using namespace std;

Encoder::Encoder(TModel* model)
{
    mLow = 0;
    mHigh = g_Max;//0x7FFFFFFFF; // just work with least significant 31 bits
    mScale = 0;
    mStep = 0;
    Model = model;
}

void Encoder::SetBit(bool bit)
{
    Data.push_back(bit);
}


void Encoder::Encode(const uint32 low_count, const uint32 high_count, const uint32 total)
    // total < 2ˆ29
{
    // partition number space into single steps
    mStep = (mHigh - mLow + 1) / total; // interval open at the top => +1
    // update upper bound
    mHigh = mLow + mStep * high_count - 1; // interval open at the top => -1
    // update lower bound
    mLow = mLow + mStep * low_count;
    // apply e1/e2 scalingwhile( ( mHigh < g_Half ) || ( mLow >= g_Half ) )
    while((mHigh < g_Half) || (mLow >= g_Half)) {
    if(mHigh < g_Half) {
        SetBit(0);
        mLow = mLow * 2;
        mHigh = mHigh * 2 + 1;
        // perform e3 scalings
        for(; mScale > 0; mScale-- )
            SetBit(1);
    }
    else if(mLow >= g_Half)
    {
        SetBit(1);
        mLow = 2 * (mLow - g_Half);
        mHigh = 2 * (mHigh - g_Half) + 1;
        // perform e3 scalings
        for(; mScale > 0; mScale--)
            SetBit(0);
    }
    }
    // e3
    while((g_FirstQuarter <= mLow) && (mHigh < g_ThirdQuarter))
    {
    // keep necessary e3 scalings in mind
    mScale++;
    mLow = 2 * (mLow - g_FirstQuarter);
    mHigh = 2 * (mHigh - g_FirstQuarter) + 1;
    }
}

void Encoder::Encode(const std::vector<uint8> &input) {
    uint32 lower, upper, normalizer;
    for(auto c: input) {
        Model->Encode(c, lower, upper, normalizer);
        Encode(lower, upper, normalizer);
        Model->Observe(c);
    };

    // There are two possibilities of how mLow and mHigh can be distributed,
    // which means that two bits are enough to distinguish them.
    if(mLow < g_FirstQuarter) // mLow < FirstQuarter < Half <= mHigh
    {
        SetBit(0);
        for(int i = 0; i < mScale + 1; i++) // perform e3-scaling
            SetBit(1);
    }
    else // mLow < Half < ThirdQuarter <= mHigh
    {
        SetBit(1); // zeros added automatically by the decoder; no need to send them
    };


};

void Encoder::GetEncodedData(vector<bool> &output) {
    output = Data;
    return;
}
