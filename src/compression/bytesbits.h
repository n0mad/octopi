#pragma once
#include <vector>

class TBytesBits {
    std::vector<vector<bool>> Mapping;

    public:
    TBytesBits() {
        Mapping.resize(256);

        for(int i = 0; i < 256; ++i) {
            int k = i;
            int j = 128;
            for(; j > 0; j = j / 2) {
                Mapping[i].push_back(k & j);
            };
        };
     };

    void Bytes2Bits(const std::vector<uint8> &input, std::vector<bool> &output) {
        output.clear();

        for(const auto& byte: input)
            output.insert(output.end(), Mapping[byte].begin(), Mapping[byte].end());
    };


    void Bits2Bytes(const std::vector<bool> &input, std::vector<uint8> &output) {
        output.clear();

        int target_size = (0 == input.size() % 8) ? input.size() / 8 : input.size() / 8 + 1;
        output.resize(target_size);

        for (int i = 0; i < input.size(); ++i) {
            output[i / 8] |= (input[i] << (7 - i % 8));
        };
    };
};

