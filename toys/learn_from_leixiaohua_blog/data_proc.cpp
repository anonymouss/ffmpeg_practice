#include "image_proc.hpp"
#include "simple_h264_stream_parser.hpp"

constexpr const char *yuv_420p_file = "../media/lena_256x256_yuv420p.yuv";
constexpr const char *yuv_422p_file = "../media/lena_256x256_yuv422p.yuv";
constexpr const char *yuv_444p_file = "../media/lena_256x256_yuv444p.yuv";
constexpr const char *rgb_888_file  = "../media/cie1931_500x500.rgb";
constexpr const char *h264_file = "../media/sintel.h264";

int main() {
    vid::extract_channels({yuv_420p_file, 256, 256, 1, vid::ColorFormat::YUV_420P});
    vid::extract_channels({yuv_444p_file, 256, 256, 1, vid::ColorFormat::YUV_444P});
    vid::extract_channels({rgb_888_file, 500, 500, 1, vid::ColorFormat::RGB_888});

    vid::convert_420p_to_gray({yuv_420p_file, 256, 256, 1, vid::ColorFormat::YUV_420P});
    vid::reduce_420p_y({yuv_420p_file, 256, 256, 1, vid::ColorFormat::YUV_420P}, 0.5);

    vid::parse_h264(h264_file);
}