all:
	g++ jvi.cpp -o jvi `pkg-config gtkmm-3.0 --cflags --libs` -I/usr/include/jsoncpp -ljsoncpp -std=c++14 -Wall
