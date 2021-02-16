#include <string>
#include<vector>
#include "Extern.h"
using namespace std;

#ifndef ERROR_H
#define ERROR_H

extern vector<string>ErrorList; //后续在main里打印

void outputError();
void Error(char index, int lineNumber);

#endif