#include <opencv4/opencv2/opencv.hpp>
#include "Encoder.h"
#include "Decoder.h"

int main() {
    cv::Mat image;
    cv::namedWindow("Display window");
    cv::VideoCapture cap(0);
    int in_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int in_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    //float fps = cap.get(cv::CAP_PROP_FPS);
    //int out_width = 750;
    //int out_height = 700;
    float fps = 30;
    bool image_ready = false;

    auto *encoder = new Encoder(in_width, in_height, in_width, in_height, fps);
    auto *decoder = new Decoder();

    if (!cap.isOpened()) {
        std::cout << "cannot open camera";
    }

    while (true) {
        cap >> image;
        unsigned char *frame = image.data;
        int encoded_size = encoder->encode(frame, &image_ready);
        int display_size = decoder->decode(frame, encoded_size);

        if (display_size) {
            cv::Mat display_frame = decoder->getMat();
            //cv::Mat display_frame = cv::Mat(in_height, in_width, CV_8UC1, frame);

            imshow("Display window", display_frame);
        }
        cv::waitKey(20);
    }

    // may need libav
    return 0;
}