/**
 * https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
 */

#include "ff_headers.h"
#include "ff_logging.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

using namespace ff;

#define OK 0
#define ERROR -1

constexpr uint32_t kDefaultPacketsNumToProcess = 20;

const char *Str(AVMediaType codec) {
    switch (codec) {
        case AVMediaType::AVMEDIA_TYPE_VIDEO:
            return "video";
        case AVMediaType::AVMEDIA_TYPE_AUDIO:
            return "audio";
        default:
            return "?";
    }
}

struct StreamCodecInfo {
    AVCodec *codec = nullptr;
    AVCodecParameters *params = nullptr;
    int id;
};

void save_gray_frame(uint8_t *buf, int stride, int width, int height, const char *filename) {
    std::fstream of(filename, std::ios::binary | std::ios::out);
    if (!of.is_open()) {
        logging("ERROR: failed to open %s", filename);
        of.close();
        return;
    }

    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    of << "P5\n"
       << width << " " << height << "\n"
       << "255\n";

    // writing line by line
    for (auto i = 0; i < height; ++i) {
        const char *head = reinterpret_cast<const char *>(buf + i * stride);
        of.write(head, width);
    }

    of.close();
}

void save_pcm_data(uint8_t **buf, int buf_size, int samples, int channels, const char *filename) {
    std::fstream of(filename, std::ios::binary | std::ios::out);
    if (!of.is_open()) {
        logging("ERROR: failed to open %s", filename);
        of.close();
        return;
    }

    for (auto i = 0; i < samples; ++i) {
        for (auto ch = 0; ch < channels; ++ch) {
            const char *head = reinterpret_cast<const char *>(buf[ch] + buf_size * i);
            of.write(head, buf_size);
        }
    }

    of.close();
}

int decode_packet(AVPacket *packet, AVCodecContext *context, AVFrame *frame, bool is_audio) {
    // raw packet data
    auto response = avcodec_send_packet(context, packet);
    if (response < 0) {
        logging("ERROR: in sending packet to codec (%s)", av_err2str(response));
        return response;
    }
    while (response >= 0) {
        // decoded frame data
        response = avcodec_receive_frame(context, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            // logging("EOF");
            break;
        } else if (response < 0) {
            logging("ERROR: in receiving frame from codec (%s)", av_err2str(response));
            return response;
        }

        if (response >= 0) {
            if (is_audio) {
                logging(
                    "\tA: Frame %d (channels=%d samples=%d, sample_rate=%d, size=%d bytes) pts %d "
                    "[DTS %d]",
                    context->frame_number, context->channels, frame->nb_samples, frame->sample_rate,
                    frame->pkt_size, frame->pts, frame->pkt_dts);
                auto data_size = av_get_bytes_per_sample(context->sample_fmt);
                if (data_size < 0) {
                    /* This should not occur, checking just for paranoia */
                    logging("ERROR: failed to calculate data size");
                    return ERROR;
                }
                std::string out_filename{"audio_frame-"};
                out_filename += std::to_string(context->frame_number) + ".pcm";
                save_pcm_data(frame->data, data_size, frame->nb_samples, context->channels,
                              out_filename.c_str());
            } else {
                logging(
                    "\tV: Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d(order)]",
                    context->frame_number, av_get_picture_type_char(frame->pict_type),
                    frame->pkt_size, frame->pts, frame->key_frame, frame->coded_picture_number);

                std::string out_filename{"video_frame-"};
                out_filename += std::to_string(context->frame_number) + ".pgm";
                // logging("saving %s", out_filename.c_str());
                save_gray_frame(frame->data[0], frame->linesize[0], frame->width, frame->height,
                                out_filename.c_str());
            }
        }
    }

    return OK;
}

