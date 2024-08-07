#include <iostream>
#include "Wave/Wave.hpp"
#include "FMMLCompiler.hpp"

#include <vector>
#include <fstream>

using namespace std;

const int SAMPLE_RATE = 44100;

int main(int argc, char** argv){
    if(argc <= 1){
        cerr << "Please specify FMML file." << endl;
        return -1;
    }
    Wave wav;
    FMMLCompiler comp(SAMPLE_RATE);
    FmtData fmtData;
    FILE* tpf = NULL;
    errno_t e = tmpfile_s(&tpf);

    if(e != 0) {
        cerr << "error code (" << e << "): failed to create temp file" << endl;
        return -1;
    }

    fmtData.ckSize = FMT_CKSIZE_16;
    fmtData.wFormatTag = WAVE_FORMAT_PCM;
    fmtData.nChannels = 1;
    fmtData.nSamplesPerSec = SAMPLE_RATE;
    fmtData.nAvgBytesPerSec = SAMPLE_RATE;
    fmtData.nBlockAlign = 1;
    fmtData.wBitsPerSample = 8;

    comp.compile(argv[1], tpf);

    wav.setFmtData(fmtData);
    wav.writeWaveDataFile("out.wav", tpf);

    fclose(tpf);
    
    return 0;
}
