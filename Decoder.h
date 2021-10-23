//
// Created by corey on 10/23/21.
//

#ifndef OPENH264_EXAMPLE_DECODER_H
#define OPENH264_EXAMPLE_DECODER_H

#include <libavcodec/avcodec.h>

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#endif

class Decoder {
public:
    Decoder() {
        matReady = false;

        avcodec_register_all();
        av_init_packet(&avpkt);

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

        if (avcodec_open2(c, codec, NULL) < 0) {
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

    void decode(unsigned char *inputbuf, size_t size){

        avpkt.size = size;
        if(avpkt.size == 0)
            return;

        avpkt.data = inputbuf;

        int len, got_frame;

        len = avcodec_receive_frame(c, frame);

        //deprecated
        //len = avcodec_decode_video2(c, frame, &got_frame, &avpkt);

        if (len < 0) {
            matReady = false;
            fprintf(stderr, "Error while decoding frame %d\n", frame_count);
            frame_count++;

            return ;
        }
        if(out_buffer == nullptr) {
            //BGRsize = avpicture_get_size(AV_PIX_FMT_BGR24, c->width,
            //                             c->height);


            BGRsize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, c->width, c->height);
            out_buffer = (uint8_t *) av_malloc(BGRsize);
            avpicture_fill((AVPicture *) pFrameBGR, out_buffer, AV_PIX_FMT_BGR24,
                           c->width, c->height);

            img_convert_ctx =
                    sws_getContext(c->width, c->height, c->pix_fmt,
                                   c->width, c->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL,
                                   nullptr);
            pCvMat.create(cv::Size(c->width, c->height), CV_8UC3);

        }
        if (got_frame) {
            matReady = true;
            sws_scale(img_convert_ctx, (const uint8_t *const *)frame->data,
                      frame->linesize, 0, c->height, pFrameBGR->data, pFrameBGR->linesize);

            memcpy(pCvMat.data, out_buffer, BGRsize);

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