int decodeAVStreams(StreamCodecInfo *video_stream, StreamCodecInfo *audio_stream,
                    AVFormatContext *context) {
    logging("decoding a/v streams");
    AVCodecContext *pVideoCodecContext = avcodec_alloc_context3(video_stream->codec);
    AVCodecContext *pAudioCodecContext = avcodec_alloc_context3(audio_stream->codec);
    if (!pVideoCodecContext || !pAudioCodecContext) {
        logging("ERROR: failed to allocate codec context");
        return ERROR;
    }

    if (avcodec_parameters_to_context(pVideoCodecContext, video_stream->params) < 0) {
        logging("ERROR: failed to copy video codec params");
        return ERROR;
    }
    if (avcodec_parameters_to_context(pAudioCodecContext, audio_stream->params) < 0) {
        logging("ERROR: failed to copy audio codec params");
        return ERROR;
    }

    if (avcodec_open2(pVideoCodecContext, video_stream->codec, NULL) < 0) {
        logging("ERROR: failed to open video codec through avcodec_open2");
        return ERROR;
    }
    if (avcodec_open2(pAudioCodecContext, audio_stream->codec, NULL) < 0) {
        logging("ERROR: failed to open video codec through avcodec_open2");
        return ERROR;
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame) {
        logging("ERROR: failed to allocate frame");
        return ERROR;
    }
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        logging("ERROR: failed to allocate packet for");
        return ERROR;
    }

    int response = 0;
    auto nPackets = kDefaultPacketsNumToProcess;
    while (av_read_frame(context, pPacket) >= 0) {
        // if it's the video stream
        if (pPacket->stream_index == video_stream->id) {
            logging("video stream");
            logging("\tAVPacket->pts %" PRId64, pPacket->pts);
            response = decode_packet(pPacket, pVideoCodecContext, pFrame, false);
            if (response < 0) break;
            // stop it, otherwise we'll be saving hundreds of frames
        } else if (pPacket->stream_index == audio_stream->id) {
            logging("autio stream");
            logging("\tAVPacket->pts %" PRId64, pPacket->pts);
            response = decode_packet(pPacket, pAudioCodecContext, pFrame, true);
            if (response < 0) break;
            // stop it, otherwise we'll be saving hundreds of frames
        } else {
            logging("unknown stream");
        }
        if (--nPackets <= 0) break;
        av_packet_unref(pPacket);
    }

    // logging("exiting %s", __func__);

    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pVideoCodecContext);
    avcodec_free_context(&pAudioCodecContext);
    return OK;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        printf("please specify an media file\n");
        return ERROR;
    }

    logging("initializing");

    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        logging("ERROR: failed to allocate memory for format context");
        return ERROR;
    }

    for (auto i = 1; i < argc; ++i) {
        const char *filename = argv[i];
        logging("opening the %dst input file (%s) and loading format (container) header", i,
                filename);

        // open file and read its header
        auto ret = avformat_open_input(&pFormatContext, filename, NULL, NULL);
        if (ret != OK) {
            logging("ERROR: failed to open input file (%d)", ret);
            return ret;
        }

        logging("format %s, duration %lld us, bit_rate %lld", pFormatContext->iformat->name,
                pFormatContext->duration, pFormatContext->bit_rate);

        logging("finding stream info from format");
        // read packet from format to get stream info
        ret = avformat_find_stream_info(pFormatContext, NULL);
        if (ret != OK) {
            logging("ERROR: failed to read packet info (%d)", ret);
        }

        AVCodec *pVideoCodec = nullptr;
        AVCodec *pAudioCodec = nullptr;
        AVCodecParameters *pVideoCodecParams = nullptr;
        AVCodecParameters *pAudioCodecParams = nullptr;
        int video_stream_id = -1;
        int audio_stream_id = -1;

        // iterate streams and select audio/video stream
        for (auto stream_id = 0; stream_id < pFormatContext->nb_streams; ++stream_id) {
            auto *pLocalCodecParams = pFormatContext->streams[stream_id]->codecpar;
            logging("AVStream->time_base before open coded %d/%d",
                    pFormatContext->streams[i]->time_base.num,
                    pFormatContext->streams[i]->time_base.den);
            logging("AVStream->r_frame_rate before open coded %d/%d",
                    pFormatContext->streams[i]->r_frame_rate.num,
                    pFormatContext->streams[i]->r_frame_rate.den);
            logging("AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
            logging("AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);

            logging("finding the proper decoder (CODEC)");
            auto *pLocalCodec = avcodec_find_decoder(pLocalCodecParams->codec_id);
            if (!pLocalCodec) {
                logging("ERROR: unsupported codec of stream %d", stream_id);
                continue;
            }

            if (pLocalCodecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (video_stream_id == -1) {
                    video_stream_id = stream_id;
                    pVideoCodec = pLocalCodec;
                    pVideoCodecParams = pLocalCodecParams;
                }
                logging("Video Codec: stream %d, resolution %d x %d", stream_id,
                        pLocalCodecParams->width, pLocalCodecParams->height);
            } else if (pLocalCodecParams->codec_type == AVMEDIA_TYPE_AUDIO) {
                if (audio_stream_id == -1) {
                    audio_stream_id = stream_id;
                    pAudioCodec = pLocalCodec;
                    pAudioCodecParams = pLocalCodecParams;
                }
                logging("Audio Codec: stream %d, %d channels, sample rate %d", stream_id,
                        pLocalCodecParams->channels, pLocalCodecParams->sample_rate);
            }
            logging("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id,
                    pLocalCodecParams->bit_rate);
        }

        StreamCodecInfo video_stream{pVideoCodec, pVideoCodecParams, video_stream_id};
        StreamCodecInfo audio_stream{pAudioCodec, pAudioCodecParams, audio_stream_id};

        if (decodeAVStreams(&video_stream, &audio_stream, pFormatContext) != OK) {
            logging("ERROR: failed to decode a/v streams");
            return ERROR;
        }

        avformat_close_input(&pFormatContext);
        logging("OK: process (%s) done!", filename);
    }
    avformat_free_context(pFormatContext);
}