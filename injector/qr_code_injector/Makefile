CC=g++
CXXFLAGS= -std=c++11
LDFLAGS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lqrencode

qr_code_injector: qr_code_injector.cpp 
	$(CC) -o qr_code_injector qr_code_injector.cpp $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f qr_code_injector
