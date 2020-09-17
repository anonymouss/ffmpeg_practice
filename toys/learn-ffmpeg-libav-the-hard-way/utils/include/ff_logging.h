#ifndef __FF_LOGGING_H__
#define __FF_LOGGING_H__

#include "ff_headers.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>

namespace ff {

void logging(const char *fmt, ...);
void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);
void print_timing(char *name, AVFormatContext *avf, AVCodecContext *avc, AVStream *avs);

}  // namespace ff

#endif  // __FF_LOGGING_H__