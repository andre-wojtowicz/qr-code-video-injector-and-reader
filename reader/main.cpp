/*
 * main.cpp
 *
 *  Created on: 18 gru 2014
 *      Author: Marcin Baranowski
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <zbar.h>

#include <iostream>
#include <fstream>
#include <string>

#include "include/yuv422_loader.h"

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

#define DEFAULT_QRCODE_SIZE 50

int main(int argc, char** argv)
{
#if defined(_WIN32)
	int result_in = _setmode(_fileno(stdin), _O_BINARY),
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

	if (argc >= 3)
	{
		int width = atoi(argv[1]);
		int height = atoi(argv[2]);
		int bytes_to_ign = (argc >= 6 ? atoi(argv[5]) : 0);

		int qr_scale_factor = (argc >= 5 ? atoi(argv[4]) : 2);

		if ((qr_scale_factor % 2) != 0)
		{
			std::cerr << "QR code scale factor must be even; set to 2."
					<< std::endl;
			qr_scale_factor = 2;
		}

		cv::Rect myROI;
		int roi_size = (DEFAULT_QRCODE_SIZE * qr_scale_factor);

		std::string str_qr_postion = "";
		if (argc >= 4)
		{
			str_qr_postion = std::string(argv[3]);

			if (str_qr_postion == "top-left")
				myROI = cv::Rect(0, 0, roi_size, roi_size);
			else if (str_qr_postion == "top-right")
				myROI = cv::Rect(width - roi_size, 0, roi_size, roi_size);
			else if (str_qr_postion == "bottom-left")
				myROI = cv::Rect(0, height - roi_size, roi_size, roi_size);
			else if (str_qr_postion == "bottom-right")
				myROI = cv::Rect(width - roi_size, height - roi_size, roi_size,
						roi_size);
			else
			{
				std::cerr << "Unknown QR code position; set to top-left."
						<< std::endl;
				myROI = cv::Rect(0, 0, roi_size, roi_size);
			}
		}

		// default true
		bool show_frames = true;
		bool show_full_frame = false;

		if (argc >= 7)
		{
			std::string str_view(argv[6]);
			if (str_view == "no")
				show_frames = false;
			else if (str_view == "full")
				show_full_frame = true;
			else if (str_view != "cropped")
				std::cerr << "Unknown view option; set to yes." << std::endl;
		}

		YUV422_Loader loader(width, height);

		char* frame_raw = new char[(width * 2) * height];

		// create a reader
		zbar::ImageScanner scanner;

		// configure the reader
		scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

		float current_frame = 0;
		float decoded_frames = 0;

		while (true)
		{
			// read next frame
			std::cin.read(frame_raw, (width * 2) * height);

			if (std::cin.eof() || std::cin.fail())
				break;

			cv::Mat* mat_frame_y = loader.grab_y_frame(frame_raw);
			cv::Mat binary;
			int w, h;

			if (myROI.width == 0)
			{
				threshold(*mat_frame_y, binary, 128, 255, cv::THRESH_BINARY);
				w = width;
				h = height;
			}
			else
			{
				cv::Mat croppedImage = (*mat_frame_y)(myROI);
				threshold(croppedImage, binary, 128, 255, cv::THRESH_BINARY);

				w = h = roi_size;
			}

			const void *raw = binary.data;

			// wrap image data
			zbar::Image image(w, h, "Y800", raw, w * h);

			// scan the image for qr codes
			int n = scanner.scan(image);

			if (n > 0)
			{
				++decoded_frames;

				// extract results
				for (zbar::Image::SymbolIterator symbol = image.symbol_begin();
						symbol != image.symbol_end(); ++symbol)
				{
					if (myROI.width == 0)
					{
						if (symbol->get_location_x(0) > (width / 2))
						{
							if (symbol->get_location_y(0) > (width / 2))
							{
								myROI = cv::Rect(width - roi_size,
										height - roi_size, roi_size, roi_size);
							}
							else
							{
								myROI = cv::Rect(width - roi_size, 0, roi_size,
										roi_size);
							}
						}
						else
						{
							if (symbol->get_location_y(0) > (width / 2))
							{
								myROI = cv::Rect(0, height - roi_size, roi_size,
										roi_size);
							}
							else
								myROI = cv::Rect(0, 0, roi_size, roi_size);
						}

					}
					std::cout << symbol->get_data() << std::endl;
				}
			}
			else
				std::cout << -1;

			// clean up
			image.set_data(0, 0);

			if (show_frames)
			{
				if (show_full_frame)
					cv::imshow("", *mat_frame_y);
				else
					cv::imshow("", binary);
				//needed to show current frame
				cv::waitKey(1);
			}
			delete mat_frame_y;

			std::cin.read(frame_raw, bytes_to_ign);

			if (std::cin.eof() || std::cin.fail())
				break;

			++current_frame;
		}

		std::cerr << "\n\nSuccessful decoding rate: "
				<< decoded_frames / current_frame * 100 << "%\n\n";

		delete[] frame_raw;
	}
	else
	{
		std::cout
				<< "This program reads qr codes from stdin YUV movie, in which each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format."
				<< std::endl << std::endl;
		std::cout << "Usage:   " << argv[0]
				<< " WIDTH HEIGHT [QR-POSITION] [QR-SCALE-FACTOR] [NUM-IGNORE-LAST-BYTES] [SHOW-FRAMES]"
				<< std::endl;
		std::cout << "      WIDTH  - width of a frame" << std::endl;
		std::cout << "      HEIGHT - height of a frame" << std::endl;
		std::cout
				<< "      QR-POSITION - optional; one of the following values: top-left top-right bottom-left bottom-right; default: top-left"
				<< std::endl;
		std::cout
				<< "      QR-SCALE-FACTOR - optional; even integer to set QR code size; default: 2"
				<< std::endl;
		std::cout
				<< "      NUM-IGNORE-LAST-BYTES  - optional; number of bytes to ignore in a movie stream (in case of concatenated video and audio raw, see bmdcapture); default: 0"
				<< std::endl;
		std::cout
				<< "      SHOW-FRAMES - optional; one of the following values: full cropped no; default: cropped"
				<< std::endl << std::endl;
		std::cout << "Example: cat movie.yuv | " << argv[0]
				<< "  720 576 top-left 6 0 full" << std::endl;
	}

	return 0;
}

