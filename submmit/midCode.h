#include <string>
#include<vector>
using namespace std;

#ifndef CIFA_H
#define CIFA_H

enum operation { // 用ENUM来建立而不是"" 
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
	BNZ,//正确跳转
	BZ,//不正确跳转
	ASSIGN,  //= 赋值
	GOTO,  //无条件跳转
	SCANF,  //读
	PRINTF, //写
	LABEL, //标记
	CONST, //常量
	ARRAY, //数组
	VAR,   //变量
	FUNC,  //函数定义
	PARAM, //函数参数
	GETARRAY,//从ARRAY中拿出元素
	PUTARRAY,//放进ARRAY中元素
	RET,
	CALL, //函数调用语句
	PARA, //函数参数表
	USE, //函数调用参数
	RETVALUE,//调用有返回值函数语句时返回的值
	SWITCH,//声明是SWITCH语句
	CASE, //case语句
	EXIT, //退出语句
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