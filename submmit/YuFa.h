#include <string>
#include<vector>
#include "Extern.h"
#include "SymItem.h"

using namespace std;

#ifndef YUFA_H
#define YUFA_H

class YuFa {
public:
	
	vector<string> ans;
	void ProgramAnalyse(); 
	vector<string> getAns();

private:
	bool findString(string in, vector<string> from);//查找字符是否出现
	// *********** 自己用的
	string preRead(); //预读下一个词
	void NextWord();
	void BackWord();
	void Recover(int in); // 并行时回复指针
	void Getword(int in); //获取[in]单词
	string getMin(string in); //获取小写字符串
	SymItem judgeIdenfr(string in);
	bool changeLine();
	void VarIdenfrInsert(string name, int inKind, int inType, int inIntValue, char inCharValue, int index,int addr,int length); //帮助变量加入符号表
	void VariableDefinationWithInitOError(int tempType); //提取的函数
	void ErrorKLM(char type);
	// ********************// 
	int RelationSymbol(); //关系运算符
	int MultiSymbol(int init); //乘法运算符
	int AddSymbol(int init); //加法运算符。
	// *******************//
	int Term(int init,int isPara,string &value); //项
	int Factor(int init,int isPara,string &value); //因子
	int ParameterList(int init, string funcName); // 参数表
	int headAnnounce();// 头部声明
	// *******************//
	int Sentence(int funcType); //语句
	int SentenceList(int init,int funcType); // 语句列
	int MultiSentence(int init,int funcType); //复合语句
	int LoopSentence(int init); //循环语句
	int ConditionSentence(int init,int funcType); //条件语句
	int ReturnSentence(int init); //有返回值函数调用语句。
	int VoidSentence(int init);//无返回值函数调用语句。
	int AssignSentence(int init); //赋值语句。
	int ReadSentence(int init); //读语句。
	int WriteSentence(int init); //写语句
	int CaseSentence(int init); //情况语句
	int BackSentence(int init,int funcType); //返回语句
	int Default(int init); // 缺省
	int CaseTable(int init,int type); //情况表;
	int CaseSonSentence(int init,int type); //情况子语句
	// *******************//
	int StepLength(int init); //步长
	int ValueList(int init,string funcName); //<值参数表>
	int Condition(int init,string &result); //条件
	int Expression(int init,int isPara,string &value); //表达式 是否是参数
	string String(); //<字符串>
	int Integer(); //整数;
	int Const(); //常量
	int NunInteger();//无符号整数
	int ConstState(int index); //常量说明
	void VariableState(int index); //变量说明
	void ConstDefination(int index); //常量定义
	int VariableDefination(int index); //变量定义
	int ReturnFunctionDefination(int init); //有返回值函数定义。
	int VoidFunctionDefination(int init); //有返回值函数定义。
	int VariableDefinationWithoutInit(int init,int index); //变量定义无初始化
	int VariableDefinationWithInit(int init,int index); //变量定义以及初始化
	int MainFunction(int init); //主函数

	void Out(string in);
};	
string int2string(int in);

int string2int(string in);
#endif