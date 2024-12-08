#include "FMMLCompiler.hpp"
#include "Wave.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <ctime>

using namespace std;

namespace{
    const float PI = 3.14159265358;
    const int AF = 880;
    const float SND_NORMAL_RATIO = (31 / 32.0); // sound ratio
    const float SND_LONG_RATIO = 1.0;
    const float SND_SHORT_RATIO = 0.5;
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
    enum ScaleType{C = 3, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B, R};

    typedef struct _MusicalScale {
        ScaleType s;
        float octave = 1;
    } MusicalScale;

    typedef struct _ReadingStatus {
        int note_times = 1;
        float octave = 1;
        float note = 4;
        float fraction = 0;
        float volum = 1.0;
        bool is_chord = false;
        bool is_signed = false;
        bool is_noise = false;
        bool is_for_match = false;
        int adpDigit = 0;
        ReadMode mode = Normal;
        string c_buffer;
        vector<MusicalScale> chord_list;
        MusicalScale ms_buffer;
    } ReadingStatus;

    // return wheter c is number
    bool isNum(char c){
        return static_cast<int>('0') <= static_cast<int>(c) && static_cast<int>(c) <= static_cast<int>('9');
    }

    // return whether c is musical scale
    bool isScale(char c) {
        return int('a') <= static_cast<int>(c) && static_cast<int>(c) <= int('g') || c == 'r';
    }

    int calcFrequency(MusicalScale ms) {
        int f = AF * pow(2, ms.s / 12.0) * ms.octave;

        return f;
    }

    bool checkCommand(string str, string command, bool* for_match){
        int min_len = str.length() < command.length() ? str.length() : command.length();
        if(str.substr(0, min_len) == command.substr(0, min_len)){
            *for_match = true;
        }

        if(str.length() >= command.length()){
            return str.substr(0, command.length()) == command;
        }

        return false;
    }

    void genSqWave(int length, char max_amp, vector<MusicalScale> ms_list, FILE* fp, int sample_rate){
        static int ms_last_counts[12] = { 0 };

        for(int i = 0; i < length; i++){
            char c;
            int tmp;
            float v = 0;

            for(MusicalScale ms : ms_list){
                if(ms.s != R){
                    int f = calcFrequency(ms);
                    v += ((i + ms_last_counts[ms.s - 3]) * f / sample_rate) & 1;
                }
            }
            v = v / ms_list.size() * 2;
            tmp = v * max_amp * ms_list.size();
            c = tmp > 0xFF ? 0xFF : (tmp < 0x00 ? 0x00 : tmp);
            fwrite(&c, 1, 1, fp);
        }

        memset(ms_last_counts, 0, sizeof(ms_last_counts));

        for(MusicalScale ms : ms_list) {
            ms_last_counts[ms.s - 3] = length % (sample_rate / calcFrequency(ms));
        }
    }

    void genSinWave(int length, char max_amp, vector<MusicalScale> ms_list, FILE* fp, int sample_rate){
        for(int i = 0; i < length; i++){
            char c;
            int tmp;
            float v = 0;

            for(MusicalScale ms : ms_list){
                if(ms.s != R){
                    float f = AF * pow(2, ms.s / 12.0) * ms.octave;
                    v += sin((i * f / sample_rate) * PI);
                }
            }
            v = v / ms_list.size() * 2;
            tmp = v * max_amp;
            c = tmp > 0xFF ? 0xFF : (tmp < 0x00 ? 0x00 : tmp);
            fwrite(&c, 1, 1, fp);
        }
    }

    void genNoiseWave(int length, char max_amp, MusicalScale ms, FILE* fp, int sample_rate){
        srand(time(NULL));
        int f;
        if(ms.s == R){
            f = sample_rate / length;
        }else{
            f = AF * pow(2, ms.s / 12.0) * ms.octave;
        }
        int v = 0;
        for(int i = 0; i < length; i++){
            if(i % (sample_rate / f) == 0){
                v = rand() & 1;
            }
            char c;
            c = v * max_amp;
            fwrite(&c, 1, 1, fp);
        }
    }

