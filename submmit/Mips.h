#include <string>
#include<vector>
using namespace std;

#ifndef Mips
#define Mips
enum  mipsOp{
	add,
	addi,
	moveop,
	sub,
	subi,
	mult,
	mul,
	dive,
	beq,
	bne,
	blez,
	blt,
	ble,
	bge,
	bgtz,
	bgt,
	bgez,
	bltz,
	sll,
	mflo,
	mfhi,
	j,
	jal,
	jr,
	lw,
	sw,
	syscall,
	dataSet,
	textSet,
	strSet,
	globalSet,
	la,
	li,
	label //产生标号
};
class mips {
public:
	mipsOp op;
	string result;//结果
	string x;// 左操作数
	string y;//右操作数
	int imm;//立即数
	mips(mipsOp inOp,string inResult, string inX,string inY,int inImm):op(inOp),result(inResult),x(inX),y(inY),imm(inImm){ }
};
void mipsGen();
void mipsOut();
void midCodeOut();

#endif