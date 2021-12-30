#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "Cifa.h"
#include "Extern.h"
#include "YuFa.h"
#include "Error.h"
#include "midCode.h"
#include "Mips.h"
using namespace std;

vector<string> wordsType;
vector<string> words;
vector<string> ans;

int main() {
    // class declaration
    CiFa cifa;
    YuFa yufa;

    // input & output define & read the data
    ifstream infile;
    ofstream outfile;
    infile.open("testfile.txt");
    stringstream readBuffer;
    readBuffer << infile.rdbuf();
    string contents(readBuffer.str());
    outfile.open("output.txt");
    //end
       
    words.push_back("begin");
    wordsType.push_back("begin");
    lineNumberList.push_back(0);
    // do the Cifa analyse
    cifa.CiFaAnalyse(contents); //read together
    // Cifa analyse end

    words.push_back("finish1");
    wordsType.push_back("finish1");
    words.push_back("finish2");
    wordsType.push_back("finish2");

    // do the YuFa analyse
    yufa.ProgramAnalyse();
    ans = yufa.getAns();
    // YuFa analyse end

    // do the genMips:
    mipsGen();
    mipsOut();
    midCodeOut();
    //mipsEnd;

    for (int i = 0; i < ans.size(); i++) {
        outfile << ans[i] << endl;
    }
    outputError();
    infile.close();
    outfile.close();
    return 0;
}
