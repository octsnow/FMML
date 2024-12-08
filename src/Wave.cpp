#include "Wave.hpp"

#include <iostream>
#include <fstream>

using namespace std;

namespace{
    void writeChunk(ofstream& ofs, Chunk& chunk){
        ofs.write((char*)&chunk.mCkID, 4);
        ofs.write((char*)&chunk.mCkSize, 4);
        for(int i = 0; i < chunk.mCkSize; i++){
            ofs.write(chunk.mData + i, 1);
        }
    }
}

void Wave::writeWave(string filename){
    ofstream ofs = ofstream(filename, ios::binary);
    if(!ofs){
        return;
    }

    int riffCkID   = 0x46464952;
    int riffCkSize = 20 + this->mFmt.mCkSize + this->mData.mCkSize;
    int riffData   = 0x45564157;

    ofs.write((char*)&riffCkID,     4);
    ofs.write((char*)&riffCkSize,   4);
    ofs.write((char*)&riffData,     4);
    writeChunk(ofs, this->mFmt);
    writeChunk(ofs, this->mData);

    ofs.close();
}

void Wave::writeWaveDataFile(string filename, FILE* fp){
    ofstream ofs = ofstream(filename, ios::binary);
    if(!ofs){
        return;
    }

    unsigned int dataSize = 0;

    if(fseek(fp, 0L, SEEK_END) == 0){
        fpos_t pos;
        if(fgetpos(fp, &pos) == 0){
            dataSize = (unsigned int)pos;
        }
        
    }
    fseek(fp, 0L, SEEK_SET);

    cout << "size: " << dataSize << endl;

    int riffCkID   = 0x46464952;
    unsigned int riffCkSize = 8 + this->mFmt.mCkSize + 8 + dataSize + 8;
    int riffData   = 0x45564157;
    int dataID     = 0x61746164;

    ofs.write((char*)&riffCkID,     4);
    ofs.write((char*)&riffCkSize,   4);
    ofs.write((char*)&riffData,     4);
    writeChunk(ofs, this->mFmt);
    
    ofs.write((char*)&dataID, 4);
    ofs.write((char*)&dataSize, 4);

    char c = 0;
    while(fread(&c, 1, 1, fp) == 1){
        ofs.write(&c, 1);
    }

    ofs.close();
}

void Wave::setFmtData(FmtData& fmtData){
    this->mFmt.setFmtData(fmtData);
}

void Wave::setData(int size, char* data){
    this->mData.setData(size, data);
}
