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
    void Encode(const unsigned int low_count, const unsigned int high_count, const unsigned int total);

    uint32 mLow;
    uint32 mHigh;
    uint32 mStep;
    uint32 mScale;

    std::vector<bool> Data;
    TModel* Model;
};
