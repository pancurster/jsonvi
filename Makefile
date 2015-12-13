all:
	g++ hello.cpp -o hello `pkg-config gtkmm-3.0 --cflags --libs` -ljsoncpp -std=c++14
