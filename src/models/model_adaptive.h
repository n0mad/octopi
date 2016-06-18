#pragma once
#include "./../model_abstract.h"
class TModelAdaptive : public TModel
{
public:
	TModelAdaptive();
    virtual void Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &total);
    virtual uint8 Decode(uint32 value, uint32 &lower_count, uint32 &upper_count);
    virtual uint32 GetNormalizer() {
        return Normalizer;
    };
    virtual void Observe(uint8 symbol);
    virtual void Reset();
    virtual std::string GetName() {
        return "Adaptive";
    };
protected:

	uint32 Probs[256];
	uint32 Normalizer;
};
