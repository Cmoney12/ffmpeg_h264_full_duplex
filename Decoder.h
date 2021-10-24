//
// Created by corey on 10/23/21.
//

#ifndef OPENH264_EXAMPLE_DECODER_H
#define OPENH264_EXAMPLE_DECODER_H

#include <libavcodec/avcodec.h>
#include <iostream>

//#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
//#define av_frame_alloc  avcodec_alloc_frame
//#endif

#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libimagequant.h>


class Decoder {
public:
    Decoder() {
        matReady = false;

        //avcodec_register_all();
        //av_init_packet(&avpkt);
        //av_packet_alloc(avpkt);

        codec = avcodec_find_decoder(AV_CODEC_ID_H264);

        if (!codec) {
            fprintf(stderr, "Codec not found\n");
            exit(1);
        }
        c = avcodec_alloc_context3(codec);
        if (!c) {
            fprintf(stderr, "Could not allocate video codec context\n");
            exit(1);
        }

        if (avcodec_open2(c, codec, nullptr) < 0) {
            fprintf(stderr, "Could not open codec\n");
            exit(1);
        }

        frame = av_frame_alloc();
        if (!frame) {
            fprintf(stderr, "Could not allocate video frame\n");
            exit(1);
        }

        frame_count = 0;

        pFrameBGR = av_frame_alloc();
    }

    int decode(unsigned char *inputbuf, size_t size){

        avpkt.size = size;
        if(avpkt.size == 0)
            return 0;

        avpkt.data = inputbuf;

        int len, got_frame;

        //int send_size = avcodec_send_frame(c, frame);

        //got_frame = avcodec_receive_frame(c, frame);

        //deprecated
        //len = avcodec_decode_video2(c, frame, &got_frame, &avpkt);

        got_frame = avcodec_send_packet(c, &avpkt);

        if (got_frame < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
            return 0;
        }

        len = avcodec_receive_frame(c, frame);

        if (len < 0) {
            matReady = false;
            fprintf(stderr, "Error while decoding frame %d\n", frame_count);
            frame_count++;

            return 0;
        }
        if(out_buffer == nullptr) {
            //BGRsize = avpicture_get_size(AV_PIX_FMT_BGR24, c->width,
            //                             c->height);


            BGRsize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, c->width, c->height, 1);
            out_buffer = (uint8_t *) av_malloc(BGRsize);
            //avpicture_fill((AVPicture *) pFrameBGR, out_buffer, AV_PIX_FMT_BGR24,
            //               c->width, c->height);

            av_image_fill_arrays(pFrameBGR->data,pFrameBGR->linesize, out_buffer, AV_PIX_FMT_BGR24, c->width, c->height, 1);

            img_convert_ctx =
                    sws_getContext(c->width, c->height, c->pix_fmt,
                                   c->width, c->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, nullptr, nullptr,
                                   nullptr);
            pCvMat.create(cv::Size(c->width, c->height), CV_8UC3);

        }
        if (got_frame) {
            matReady = true;
            sws_scale(img_convert_ctx, (const uint8_t *const *)frame->data,
                      frame->linesize, 0, c->height, pFrameBGR->data, pFrameBGR->linesize);

            memcpy(pCvMat.data, out_buffer, BGRsize);
            std::cout << "decoding frame " << frame_count << std::endl;

//        printf("decoding frame: %d\n",frame_count);
            frame_count++;
        }
        else{
            matReady = false;
        }
        if (avpkt.data) {
            avpkt.size -= len;
            avpkt.data += len;
        }

        return 1;

    }

    cv::Mat getMat() {
        return pCvMat;
    }



private:
    const AVCodec *codec;
    AVCodecContext *c = nullptr;
    int frame_count;
    AVFrame *frame;
    AVPacket avpkt;
    AVFrame *pFrameBGR;

    int BGRsize;
    uint8_t *out_buffer = nullptr;

    struct SwsContext *img_convert_ctx;
    cv::Mat pCvMat;
    bool matReady;

};

#endif //OPENH264_EXAMPLE_DECODER_H