    int readNormal(ReadingStatus& status, float& bpm) {
        if(isScale(status.c_buffer[0])) {
            status.ms_buffer.octave = status.octave;
            status.octave = 1;
            if(status.is_signed){
                switch(status.c_buffer[0]){
                    case 'a': status.ms_buffer.s = As; break;
                    case 'c': status.ms_buffer.s = Cs; break;
                    case 'd': status.ms_buffer.s = Ds; break;
                    case 'f': status.ms_buffer.s = Fs; break;
                    case 'g': status.ms_buffer.s = Gs; break;
                }
                status.is_signed = false;
            }else{
                switch(status.c_buffer[0]){
                    case 'a': status.ms_buffer.s = A; break;
                    case 'b': status.ms_buffer.s = B; break;
                    case 'c': status.ms_buffer.s = C; break;
                    case 'd': status.ms_buffer.s = D; break;
                    case 'e': status.ms_buffer.s = E; break;
                    case 'f': status.ms_buffer.s = F; break;
                    case 'g': status.ms_buffer.s = G; break;
                    case 'r': status.ms_buffer.s = R; break;
                }
            }
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = Scale;
        }else if(status.c_buffer[0] == '#'){
            status.c_buffer = status.c_buffer.substr(1);
            status.is_signed = true;
        }else if(status.c_buffer[0] == 'z'){
            status.c_buffer = status.c_buffer.substr(1);
            status.is_noise = true;
        }else if(status.c_buffer[0] == '+'){
            status.octave = 0;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = OctavePos;
        }else if(status.c_buffer[0] == '-'){
            status.octave = 0;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = OctaveNeg;
        }else if(status.c_buffer[0] == 'v'){
            status.volum = 0.0;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = Volum;
        }else if(status.c_buffer[0] == 'n'){
            status.note_times = 1;
            status.note = 0;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = Note;
        }else if(status.c_buffer[0] == 't'){
            bpm = 0;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = Tempo;
        }else if(status.c_buffer[0] == '('){
            status.c_buffer = status.c_buffer.substr(1);
            status.is_chord = true;
        }else if(status.c_buffer[0] == ')'){
            status.is_chord = false;
            status.c_buffer = status.c_buffer.substr(1);
            status.mode = ChordEnd;
        }else if(checkCommand(status.c_buffer, "//", &status.is_for_match)){
            status.c_buffer = status.c_buffer.substr(strlen("//"));
            status.mode = Comment;
        }else if(checkCommand(status.c_buffer, "/*", &status.is_for_match)){
            status.c_buffer = status.c_buffer.substr(strlen("/*"));
            status.mode = Comments;
        }else if(status.c_buffer[0] == ' ' || status.c_buffer[0] == '\n' || status.c_buffer[0] == '\t'){
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            if(!status.is_for_match){
                cerr << "error: '" << status.c_buffer << "' is not defined." << endl;
                return -1;
            }
        }
        return 0;
    }

    int readScale(ReadingStatus& status, FILE* fp, int block_size, char max_amp, int sample_rate) {
        if(status.is_chord){
            status.chord_list.push_back(status.ms_buffer);
        }else{
            float snd_ratio = 0;
            if(status.c_buffer[0] == '~'){
                snd_ratio = SND_LONG_RATIO;
                status.c_buffer = status.c_buffer.substr(1);
            }else if(status.c_buffer[0] == '^'){
                snd_ratio = SND_SHORT_RATIO;
                status.c_buffer = status.c_buffer.substr(1);
            }else{
                snd_ratio = SND_NORMAL_RATIO;
            }
            float full_size_f = block_size / status.note * status.note_times + status.fraction;
            int full_size = (int)full_size_f;
            int snd_size = full_size * snd_ratio;
            status.fraction = full_size_f - full_size;
            if(status.is_noise){
                genNoiseWave(snd_size, max_amp * status.volum, {status.ms_buffer}, fp, sample_rate);
                status.volum = 1.0;
                status.is_noise = false;
            }else{
                genSqWave(snd_size, max_amp * status.volum, {status.ms_buffer}, fp, sample_rate);
                status.volum = 1.0;
            }
            for(int i = 0; i < full_size - snd_size; i++){
                char c = 0;
                fwrite(&c, 1, 1, fp);
            }
        }
        status.mode = Normal;
        return 0;
    }

