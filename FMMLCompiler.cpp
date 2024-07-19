#include "FMMLCompiler.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>

using namespace std;
namespace{
    const float PI = 3.14159265358;
    const int AF = 880;
    const float SndNormalRatio = (31 / 32.0); // sound ratio
    const float SndLongRatio = 1.0;
    const float SndShortRatio = 0.5;
    enum ReadMode{
        Normal = 0,
        Scale,
        OctavePos,
        OctaveNeg,
        Volum,
        Note,
        NoteTms,
        ChordEnd,
        Tempo,
        Comment,
        Comments
    };
    enum ScaleType{
        C = 3,
        Cs,
        D,
        Ds,
        E,
        F,
        Fs,
        G,
        Gs,
        A,
        As,
        B,
        R
    };

    typedef struct _MusicalScale{
        ScaleType s;
        float octave = 1;
    } MusicalScale;

    bool isNum(char c){
        return int('0') <= int(c) && int(c) <= int('9');
    }

    int calcFrequency(MusicalScale ms) {
        int f = AF * pow(2, ms.s / 12.0) * ms.octave;

        return f;
    }

    bool checkCommand(string str, string command, bool* forMatch){
        int minLen = str.length() < command.length() ? str.length() : command.length();
        if(str.substr(0, minLen) == command.substr(0, minLen)){
            *forMatch = true;
        }

        if(str.length() >= command.length()){
            return str.substr(0, command.length()) == command;
        }

        return false;
    }

    void genSqWave(int length, char maxAmp, vector<MusicalScale> msList, FILE* fp, int sampleRate){
        static int msLastCounts[12] = { 0 };

        for(int i = 0; i < length; i++){
            char c;
            int tmp;
            float v = 0;

            for(MusicalScale ms : msList){
                if(ms.s != R){
                    int f = calcFrequency(ms);
                    v += ((i + msLastCounts[ms.s - 3]) * f / sampleRate) & 1;
                }
            }
            v = v / msList.size() * 2;
            tmp = v * maxAmp * msList.size();
            c = tmp > 0xFF ? 0xFF : (tmp < 0x00 ? 0x00 : tmp);
            fwrite(&c, 1, 1, fp);
        }

        memset(msLastCounts, 0, sizeof(msLastCounts));

        for(MusicalScale ms : msList) {
            msLastCounts[ms.s - 3] = length % (sampleRate / calcFrequency(ms));
        }
    }

    void genSinWave(int length, char maxAmp, vector<MusicalScale> msList, FILE* fp, int sampleRate){
        for(int i = 0; i < length; i++){
            char c;
            int tmp;
            float v = 0;

            for(MusicalScale ms : msList){
                if(ms.s != R){
                    float f = AF * pow(2, ms.s / 12.0) * ms.octave;
                    v += sin((i * f / sampleRate) * PI);
                }
            }
            v = v / msList.size() * 2;
            tmp = v * maxAmp;
            c = tmp > 0xFF ? 0xFF : (tmp < 0x00 ? 0x00 : tmp);
            fwrite(&c, 1, 1, fp);
        }
    }

    void genNoiseWave(int length, char maxAmp, MusicalScale ms, FILE* fp, int sampleRate){
        srand(time(NULL));
        int f;
        if(ms.s == R){
            f = sampleRate / length;
        }else{
            f = AF * pow(2, ms.s / 12.0) * ms.octave;
        }
        int v = 0;
        for(int i = 0; i < length; i++){
            if(i % (sampleRate / f) == 0){
                v = rand() & 1;
            }
            char c;
            c = v * maxAmp;
            fwrite(&c, 1, 1, fp);
        }
    }
}

double FMMLCompiler::getTimePerBeat(){
    return 240.0 / this->mBpm;
}

int FMMLCompiler::getBlockSize(){
    return this->mSampleRate * this->getTimePerBeat();
}

