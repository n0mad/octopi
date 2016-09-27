#pragma once
#include <iostream>

using uint32 = unsigned long;
using uint8  = unsigned char;

// constants to split the number space of 32 bit integers
// most significant bit kept free to prevent overflows

const uint32 g_FirstQuarter = 0x2000000000;
const uint32 g_ThirdQuarter = 0x6000000000;//0;
const uint32 g_Half = 0x4000000000;//0;

const uint32 g_Max =  0x7FFFFFFFFF;//F;
