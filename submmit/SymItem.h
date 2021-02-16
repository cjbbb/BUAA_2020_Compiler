#include <string>
#include<vector>
#include "Extern.h"
using namespace std;

#ifndef SymItem_H //符号表项
#define SymItem_H

class SymItem { //
public:
    string name; //名字 
    int kind; //标识符类型
    // 1 const 2 var 3 function 4 array1维(都是var) 5 array2维 6 string
    int type; //函数返回类型
    // 1 void 2 int 3 char
    int length; //数组长度，函数参数个数，其余为FIAL 只记录二维数组的第二维长度
    int intValue; // int型的值 默认为FAIL
    char charValue; //char型的值，默认为' '
    int addr; // 数组首地址偏移量。标识符相对于所在AR首地址偏移量
    string STRING; //字符串中的内容
    vector<int> paraList; //参数类型

    SymItem(string inName = " ", int inKind = 0, int inType = 0,int inAddr = 0, int inIntValue = FAIL, char inCharValue = ' ',
        int inLength = FAIL) :
        name(inName), kind(inKind), type(inType),addr(inAddr), intValue(inIntValue), charValue(inCharValue), length(inLength) {
    }

    void insert(int t);
    /*SymItem(){
    }*/
};

#endif