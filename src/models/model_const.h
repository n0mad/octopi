#pragma once
#include "./../model_abstract.h"

class TModelConst : public TModel
{
public:
	TModelConst();
    virtual void Encode(uint8 symbol, uint64 &low_count, uint64 &upper_count, uint64 &total);
    virtual uint8 Decode(uint64 value, uint64 &lower_count, uint64 &upper_count);
    virtual uint64 GetNormalizer() {
        return Normalizer;
    };
    virtual void Observe(uint8 symbol);
    virtual void Reset();
    virtual std::string GetName() {
        return "ConstUniform";
    };
protected:

	uint64 Probs[256];
	uint64 Normalizer;
};
