#include <opencv4/opencv2/opencv.hpp>
#include "Encoder.h"
#include "Decoder.h"

int main() {
    cv::Mat image;
    int width = 800;
    int height = 720;
    int fps = 30;
    cv::namedWindow("Display window");
    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FPS, fps);
    cap.open(0);
    //int in_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    //int in_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    bool image_ready = false;

    auto *encoder = new Encoder(width, height, width, height, fps);
    auto *decoder = new Decoder(width, height);

    if (!cap.isOpened()) {
        std::cout << "cannot open camera";
    }

    while (true) {
        cap >> image;
        unsigned char *frame = image.data;

        int encoded_size = encoder->encode(frame, &image_ready);
        int display_size = decoder->decode(encoder->image_buff, encoded_size);
        encoder->delete_data();
        cv::Mat display_frame = decoder->getMat();

        if (!display_frame.empty()) {
            cv::Mat display_frame = decoder->getMat();
            imshow("Display window", display_frame);
            cv::waitKey(1);
        }
    }
    return 0;
}