#include <string>
#include<vector>
using namespace std;

#ifndef CIFA_H
#define CIFA_H

enum operation { // ��ENUM������������"" 
	PLUS, //+
	MINUS, //-
	MULT, //*
	DIV,  // /
	LSS,  //<
	LEQ,  //<=
	GRE,  //>
	GEQ,  //>=
	EQL,  //==
	NEQ,  //!=
	BNZ,//��ȷ��ת
	BZ,//����ȷ��ת
	ASSIGN,  //= ��ֵ
	GOTO,  //��������ת
	SCANF,  //��
	PRINTF, //д
	LABEL, //���
	CONST, //����
	ARRAY, //����
	VAR,   //����
	FUNC,  //��������
	PARAM, //��������
	GETARRAY,//��ARRAY���ó�Ԫ��
	PUTARRAY,//�Ž�ARRAY��Ԫ��
	RET,
	CALL, //�����������
	PARA, //����������
	USE, //�������ò���
	RETVALUE,//�����з���ֵ�������ʱ���ص�ֵ
	SWITCH,//������SWITCH���
	CASE, //case���
	EXIT, //�˳����
};

class midCode {
public:
	operation op;
	string result;
	string x;
	string y;
	midCode(operation opIn,  string xIn, string yIn, string resultIn ) :op(opIn),  x(xIn), y(yIn), result(resultIn) {}
};
#endif