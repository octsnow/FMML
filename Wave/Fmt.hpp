#pragma once

#include "Chunk.hpp"

const short WAVE_FORMAT_PCM         = 0x0001;
const short WAVE_FORMAT_IEEE_FLOAT  = 0x0003;
const short WAVE_FORMAT_ALAW        = 0x0006;
const short WAVE_FORMAT_MULAW       = 0x0007;
const short WAVE_FORMAT_EXTENSIBLE  = 0xFFFE;

enum FMT_CKSIZE{
    FMT_CKSIZE_16 = 0,
    FMT_CKSIZE_18,
    FMT_CKSIZE_40,
    FMT_CKSIZE_NUM
};

typedef struct _FmtData{
    FMT_CKSIZE ckSize;
    short wFormatTag;
    short nChannels;
    int nSamplesPerSec;
    int nAvgBytesPerSec;
    short nBlockAlign;
    short wBitsPerSample;
    short cbSize;
    short wValidBitsPerSample;
    int dwChannelMask;
    char subFormat[16];
} FmtData;

class Fmt : public Chunk{
public:
    Fmt(){
        this->mCkID = 0x20746d66;
    }

    void setFmtData(FmtData& fmtData);
};