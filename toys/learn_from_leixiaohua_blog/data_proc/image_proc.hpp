#ifndef __DATA_PROC_IMAGE_H__
#define __DATA_PROC_IMAGE_H__

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cstring>

namespace vid {

enum ColorFormat : uint32_t {
    YUV_420P,
    YUV_444P,
    RGB_888,
};

struct ImageInfo {
    std::string uri;
    uint32_t width;
    uint32_t height;
    uint32_t frames;
    ColorFormat colorFormat;
};

void extract_yuv420p(std::ifstream *input, uint32_t width, uint32_t height, uint32_t nframes) {
    std::ofstream yout{"yuv_420p.y", std::ios::binary};
    assert(yout.is_open());
    std::ofstream uout{"yuv_420p.u", std::ios::binary};
    assert(yout.is_open());
    std::ofstream vout{"yuv_420p.v", std::ios::binary};
    assert(yout.is_open());

    auto frame_size = width * height * 3 / 2;
    auto y_size = width * height;
    auto uv_size = y_size / 4;
    char data[frame_size];
    input->seekg(0);
    std::memset(data, 0, frame_size);
    for (auto frame = 0; frame < nframes; ++frame) {
        input->read(data, frame_size);
        input->seekg(frame_size, std::ios::cur);

        yout.write(data, y_size);
        uout.write(data + y_size, uv_size);
        vout.write(data + y_size + uv_size, uv_size);
    }

    yout.close();
    uout.close();
    vout.close();
}

void extract_yuv444p(std::ifstream *input, uint32_t width, uint32_t height, uint32_t nframes) {
    std::ofstream yout{"yuv_444p.y", std::ios::binary};
    assert(yout.is_open());
    std::ofstream uout{"yuv_444p.u", std::ios::binary};
    assert(yout.is_open());
    std::ofstream vout{"yuv_444p.v", std::ios::binary};
    assert(yout.is_open());

    auto frame_size = width * height * 3;
    auto plane_size = width * height;
    char data[frame_size];
    input->seekg(0);
    std::memset(data, 0, frame_size);
    for (auto frame = 0; frame < nframes; ++frame) {
        input->read(data, frame_size);
        input->seekg(frame_size, std::ios::cur);

        yout.write(data, plane_size);
        uout.write(data + plane_size, plane_size);
        vout.write(data + plane_size * 2, plane_size);
    }

    yout.close();
    uout.close();
    vout.close();
}

void extract_rgb888(std::ifstream *input, uint32_t width, uint32_t height, uint32_t nframes) {
    std::ofstream rout{"rgb_888.r", std::ios::binary};
    assert(rout.is_open());
    std::ofstream gout{"rgb_888.g", std::ios::binary};
    assert(gout.is_open());
    std::ofstream bout{"rgb_888.b", std::ios::binary};
    assert(bout.is_open());

    auto frame_size = width * height * 3;
    char data[frame_size];
    input->seekg(0);
    std::memset(data, 0, frame_size);
    for (auto frame = 0; frame < nframes; ++frame) {
        input->read(data, frame_size);
        input->seekg(frame_size, std::ios::cur);

        for (auto i = 0; i < frame_size; i += 3) {
            rout.write(data + i, 1);
            gout.write(data + i + 1, 1);
            bout.write(data + i + 2, 1);
        }
    }

    rout.close();
    gout.close();
    bout.close();
}

void extract_channels(const ImageInfo &info) {
    std::ifstream input{info.uri, std::ios::binary};
    assert(input.is_open());

    switch (info.colorFormat) {
        case ColorFormat::YUV_420P: {
            extract_yuv420p(&input, info.width, info.height, info.frames);
            break;
        }
        case ColorFormat::YUV_444P: {
            extract_yuv444p(&input, info.width, info.height, info.frames);
            break;
        }
        case ColorFormat::RGB_888: {
            extract_rgb888(&input, info.width, info.height, info.frames);
            break;
        }
        default: {
            std::cout << "Unsupported color format " << info.colorFormat << std::endl;
            break;
        }
    }

    if (input.is_open()) {
        input.close();
    }
}

void convert_420p_to_gray(const ImageInfo &info) {
    std::ifstream input{info.uri, std::ios::binary};
    assert(input.is_open());

    std::ofstream output{"yuv_420p.gray", std::ios::binary};
    assert(output.is_open());

    auto frame_size = info.width * info.height * 3 / 2;
    auto img_size = info.width * info.height;
    char data[frame_size];
    input.seekg(0);
    for (auto frame = 0; frame < info.frames; ++frame) {
        input.read(data, frame_size);
        input.seekg(frame_size, std::ios::cur);

        // YUV 变灰度只需要保留亮度分量Y，对UV色度分量设128（0）
        std::memset(data + img_size, 128, img_size / 2);
        output.write(data, frame_size);
    }

    input.close();
    output.close();
}

void reduce_420p_y(const ImageInfo &info, float ratio) {
    assert(ratio >= 0.0 && ratio <= 1.0);
    std::ifstream input{info.uri, std::ios::binary};
    assert(input.is_open());

    std::ofstream output{"yuv_420p.y_reduce", std::ios::binary};
    assert(output.is_open());

    auto frame_size = info.width * info.height * 3 / 2;
    auto img_size = info.width * info.height;
    char data[frame_size];
    input.seekg(0);
    for (auto frame = 0; frame < info.frames; ++frame) {
        input.read(data, frame_size);
        input.seekg(frame_size, std::ios::cur);

        for (auto pix = 0; pix < img_size; ++pix) {
            data[pix] = static_cast<char>(data[pix] * ratio);
        }

        output.write(data, frame_size);
    }

    input.close();
    output.close();
}

}  // namespace vid

#endif  // __DATA_PROC_IMAGE_H__