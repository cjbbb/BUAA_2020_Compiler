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
//外部引用
extern map <string, SymItem> globalSymTable; //全局表
extern map<string, map<string, SymItem>> allTempSymTable;//储存所有的临时表
extern vector<string> funcList; //funcName的list
extern vector<string> strList;//str 记录
extern vector<midCode> midCodeTable;//str 记录
//内部定义
stack <midCode> funcParaList; //调用函数时记录传值的
vector<midCode> switchList; //记录case语句的
string tempFuncName = "";//现在临时的变量名
vector<mips> mipsTable; //mips 指令集的记录
map <string, SymItem> tmpSymTable; //从allTempSymTable中拿出的临时表 注意为tmp而不是temp
bool isReturn = false;
string tmpRegUse[10] = { " "," "," "," "," "," "," "," "," ", " ", };//t5,t6,t7,t8,t9可以作为分配
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
		if (in[0] == '@') { //如果是临时变量就false
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
	return -1;//没找到
}
string findUseAllReg(string name) { //找到正在使用的寄存器
	for (int i = 0; i <= 7; i++) {
		if (allRegUse[i] == name) {
			return "$s"+int2string(i);
		}
	}
	return " ";
}
int findUseTmpReg(string name) { //找到正在使用的寄存器
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
	return " ";//失败了
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


void storeValue(string& name, string& reg) { //存储值 name是变量名
	int addr;
	if (tmpSymTable.find(name) != tmpSymTable.end()) { //临时变量里搜
		addr = tmpSymTable[name].addr; //变量地址
		mipsTable.push_back(mips(sw, reg, "$sp", " ", 4 * addr));// 
	}
	else if (globalSymTable.find(name) != globalSymTable.end()) {
		addr = globalSymTable[name].addr;
		mipsTable.push_back(mips(sw, reg, "$gp", "", 4 * addr));
	}
}


void loadValue(string& name, string& reg, int& va, bool need) { //将name的值付给reg 
	// need 是 如果是const 是否生成li指令
	int addr;
	SymItem sym;
	int value; //故意强制类型转换
	if (tmpSymTable.find(name) != tmpSymTable.end()) { //临时变量里搜
		sym = tmpSymTable[name];
		if (sym.kind == 1) { //如果是const
			if (sym.type == 2) { //是int
				value = sym.intValue;
			}
			else if (sym.type == 3) { //是char
				value = int(sym.charValue);
			}
			va = value;
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", value));
			}
		}
		else { //如果是var
			int find = findUseTmpReg(name);
			if (find != FAIL) { //找到了
				reg = "$t" + int2string(find);
				tmpRegUse[find] = " ";//制空
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
	else if (globalSymTable.find(name) != globalSymTable.end()) { //是全局变量
		sym = globalSymTable[name];
		if (sym.kind == 1) { //如果是const
			if (sym.type == 2) { //是int
				value = sym.intValue;
			}
			else if (sym.type == 3) { //是char
				value = sym.charValue;
			}
			va = value;
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", value));
			}
		}
		else { //如果是var
			addr = sym.addr;
			mipsTable.push_back(mips(lw, reg, "$gp", "", 4 * addr));
		}
	}
	else { //是立即数
		if (name.size() > 0) {
			va = string2int(name);
			if (need == true) {
				mipsTable.push_back(mips(li, reg, "", "", string2int(name)));
			}
		}
	}
}
string getStrIndex(string in) { //获取字符串在strList中的下标
	for (int i = 0; i < strList.size(); i++) {
		if (strList[i] == in) {
			return "str" + int2string(i);
		}
	}
}
void mipsGen() { //生成mips

	mipsTable.push_back(mips(dataSet, "", "", "", 0));//设置data地址
	for (int i = 0; i < strList.size(); i++) {  //加.ascii部分
		mipsTable.push_back(mips(strSet, "str" + int2string(i), strList[i], "", 0));//x是值，result是名字
	}
	mipsTable.push_back(mips(strSet, "nextLine", "\\n", "", 0));//下一行的标志
	mipsTable.push_back(mips(textSet, "", "", "", 0));//打text标志
	for (int midIndex = 0; midIndex < midCodeTable.size(); midIndex++) {
		int va = FAIL, addr = 0, va1 = FAIL, va2 = FAIL,va3= FAIL;//init var
		midCode order = midCodeTable[midIndex];
		switch (order.op)
		{
		case PRINTF: {
			if (order.x == "4") { //字符串
				string strName = getStrIndex(order.result);
				mipsTable.push_back(mips(la, "$a0", strName, "", 0));
				mipsTable.push_back(mips(li, "$v0", "", "", 4));
				mipsTable.push_back(mips(syscall, "", "", "", 0));
			}
			else if (order.x == "5") { //换行符
				mipsTable.push_back(mips(la, "$a0", "nextLine", "", 0));
				mipsTable.push_back(mips(li, "$v0", "", "", 4));
				mipsTable.push_back(mips(syscall, "", "", "", 0));
			}
			else { //表达式,int / char
				string a0 = "$a0";
				loadValue(order.result, a0, va, true);
				if (a0 != "$a0") { //如果result是其他寄存器
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
				if (tmpSymTable.find(order.result) != tmpSymTable.end()) { //在临时变量里
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
			else if (va1 != FAIL && va2 == FAIL) { //找到了Va1
				mipsTable.push_back(mips(addi, result, y, "", -va1));// result = y - x
				mipsTable.push_back(mips(sub, result, "$0", result,0)); // result = - result
			}
			else if (va1 == FAIL && va2 != FAIL) {
				mipsTable.push_back(mips(addi, result,x, "", -va2)); // result =x - y
			}
			else {
				mipsTable.push_back(mips(sub, result, x, y, 0));
			}
			if (order.result[0] == '@') { //是临时变量且没找到
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
			if (tmpSymTable.find(order.result) != tmpSymTable.end()) { //临时变量里有
				if (order.result[0] == '@') { //是临时寄存器
					string find = insertTmpReg(order.result);
					if (find != " ") {
						mipsTable.push_back(mips(moveop, find, "$v0", "", 0));//把v0的值存入临时变量
					}
				}
				string sfind = findUseAllReg(order.result);
				if (sfind != " ") { //是全局寄存器
					mipsTable.push_back(mips(moveop, sfind, "$v0", "", 0));//把v0的值存入临时变量
				}
				else {
					SymItem sym = tmpSymTable[order.result];
					int addr = sym.addr;
					mipsTable.push_back(mips(sw, "$v0", "$sp", "", 4 * addr));//把v0的值存入临时变量
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
			else if (va1 != FAIL && va2 == FAIL) { //只有x没有y
				mipsTable.push_back(mips(addi, result, y, "", va1 ));
			}
			else if (va1 == FAIL && va2 != FAIL) { //只有y没有x
				mipsTable.push_back(mips(addi, result, x, "", va2));
			}
			else {
				mipsTable.push_back(mips(add, result, x, y, 0));
			}
			if (order.result[0] == '@') { //是临时变量且没找到
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
			if (order.result[0] == '@') { //是中间变量
				string find = insertTmpReg(order.result);
				if (find != " ") { //有空闲
					string t0 = find;
					loadValue(order.x, t0, va, true);//直接存到t0中
					if (t0 !=  find) {
						mipsTable.push_back(mips(moveop,find, t0, "", 0));
					}
				}
				else {
					string t0 = "$t0";
					loadValue(order.x, t0, va, true);//先把order.x 赋值给t0
					storeValue(name, t0);
				}
			}
			else {
				if (tmpSymTable.find(name) != tmpSymTable.end()) { //在局部变量中
					string t0 = "$t0";
					string sfind = findUseAllReg(order.result); //是全局寄存器
					if (sfind != " ") {
						t0 = sfind;
						loadValue(order.x, t0, va, true);
						if (t0 != sfind) {
							mipsTable.push_back(mips(moveop, sfind, t0, "", 0));
						}
					}
					else {
						loadValue(order.x, t0, va, true);//先把order.x 赋值给t0
						storeValue(name, t0);//再把他放到内存中
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) { //在全局变量中
					string t0 = "$t0";
					int va;
					loadValue(order.x, t0, va, true);//先把order.x 赋值给t0
					storeValue(name, t0);//再把他放到内存中
				}
			}
			break;
		}
		case MULT: { //result = x * y;
			string result = "$t2", x = "$t0", y = "$t1";
			string find = " ",sfind = " ";
			if (order.result[0] == '@') { //是临时寄存器
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
			else if (va1 != FAIL && va2 == FAIL) { // 只得到了va1
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
			else if (va1 == FAIL && va2 != FAIL) { // 只得到了va2
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
			if (order.result[0] == '@') { //是临时变量且没找到
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
			else if (va1 != FAIL && va2 == FAIL) { //只找到了va1
				if (va1 == 0) { //result = 0/y
					mipsTable.push_back(mips(li, result, "", "", 0));
				}
				else {
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(dive, x, y, "", 0));
					mipsTable.push_back(mips(mflo, result, "", "", 0));
				}
			}
			else if (va1 == FAIL && va2 != FAIL) { //只找到了va2 y
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
			if (order.result[0] == '@') { //是临时变量且没找到
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
			if (indexs.size() == 1) { //一维数组
				string x = "$t1";//数组下标
				string address = "$t0";//t0记录地址 t2用来存值
				loadValue(order.y, x, va, false);//取下标 false说明可以用到立即数
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x, "", 0));
					x = "$t1";
				}
				if (tmpSymTable.find(order.x) != tmpSymTable.end()) {
					addr = tmpSymTable[order.x].addr;
					if (va != FAIL) { //找到了x
						mipsTable.push_back(mips(lw, "$t2", "$sp", "", 4 * (addr + va))); //t2来存值
					}
					else { //没找到x
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr)); //t0 = sp+addr
						mipsTable.push_back(mips(sll, x, x, "", 2));//x乘4
						mipsTable.push_back(mips(add, address, address, x, 0)); //t0 = t0 + x*4 
						mipsTable.push_back(mips(lw, "$t2", address, "", 0));//从地址t0中求出
					}
				}
				else if (globalSymTable.find(order.x) != globalSymTable.end()) {
					addr = globalSymTable[order.x].addr;
					if (va != FAIL) {
						mipsTable.push_back(mips(lw, "$t2", "$gp", "", 4 * (addr + va))); //t2存值，注意是gp
					}
					else {
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr)); //t0 = gp+addr
						mipsTable.push_back(mips(sll, x, x, "", 2));//x乘4
						mipsTable.push_back(mips(add, address, address, x, 0)); //t0 = t0 + x*4 
						mipsTable.push_back(mips(lw, "$t2", address, "", 0));//从地址t0中求出
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
			else if (indexs.size() == 2) { //二维数组
				string address = "$t0";//用t0来存储位置
				string x = "$t1", y = "$t2";//x下标,y下标
				string arryValue = "$t3";//用t3来存取到的数组的值
				loadValue(indexs[0], x, va1, false);
				loadValue(indexs[1], y, va2, false);//可以本地处理该立即数
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x, "", 0));
					x = "$t1";
				}
				if (y != "$t2") {
					mipsTable.push_back(mips(moveop, "$t2", y, "", 0));
					y = "$t2";
				}
				if (tmpSymTable.find(order.x) != tmpSymTable.end()) {//临时变量
					addr = tmpSymTable[order.x].addr;
					int length = tmpSymTable[order.x].length;
					if (va1 != FAIL && va2 != FAIL) { //都找到了
						mipsTable.push_back(mips(lw, arryValue, "$sp", "", 4 * (addr + length * va1 + va2))); //下标 = 地址+length*x+y
					}
					else if (va1 != FAIL && va2 == FAIL) { //只找到了va1
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						mipsTable.push_back(mips(addi, x, y, "", length * va1));
						mipsTable.push_back(mips(sll, x, x, "", 2)); //*4
						mipsTable.push_back(mips(add, address, address, x, 0));//地址 = addr+4*(x*length+y)
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));
					}
					else if (va1 == FAIL && va2 != FAIL) { //只找到了va2
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(addi, x, x, "", va2));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//取得value
					}
					else { //都没找到
						mipsTable.push_back(mips(addi, address, "$sp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(add, x, x, y, 0));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//取得value
					}
				}
				else if (globalSymTable.find(order.x) != globalSymTable.end()) { //全局变量
					addr = globalSymTable[order.x].addr;
					int length = globalSymTable[order.x].length;
					if (va1 != FAIL && va2 != FAIL) { //都找到了
						mipsTable.push_back(mips(lw, arryValue, "$gp", "", 4 * (addr + length * va1 + va2))); //下标 = 地址+length*x+y
					}
					else if (va1 != FAIL && va2 == FAIL) { //只找到了va1
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						mipsTable.push_back(mips(addi, x, y, "", length * va1));
						mipsTable.push_back(mips(sll, x, x, "", 2)); //*4
						mipsTable.push_back(mips(add, address, address, x, 0));//地址 = addr+4*(x*length+y)
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));
					}
					else if (va1 == FAIL && va2 != FAIL) { //只找到了va2
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(addi, x, x, "", va2));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//取得value
					}
					else { //都没找到
						mipsTable.push_back(mips(addi, address, "$gp", "", 4 * addr));
						int vaTemp;
						string len = "$t4", strLength = int2string(length);
						loadValue(strLength, len, vaTemp, true);
						mipsTable.push_back(mips(mul, x, x, len, 0)); // x = x*len
						mipsTable.push_back(mips(add, x, x, y, 0));//x = x + y
						mipsTable.push_back(mips(sll, x, x, "", 2));//x = x * 4
						mipsTable.push_back(mips(add, address, address, x, 0)); // addr = addr + x;
						mipsTable.push_back(mips(lw, arryValue, address, "", 0));//取得value
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
			// x1,x2,y可能是标识符，也可能是数值
			string index = order.x;
			string name = order.result;
			vector<string> indexs = split(index, " ");
			if (indexs.size() == 1) { //1维数组
				string y = "$t0";//用t0来存赋值value == y.
				string x = "$t1";//x为下标，用t1来存
				loadValue(order.y, y, va1, true); //获得y的值，赋值存在y中
				loadValue(indexs[0], x, va2, false); //获得x的值，下表存在x中
				if (x != "$t1") {
					mipsTable.push_back(mips(moveop,"$t1",x,"",0));
					x = "$t1";
				}
				if (tmpSymTable.find(name) != tmpSymTable.end()) {//临时变量。
					addr = tmpSymTable[name].addr;
					if (va2 != FAIL) {
						mipsTable.push_back(mips(sw, y, "$sp", "", (4 * (addr+va2)))); //地址为4*(addr+va2)
					}
					else {
						mipsTable.push_back(mips(addi, "$t2", "$sp", "", 4 * addr)); //t2是地址的作用
						mipsTable.push_back(mips(sll, x, x, "", 2));//下标*4
						mipsTable.push_back(mips(add, "$t2", "$t2", x, 0));//获取最终地址。t2
						mipsTable.push_back(mips(sw, y, "$t2", "", 0));//把y往t2里存
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) { //全局变量里
					//全局变量记得往上加
					addr = globalSymTable[name].addr;
					if (va2 != FAIL) {
						mipsTable.push_back(mips(sw, y, "$gp", "", (4 * (addr + va2)))); //地址为4*(addr+va2)
					}
					else {
						mipsTable.push_back(mips(addi, "$t2", "$gp", "", 4 * addr)); //t2是地址的作用
						mipsTable.push_back(mips(sll, x, x, "", 2));//下标*4
						mipsTable.push_back(mips(add, "$t2", "$t2", x, 0));//获取最终地址。t2
						mipsTable.push_back(mips(sw, y, "$t2", "", 0));//把y往t2里存
					}
				}
			}
			else if (indexs.size() == 2) { //2维数组
				string y = "$t0";//用t0来存赋值value == y.
				string x1 = "$t1";//x1为第一位下标，用t1来存
				string x2 = "$t2";//x2为第二位下标，用t2来存
				string len = "$t3";//t3 为长度
				loadValue(order.y, y, va, true); //获得y的值，赋值存在y中
				loadValue(indexs[0], x1, va2, false); //获得x1的值，下表存在x1中
				loadValue(indexs[1], x2, va3, false); //获得x2的值，下表存在x2中

				if (x1 != "$t1") {
					mipsTable.push_back(mips(moveop, "$t1", x1, "", 0));
					x1 = "$t1";
				}
				if (x2 != "$t2") {
					mipsTable.push_back(mips(moveop, "$t2", x2, "", 0));
					x2 = "$t2";
				}
				if (tmpSymTable.find(name) != tmpSymTable.end()) { //临时符号表中
					int length = tmpSymTable[name].length;
					string step = int2string(length);
					addr = tmpSymTable[name].addr;
					if (va2 != FAIL && va3 != FAIL) { //两给地址都找到了
						mipsTable.push_back(mips(sw, y, "$sp", "", 4*(addr+va2*length+va3)));//把y往t2里存
					}
					else if (va2 == FAIL && va3 != FAIL) { //只找到了x2下标
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(addi, x1, x2, "", va3));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}
					else if (va2 != FAIL && va3 == FAIL) { //只找到了x1下标
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(li, x1, "", "", va2*length));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}
					else {
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$sp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}
				}
				else if (globalSymTable.find(name) != globalSymTable.end()) {
					int length = globalSymTable[name].length;
					string step = int2string(length);
					addr = globalSymTable[name].addr;
					if (va2 != FAIL && va3 != FAIL) { //两给地址都找到了
						mipsTable.push_back(mips(sw, y, "$gp", "", 4 * (addr + va2 * length + va3)));//把y往t2里存
					}
					else if (va2 == FAIL && va3 != FAIL) { //只找到了x2下标
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(addi, x1, x2, "", va3));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}
					else if (va2 != FAIL && va3 == FAIL) { //只找到了x1下标
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(li, x1, "", "", va2 * length));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}
					else {
						loadValue(step, len, va1, true);
						mipsTable.push_back(mips(addi, "$t4", "$gp", "", 4 * addr)); //t4是地址的作用
						mipsTable.push_back(mips(mul, x1, len, x1, 0));//x1 = x1*length
						mipsTable.push_back(mips(add, x1, x1, x2, 0));//x1 =x1+x2;
						mipsTable.push_back(mips(sll, x1, x1, "", 2));// x1 = x1*4
						mipsTable.push_back(mips(add, "$t4", "$t4", x1, 0));//获取最终地址。t4
						mipsTable.push_back(mips(sw, y, "$t4", "", 0));//把y往t2里存
					}

				}
			}
			break;
		}
		case USE: { //调用函数时传参用的
			funcParaList.push(order); //放入参数
			break;
		}
		case CALL: { // 从上到下: ra, tempVar, para
			SymItem sym = globalSymTable[order.result];
			int paraLength = globalSymTable[order.result].paraList.size();//取得长度
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
				mipsTable.push_back(mips(sw, tmpReg, "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen) + 4 * (paraLength - i - 1)));//放到栈底端
			}
			mipsTable.push_back(mips(addi, "$sp", "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen)));//减栈
			mipsTable.push_back(mips(sw, "$ra", "$sp", "", sym.length * 4 + 4));//放到栈顶
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
			mipsTable.push_back(mips(lw, "$ra", "$sp", "", sym.length * 4 + 4));//去除ra
			mipsTable.push_back(mips(addi, "$sp", "$sp", "", 4 * sym.length + 8 + 4 * (tmpListLen + allListLen)));
			break;
		}
		case LABEL: {
			mipsTable.push_back(mips(label, order.result, "", "", 0));//下标注
			break;
		}
		case LEQ: { // <= 
			string x = "$t0";
			string y = "$t1";
			loadValue(order.x, x, va1, false);
			loadValue(order.y, y, va2, false);
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x > y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 > va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 > va2
					mipsTable.push_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 > t1
					mipsTable.push_back(mips(blt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
				}
				else {
					mipsTable.push_back(mips(bgt, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x<=y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 <= va2
					mipsTable.push_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 >= va1
					mipsTable.push_back(mips(bge, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
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
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x >= y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 >= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 >= va2
					mipsTable.push_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 >= t1
					mipsTable.push_back(mips(ble, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
				}
				else {
					mipsTable.push_back(mips(bge, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x<y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 < va2
					mipsTable.push_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 > va1
					mipsTable.push_back(mips(bgt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
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
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x <=  y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 <= va2
					mipsTable.push_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 <= t1
					mipsTable.push_back(mips(bge, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
				}
				else {
					mipsTable.push_back(mips(ble, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x>y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 > va2
					mipsTable.push_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 < va1
					mipsTable.push_back(mips(blt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
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
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x <  y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 < va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 < va2
					mipsTable.push_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 < t1
					mipsTable.push_back(mips(bgt, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
				}
				else {
					mipsTable.push_back(mips(blt, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x>=y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 <= va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 >= va2
					mipsTable.push_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // t1 <= va1
					mipsTable.push_back(mips(ble, y, int2string(va1), nextOrder.result, 0)); //t1 < va1跳转
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
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x != y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 != va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 != va2 没得到t1
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 != t1 没得到t0
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
				else {
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x==y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
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
					mipsTable.push_back(mips(beq, y, x, nextOrder.result, 0)); //t1 < va1跳转
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
			// CONDITION后一定接跳转判断。
			midCode nextOrder = midCodeTable[midIndex + 1];
			if (nextOrder.op == BZ) { //不满足条件则跳转，即x == y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
					if (va1 == va2) {
						mipsTable.push_back(mips(j, nextOrder.result, "", "", 0));
					}
				}
				else if (va1 == FAIL && va2 != FAIL) { // t0 != va2 没得到t1
					mipsTable.push_back(mips(li, y, "", "", va2));
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
				else if (va2 == FAIL && va1 != FAIL) { // va1 != t1 没得到t0
					mipsTable.push_back(mips(li, x, "", "", va1));
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
				else {
					mipsTable.push_back(mips(beq, x, y, nextOrder.result, 0));
				}
			}
			else if (nextOrder.op == BNZ) { //为真跳转 x!=y
				if (va1 != FAIL && va2 != FAIL) { //如果都找到了
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
					mipsTable.push_back(mips(bne, y, x, nextOrder.result, 0)); //t1 < va1跳转
				}
				else { //t0 >= t1
					mipsTable.push_back(mips(bne, x, y, nextOrder.result, 0));
				}
			}
			midIndex++;
			break;
		}
		case GOTO: {
			mipsTable.push_back(mips(j, order.result, "", "", 0)); //无条件跳转
			break;
		}
		case RET: { //函数中的return语句
			int va;
			string v0 = "$v0";
			if (order.result != int2string(FAIL)) {
				loadValue(order.result, v0, va, true);
				if (v0 != "$v0") {
					mipsTable.push_back(mips(moveop, "$v0", v0, "", 0));
				}
			}
			mipsTable.push_back(mips(jr, "$ra", "", "", 0));// 递归必须加
			break;
		}
		case PARA: { //参数
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
			int length = sym.length; //获取长度
			if (isReturn == true) { //在之前有一个函数了
				mipsTable.push_back(mips(jr, "$ra", "", "", 0));
			}
			mipsTable.push_back(mips(label, order.result, "", "", 0));
			if (order.result == "main") {
				mipsTable.push_back(mips(addi, "$sp", "$sp", "", -4 * length - 8)); //初始化指针
			}
			tempFuncName = order.result; //更新
			tmpSymTable = allTempSymTable[tempFuncName];  //更新
			for (int i = 0; i <= 7; i++) { //清空
				allRegUse[i] = " ";
			}
			updateAllReg(midIndex+1);
			isReturn = true;
			break;
		}
		case CASE: {
			switchList.push_back(order); //加入list
			break;
		}
		case SWITCH: {
			string var = order.result;
			string switchVar = "$t0";//t0为比较对象
			loadValue(var, switchVar, va, false); //加载变量
			for (int i = 0; i < switchList.size(); i++) { //有default
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
	midCodeFile.open("中间代码.txt");
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
