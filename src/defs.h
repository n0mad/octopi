#pragma once
#include <iostream>

using uint64 = unsigned long;
using uint8  = unsigned char;

// constants to split the number space of 64 bit integers
// most significant bit kept free to prevent overflows

const uint64 g_FirstQuarter = 0x2000000000000;
const uint64 g_ThirdQuarter = 0x6000000000000;
const uint64 g_Half = 0x4000000000000;
const uint64 g_Max = 0x7FFFFFFFFFFFF;

const int g_BitsUsed = 51;
