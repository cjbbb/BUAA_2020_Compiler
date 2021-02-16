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
	label //�������
};
class mips {
public:
	mipsOp op;
	string result;//���
	string x;// �������
	string y;//�Ҳ�����
	int imm;//������
	mips(mipsOp inOp,string inResult, string inX,string inY,int inImm):op(inOp),result(inResult),x(inX),y(inY),imm(inImm){ }
};
void mipsGen();
void mipsOut();
void midCodeOut();

#endif