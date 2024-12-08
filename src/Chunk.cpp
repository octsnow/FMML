#include "Chunk.hpp"

void Chunk::AllocDataSize(){
    if(this->mData != nullptr){
        free(this->mData);
    }
    this->mData = (char*)malloc(this->mCkSize);
}