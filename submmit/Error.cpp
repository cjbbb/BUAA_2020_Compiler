#include <fstream>
#include <iostream>
#include <vector>
#include<string>
#include <fstream>
#include <sstream>
#include "Extern.h"
#include "Error.h"

using namespace std;

vector<string> ErrorList;
char ErrorArray[10000] = { '0' };

void Error(char index, int lineNumber) { //i为错误的下标
    ErrorArray[lineNumber] = index;

    if (index == 'a') { //词法错误
        cout << "a type wrong occure: " << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "a");
    }
    else if (index == 'b') { //名字定义重名
        cout << "b type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "b");
    }
    else if (index == 'c') { //引用未定义名字
        cout << "c type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "c");
    }
    else if (index == 'd') { //函数参数个数不匹配
        cout << "d type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "d");
    }
    else if (index == 'e') { //函数参数类型不匹配
        cout << "e type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "e");
    }
    else if (index == 'f') { //if 语句左右两边只能为整形;
        cout << "f type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "f");
    }
    else if (index == 'g') { //数组元素的下标不为整数型
        cout << "g type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "g");
    }
    else if (index == 'h') { //数组元素的下标不为整数型
        cout << "h type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "h");
    }
    else if (index == 'i') { //数组元素的下标不为整数型
        cout << "i type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "i");
    }
    else if (index == 'j') { //给CONST赋值;
        cout << "j type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "j");
    }
    else if (index == 'k') { //缺少;
        cout << "k type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "k");
    }
    else if (index == 'l') { //缺少)
        cout << "l type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "l");
    }
    else if (index == 'm') { //缺少]
        cout << "m type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "m");
    }
    else if (index == 'n') { //数组初始化个数不匹配
        cout << "n type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "n");
    }
    else if (index == 'o') { //数组元素的下标不为整数型
        cout << "o type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "o");
    }
    else if (index == 'p') { //数组元素的下标不为整数型
        cout << "p type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "p");
    }
}

void outputError() {
    ofstream errorFile;
    errorFile.open("error.txt");
    int lineNumber = lineNumberList.back() + 1;
    for (int i = 0; i < lineNumber; i++) {
        if (ErrorArray[i] <= 'p' && ErrorArray[i] >= 'a') {
            errorFile << i << ' ' << ErrorArray[i] << endl;
        }
    }
    errorFile.close();

}