//
// Created by corey on 10/23/21.
//

#ifndef OPENH264_EXAMPLE_ENCODER_H
#define OPENH264_EXAMPLE_ENCODER_H


#include <cstdint>
#include <queue>

extern "C" {
#include <x264.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
}


class Encoder {
public:
    x264_nal_t* nals{};
    int num_nals{};
    unsigned char* image_buff{};

    Encoder(int inW, int inH, int outW, int outH, float fps):
            in_xres(inW),
            in_yres(inH),
            out_xres(outW),
            out_yres(outH){
        x264_param_default_preset(&prms, "ultrafast", "zerolatency,fastdecode");
        x264_param_apply_profile(&prms, "baseline");
        prms.i_width = out_xres;
        prms.i_height = out_yres;
        prms.i_fps_num = (int)fps;
        prms.i_fps_den = 1;
        prms.rc.i_qp_constant = 20;
        //added
        prms.i_sps_id = 7;
        // the following two value you should keep 1
        prms.b_repeat_headers = 1;    // to get header before every I-Frame
        prms.b_annexb = 1;

        prms.rc.i_rc_method = X264_RC_CRF;
        prms.rc.f_rf_constant = 20;
        prms.rc.f_rf_constant_max = 25;

        prms.i_csp = X264_CSP_I420;
        enc = x264_encoder_open(&prms);
        x264_encoder_headers(enc, &nals, &nheader);

        //Initialize X264 Picture
        x264_picture_alloc(&pic_in, X264_CSP_I420, out_xres, out_yres);

        // Color conversion setup
        sws = sws_getContext(in_xres, in_yres, cam_pixel_fmt, //AV_PIX_FMT_BAYER_GBRG8, AV_PIX_FMT_RGB24
                             out_xres, out_yres, AV_PIX_FMT_YUV420P,
                             SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

        if(!sws) {
            std::cout << "Cannot create SWS context" << std::endl;

        }

    }


    int encode(unsigned char* img, bool *imgReady) {

        // Put raw image data to AV picture
        int bytes_filled = av_image_fill_arrays(pic_raw.data,pic_raw.linesize, img,
                                                cam_pixel_fmt, in_xres, in_yres,1);
        if(!bytes_filled) {
            std::cout << "Cannot fill the raw input buffer" << std::endl;
            return 0;
        }

        // convert to I420 for x264
        int h = sws_scale(sws, pic_raw.data, pic_raw.linesize, 0,
                          in_yres, pic_in.img.plane, pic_in.img.i_stride);
        if(h != out_yres) {
            std::cout << "scale failed: %d" << std::endl;
            return 0;
        }

        // Set imgReady to false here (cameraGrabber thread may now write a new image into it), since the data we process now with is already in another place
        *imgReady = false;

        // Encode
        pic_in.i_pts = framecounter++;
        int frame_size = x264_encoder_encode(enc, &nals, &num_nals, &pic_in, &pic_out);
        image_buff = new unsigned char[out_xres * out_yres * 3];

        int offset;
        for (int i = 0; i < num_nals; i++) {
            std::memcpy(image_buff + offset, nals[i].p_payload, nals[i].i_payload);
            offset += nals[i].i_payload;
        }
        return offset;
    }

    void delete_data() const {
        delete[] image_buff;
    }

private:
    int in_xres, in_yres, out_xres, out_yres;
    int framecounter{};
    x264_t* enc;
    int nheader{};
    x264_param_t prms{};
    x264_picture_t pic_in{}, pic_out{};

    struct SwsContext* sws;
    AVFrame pic_raw{};     /* used for our "raw" input container */
    //AVPixelFormat cam_pixel_fmt = AV_PIX_FMT_BGR24;
    AVPixelFormat cam_pixel_fmt = AV_PIX_FMT_RGB24;

};


#endif //OPENH264_EXAMPLE_ENCODER_H
