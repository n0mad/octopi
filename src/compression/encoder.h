#pragma once
#include <vector>
#include "../model_abstract.h"
#include "./../defs.h"

class Encoder
{
public:
    Encoder(TModel *model);
    void Encode(const std::vector<uint8> &input);
    void GetEncodedData(std::vector<bool> &output);
protected:
    // bit operations
    void SetBit(bool bit);
    void SetBitFlush();
    void Encode(const uint64 low_count, const uint64 high_count, const uint64 total);

    uint64 mLow;
    uint64 mHigh;
    uint64 mStep;
    uint64 mScale;

    std::vector<bool> Data;
    TModel* Model;
};
