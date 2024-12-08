#pragma once

#include "Fmt.hpp"
#include "Fact.hpp"
#include "Data.hpp"
#include "Info.hpp"

#include <string>

using namespace std;

class Wave{
public:
    void writeWave(string filename);
    void writeWaveDataFile(string filename, FILE* fp);
    void setFmtData(FmtData& fmtData);
    void setData(int size, char* data);
private:
    Fmt mFmt;
    //Fact fact;
    Data mData;
    //Info info;
};