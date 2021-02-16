#include <fstream>
#include <iostream>
#include <string>
#include<stack>
#include<vector>
#include "Extern.h"
#include<algorithm>
#include "YuFa.h"
#include "Mips.h"
using namespace std;
//�ⲿ����
extern map <string, SymItem> globalSymTable; //ȫ�ֱ�
extern map<string, map<string, SymItem>> allTempSymTable;//�������е���ʱ��
extern vector<string> funcList; //funcName��list
extern vector<string> strList;//str ��¼
extern vector<midCode> midCodeTable;//str ��¼
//�ڲ�����
stack <midCode> funcParaList; //���ú���ʱ��¼��ֵ��
vector<midCode> switchList; //��¼case����
string tempFuncName = "";//������ʱ�ı�����
vector<mips> mipsTable; //mips ָ��ļ�¼
map <string, SymItem> tmpSymTable; //��allTempSymTable���ó�����ʱ�� ע��Ϊtmp������temp
bool isReturn = false;
string tmpRegUse[10] = { " "," "," "," "," "," "," "," "," ", " ", };//t5,t6,t7,t8,t9������Ϊ����
string allRegUse[10] = { " "," "," "," "," "," "," "," "," ", " ", };//S0-S1-S2-S3-..-S7

typedef struct var {
	string name;
		int time;
}var;
bool cmp(var a, var b) {
	if (a.time > b.time) {
		return true;
	}
	else {
		return false;
	}
}
bool isVar(string in) {
	if (tmpSymTable.find(in) != tmpSymTable.end()) {
		if (in[0] == '@') { //�������ʱ������false
			return false;
		}
		if (tmpSymTable[in].kind == 2) {
			return true;
		}
	}
	return false;
}
void updateAllReg(int i) {
	map<string , SymItem>::iterator iter;
	vector<var> tmpVector;
	while(i<midCodeTable.size()){
		if (midCodeTable[i].op == FUNC) {
			break;
		}
		midCode order = midCodeTable[i];
		if (isVar(order.result)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == order.result) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (find == false){
				var tmp;
				tmp.time = 1, tmp.name = order.result;
				tmpVector.push_back(tmp);
			}
		}
		if (isVar(order.x)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == order.x) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (find == false) {
				var tmp;
				tmp.time = 1, tmp.name = order.x;
				tmpVector.push_back(tmp);
			}
		}
		if (isVar(order.y)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == order.y) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (find == false) {
				var tmp;
				tmp.time = 1, tmp.name = order.y;
				tmpVector.push_back(tmp);
			}
		}
		i++;
	}
	sort(tmpVector.begin(), tmpVector.end(), cmp);
	for (int i = 0; i < tmpVector.size(); i++) {
		if (i <= 7) {
			allRegUse[i] = tmpVector[i].name;
		}
		else {
			break;
		}
	}

}

int findEmptyTmpReg() {
	for (int i = 5; i <= 9; i++) {
		if (tmpRegUse[i] == " ") {
			return i;
		}
	}
	return -1;//û�ҵ�
}
string findUseAllReg(string name) { //�ҵ�����ʹ�õļĴ���
	for (int i = 0; i <= 7; i++) {
		if (allRegUse[i] == name) {
			return "$s"+int2string(i);
		}
	}
	return " ";
}
int findUseTmpReg(string name) { //�ҵ�����ʹ�õļĴ���
	for (int i = 5; i <= 9; i++) {
		if (tmpRegUse[i] == name) {
			return i;
		}
	}
	return FAIL;
}

string insertTmpReg(string in) {
	int find = findEmptyTmpReg();
	if (find != -1) {
		tmpRegUse[find] = in;
		return "$t" + int2string(find);
	}
	return " ";//ʧ����
}

vector<string>  split(const string& str, const string& delim) {
	vector<string> res;
	if ("" == str) return  res;

	string strs = str + delim;
	size_t pos;
	size_t size = strs.size();

	for (int i = 0; i < size; ++i) {
		pos = strs.find(delim, i);
		if (pos < size) {
			string s = strs.substr(i, pos - i);
			res.push_back(s);
			i = pos + delim.size() - 1;
		}

	}
	return res;
}


void storeValue(string& name, string& reg) { //�洢ֵ name�Ǳ�����
	int addr;
	if (tmpSymTable.find(name) != tmpSymTable.end()) { //��ʱ��������
		addr = tmpSymTable[name].addr; //������ַ
		mipsTable.push_back(mips(sw, reg, "$sp", " ", 4 * addr));// 
	}
	else if (globalSymTable.find(name) != globalSymTable.end()) {
		addr = globalSymTable[name].addr;
		mipsTable.push_back(mips(sw, reg, "$gp", "", 4 * addr));
	}
}


