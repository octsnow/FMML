#pragma once

#include <vector>
#include <string>

class FMMLCompiler{
public:
    FMMLCompiler(int sampleRate)
    : mSampleRate(sampleRate)
    , mBpm(120){}

    double getTimePerBeat();
    int getBlockSize();
    int compile(std::string filename, FILE* fp);
private:
    const int mMaxAmp = 0x10;
    int mSampleRate;
    int mBpm;
};
