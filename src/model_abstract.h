#pragma once
#include "defs.h"
class TModel
{
    public:
    virtual void Encode(uint8 symbol, uint64 &low_count, uint64 &upper_count, uint64 &total) = 0;
    virtual uint8 Decode(uint64 value, uint64 &lower_count, uint64 &upper_count) = 0;
    virtual uint64 GetNormalizer() = 0;
    virtual void Reset() = 0;
    virtual void Observe(uint8 symbol) = 0;
    virtual std::string GetName() = 0;
    virtual ~TModel()
        {};

};

