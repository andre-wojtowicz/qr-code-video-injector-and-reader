This repository consists of two programs: the first is to insert QR code into a movie, the second is to decode such QR codes in movies.

# QR code injector

This program inserts QR code with a frame number into YUV movie. The program uses [OpenCV](https://opencv.org) library (tested on 2.4.10) and [libqrencode](https://fukuchi.org/works/qrencode/).

Usage: `qr_code_injector QR-POSITION QR-SCALE-FACTOR WIDTH HEIGHT [NUM-IGNORE-LAST-BYTES]`
  * input:  YUV movie is read from stdin; each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format; may be concatenated with raw audio.
  * output: YUV movie is printed to stdout, each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format.

Parameters:
```
QR-POSITION            one of the following values: top-left top-right bottom-left bottom-right
QR-SCALE-FACTOR        even integer to set QR code size
WIDTH                  width of the frame
HEIGHT                 height of the frame
NUM-IGNORE-LAST-BYTES  optional; number of bytes to in a movie stream (in case of concatenated 
                       video and audio raw)
```
Example: `cat movie.yuv | qr_code_injector top-left 6 1280 720 > movie-with-qr-code.yuv`

# QR code reader

This program decodes QR codes inserted into YUV movie. The program uses [OpenCV](https://opencv.org) library (tested on 2.4) and [zbar](http://zbar.sourceforge.net/).

Usage: `qr_code_reader WIDTH HEIGHT [QR-POSITION] [QR-SCALE-FACTOR] [NUM-IGNORE-LAST-BYTES] [SHOW-FRAMES]`
  * input:  YUV movie is read from stdin; each line is multiplexed 4:2:2 component video (Cb Y Cr Y...) format; may be concatenated with raw audio.
  * output: integers printed to stdout, each line is either a decoded frame number or -1 (if decoding fails).

Parameters:
```
WIDTH                  width of the frame
HEIGHT                 height of the frame
QR-POSITION            optional; one of the following values: top-left top-right bottom-left 
                       bottom-right; default: top-left
QR-SCALE-FACTOR        optional; even integer to set QR code size; default: 2
NUM-IGNORE-LAST-BYTES  optional; number of bytes to in a movie stream (in case of concatenated 
                       video and audio raw); default: 0
SHOW-FRAMES            optional; one of the following values: full cropped no; default: cropped
```
Example: `cat movie.yuv | qr_code_reader 720 576 top-left 6 0 full`

Author of QR code reader: Marcin Baranowski.