int FMMLCompiler::compile(string filename, FILE* fp){
    int noteTimes = 1;
    float octave = 1;
    float note = 4;
    float fraction = 0;
    float volum = 1.0;
    bool isChord = false;
    bool isSigned = false;
    bool isNoise = false;
    int adpDigit = 0;
    vector<MusicalScale> chordList;
    MusicalScale msBuf;
    string cBuf;
    ReadMode readMode = Normal;

    ifstream ifs(filename);

    if(ifs.fail()){
        cerr << "Failed to open file: " << filename << endl;
        return -1;
    }

    while(1){
        bool isForMatch = false;
        char c;
        ifs.read(&c, 1);

        if(!ifs.eof()){
            cBuf.push_back(c);
        }else if(cBuf.size() <= 0 && readMode == Normal){
            break;
        }

        switch(readMode){
            case Normal:{
                if(int('a') <= int(cBuf[0]) && int(cBuf[0]) <= int('g') || cBuf[0] == 'r'){
                    msBuf.octave = octave;
                    octave = 1;
                    if(isSigned){
                        switch(cBuf[0]){
                            case 'a': msBuf.s = As; break;
                            case 'c': msBuf.s = Cs; break;
                            case 'd': msBuf.s = Ds; break;
                            case 'f': msBuf.s = Fs; break;
                            case 'g': msBuf.s = Gs; break;
                        }
                        isSigned = false;
                    }else{
                        switch(cBuf[0]){
                            case 'a': msBuf.s = A; break;
                            case 'b': msBuf.s = B; break;
                            case 'c': msBuf.s = C; break;
                            case 'd': msBuf.s = D; break;
                            case 'e': msBuf.s = E; break;
                            case 'f': msBuf.s = F; break;
                            case 'g': msBuf.s = G; break;
                            case 'r': msBuf.s = R; break;
                        }
                    }
                    cBuf = cBuf.substr(1);
                    readMode = Scale;
                }else if(cBuf[0] == '#'){
                    cBuf = cBuf.substr(1);
                    isSigned = true;
                }else if(cBuf[0] == 'z'){
                    cBuf = cBuf.substr(1);
                    isNoise = true;
                }else if(cBuf[0] == '+'){
                    octave = 0;
                    cBuf = cBuf.substr(1);
                    readMode = OctavePos;
                }else if(cBuf[0] == '-'){
                    octave = 0;
                    cBuf = cBuf.substr(1);
                    readMode = OctaveNeg;
                }else if(cBuf[0] == 'v'){
                    volum = 0.0;
                    cBuf = cBuf.substr(1);
                    readMode = Volum;
                }else if(cBuf[0] == 'n'){
                    noteTimes = 1;
                    note = 0;
                    cBuf = cBuf.substr(1);
                    readMode = Note;
                }else if(cBuf[0] == 't'){
                    this->mBpm = 0;
                    cBuf = cBuf.substr(1);
                    readMode = Tempo;
                }else if(cBuf[0] == '('){
                    cBuf = cBuf.substr(1);
                    isChord = true;
                }else if(cBuf[0] == ')'){
                    isChord = false;
                    cBuf = cBuf.substr(1);
                    readMode = ChordEnd;
                }else if(checkCommand(cBuf, "//", &isForMatch)){
                    cBuf = cBuf.substr(strlen("//"));
                    readMode = Comment;
                }else if(checkCommand(cBuf, "/*", &isForMatch)){
                    cBuf = cBuf.substr(strlen("/*"));
                    readMode = Comments;
                }else if(cBuf[0] == ' ' || cBuf[0] == '\n' || cBuf[0] == '\t'){
                    cBuf = cBuf.substr(1);
                }else{
                    if(!isForMatch){
                        cerr << "error: '" << cBuf << "' is not defined." << endl;
                        return -1;
                    }
                }
            }break;
            case Scale:{
                if(isChord){
                    chordList.push_back(msBuf);
                }else{
                    float sndRatio = 0;
                    if(cBuf[0] == '~'){
                        sndRatio = SndLongRatio;
                        cBuf = cBuf.substr(1);
                    }else if(cBuf[0] == '^'){
                        sndRatio = SndShortRatio;
                        cBuf = cBuf.substr(1);
                    }else{
                        sndRatio = SndNormalRatio;
                    }
                    float fullSizeF = this->getBlockSize() / note * noteTimes + fraction;
                    int fullSize = (int)fullSizeF;
                    int sndSize = fullSize * sndRatio;
                    fraction = fullSizeF - fullSize;
                    if(isNoise){
                        genNoiseWave(sndSize, this->mMaxAmp * volum, {msBuf}, fp, this->mSampleRate);
                        volum = 1.0;
                        isNoise = false;
                    }else{
                        genSqWave(sndSize, this->mMaxAmp * volum, {msBuf}, fp, this->mSampleRate);
                        volum = 1.0;
                    }
                    for(int i = 0; i < fullSize - sndSize; i++){
                        char c = 0;
                        fwrite(&c, 1, 1, fp);
                    }
                }
                readMode = Normal;
            }break;
            case OctavePos:{
                if(isNum(cBuf[0])){
                    octave = (octave * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else if((int('a') <= int(cBuf[0]) && int(cBuf[0]) <= int('g'))
                ||       (cBuf[0] == '#')){
                    octave = 1 << (int)octave;
                    readMode = Normal;
                }else{
                    octave = 1;
                    readMode = Normal;
                }
            }break;
            case OctaveNeg:{
                if(isNum(cBuf[0])){
                    octave = (octave * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else if((int('a') <= int(cBuf[0]) && int(cBuf[0]) <= int('g'))
                ||       (cBuf[0] == '#')){
                    octave = 1.0 / (1 << (int)octave);
                    readMode = Normal;
                }else{
                    octave = 1;
                    readMode = Normal;
                }
            }break;
            case Volum:{
                if(isNum(cBuf[0])){
                    if(adpDigit > 0) {
                        adpDigit++;
                    }
                    volum = (volum * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else if(cBuf[0] == '.'){
                    if(adpDigit > 0) {
                        cerr << cBuf[0] << endl;
                        return -1;
                    }
                    adpDigit++;
                    cBuf = cBuf.substr(1);
                }else{
                    for(int i = 0; i < adpDigit - 1; i++) {
                        volum /= 10;
                    }
                    adpDigit = 0;
                    readMode = Normal;
                }
            }break;
            case Note:{
                if(isNum(cBuf[0])){
                    note = (note * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else if(cBuf[0] == '*'){
                    readMode = NoteTms;
                    noteTimes = 0;
                    cBuf = cBuf.substr(1);
                }else{
                    readMode = Normal;
                }
            }break;
            case NoteTms:{
                if(isNum(cBuf[0])){
                    noteTimes = (noteTimes * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else{
                    readMode = Note;
                }
            }break;
            case Tempo:{
                if(isNum(cBuf[0])){
                    if(adpDigit > 0) {
                        adpDigit++;
                    }
                    this->mBpm = (this->mBpm * 10) + int(cBuf[0]) - int('0');
                    cBuf = cBuf.substr(1);
                }else if(cBuf[0] == '.'){
                    if(adpDigit > 0) {
                        cerr << cBuf[0] << endl;
                        return -1;
                    }
                    adpDigit++;
                    cBuf = cBuf.substr(1);
                }else{
                    for(int i = 0; i < adpDigit - 1; i++) {
                        this->mBpm /= 10;
                    }
                    adpDigit = 0;
                    readMode = Normal;
                }
            }break;
            case ChordEnd:{
                float sndRatio = 0;
                if(cBuf[0] == '~'){
                    sndRatio = SndLongRatio;
                    cBuf = cBuf.substr(1);
                }else if(cBuf[0] == '^'){
                    sndRatio = SndShortRatio;
                    cBuf = cBuf.substr(1);
                }else{
                    sndRatio = SndNormalRatio;
                }
                float fullSizeF = this->getBlockSize() / note * noteTimes + fraction;
                int fullSize = (int)fullSizeF;
                int sndSize = fullSize * sndRatio;
                fraction = fullSizeF - fullSize;
                genSqWave(sndSize, this->mMaxAmp * volum, chordList, fp, this->mSampleRate);
                for(int i = 0; i < fullSize - sndSize; i++){
                    char c = 0;
                    fwrite(&c, 1, 1, fp);
                }
                octave = 1;
                volum = 1.0;
                chordList.clear();
                readMode = Normal;
            }break;
            case Comment:{
                if(cBuf[0] == '\n'){
                    readMode = Normal;
                }else{
                    cBuf = cBuf.substr(1);
                }
            }break;
            case Comments:{
                isForMatch = false;
                if(checkCommand(cBuf, "*/", &isForMatch)){
                    cBuf = cBuf.substr(strlen("*/"));
                    readMode = Normal;
                }else if(!isForMatch){
                    cBuf = cBuf.substr(1);
                }
            }
        }
    }
    ifs.close();

    return 0;
}
