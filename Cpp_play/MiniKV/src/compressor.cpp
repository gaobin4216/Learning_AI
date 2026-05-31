#include "compressor.h"
#include <zlib.h>
#include <spdlog/spdlog.h>

std::vector<uint8_t> Compressor::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};

    z_stream zs{};
    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        throw std::runtime_error("deflateInit failed");
    }

    zs.next_in = const_cast<Bytef*>(data.data());
    zs.avail_in = static_cast<uInt>(data.size());

    std::vector<uint8_t> output;
    output.resize(data.size() + data.size() / 100 + 64);

    zs.next_out = output.data();
    zs.avail_out = static_cast<uInt>(output.size());

    int ret = deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("deflate failed: " + std::to_string(ret));
    }

    output.resize(zs.total_out);
    spdlog::debug("Compress: {} -> {} bytes ({}% reduction)",
                  data.size(), output.size(),
                  data.size() > 0 ? (100 - output.size() * 100 / data.size()) : 0);
    return output;
}

std::vector<uint8_t> Compressor::decompress(const std::vector<uint8_t>& data) {
    if (data.empty()) return {};

    z_stream zs{};
    if (inflateInit(&zs) != Z_OK) {
        throw std::runtime_error("inflateInit failed");
    }

    zs.next_in = const_cast<Bytef*>(data.data());
    zs.avail_in = static_cast<uInt>(data.size());

    std::vector<uint8_t> output;
    size_t chunk = data.size() * 2;
    output.resize(chunk);

    zs.next_out = output.data();
    zs.avail_out = static_cast<uInt>(chunk);

    int ret;
    do {
        ret = inflate(&zs, Z_NO_FLUSH);
        if (ret == Z_BUF_ERROR || zs.avail_out == 0) {
            size_t old_size = output.size();
            output.resize(old_size * 2);
            zs.next_out = output.data() + old_size;
            zs.avail_out = static_cast<uInt>(old_size);
        }
    } while (ret == Z_OK || ret == Z_BUF_ERROR);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("inflate failed: " + std::to_string(ret));
    }

    output.resize(zs.total_out);
    spdlog::debug("Decompress: {} -> {} bytes", data.size(), output.size());
    return output;
}