    int readOctavePos(ReadingStatus& status) {
        if(isNum(status.c_buffer[0])){
            status.octave = (status.octave * 10) + int(status.c_buffer[0]) - int('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else if((int('a') <= int(status.c_buffer[0]) && int(status.c_buffer[0]) <= int('g'))
        ||       (status.c_buffer[0] == '#')){
            status.octave = 1 << static_cast<int>(status.octave);
            status.mode = Normal;
        }else{
            status.octave = 1;
            status.mode = Normal;
        }

        return 0;
    }

    int readOctaveNeg(ReadingStatus& status) {
        if(isNum(status.c_buffer[0])){
            status.octave = (status.octave * 10) + int(status.c_buffer[0]) - int('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else if((int('a') <= int(status.c_buffer[0]) && int(status.c_buffer[0]) <= int('g'))
        ||       (status.c_buffer[0] == '#')){
            status.octave = 1.0 / (1 << static_cast<int>(status.octave));
            status.mode = Normal;
        }else{
            status.octave = 1;
            status.mode = Normal;
        }

        return 0;
    }

    int readVolum(ReadingStatus& status) {
        if(isNum(status.c_buffer[0])){
            if(status.adpDigit > 0) {
                status.adpDigit++;
            }
            status.volum = (status.volum * 10) + int(status.c_buffer[0]) - int('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else if(status.c_buffer[0] == '.'){
            if(status.adpDigit > 0) {
                cerr << status.c_buffer[0] << endl;
                return -1;
            }
            status.adpDigit++;
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            for(int i = 0; i < status.adpDigit - 1; i++) {
                status.volum /= 10;
            }
            status.adpDigit = 0;
            status.mode = Normal;
        }

        return 0;
    }

    int readNote(ReadingStatus& status) {
        if(isNum(status.c_buffer[0])){
            status.note = (status.note * 10) + int(status.c_buffer[0]) - int('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else if(status.c_buffer[0] == '*'){
            status.mode = NoteTms;
            status.note_times = 0;
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            status.mode = Normal;
        }

        return 0;
    }

    int readNoteTimes(ReadingStatus& status) {
        if(isNum(status.c_buffer[0])){
            status.note_times = (status.note_times * 10) + int(status.c_buffer[0]) - int('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            status.mode = Note;
        }

        return 0;
    }
    int readTempo(ReadingStatus& status, float& bpm) {
        if(isNum(status.c_buffer[0])){
            if(status.adpDigit > 0) {
                status.adpDigit++;
            }
            bpm = (bpm * 10) + static_cast<int>(status.c_buffer[0]) - static_cast<int>('0');
            status.c_buffer = status.c_buffer.substr(1);
        }else if(status.c_buffer[0] == '.'){
            if(status.adpDigit > 0) {
                cerr << status.c_buffer[0] << endl;
                return -1;
            }
            status.adpDigit++;
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            for(int i = 0; i < status.adpDigit - 1; i++) {
                bpm /= 10;
            }
            status.adpDigit = 0;
            status.mode = Normal;
        }

        return 0;
    }

    int readChordEnd(ReadingStatus& status, FILE* fp, int block_size, int max_amp, int sample_rate) {
        float snd_ratio = 0;
        if(status.c_buffer[0] == '~'){
            snd_ratio = SND_LONG_RATIO;
            status.c_buffer = status.c_buffer.substr(1);
        }else if(status.c_buffer[0] == '^'){
            snd_ratio = SND_SHORT_RATIO;
            status.c_buffer = status.c_buffer.substr(1);
        }else{
            snd_ratio = SND_NORMAL_RATIO;
        }
        float full_size_f = block_size / status.note * status.note_times + status.fraction;
        int full_size = (int)full_size_f;
        int snd_size = full_size * snd_ratio;
        status.fraction = full_size_f - full_size;
        genSqWave(snd_size, max_amp * status.volum, status.chord_list, fp, sample_rate);
        for(int i = 0; i < full_size - snd_size; i++){
            char c = 0;
            fwrite(&c, 1, 1, fp);
        }
        status.octave = 1;
        status.volum = 1.0;
        status.chord_list.clear();
        status.mode = Normal;

        return 0;
    }

    int readComment(ReadingStatus& status) {
        if(status.c_buffer[0] == '\n'){
            status.mode = Normal;
        }else{
            status.c_buffer = status.c_buffer.substr(1);
        }

        return 0;
    }

    int readComments(ReadingStatus& status) {
        status.is_for_match = false;
        if(checkCommand(status.c_buffer, "*/", &status.is_for_match)){
            status.c_buffer = status.c_buffer.substr(strlen("*/"));
            status.mode = Normal;
        }else if(!status.is_for_match){
            status.c_buffer = status.c_buffer.substr(1);
        }

        return 0;
    }
}

double FMMLCompiler::getTimePerBeat(){
    return 240.0 / this->m_bpm;
}

int FMMLCompiler::getBlockSize(){
    return this->m_sample_rate * this->getTimePerBeat();
}

int FMMLCompiler::compile(string filename){
    FILE* fp = NULL;
    errno_t e;

    e = fopen_s(&fp, ".tmp", "w+b");
    if(e != 0 || fp == NULL) {
        cerr << "error code (" << e << "): failed to create temporary file" << endl;
        return -1;
    }

    ifstream ifs(filename);
    ReadingStatus status;

    if(ifs.fail()){
        cerr << "Failed to open file: " << filename << endl;
        return -1;
    }

    while(1){
        status.is_for_match = false;
        char c;
        int r;
        ifs.read(&c, 1);

        if(!ifs.eof()){
            status.c_buffer.push_back(c);
        }else if(status.c_buffer.size() <= 0 && status.mode == Normal){
            break;
        }

        switch(status.mode){
            case Normal:{
                r = readNormal(status, this->m_bpm);
            }break;
            case Scale:{
                r = readScale(status, fp, this->getBlockSize(), this->m_max_amp, this->m_sample_rate);
            }break;
            case OctavePos:{
                r = readOctavePos(status);
            }break;
            case OctaveNeg:{
                r = readOctaveNeg(status);
            }break;
            case Volum:{
                r = readVolum(status);
            }break;
            case Note:{
                r = readNote(status);
            }break;
            case NoteTms:{
                r = readNoteTimes(status);
            }break;
            case Tempo:{
                r = readTempo(status, this->m_bpm);
            }break;
            case ChordEnd:{
                r = readChordEnd(status, fp, this->getBlockSize(), this->m_max_amp, this->m_sample_rate);
            }break;
            case Comment:{
                r = readComment(status);
            }break;
            case Comments:{
                r = readComments(status);
            }
        }

        if(r < 0) {
            return -1;
        }
    }
    ifs.close();

    Wave wav;
    FmtData fmtData;

    fmtData.ckSize = FMT_CKSIZE_16;
    fmtData.wFormatTag = WAVE_FORMAT_PCM;
    fmtData.nChannels = 1;
    fmtData.nSamplesPerSec = this->m_sample_rate;
    fmtData.nAvgBytesPerSec = this->m_sample_rate;
    fmtData.nBlockAlign = 1;
    fmtData.wBitsPerSample = 8;
    wav.setFmtData(fmtData);
    wav.writeWaveDataFile("out.wav", fp);

    fclose(fp);
    remove(".tmp");

    return 0;
}
