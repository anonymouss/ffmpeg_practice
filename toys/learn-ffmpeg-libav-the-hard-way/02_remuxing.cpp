/**
 * https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/2_remuxing.c
 */

#include "ff_headers.h"
#include "ff_logging.h"

int main(int argc, const char *argv[]) {
    bool fragmentedMp4 = false;
    if (argc < 3) {
        logging("please provide at least two params\n");
        return ERROR;
    } else if (argc == 4) {
        logging("fragmented mp4");
        fragmentedMp4 = true;
    }

    logging("initializing");

    AVFormatContext *inputFormatCtx = nullptr, *outputFormatCtx = nullptr;
    AVPacket packet;
    const char *inputFilename = argv[1], *outputFilename = argv[2];
    int ret;
    int streamId = 0, nStreams = 0;
    int *streamList = nullptr;
}