#pragma once

#include <vector>
#include <string>

class FMMLCompiler{
public:
    FMMLCompiler(int sampleRate)
    : m_sample_rate(sampleRate)
    , m_bpm(120){}

    double getTimePerBeat();
    int getBlockSize();
    int compile(std::string filename, FILE* fp);
private:
    const int m_max_amp = 0x10;
    int m_sample_rate;
    float m_bpm;
};
