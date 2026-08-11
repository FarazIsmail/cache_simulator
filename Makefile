CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2
WX_CXXFLAGS := $(shell wx-config --cxxflags)
WX_LIBS := $(shell wx-config --libs)

SRC = main.cpp gui.cpp simulator.cpp

cache_simulator: $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(WX_CXXFLAGS) $(WX_LIBS) -o cache_simulator

clean:
	rm -f cache_simulator