void loadValue(string& name, string& reg, int& va, bool need) { //��name��ֵ����reg 
	// need �� �����const �Ƿ�����liָ��
	int addr;
	SymItem sym;
	int value; //����ǿ������ת��
	if (tmpSymTable.find(name) != tmpSymTable.end()) { //��ʱ��������
		sym = tmpSymTable[name];
		if (sym.kind == 1) { //�����const
			if (sym.type == 2) { //��int
				value = sym.intValue;
			}
			else if (sym.type == 3) { //��char
				value = int(sym.charValue);
			}
			va = value;
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", value));
			}
		}
		else { //�����var
			int find = findUseTmpReg(name);
			if (find != FAIL) { //�ҵ���
				reg = "$t" + int2string(find);
				tmpRegUse[find] = " ";//�ƿ�
			}
			else {
				string sfind = findUseAllReg(name);
				if (sfind != " ") {
					reg = sfind;
				}
				else {
					addr = sym.addr;
					mipsTable.push_back(mips(lw, reg, "$sp", "", 4 * addr));
				}
			}
		}
	}
	else if (globalSymTable.find(name) != globalSymTable.end()) { //��ȫ�ֱ���
		sym = globalSymTable[name];
		if (sym.kind == 1) { //�����const
			if (sym.type == 2) { //��int
				value = sym.intValue;
			}
			else if (sym.type == 3) { //��char
				value = sym.charValue;
			}
			va = value;
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", value));
			}
		}
		else { //�����var
			addr = sym.addr;
			mipsTable.push_back(mips(lw, reg, "$gp", "", 4 * addr));
		}
	}
	else { //��������
		if (name.size() > 0) {
			va = string2int(name);
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", string2int(name)));
			}
		}
	}
}
string getStrIndex(string in) { //��ȡ�ַ�����strList�е��±�
	for (int i = 0; i < strList.size(); i++) {
		if (strList[i] == in) {
			return "str" + int2string(i);
		}
	}
}
void mipsGen() { //����mips

	mipsTable.push_back(mips(dataSet, "", "", "", 0));//����data��ַ
	for (int i = 0; i < strList.size(); i++) {  //��.ascii����
		mipsTable.push_back(mips(strSet, "str" + int2string(i), strList[i], "", 0));//x��ֵ��result������
	}
	mipsTable.push_back(mips(strSet, "nextLine", "\\n", "", 0));//��һ�еı�־
	mipsTable.push_back(mips(textSet, "", "", "", 0));//��text��־
	for (int midIndex = 0; midIndex < midCodeTable.size(); midIndex++) {
		int va = FAIL, addr = 0, va1 = FAIL, va2 = FAIL,va3= FAIL;//init var
		midCode order = midCodeTable[midIndex];
		switch (order.op)
		{
		case PRINTF: {
			if (order.x == "4") { //�ַ���
				string strName = getStrIndex(order.result);
				mipsTable.push_back(mips(la, "$a0", strName, "", 0));
				mipsTable.push_back(mips(li, "$v0", "", "", 4));
				mipsTable.push_back(mips(syscall, "", "", "", 0));
			}
			else if (order.x == "5") { //���з�
				mipsTable.push_back(mips(la, "$a0", "nextLine", "", 0));
				mipsTable.push_back(mips(li, "$v0", "", "", 4));
				mipsTable.push_back(mips(syscall, "", "", "", 0));
			}
			else { //���ʽ,int / char
				string a0 = "$a0";
				loadValue(order.result, a0, va, true);
				if (a0 != "$a0") { //���result�������Ĵ���
					mipsTable.push_back(mips(moveop, "$a0", a0, "", 0));
				}
				if (order.x == "2") { //int
					mipsTable.push_back(mips(li, "$v0", "", "", 1));
				}
				else {
					mipsTable.push_back(mips(li, "$v0", "", "", 11));
				}
				mipsTable.push_back(mips(syscall, "", "", "", 0));
			}
			break;
		}
		case SCANF: {
			SymItem scan;
			string sfind = findUseAllReg(order.result);
			if (sfind != " ") {
				scan = tmpSymTable[order.result];
				mipsTable.push_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
				mipsTable.push_back(mips(syscall, "", "", "", 0));
				mipsTable.push_back(mips(moveop, sfind, "$v0", "", 0));
			}
			else {
				if (tmpSymTable.find(order.result) != tmpSymTable.end()) { //����ʱ������
					scan = tmpSymTable[order.result];
					mipsTable.push_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
					mipsTable.push_back(mips(syscall, "", "", "", 0));
					addr = tmpSymTable[order.result].addr;
					mipsTable.push_back(mips(sw, "$v0", "$sp", "", 4 * addr));
				}
				else if (globalSymTable.find(order.result) != globalSymTable.end()) {
					scan = globalSymTable[order.result];
					mipsTable.push_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
					mipsTable.push_back(mips(syscall, "", "", "", 0));
					addr = globalSymTable[order.result].addr;
					mipsTable.push_back(mips(sw, "$v0", "$gp", "", 4 * addr));
				}
			}
			break;
		}

		case MINUS: { // result = x - y
			string x = "$t0", y = "$t1", result = "$t2";
			string find = " ";
			if (order.result[0] == '@') {
				find = insertTmpReg(order.result);
				if (find != " ") {
					result = find;
				}
			}
			string sfind = findUseAllReg(order.result);
			if (sfind != " ") {
				result = sfind;
			}
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			if (va1 != FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(li, result, "", "", va1-va2));
			} 
			else if (va1 != FAIL && va2 == FAIL) { //�ҵ���Va1
				mipsTable.push_back(mips(addi, result, y, "", -va1));// result = y - x
				mipsTable.push_back(mips(sub, result, "$0", result,0)); // result = - result
			}
			else if (va1 == FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(addi, result,x, "", -va2)); // result =x - y
			}
			else {
				mipsTable.push_back(mips(sub, result, x, y, 0));
			}
			if (order.result[0] == '@') { //����ʱ������û�ҵ�
				if (find == " ") {
					storeValue(order.result, result);
				}
			}
			else {
				if (sfind == " ") {
					storeValue(order.result, result);
				}
			}
			break;
		}

		case RETVALUE: { // tempVar = result;
			if (tmpSymTable.find(order.result) != tmpSymTable.end()) { //��ʱ��������
				if (order.result[0] == '@') { //����ʱ�Ĵ���
					string find = insertTmpReg(order.result);
					if (find != " ") {
						mipsTable.push_back(mips(moveop, find, "$v0", "", 0));//��v0��ֵ������ʱ����
					}
				}
				string sfind = findUseAllReg(order.result);
				if (sfind != " ") { //��ȫ�ּĴ���
					mipsTable.push_back(mips(moveop, sfind, "$v0", "", 0));//��v0��ֵ������ʱ����
				}
				else {
					SymItem sym = tmpSymTable[order.result];
					int addr = sym.addr;
					mipsTable.push_back(mips(sw, "$v0", "$sp", "", 4 * addr));//��v0��ֵ������ʱ����
				}
			}
			break;
		}
		case PLUS: { //result = x - y
			string x = "$t0", y = "$t1", result = "$t2";
			string find = " ";
			if (order.result[0] == '@') {
				find = insertTmpReg(order.result);
				if (find != " ") {
					result = find;
				}
			}
			string sfind = findUseAllReg(order.result);
			if (sfind != " ") {
				result = sfind;
			}
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			if (va1 != FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(li, result, "", "", va1 + va2));
			}
			else if (va1 != FAIL && va2 == FAIL) { //ֻ��xû��y
				mipsTable.push_back(mips(addi, result, y, "", va1 ));
			}
			else if (va1 == FAIL && va2 != FAIL) { //ֻ��yû��x
				mipsTable.push_back(mips(addi, result, x, "", va2));
			}
			else {
				mipsTable.push_back(mips(add, result, x, y, 0));
			}
			if (order.result[0] == '@') { //����ʱ������û�ҵ�
				if (find == " ") {
					storeValue(order.result, result);
				}
			}
			else {
				if (sfind == " ") {
					storeValue(order.result, result);
				}
			}
			break;
		}
		case ASSIGN: { // resut = x;
			string name = order.result;
			if (order.result[0] == '@') { //���м����
				string find = insertTmpReg(order.result);
				if (find != " ") { //�п���
					string t0 = find;
					loadValue(order.x, t0, va, true);//ֱ�Ӵ浽t0��
					if (t0 !=  find) {
						mipsTable.push_back(mips(moveop,find, t0, "", 0));
					}
				}
				else {
					string t0 = "$t0";
					loadValue(order.x, t0, va, true);//�Ȱ�order.x ��ֵ��t0
					storeValue(name, t0);
				}
			}
			else {
				if (tmpSymTable.find(name) != tmpSymTable.end()) { //�ھֲ�������
					string t0 = "$t0";
					string sfind = findUseAllReg(order.result); //��ȫ�ּĴ���
					if (sfind != " ") {
						t0 = sfind;
						loadValue(order.x, t0, va, true);
						if (t0 != sfind) {
							mipsTable.push_back(mips(moveop, sfind, t0, "", 0));
						}
					}
					else {
						loadValue(order.x, t0, va, true);//�Ȱ�order.x ��ֵ��t0
						storeValue(name, t0);//�ٰ����ŵ��ڴ���
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) { //��ȫ�ֱ�����
					string t0 = "$t0";
					int va;
					loadValue(order.x, t0, va, true);//�Ȱ�order.x ��ֵ��t0
					storeValue(name, t0);//�ٰ����ŵ��ڴ���
				}
			}
			break;
		}
		case MULT: { //result = x * y;
			string result = "$t2", x = "$t0", y = "$t1";
			string find = " ",sfind = " ";
			if (order.result[0] == '@') { //����ʱ�Ĵ���
				find = insertTmpReg(order.result);
				if (find != " ") {
					result = find;
				}
			}
			sfind = findUseAllReg(order.result);
			if (sfind != " ") {
				result = sfind;
			}
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			if (va1 != FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(li, result, "", "", va1*va2));
			}
			else if (va1 != FAIL && va2 == FAIL) { // ֻ�õ���va1
				if (va1 == 1) { //result = y
					mipsTable.push_back(mips(moveop, result, y, "", 0));
				}
				else if (va1 == 0) {
					mipsTable.push_back(mips(li, result, "", "", 0));
				}
				else {
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(mul, result, x, y, 0));
				}
			}
			else if (va1 == FAIL && va2 != FAIL) { // ֻ�õ���va2
				if (va2 == 1) { //result = x
					mipsTable.push_back(mips(moveop,result,x,"",0));
				}
				else if (va2 == 0) {
					mipsTable.push_back(mips(li, result, "", "", 0));
				}
				else {
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(mul, result, x,y, 0));
				}
			}
			else {
				mipsTable.push_back(mips(mul, result, x, y, 0));
			}
			if (order.result[0] == '@') { //����ʱ������û�ҵ�
				if (find == " ") {
					storeValue(order.result, result);
				}
			}
			else {
				if (sfind == " ") {
					storeValue(order.result, result);
				}
			}
			break;
		}
		case DIV: { //lo = x / y;
			string result = "$t2", x = "$t0", y = "$t1";
			string find,sfind;
			if (order.result[0] == '@') {
				string find = insertTmpReg(order.result);
				if (find != " ") {
					result = find;
				}
			}
			sfind = findUseAllReg(order.result);
			if (sfind != " "){
				result = sfind;
			}
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			if (va1 != FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(li, result, "", "", va1 / va2));
			}
			else if (va1 != FAIL && va2 == FAIL) { //ֻ�ҵ���va1
				if (va1 == 0) { //result = 0/y
					mipsTable.push_back(mips(li, result, "", "", 0));
				}
				else {
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(dive, x, y, "", 0));
					mipsTable.push_back(mips(mflo, result, "", "", 0));
				}
			}
			else if (va1 == FAIL && va2 != FAIL) { //ֻ�ҵ���va2 y
				if (va2 == 1) { //result = x/1 = x
					mipsTable.push_back(mips(moveop, result, x, "", 0));
				}
				else {
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(dive, x, y, "", 0));
					mipsTable.push_back(mips(mflo, result, "", "", 0));
				}
			}
			else {
				mipsTable.push_back(mips(dive, x, y, "", 0));
				mipsTable.push_back(mips(mflo, result, "", "", 0));
			}
			if (order.result[0] == '@') { //����ʱ������û�ҵ�
				if (find == " ") {
					storeValue(order.result, result);
				}
			}
			else {
				if (sfind == " ") {
					storeValue(order.result, result);
				}
			}
			break;
		}
		case GETARRAY: { //result = x[y1.1][y1.2]
			string index = order.y;
			string name = order.result;
			vector<string> indexs = split(index, " ");
			if (indexs.size() == 1) { //һά����
				string x = "$t1";//�����±�
				string address = "$t0";//t0��¼��ַ t2������ֵ
				loadValue(order.y, x, va, false);//ȡ�±� false˵�������õ�������
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x, "", 0));
					x = "$t1";
				}
				if (tmpSymTable.find(order.x) != tmpSymTable.end()) {
					addr = tmpSymTable[order.x].addr;
					if (va != FAIL) { //�ҵ���x
						mipsTable.push_back(mips(lw, "$t2", "$sp", "", 4 * (addr + va))); //t2����ֵ
					}
					else { //û�ҵ�x
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr)); //t0 = sp+addr
						mipsTable.push_back(mips(sll, x, x, "", 2));//x��4
						mipsTable.push_back(mips(add, address, address, x, 0)); //t0 = t0 + x*4 
						mipsTable.push_back(mips(lw, "$t2", address, "", 0));//�ӵ�ַt0�����
					}
				}
				else if (globalSymTable.find(order.x) != globalSymTable.end()) {
					addr = globalSymTable[order.x].addr;
					if (va != FAIL) {
						mipsTable.push_back(mips(lw, "$t2", "$gp", "", 4 * (addr + va))); //t2��ֵ��ע����gp
					}
					else {
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr)); //t0 = gp+addr
						mipsTable.push_back(mips(sll, x, x, "", 2));//x��4
						mipsTable.push_back(mips(add, address, address, x, 0)); //t0 = t0 + x*4 
						mipsTable.push_back(mips(lw, "$t2", address, "", 0));//�ӵ�ַt0�����
					}
				}
				string storeReg = "$t2";
				string find = " ", sfind = " ";
				sfind = findUseAllReg(order.result);
				if (order.result[0] == '@') {
					find = insertTmpReg(order.result);
					if (find != " ") {
						mipsTable.push_back(mips(moveop, find, storeReg, "", 0));
					}
				}
				else if (sfind != " ") {
					mipsTable.push_back(mips(moveop, sfind, storeReg, "", 0));
				}
				else {
					storeValue(order.result, storeReg);
				}
			}
			else if (indexs.size() == 2) { //��ά����
				string address = "$t0";//��t0���洢λ��
				string x = "$t1", y = "$t2";//x�±�,y�±�
				string arryValue = "$t3";//��t3����ȡ���������ֵ
				loadValue(indexs[0], x, va1, false);
				loadValue(indexs[1], y, va2, false);//���Ա��ش����������
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x, "", 0));
					x = "$t1";
				}
				if (y != "$t2") {
					mipsTable.push_back(mips(moveop, "$t2", y, "", 0));
					y = "$t2";
				}
				if (tmpSymTable.find(order.x) != tmpSymTable.end()) {//��ʱ����
					addr = tmpSymTable[order.x].addr;
					int length = tmpSymTable[order.x].length;
					if (va1 != FAIL && va2 != FAIL) { //���ҵ���
						mipsTable.push_back(mips(lw, arryValue, "$sp", "", 4 * (addr + length * va1 + va2))); //�±� = ��ַ+length*x+y
					}
					else if (va1 != FAIL && va2 == FAIL) { //ֻ�ҵ���va1
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						mipsTable.push_back(mips(addi, x, y, "", length * va1));
						mipsTable.push_back(mips(sll, x, x, "", 2)); //*4
						mipsTable.push_back(mips(add, address, address, x, 0));//��ַ = addr+4*(x*length+y)
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));
					}
					else if (va1 == FAIL && va2 != FAIL) { //ֻ�ҵ���va2
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(addi, x, x, "", va2));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//ȡ��value
					}
					else { //��û�ҵ�
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(add, x, x, y, 0));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//ȡ��value
					}
				}
				else if (globalSymTable.find(order.x) != globalSymTable.end()) { //ȫ�ֱ���
					addr = globalSymTable[order.x].addr;
					int length = globalSymTable[order.x].length;
					if (va1 != FAIL && va2 != FAIL) { //���ҵ���
						mipsTable.push_back(mips(lw, arryValue, "$gp", "", 4 * (addr + length * va1 + va2))); //�±� = ��ַ+length*x+y
					}
					else if (va1 != FAIL && va2 == FAIL) { //ֻ�ҵ���va1
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						mipsTable.push_back(mips(addi, x, y, "", length * va1));
						mipsTable.push_back(mips(sll, x, x, "", 2)); //*4
						mipsTable.push_back(mips(add, address, address, x, 0));//��ַ = addr+4*(x*length+y)
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));
					}
					else if (va1 == FAIL && va2 != FAIL) { //ֻ�ҵ���va2
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(addi, x, x, "", va2));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//ȡ��value
					}
					else { //��û�ҵ�
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(add, x, x, y, 0));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//ȡ��value
					}
				}
				string find = " ", sfind = " ";
				sfind = findUseAllReg(order.result);
				if (order.result[0] == '@') {
					find = insertTmpReg(order.result);
					if (find != " ") {
						mipsTable.push_back(mips(moveop, find, arryValue, "", 0));
					}
				}
				else if (sfind != " ") {
					mipsTable.push_back(mips(moveop, sfind, arryValue, "", 0));
				}
				else {
					storeValue(order.result, arryValue);
				}
			}
			break;
		}
		case PUTARRAY: { //result[x1][x2] = y
			// x1,x2,y�����Ǳ�ʶ����Ҳ��������ֵ
			string index = order.x;
			string name = order.result;
			vector<string> indexs = split(index, " ");
			if (indexs.size() == 1) { //1ά����
				string y = "$t0";//��t0���渳ֵvalue == y.
				string x = "$t1";//xΪ�±꣬��t1����
				loadValue(order.y, y, va1, true); //���y��ֵ����ֵ����y��
				loadValue(indexs[0], x, va2, false); //���x��ֵ���±����x��
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop,"$t1",x,"",0));
					x = "$t1";
				}
				if (tmpSymTable.find(name) != tmpSymTable.end()) {//��ʱ������
					addr = tmpSymTable[name].addr;
					if (va2 != FAIL) {
						mipsTable.push_back(mips(sw, y, "$sp", "", (4 * (addr+va2)))); //��ַΪ4*(addr+va2)
					}
					else {
						mipsTable.push_back(mips(addi, "$t2", "$sp", "", 4 * addr)); //t2�ǵ�ַ������
						mipsTable.push_back(mips(sll, x, x, "", 2));//�±�*4
						mipsTable.push_back(mips(add, "$t2", "$t2", x, 0));//��ȡ���յ�ַ��t2
						mipsTable.push_back(mips(sw, y, "$t2", "", 0));//��y��t2���
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) { //ȫ�ֱ�����
					//ȫ�ֱ����ǵ����ϼ�
					addr = globalSymTable[name].addr;
					if (va2 != FAIL) {
						mipsTable.push_back(mips(sw, y, "$gp", "", (4 * (addr + va2)))); //��ַΪ4*(addr+va2)
					}
					else {
						mipsTable.push_back(mips(addi, "$t2", "$gp", "", 4 * addr)); //t2�ǵ�ַ������
						mipsTable.push_back(mips(sll, x, x, "", 2));//�±�*4
						mipsTable.push_back(mips(add, "$t2", "$t2", x, 0));//��ȡ���յ�ַ��t2
						mipsTable.push_back(mips(sw, y, "$t2", "", 0));//��y��t2���
					}
				}
			}
			else if (indexs.size() == 2) { //2ά����
				string y = "$t0";//��t0���渳ֵvalue == y.
				string x1 = "$t1";//x1Ϊ��һλ�±꣬��t1����
				string x2 = "$t2";//x2Ϊ�ڶ�λ�±꣬��t2����
				string len = "$t3";//t3 Ϊ����
				loadValue(order.y, y, va, true); //���y��ֵ����ֵ����y��
				loadValue(indexs[0], x1, va2, false); //���x1��ֵ���±����x1��
				loadValue(indexs[1], x2, va3, false); //���x2��ֵ���±����x2��

				if (x1 != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x1, "", 0));
					x1 = "$t1";
				}
				if (x2 != "$t2") {
					mipsTable.push_back(mips(moveop, "$t2", x2, "", 0));
					x2 = "$t2";
				}
				if (tmpSymTable.find(name) != tmpSymTable.end()) { //��ʱ���ű���
					int length = tmpSymTable[name].length;
					string step = int2string(length);
					addr = tmpSymTable[name].addr;
					if (va2 != FAIL && va3 != FAIL) { //������ַ���ҵ���
						mipsTable.push_back(mips(sw, y, "$sp", "", 4*(addr+va2*length+va3)));//��y��t2���
					}
					else if (va2 == FAIL && va3 != FAIL) { //ֻ�ҵ���x2�±�
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(addi, x1, x2, "", va3));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}
					else if (va2 != FAIL && va3 == FAIL) { //ֻ�ҵ���x1�±�
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(li, x1, "", "", va2*length));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}
					else {
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) {
					int length = globalSymTable[name].length;
					string step = int2string(length);
					addr = globalSymTable[name].addr;
					if (va2 != FAIL && va3 != FAIL) { //������ַ���ҵ���
						mipsTable.push_back(mips(sw, y, "$gp", "", 4 * (addr + va2 * length + va3)));//��y��t2���
					}
					else if (va2 == FAIL && va3 != FAIL) { //ֻ�ҵ���x2�±�
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(addi, x1, x2, "", va3));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}
					else if (va2 != FAIL && va3 == FAIL) { //ֻ�ҵ���x1�±�
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(li, x1, "", "", va2 * length));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}
					else {
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4�ǵ�ַ������
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//��ȡ���յ�ַ��t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//��y��t2���
					}

				}
			}
			break;
		}
		case USE: { //���ú���ʱ�����õ�
			funcParaList.push(order); //�������
			break;
		}
		case CALL: { // ���ϵ���: ra, tempVar, para
			SymItem sym = globalSymTable[order.result];
			int paraLength = globalSymTable[order.result].paraList.size();//ȡ�ó���
			vector<string>tmpList, allList;
			for (int i = 5; i <= 9; i++) {
				if (tmpRegUse[i] != " ") {
					tmpList.push_back("$t" + int2string(i));
				}
			}
			for (int i = 0; i <= 7; i++) {
				if (allRegUse[i] != " ") {
					allList.push_back("$s" + int2string(i));
				}
			}
			int tmpListLen = tmpList.size();
			int allListLen = allList.size();
			for (int i = 0; i < paraLength; i++) {
				int va;
				string tmpReg = "$t4";
				midCode para = funcParaList.top();
				funcParaList.pop();
				loadValue(para.result, tmpReg, va, true);
				mipsTable.push_back(mips(sw, tmpReg, "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen) + 4 * (paraLength - i - 1)));//�ŵ�ջ�׶�
			}
			mipsTable.push_back(mips(addi, "$sp", "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen)));//��ջ
			mipsTable.push_back(mips(sw, "$ra", "$sp", "", sym.length * 4 + 4));//�ŵ�ջ��
			for (int i = 0; i < tmpListLen; i++) {
				mipsTable.push_back(mips(sw, tmpList[i], "$sp", "", sym.length * 4 + 4 + (i + 1) * 4));
			}
			for (int i = 0; i < allListLen; i++) {
				mipsTable.push_back(mips(sw, allList[i], "$sp", "", sym.length * 4 + 4 + 4 * tmpListLen + (i + 1) * 4));
			}
			mipsTable.push_back(mips(jal, order.result, "", "", 0));
			for (int i = 0; i < allListLen; i++) {
				mipsTable.push_back(mips(lw, allList[i], "$sp", "", sym.length * 4 + 4 + 4 * tmpListLen + (i + 1) * 4));
			}
			for (int i = 0; i < tmpListLen; i++) {
				mipsTable.push_back(mips(lw, tmpList[i], "$sp", "", sym.length * 4 + 4 + (i + 1) * 4));
			}
			mipsTable.push_back(mips(lw, "$ra", "$sp", "", sym.length * 4 + 4));//ȥ��ra
			mipsTable.push_back(mips(addi, "$sp", "$sp", "", 4 * sym.length + 8 + 4 * (tmpListLen + allListLen)));
			break;
		}
		case LABEL: {
			mipsTable.push_back(mips(label, order.result, "", "", 0));//�±�ע
			break;
		}
		case LEQ: { // <= 
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x > y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 > va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 > va2
					mipsTable.push_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 > t1
					mipsTable.push_back(mips(blt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else {
					mipsTable.push_back(mips(bgt, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x<=y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 <= va2
					mipsTable.push_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 >= va1
					mipsTable.push_back(mips(bge, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 <= t1
					mipsTable.push_back(mips(ble, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case LSS: { // <
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x >= y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 >= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 >= va2
					mipsTable.push_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 >= t1
					mipsTable.push_back(mips(ble, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else {
					mipsTable.push_back(mips(bge, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x<y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 < va2
					mipsTable.push_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 > va1
					mipsTable.push_back(mips(bgt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 < t1
					mipsTable.push_back(mips(blt, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case GRE: { // >
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x <=  y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 <= va2
					mipsTable.push_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 <= t1
					mipsTable.push_back(mips(bge, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else {
					mipsTable.push_back(mips(ble, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x>y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 > va2
					mipsTable.push_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 < va1
					mipsTable.push_back(mips(blt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 > t1
					mipsTable.push_back(mips(bgt, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case GEQ: { // >=
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x <  y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 < va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 < va2
					mipsTable.push_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 < t1
					mipsTable.push_back(mips(bgt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else {
					mipsTable.push_back(mips(blt, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x>=y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 >= va2
					mipsTable.push_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 <= va1
					mipsTable.push_back(mips(ble, y, int2string(va1), nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 >= t1
					mipsTable.push_back(mips(bge, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case EQL: { // ==
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x != y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 != va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 != va2 û�õ�t1
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 != t1 û�õ�t0
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
				else {
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x==y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 == va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 == va2
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 == va1
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(beq, y, x, nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 >= t1
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case NEQ: { // !=
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION��һ������ת�жϡ�
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //��������������ת����x == y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 == va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 != va2 û�õ�t1
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 != t1 û�õ�t0
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
				else {
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //Ϊ����ת x!=y
				if (va1 != FAIL && va2 != FAIL) { //������ҵ���
					if (va1 != va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 == va2
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 == va1
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(bne, y, x, nextOrder.result, 0)); //t1 < va1��ת
				}
				else { //t0 >= t1
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case GOTO: {
			mipsTable.push_back(mips(j, order.result, "", "", 0)); //��������ת
			break;
		}
		case RET: { //�����е�return���
			int va;
			string v0 = "$v0";
			if (order.result != int2string(FAIL)) {
				loadValue(order.result, v0, va, true);
				if (v0 != "$v0") {
					mipsTable.push_back(mips(moveop, "$v0", v0, "", 0));
				}
			}
			mipsTable.push_back(mips(jr, "$ra", "", "", 0));// �ݹ�����
			break;
		}
		case PARA: { //����
			string sfind = findUseAllReg(order.result);
			if (tmpSymTable.find(order.result) != tmpSymTable.end()) {
				SymItem sym = tmpSymTable[order.result];
				if (sfind != " ") {
					mipsTable.push_back(mips(lw, sfind, "$sp", "", 4 * sym.addr));
				}
			}
			break;
		}
		case FUNC: { //todo
			SymItem sym = globalSymTable[order.result];
			int length = sym.length; //��ȡ����
			if (isReturn == true) { //��֮ǰ��һ��������
				mipsTable.push_back(mips(jr, "$ra", "", "", 0));
			}
			mipsTable.push_back(mips(label, order.result, "", "", 0));
			if (order.result == "main") {
				mipsTable.push_back(mips(addi, "$sp", "$sp", "", -4 * length - 8)); //��ʼ��ָ��
			}
			tempFuncName = order.result; //����
			tmpSymTable = allTempSymTable[tempFuncName];  //����
			for (int i = 0; i <= 7; i++) { //���
				allRegUse[i] = " ";
			}
			updateAllReg(midIndex+1);
			isReturn = true;
			break;
		}
		case CASE: {
			switchList.push_back(order); //����list
			break;
		}
		case SWITCH: {
			string var = order.result;
			string switchVar = "$t0";//t0Ϊ�Ƚ϶���
			loadValue(var, switchVar, va, false); //���ر���
			for (int i = 0; i < switchList.size(); i++) { //��default
				midCode caseSym = switchList[i];
				if (va == FAIL) {
					mipsTable.push_back(mips(beq, switchVar, caseSym.x, caseSym.result, 0));
				}
				else {
					if (string2int(caseSym.x) == va) {
						mipsTable.push_back(mips(j, caseSym.result, "","" , 0));
					}
				}
			}
			switchList.clear();
			break;
		}
		case EXIT: {
			mipsTable.push_back(mips(li, "$v0", "", "", 10));
			mipsTable.push_back(mips(syscall, "", "", "", 0));
			break;
		}
		default:
			break;
		}

	}
	mipsTable.push_back(mips(li, "$v0", "", "", 10));
	mipsTable.push_back(mips(syscall, "", "", "", 0));

}

void midCodeOut() {
	ofstream midCodeFile;
	midCodeFile.open("�м����.txt");
	for (int i = 0; i < midCodeTable.size(); i++) {
		midCode order = midCodeTable[i];
		switch (order.op) {
		case PLUS:
			midCodeFile << order.result << " " << order.x << " + " << order.y << endl;
			break;
		case MINUS:
			midCodeFile << order.result << " " << order.x << " - " << order.y << endl;
			break;
		case MULT:
			midCodeFile << order.result << " " << order.x << " * " << order.y << endl;
			break;
		case DIV:
			midCodeFile << order.result << " " << order.x << " / " << order.y << endl;
			break;
		case LSS:
			midCodeFile << order.x << " < " << order.y << endl;
			break;
		case LEQ:
			midCodeFile << order.x << " <= " << order.y << endl;
			break;
		case GRE:
			midCodeFile << order.x << " > " << order.y << endl;
			break;
		case GEQ:
			midCodeFile << order.x << " >= " << order.y << endl;
			break;
		case EQL:
			midCodeFile << order.x << " == " << order.y << endl;
			break;
		case NEQ:
			midCodeFile << order.x << " != " << order.y << endl;
			break;
		case BNZ:
			midCodeFile << "BNZ " << order.result << endl;
			break;
		case BZ:
			midCodeFile << "BZ " << order.result << endl;
			break;
		case ASSIGN:
			midCodeFile << order.result << " = " << order.x << endl;
			break;
		case GOTO:
			midCodeFile << "GOTO " << order.result << endl;
			break;
		case SCANF:
			midCodeFile << "SCANF " << order.result << endl;
			break;
		case PRINTF:
			midCodeFile << "PRINTF " << order.result << endl;
			break;
		case LABEL:
			midCodeFile << "LABEL " << order.result << endl;
			break;
		case FUNC:
			midCodeFile << "FUNC " << order.x << " " << order.result << "()" << endl;
			break;
		case PARA:
			midCodeFile << "PARA " << order.x << " " << order.result << endl;
			break;
		case GETARRAY:
			midCodeFile << order.result << " = " << order.x << "[" << order.y << "]" << endl;
			break;
		case PUTARRAY:
			midCodeFile << order.result << "[" << order.x << "]" << " = " << order.y << endl;
			break;
		case CALL:
			midCodeFile << "CALL " << order.result << endl;
			break;
		case USE:
			midCodeFile << "PUSH " << order.result << endl;
			break;
		case RETVALUE:
			midCodeFile << "RET " << order.result << endl;
			break;
		case SWITCH:
			midCodeFile << "SWITCH " << order.result << endl;
			break;
		case CASE:
			midCodeFile << "CASE " << order.result << " = " << order.x << endl;
			break;
		}
	}
}

void mipsOut() {
	ofstream mipsFile;
	mipsFile.open("mips.txt");
	for (int i = 0; i < mipsTable.size(); i++) {
		mips order = mipsTable[i];
		switch (order.op)
		{
		case add:
			mipsFile << "addu " << order.result << "," << order.x << "," << order.y << "\n";
			break;

		case addi:
			mipsFile << "addi " << order.result << "," << order.x << "," << order.imm << "\n";
			break;
		case sub:
			mipsFile << "sub " << order.result << "," << order.x << "," << order.y << "\n";
			break;

		case subi:
			mipsFile << "subi " << order.result << "," << order.x << "," << order.imm << "\n";
			break;
		case mult:
			mipsFile << "mult " << order.result << "," << order.x << "\n";
			break;
		case mul:
			mipsFile << "mul " << order.result << "," << order.x << "," << order.y << "\n";
			break;
		case dive:
			mipsFile << "div " << order.result << "," << order.x << "\n";
			break;
		case mflo:
			mipsFile << "mflo " << order.result << "\n";
			break;
		case moveop:
			mipsFile << "move " << order.result << "," << order.x << "\n";
			break;
		case mfhi:
			mipsFile << "mflo " << order.result << "\n";
			break;
		case dataSet:
			mipsFile << ".data" << "\n";
			break;
		case textSet:
			mipsFile << ".text" << "\n";
			break;
		case strSet:
			mipsFile << order.result << ": .asciiz \"" << order.x << "\"\n";
			break;
		case sll:
			mipsFile << "sll " << order.result << "," << order.x << "," << order.imm << "\n";
			break;
		case li:
			mipsFile << "li " << order.result << "," << order.imm << "\n";
			break;
		case la:
			mipsFile << "la " << order.result << "," << order.x << "\n";
			break;
		case sw:
			mipsFile << "sw " << order.result << "," << order.imm << "(" << order.x << ")" << "\n";
			break;
		case syscall:
			mipsFile << "syscall " << "\n";
			break;
		case lw:
			mipsFile << "lw " << order.result << ", " << order.imm << "(" << order.x << ")" << "\n";
			break;
		case jal:
			mipsFile << "jal " << order.result << "\n";
			break;
		case jr:
			mipsFile << "jr " << order.result << "\n";
			break;
		case beq:
			mipsFile << "beq " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case bne:
			mipsFile << "bne " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case bgt:
			mipsFile << "bgt " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case bge:
			mipsFile << "bge " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case blt:
			mipsFile << "blt " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case ble:
			mipsFile << "ble " << order.result << ", " << order.x << ", " << order.y << "\n";
			break;
		case label:
			mipsFile << order.result << ": \n";
			break;
		case j:
			mipsFile << "j " << order.result << "\n";
			break;
		default:
			break;
		}
	}
	mipsFile.close();
}
