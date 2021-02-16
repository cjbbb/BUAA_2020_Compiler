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

void Error(char index, int lineNumber) { //iΪ������±�
    ErrorArray[lineNumber] = index;

    if (index == 'a') { //�ʷ�����
        cout << "a type wrong occure: " << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "a");
    }
    else if (index == 'b') { //���ֶ�������
        cout << "b type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "b");
    }
    else if (index == 'c') { //����δ��������
        cout << "c type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "c");
    }
    else if (index == 'd') { //��������������ƥ��
        cout << "d type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "d");
    }
    else if (index == 'e') { //�����������Ͳ�ƥ��
        cout << "e type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "e");
    }
    else if (index == 'f') { //if �����������ֻ��Ϊ����;
        cout << "f type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "f");
    }
    else if (index == 'g') { //����Ԫ�ص��±겻Ϊ������
        cout << "g type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "g");
    }
    else if (index == 'h') { //����Ԫ�ص��±겻Ϊ������
        cout << "h type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "h");
    }
    else if (index == 'i') { //����Ԫ�ص��±겻Ϊ������
        cout << "i type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "i");
    }
    else if (index == 'j') { //��CONST��ֵ;
        cout << "j type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "j");
    }
    else if (index == 'k') { //ȱ��;
        cout << "k type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "k");
    }
    else if (index == 'l') { //ȱ��)
        cout << "l type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "l");
    }
    else if (index == 'm') { //ȱ��]
        cout << "m type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "m");
    }
    else if (index == 'n') { //�����ʼ��������ƥ��
        cout << "n type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "n");
    }
    else if (index == 'o') { //����Ԫ�ص��±겻Ϊ������
        cout << "o type wrong occure:" << lineNumber << endl;
        ErrorList.push_back(to_string(lineNumber) + " " + "o");
    }
    else if (index == 'p') { //����Ԫ�ص��±겻Ϊ������
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