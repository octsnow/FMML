SRC = $(wildcard *.cpp)
WAVE_LIB = $(wildcard Wave/*.cpp)

all: main
main:
	cl /EHsc $(SRC) $(WAVE_LIB) /Fefmml
