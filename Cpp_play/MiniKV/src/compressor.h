#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>

class Compressor {
public:
    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);

    static constexpr size_t DEFAULT_THRESHOLD = 256;
};
