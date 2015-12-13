all:
	g++ jvi.cpp -o jvi `pkg-config gtkmm-3.0 --cflags --libs` -ljsoncpp -std=c++14
