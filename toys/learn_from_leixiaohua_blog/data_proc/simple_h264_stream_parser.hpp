#ifndef __SIMPLE_H264_STREAM_PARSER_H__
#define __SIMPLE_H264_STREAM_PARSER_H__

#include <iostream>
#include <string>
#include <cstdint>
#include <cassert>
#include <fstream>
#include <cstring>
#include <cstdio>

namespace vid {

enum NaluType : uint32_t {
    NALU_TYPE_UNDEFINED,
    NALU_TYPE_SLICE,        // 不分区，非IDR图像的片
    NALU_TYPE_DPA,          // 片分区 A
    NALU_TYPE_DPB,          // 片分区 B
    NALU_TYPE_DPC,          // 片分区 C
    NALU_TYPE_IDR,          // IDR图像中的片
    NALU_TYPE_SEI,          // 补充增强信息单元
    NALU_TYPE_SPS,          // Sequence Paramater Set
    NALU_TYPE_PPS,          // Picture Parameter Set
    NALU_TYPE_AUD,          // Access unit, 序列开始
    NALU_TYPE_EOSEQ,        // 序列结束
    NALU_TYPE_EOSTREAM,     // 流结束
    NALU_TYPE_FILL,         // 填充
};

enum NalRefIdc : uint32_t {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIORITY_LOW,
    NALU_PRIORITY_HIGH,
    NALU_PRIORITY_HIGHEST,
};

struct Nalu_t {
    int         startcodeprefix_len;    // 4 for parameter sets and first slice in picture, 3 for
                                        // everything else (suggested)
    unsigned    len;                    // Length of the NAL unit (Excluding the start code, which
                                        // does not belong to the NALU)
    unsigned    max_size;               // Nal Unit Buffer size
    int         forbidden_bit;          // should always be false
    NaluType    nal_unit_type;
    NalRefIdc   nal_reference_idc;
    char        *buf;                   // contains the first byte followed by the EBSP
    uint16_t    lost_packets;
};

constexpr uint32_t MAX_BUF_SIZE = UINT32_MAX;

bool validate_startcode(const char *buf, int len) {
    assert(buf != nullptr);
    for (auto i = 0; i < len - 1; ++i) {
        if (buf[i] != 0) return false;
    }
    return buf[len - 1] == 1;
}

int get_annexb_nalu(std::ifstream *file, Nalu_t *nalu) {
    int pos = 0;
    auto *buf = new char[nalu->max_size];
    assert(buf != nullptr);

    // read 3 bytes by default
    if (!file->read(buf, 3) || file->gcount() < 3) {
        std::cout << "not enough data 3" << std::endl;
        delete [] buf;
        return -1;
    }

    if (validate_startcode(buf, 3)) {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    } else {
        // read 4th byte
        if (!file->read(buf + 3, 1) || file->gcount() < 1) {
            std::cout << "not enough data 4" << std::endl;
            delete [] buf;
            return -1;
        }
        if (validate_startcode(buf, 4)) {
            nalu->startcodeprefix_len = 4;
            pos = 4;
        } else {
            std::cout << "invalid start code" << std::endl;
            delete [] buf;
            return -1;
        }
    }

    bool found_next_startcode = false;
    bool is_type3 = false, is_type4 = false;
    while (!found_next_startcode) {
        if (file->eof()) {
            nalu->len = (pos - 1) - nalu->startcodeprefix_len;
            std::memcpy(nalu->buf, &buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = nalu->buf[0] & 0x80;
            nalu->nal_reference_idc = static_cast<NalRefIdc>(nalu->buf[0] & 0x60);
            nalu->nal_unit_type = static_cast<NaluType>(nalu->buf[0] & 0x1f);
            delete [] buf;
            return pos - 1;
        }

        buf[pos++] = file->get();
        is_type3 = validate_startcode(&buf[pos - 3], 3);
        is_type4 = validate_startcode(&buf[pos - 4], 4);
        found_next_startcode = is_type3 || is_type3;
    }

    auto rewind = is_type3 ? -3 : -4;
    file->seekg(rewind, std::ios::cur);
    nalu->len = (pos + rewind) - nalu->startcodeprefix_len;
    std::memcpy(nalu->buf, &buf[nalu->startcodeprefix_len], nalu->len);
    nalu->forbidden_bit = nalu->buf[0] & 0x80;
    nalu->nal_reference_idc = static_cast<NalRefIdc>(nalu->buf[0] & 0x60);
    nalu->nal_unit_type = static_cast<NaluType>(nalu->buf[0] & 0x1f);
    delete [] buf;

    return pos + rewind;
}

void parse_h264(const char *uri) {
    std::ifstream input(uri, std::ios::binary);
    assert(input.is_open());

    auto *nalu = new Nalu_t;
    assert(nalu != nullptr);
    nalu->max_size = MAX_BUF_SIZE;
    nalu->buf = new char[nalu->max_size];
    assert(nalu->buf != nullptr);

    int offset = 0, nal_num = 0;
    printf("-----+----- NALU Table -+-------+---------+\n");
    printf(" NUM |    POS  |  IDC   |  TYPE |    LEN  |\n");
    printf("-----+---------+--------+-------+---------+\n");
    while (!input.eof()) {
        auto len = get_annexb_nalu(&input, nalu);
        char type[20] = {'\0'};
        switch (nalu->nal_unit_type) {
            case NaluType::NALU_TYPE_SLICE: std::sprintf(type, "SLICE"); break;
            case NaluType::NALU_TYPE_DPA: std::sprintf(type, "DPA"); break;
            case NaluType::NALU_TYPE_DPB: std::sprintf(type, "DPB"); break;
            case NaluType::NALU_TYPE_DPC: std::sprintf(type, "DPC"); break;
            case NaluType::NALU_TYPE_IDR: std::sprintf(type, "IDR"); break;
            case NaluType::NALU_TYPE_SEI: std::sprintf(type, "SEI"); break;
            case NaluType::NALU_TYPE_SPS: std::sprintf(type, "SPS"); break;
            case NaluType::NALU_TYPE_PPS: std::sprintf(type, "PPS"); break;
            case NaluType::NALU_TYPE_AUD: std::sprintf(type, "AUD"); break;
            case NaluType::NALU_TYPE_EOSEQ: std::sprintf(type, "EOSEQ"); break;
            case NaluType::NALU_TYPE_EOSTREAM: std::sprintf(type, "EOSTREAM"); break;
            case NaluType::NALU_TYPE_FILL: std::sprintf(type, "FILL"); break;
            default: std::sprintf(type, "?"); break;
        }
        char idc[20] = {0};
        switch (nalu->nal_reference_idc >> 5) {
            case NalRefIdc::NALU_PRIORITY_DISPOSABLE: std::sprintf(idc, "DISPOS"); break;
            case NalRefIdc::NALU_PRIORITY_LOW: std::sprintf(idc, "LOW"); break;
            case NalRefIdc::NALU_PRIORITY_HIGH: std::sprintf(idc, "HIGH"); break;
            case NalRefIdc::NALU_PRIORITY_HIGHEST: std::sprintf(idc, "HIGHEST"); break;
            default: std::sprintf(idc, "?"); break;
        }

        printf("%5d| %8d| %7s| %6s| %8d|\n", nal_num, offset, idc, type, nalu->len);
        offset += len;
        ++nal_num;
    }

    delete [] nalu->buf;
    delete nalu;
    input.close();
}

}  // namespace vid

#endif  // __SIMPLE_H264_STREAM_PARSER_H__