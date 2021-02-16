#include <string>
#include<vector>
#include "Extern.h"
using namespace std;

#ifndef SymItem_H //���ű���
#define SymItem_H

class SymItem { //
public:
    string name; //���� 
    int kind; //��ʶ������
    // 1 const 2 var 3 function 4 array1ά(����var) 5 array2ά 6 string
    int type; //������������
    // 1 void 2 int 3 char
    int length; //���鳤�ȣ�������������������ΪFIAL ֻ��¼��ά����ĵڶ�ά����
    int intValue; // int�͵�ֵ Ĭ��ΪFAIL
    char charValue; //char�͵�ֵ��Ĭ��Ϊ' '
    int addr; // �����׵�ַƫ��������ʶ�����������AR�׵�ַƫ����
    string STRING; //�ַ����е�����
    vector<int> paraList; //��������

    SymItem(string inName = " ", int inKind = 0, int inType = 0,int inAddr = 0, int inIntValue = FAIL, char inCharValue = ' ',
        int inLength = FAIL) :
        name(inName), kind(inKind), type(inType),addr(inAddr), intValue(inIntValue), charValue(inCharValue), length(inLength) {
    }

    void insert(int t);
    /*SymItem(){
    }*/
};

#endif