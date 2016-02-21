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
