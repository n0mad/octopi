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

void TModelAdaptive::Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &normalizer)
{
    //cumulate frequencies
    low_count = 0;
    uint8 j = 0;
    for(; j < symbol; j++)
        low_count += Probs[j];
    upper_count = low_count + Probs[symbol];
    normalizer = Normalizer;

    //update believes
    //Normalizer += 1;
    //Probs[symbol] += 1;
};

uint8 TModelAdaptive::Decode(uint32 value, uint32 &lower_count, uint32 &upper_count)
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
