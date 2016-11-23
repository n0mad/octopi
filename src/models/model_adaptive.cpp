#include "model_adaptive.h"

TModelAdaptive::TModelAdaptive()
{
    Reset();
}

void TModelAdaptive::Reset()
{
	Normalizer = 256;
	for(int i = 0; i < 256; i++)
		Probs[i] = 1;
};

void TModelAdaptive::Encode(uint8 symbol, uint64 &low_count, uint64 &upper_count, uint64 &normalizer)
{
    //cumulate frequencies
    low_count = 0;
    uint8 j = 0;
    for(; j < symbol; j++)
        low_count += Probs[j];
    upper_count = low_count + Probs[symbol];
    normalizer = Normalizer;
};

uint8 TModelAdaptive::Decode(uint64 value, uint64 &lower_count, uint64 &upper_count)
{
    uint8 symbol = 0;
    lower_count = 0;
    // determine symbol
    for(; lower_count + Probs[symbol] <= value; symbol ++)
        lower_count += Probs[symbol];
    upper_count = lower_count + Probs[symbol];
    return symbol;
};


void TModelAdaptive::Observe(uint8 symbol) {
    Normalizer += 1;
    Probs[symbol] += 1;
};
