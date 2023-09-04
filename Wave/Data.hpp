#pragma once

#include "Chunk.hpp"
#include <math.h>

using namespace std;

class Data : public Chunk{
public:
    Data(){
        this->mCkID = 0x61746164;
        this->mData = 0;
    }

    void setData(int size, char* data);
};