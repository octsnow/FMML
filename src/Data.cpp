#include "Data.hpp"

void Data::setData(int size, char* data){
    this->mCkSize = size;
    this->AllocDataSize();

    for(int i = 0; i < this->mCkSize; i++){
        this->mData[i] = data[i];
    }
}