#pragma once

#include <stdlib.h>

class Chunk{
public:
    Chunk()
    : mCkID  (0)
    , mCkSize(0)
    , mData  (nullptr){}

    ~Chunk(){
        if(this->mData != nullptr){
            free(this->mData);
        }
    }

    void AllocDataSize();

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    
    int mCkID;
    int mCkSize;
    char* mData;
};