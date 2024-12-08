#include <iostream>
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
    FMMLCompiler comp(SAMPLE_RATE);
    comp.compile(argv[1]);
    
    return 0;
}
