#include "Fmt.hpp"
#include <iostream>

void Fmt::setFmtData(FmtData& fmtData){
    char* data;
    
    switch(fmtData.ckSize){
    case FMT_CKSIZE_16:
        this->mCkSize = 16;
        break;
    case FMT_CKSIZE_18:
        this->mCkSize = 18;
        break;
    case FMT_CKSIZE_40:
        this->mCkSize = 40;
        break;
    default:
        std::cerr << "unknown size" << std::endl;
        break;
    }
    this->AllocDataSize();

    data = this->mData;
    *(data++) = fmtData.wFormatTag & 0xFF;
    *(data++)   = fmtData.wFormatTag >> 8;

    *(data++) = fmtData.nChannels & 0xFF;
    *(data++) = fmtData.nChannels >> 8;

    *(data++) = fmtData.nSamplesPerSec       & 0xFF;
    *(data++) = fmtData.nSamplesPerSec >>  8 & 0xFF;
    *(data++) = fmtData.nSamplesPerSec >> 16 & 0xFF;
    *(data++) = fmtData.nSamplesPerSec >> 24;

    *(data++) = fmtData.nAvgBytesPerSec       & 0xFF;
    *(data++) = fmtData.nAvgBytesPerSec >>  8 & 0xFF;
    *(data++) = fmtData.nAvgBytesPerSec >> 16 & 0xFF;
    *(data++) = fmtData.nAvgBytesPerSec >> 24;

    *(data++) = fmtData.nBlockAlign & 0xFF;
    *(data++) = fmtData.nBlockAlign >> 8;

    *(data++) = fmtData.wBitsPerSample & 0xFF;
    *(data++) = fmtData.wBitsPerSample >> 8;
    if(this->mCkSize > 16){
        *(data++) = fmtData.cbSize & 0xFF;
        *(data++) = fmtData.cbSize >> 8;
    }
    if(this->mCkSize > 18){
        *(data++) = fmtData.wValidBitsPerSample & 0xFF;
        *(data++) = fmtData.wValidBitsPerSample >> 8;
        
        *(data++) = fmtData.dwChannelMask       & 0xFF;
        *(data++) = fmtData.dwChannelMask >>  8 & 0xFF;
        *(data++) = fmtData.dwChannelMask >> 16 & 0xFF;
        *(data++) = fmtData.dwChannelMask >> 24;
        for(int i = 0; i < 16; i++){
            *(data++) = fmtData.subFormat[i];
        }
    }
}
