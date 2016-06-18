#pragma once
#include "defs.h"
class TModel
{
    public:
    virtual void Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &total) = 0;
    virtual uint8 Decode(uint32 value, uint32 &lower_count, uint32 &upper_count) = 0;
    virtual uint32 GetNormalizer() = 0;
    virtual void Reset() = 0;
    virtual void Observe(uint8 symbol) = 0;
    virtual std::string GetName() = 0;
    virtual ~TModel()
        {};

};

