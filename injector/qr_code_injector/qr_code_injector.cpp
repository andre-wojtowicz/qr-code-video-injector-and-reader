#include <qrencode.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <fstream>
#include <string>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

enum QRposition { TopLeft, TopRight, BottomLeft, BottomRight};

int main(int argc, char** argv)
{

#if defined(_WIN32)
    int result_in  = _setmode(_fileno(stdin),  _O_BINARY),
        result_out = _setmode(_fileno(stdout), _O_BINARY);
#if defined(_DEBUG)
    if (result_in == -1)
    {
        std::cerr << "Cannot set 'stdin' mode to binary." << std::endl;
        return -1;
    }
    else
        std::cerr << "'stdin' successfully changed to binary mode." << std::endl;

    if (result_out == -1)
    {
        std::cerr << "Cannot set 'stdout' mode to binary." << std::endl;
        return -2;
    }
    else
        std::cerr << "'stdout' successfully changed to binary mode." << std::endl;
#endif
#endif

    if (argc >= 5)
    {
        QRposition qr_position;
        std::string str_qr_postion(argv[1]);

        if (str_qr_postion == "top-left")
            qr_position = QRposition::TopLeft;
        else if (str_qr_postion == "top-right")
            qr_position = QRposition::TopRight;
        else if (str_qr_postion == "bottom-left")
            qr_position = QRposition::BottomLeft;
        else if (str_qr_postion == "bottom-right")
            qr_position = QRposition::BottomRight;
        else
        {
            std::cerr << "Unknown QR code position; set to top-left." << std::endl;
            qr_position = QRposition::TopLeft;
        }

        int qr_scale_factor = atoi(argv[2]);
        if ((qr_scale_factor % 2) != 0)
        {
            std::cerr << "QR code scale factor must be even; set to 2." << std::endl;
            qr_scale_factor = 2;
        }

        int width           = atoi(argv[3]);
        int height          = atoi(argv[4]);
        int bytes_to_ign = (argc == 6 ? atoi(argv[5]) : 0);

        unsigned int frame_counter = 0;
        char* frame_raw = new char[(width * 2) * height];

        while (true)
        {
            // read next frame

            std::cin.read(frame_raw, (width * 2) * height);

            if (std::cin.eof() || std::cin.fail())
                break;

            frame_counter++;

            // generate QR code image

            QRcode* fid_qrcode = QRcode_encodeString(std::to_string(frame_counter).c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);

            cv::Mat temp_mat = cv::Mat(fid_qrcode->width, fid_qrcode->width, CV_8UC1, fid_qrcode->data);

            for (int i = 0; i < fid_qrcode->width; i++)
                for (int j = 0; j < fid_qrcode->width; j++)
                    temp_mat.at<uchar>(i, j) = ((temp_mat.at<uchar>(i, j) & 1) == 0 ? 255 : 0);

            cv::Mat fid_mat;
            copyMakeBorder( temp_mat, fid_mat, 1, 1, 1, 1, cv::BORDER_CONSTANT, 255 );

            cv::resize(fid_mat, fid_mat, cv::Size(), qr_scale_factor, qr_scale_factor, cv::INTER_NEAREST);

            QRcode_free(fid_qrcode);

            // prepare QR code image YUV channels

            cv::Mat& qr_code_Y = fid_mat;
            cv::Mat  qr_code_U = cv::Mat(fid_mat.rows, fid_mat.cols / 2, CV_8UC1, cv::Scalar(128));
            cv::Mat  qr_code_V = cv::Mat(fid_mat.rows, fid_mat.cols / 2, CV_8UC1, cv::Scalar(128));

            qr_code_Y = qr_code_Y.reshape(2);

            cv::Mat qr_code_Y_channels[2];
            cv::split(qr_code_Y, qr_code_Y_channels);

            cv::Mat& qr_code_Y1 = qr_code_Y_channels[0];
            cv::Mat& qr_code_Y2 = qr_code_Y_channels[1];

            // prepare frame to YUV channels

            cv::Mat mat_frame_raw(1, (width * 2) * height, CV_8UC1, frame_raw);
            cv::Mat mat_frame_channels[4];

            mat_frame_raw = mat_frame_raw.reshape(4);
            cv::split(mat_frame_raw, mat_frame_channels);

            cv::Mat& frame_U  = mat_frame_channels[0];
            cv::Mat& frame_Y1 = mat_frame_channels[1];
            cv::Mat& frame_V  = mat_frame_channels[2];
            cv::Mat& frame_Y2 = mat_frame_channels[3];

            frame_Y1 = frame_Y1.reshape(1, height);
            frame_Y2 = frame_Y2.reshape(1, height);
            frame_U = frame_U.reshape(1, height);
            frame_V = frame_V.reshape(1, height);

            // insert QR code image into frame

            int originX = (qr_position == QRposition::TopLeft || qr_position == QRposition::BottomLeft ? 0 : width/2 - qr_code_Y1.cols);
            int originY = (qr_position == QRposition::TopLeft || qr_position == QRposition::TopRight ? 0 : height - qr_code_Y1.rows);

            cv::Rect roi(cv::Point(originX, originY), qr_code_Y1.size());

            qr_code_Y1.copyTo(frame_Y1(roi));
            qr_code_Y2.copyTo(frame_Y2(roi));
            qr_code_U.copyTo(frame_U(roi));
            qr_code_V.copyTo(frame_V(roi));

            // resize and merge frame channels

            frame_Y1 = frame_Y1.reshape(1, width * height / 2);
            frame_Y2 = frame_Y2.reshape(1, width * height / 2);
            frame_U = frame_U.reshape(1, width * height / 2);
            frame_V = frame_V.reshape(1, width * height / 2);

            cv::Mat output_channels[4] = { frame_U, frame_Y1, frame_V, frame_Y2 };
            cv::Mat output_mat = cv::Mat(height, width, CV_8UC4);
            cv::merge(output_channels, 4, output_mat);

            output_mat = output_mat.reshape(1, width * height * 2);

            fwrite(output_mat.data, sizeof(unsigned char), width * height * 2, stdout);

            std::cin.read(frame_raw, bytes_to_ign);

            if (std::cin.eof() || std::cin.fail())
                break;
        }

        delete[] frame_raw;

    }
    else
    {
        std::cout << "This program inserts QR code into YUV movie." << std::endl;
        std::cout << "Usage:   " << argv[0] << " QR-POSITION QR-SCALE-FACTOR WIDTH HEIGHT [NUM-IGNORE-LAST-BYTES]" << std::endl << std::endl;
        std::cout << "  * input:  YUV movie is read from stdin; each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format; may be concatenated with raw audio." << std::endl;
        std::cout << "  * output: YUV movie is printed to stdout, each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format." << std::endl;
        std::cout << "Parameters:" << std::endl << std::endl;
        std::cout << "    QR-POSITION            one of the following values: top-left top-right bottom-left bottom-right" << std::endl;
        std::cout << "    QR-SCALE-FACTOR        even integer to set QR code size" << std::endl;
        std::cout << "    WIDTH                  width of the frame" << std::endl;
        std::cout << "    HEIGHT                 height of the frame" << std::endl;
        std::cout << "    NUM-IGNORE-LAST-BYTES  optional; number of bytes to in a movie stream (in case of concatenated video and audio raw, see bmdcapture)" << std::endl << std::endl;
        std::cout << "Example: cat movie.yuv | " << argv[0] << " top-left 6 1280 720 > movie-with-qr-code.yuv" << std::endl;

        // todo: describe parameters
    }


    return 0;
}
