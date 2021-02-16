#include <fstream>
#include <iostream>
#include <vector>
#include<string>
#include<map>
#include <fstream>
#include <sstream>
#include "YuFa.h"
#include "Error.h"
#include "SymItem.h"
#include "midCode.h"


using namespace std;
map <string, SymItem> globalSymTable; //全局表
map <string, SymItem> tempSymTable; //临时表
map<string, map<string, SymItem>> allTempSymTable;//储存所有的临时表
vector<string> funcList; //funcName的list
vector<string> strList;//str 记录

vector<string> ReturnList; //有返回值列表
vector<string> voidList; //无返回值列表
vector<midCode> midCodeTable;//实例化四元式表
int YuFaLineNumber = 0; //实时更新的当前行数。
int wordI = 0; // words' index
string word; //global variable word
string wordType;
int returnTime; //记录有几个BackSentence
int tempId = 0; //生成的临时变量ID
int labelId = 0;//生成的标签的ID
int switchId = 0; //switchLabel的ID
int allSwitchId = 0;//switch 的次数
int tempAddr = 0; //临时地址
int globalAddr = 0;//全局地址
int charCon = 0;// char的int值
bool isMain = false;//用来判断是否是main中的返回

string genSwitchLabel() {
    switchId++;
    return "label_switch_" + int2string(switchId);
}
string genLabel(string name) { //生成标签
    labelId++;
    return "label_" + int2string(labelId) + "_" + name;
 
}
void initTempTable(string name) { //初始化临时表
    allTempSymTable.insert(make_pair(name,tempSymTable));
    funcList.push_back(name);
    tempSymTable.clear();// 清空
    tempAddr = 0;
} 
int string2int(string in) {//字符串转int
    stringstream s;
    s << in;
    int t;
    s >> t;
    return t;
}
string int2string(int in) { //转换成string
    stringstream s;
    s << in;
    return s.str();
}
string genId() { //生成临时变量ID
    tempId++;
    return "@Temp" + int2string(tempId); //加上标识符@
}

//对有返回值函数定义 提取插入部分
void YuFa::VarIdenfrInsert(string name, int inKind, int inType, int inIntValue, char inCharValue, int index,int addr,int length) {
    if (index == 0) { //全局
        if (globalSymTable.find(name) != globalSymTable.end()) {//已经存在了
            Error('b', YuFaLineNumber);
        }
        else {
            globalSymTable.insert(make_pair(name, SymItem(name, inKind, inType,globalAddr, inIntValue, inCharValue,length)));
            globalAddr+=addr;
        }
    }
    else if (index == 1) { //临时
        if (tempSymTable.find(name) != tempSymTable.end()) {//已经存在了
            Error('b', YuFaLineNumber);
        }
        else {
            tempSymTable.insert(make_pair(name, SymItem(name, inKind, inType,tempAddr, inIntValue, inCharValue,length)));
            tempAddr+=addr;
        }
    }
}

//判断KLM类错误是否含有换行
void YuFa::ErrorKLM(char type) {
    BackWord();//先回到上一行
    int judgeLine = lineNumberList[wordI];
    Error(type, judgeLine);
}

//判断是否有换行
bool YuFa::changeLine() { //比较这个元素与上个元素的行数是否相等
    int now = YuFaLineNumber;
    int next = lineNumberList[wordI - 1];
    return now == next;
}

string YuFa::preRead() {
    return wordsType[wordI+1];
}

void YuFa::VariableDefinationWithInitOError(int tempType) { //提取变量定义及初始化的O错误
    string nextType = preRead();
    if (nextType == "CHARCON") { //预读下一词
        if (tempType == 2) {
            Error('o', YuFaLineNumber);
        }
    }
    else if (nextType == "INTCON" || nextType == "PLUS" || nextType == "MINU") {
        if (tempType == 3) { //int:char
            Error('o', YuFaLineNumber);
        }
    }
}

//根据符号表进行判断
SymItem YuFa::judgeIdenfr(string in) {
    SymItem returnValue;
    if (tempSymTable.find(in) == tempSymTable.end()) { //没有在temp里
        if (globalSymTable.find(in) == globalSymTable.end()) { //没有在global里
            Error('c', YuFaLineNumber);
        }
        else { //在global里
            returnValue = globalSymTable[in];
        }
    }
    else { //在temp里
        returnValue = tempSymTable[in];
    }
    return returnValue;
}


//得到单词
void YuFa::Getword(int in) {
    word = words[wordI];
    wordType = wordsType[wordI];
}

//获取小写字符串
string YuFa::getMin(string in) {
    for (int i = 0; i < in.size(); i++) {
        if (in[i] <= 'Z' && in[i] >= 'A') {
            in[i] = in[i] - 'A' + 'a';
        }
    }
    return in;
}

//找到字符串
bool YuFa::findString(string in, vector<string> from) {
    for (int i = 0; i < from.size(); i++) {
        if (from[i] == in) {
            return true;
        }
    }
    return false;
}

void YuFa::Recover(int in) {
    int times = wordI - in;
    wordI = in;
    word = words[wordI];
    YuFaLineNumber = lineNumberList[wordI];
    //int tempLength = ans.size();
    for (int i = 0; i < times; i++) {
        string temp = ans.at(ans.size() - 1);
        if (temp[0] == '<' && temp[temp.size() - 1] == '>') { //<>
            i--;
        }
        ans.pop_back();
    }
}

void YuFa::NextWord() { //读取下一个单词
    wordI++;
    word = words[wordI];
    wordType = wordsType[wordI];
    if (wordType == "CHARCON") {
        charCon = int(word[0]); //赋值
    }
    ans.push_back(wordType + " " + word);
    YuFaLineNumber = lineNumberList[wordI];
}

void YuFa::BackWord() {
    if (ans.size()==0) {
        return;
    }
    else {
        wordI--;
        word = words[wordI];
        YuFaLineNumber = lineNumberList[wordI];
        wordType = wordsType[wordI];
        ans.pop_back(); //删除最后一个元素
    }
}

vector<string> YuFa::getAns() {
    return ans;
}

void YuFa::Out(string in) { //输出数据
    ans.push_back(in);
}

//＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝
string YuFa::String() { //<字符串>
    NextWord(); //此处ERROR 在cifa里匹配
    if (wordType == "STRCON") {
        Out("<字符串>");
        return word;
    }
    else {
        BackWord();
        return int2string(FAIL);
    }
}

//＜步长＞::= ＜无符号整数＞
int YuFa::StepLength(int init) { //<步长>
    int length = NunInteger();
    if (length != FAIL) {
        Out("<步长>");
        return length;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞ |
// for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
int YuFa::LoopSentence(int init) { //<循环语句>
    NextWord();
    if (wordType == "WHILETK") { //while
        NextWord();
        string labBegin, labEnd;
        labBegin = genLabel("begin");
        midCodeTable.push_back(midCode(LABEL,"","",labBegin)); //放置标记点用来返回。
        if (wordType == "LPARENT") { //while(
            string result;
            if (Condition(wordI,result) != FAIL) {
                NextWord();
                if (wordType != "RPARENT") { //)
                    ErrorKLM('l');
                }
                labEnd = genLabel("end");
                midCodeTable.push_back(midCode(BZ,result , "", labEnd)); //result = 0.跳转到End 不符合条件
                if (Sentence(0) != FAIL) {
                    Out("<循环语句>");
                    midCodeTable.push_back(midCode(GOTO, "", "", labBegin));
                    midCodeTable.push_back(midCode(LABEL, "", "", labEnd));
                    return 1;
                }
            }
        }
    }
    //::= for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
    else if (wordType == "FORTK") { //FOR
        NextWord();
        if (wordType == "LPARENT") { //(
            NextWord();
            if (wordType == "IDENFR") { //标识符
                string name = getMin(word);
                SymItem id1 = judgeIdenfr(name); //对标志符进行判断
                NextWord();
                if (wordType == "ASSIGN") { // =
                    string value;
                    if (Expression(wordI, 0,value) != FAIL) {
                        NextWord();
                        midCodeTable.push_back(midCode(ASSIGN, value, "", name)); //赋值
                        if (wordType != "SEMICN") { //缺少;
                            ErrorKLM('k');
                        }
                        string result, labelEnd, labelBegin;
                        labelEnd = genLabel("End");
                        labelBegin = genLabel("Begin");
                        midCodeTable.push_back(midCode(LABEL, "", "", labelBegin)); //放置起始循环地点
                        if (Condition(wordI,result) != FAIL) { //条件
                            NextWord();
                            if (wordType != "SEMICN") { //;
                                ErrorKLM('k');
                            }
                            midCodeTable.push_back(midCode(BZ,result,"",labelEnd)); //不符合条件则跳转
                            NextWord();
                            string namel, namer; //name1,name2分别为标识符左和标识符右
                            int stepLength;
                            operation op;
                            if (wordType == "IDENFR") { //标识符
                                namel = getMin(word);
                                SymItem id2 = judgeIdenfr(namel); //对标志符进行判断
                                NextWord();
                                if (wordType == "ASSIGN") { //=
                                    NextWord();
                                    if (wordType == "IDENFR") { //标识符
                                        namer = getMin(word);
                                        SymItem id3 = judgeIdenfr(namer); //对标志符进行判断
                                        NextWord();
                                        op = wordType == "PLUS" ? PLUS : MINUS;
                                        if (wordType == "PLUS" || wordType == "MINU") { //+ / -
                                            stepLength = StepLength(wordI);
                                            if (stepLength!= FAIL) { //步长
                                                NextWord();
                                                if (wordType != "RPARENT") {
                                                    ErrorKLM('l');
                                                }
                                                if (Sentence(0) != FAIL) { //语句
                                                    midCodeTable.push_back(midCode(op, namer, int2string(stepLength), namel));//
                                                    midCodeTable.push_back(midCode(GOTO,  "", "", labelBegin));//返回begin处开始
                                                    midCodeTable.push_back(midCode(LABEL, "", "", labelEnd));
                                                    Out("<循环语句>");
                                                    return 1;
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜整数＞        ::= ［＋｜－］＜无符号整数＞
int YuFa::Integer() { //<整数>
    int number = NunInteger();
    if (number != FAIL) { //无符号整数
        Out("<整数>");
        return number;
    }
    else {
        NextWord();
        if (wordType == "PLUS") { //+ 无符号整数
            int number = NunInteger();
            if (number != FAIL) {
                Out("<整数>");
                return number;
            }
        }
        else if (wordType == "MINU") { // - 无符号整数
            int number = NunInteger();
            if (number != FAIL) {
                Out("<整数>");
                return -1 * number;
            }
        }
        else { //自动回退
            BackWord();
            return FAIL;
        }
    }
}

// ＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
//参数表里的应为临时变量
int YuFa::ParameterList(int init, string func) { //<参数表>
    NextWord();
    SymItem* funcSym;
    int type = 0;
    if (wordType == "CHARTK" || wordType == "INTTK") {
        funcSym = &globalSymTable[func];
        type = (wordType == "CHARTK") ? 3 : 2; //三目运算符确定type
        (*funcSym).insert(type);
        NextWord();
        if (wordType == "IDENFR") {
            word = getMin(word);
            if (tempSymTable.find(word) != tempSymTable.end()) { //已经存在
                Error('b', YuFaLineNumber);
            }
            else {
                midCodeTable.push_back(midCode(PARA, type == 3 ? "char" : "int", "", word));
                tempSymTable.insert(make_pair(word, SymItem(word, 2, type,tempAddr)));
                tempAddr++;
            }
            NextWord();
            while (word == ",") {
                NextWord();
                if (wordType == "CHARTK" || wordType == "INTTK") {
                    type = (wordType == "CHARTK") ? 3 : 2; //三目运算符确定type
                    (*funcSym).insert(type);
                    NextWord();
                    if (wordType == "IDENFR") { //添加
                        word = getMin(word);
                        if (tempSymTable.find(word) != tempSymTable.end()) { //已经存在
                            Error('b', YuFaLineNumber);
                        }
                        else {
                            midCodeTable.push_back(midCode(PARA, type == 3 ? "char" : "int", "", word));
                            tempSymTable.insert(make_pair(word, SymItem(word, 2, type,tempAddr)));
                            tempAddr++;
                        }
                        NextWord();
                    }
                }
            }
            BackWord(); //回退一个
            Out("<参数表>");
            return 1;
        }
        else {
            return FAIL;
        }
    }
    else if (wordType == "RPARENT") { //空语句 下一个为)
        Recover(init); //恢复初始化
        Out("<参数表>");
        return 1;
    }
    else if (wordType == "LBRACE") { //空语句，且下一个为{
        Recover(init);
        Out("<参数表>");
        return 1;
    }
    else {
        return FAIL;
    }
}

//＜情况子语句＞  ::=  case＜常量＞：＜语句＞
int YuFa::CaseSonSentence(int init, int type) { //<情况子语句>
    NextWord();
    int caseType = 0;
    if (wordType == "CASETK") { //CASE
        NextWord();
        if (wordType == "CHARCON") { //采用预读的方式
            caseType = 3;
            BackWord();
        }
        else {
            caseType = 2;
            BackWord();
        }
        if (caseType != type) { //不匹配
            Error('o', YuFaLineNumber);
        }
        int value = Const();
        if (value != FAIL) {
            NextWord();
            if (word == ":") {
                string label = genSwitchLabel();
                midCodeTable.push_back(midCode(LABEL, "", "", label));
                midCodeTable.push_back(midCode(CASE, int2string(value), "", label));
                if (Sentence(0) != FAIL) {
                    midCodeTable.push_back(midCode(GOTO, "", "", "SWITCH_END_" + int2string(allSwitchId)));
                    Out("<情况子语句>");
                    return 1;
                }
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}
int YuFa::CaseTable(int init, int type) { //<情况表>
    int times = 0;
    while (CaseSonSentence(wordI, type) != FAIL) {
        times++;
    }
    if (times >= 1) {
        Out("<情况表>");
        return 1;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’
int YuFa::CaseSentence(int init) { //<情况语句>
    NextWord();
    if (wordType == "SWITCHTK") { // swtich
        NextWord();
        if (wordType == "LPARENT") { // (
            string value;
            int switchType = Expression(wordI, 0,value); //表达式类型
            if (switchType != FAIL) {
                NextWord();
                if (wordType != "RPARENT") { //) 缺少
                    ErrorKLM('l');
                }
                NextWord();
                if (wordType == "LBRACE") { //{
                    midCodeTable.push_back(midCode(GOTO, "", "", "SWITCH_BEGIN_" + int2string(allSwitchId))); //跳转到switch
                    if (CaseTable(wordI, switchType) != FAIL) { //情况表
                        midCodeTable.push_back(midCode(LABEL, "", "", "SWITCH_BEGIN_" + int2string(allSwitchId)));
                        midCodeTable.push_back(midCode(SWITCH, "", "", value));
                        if (Default(wordI) != FAIL) {
                            NextWord();
                            if (wordType == "RBRACE") { //}
                                midCodeTable.push_back(midCode(LABEL, "", "", "SWITCH_END_" + int2string(allSwitchId)));
                                allSwitchId++;
                                Out("<情况语句>");
                                return 1;
                            }
                        }
                        else {//缺少Default
                            NextWord();
                            Error('p', YuFaLineNumber);
                            while (wordType != "RBRACE") { //往后读到}
                                NextWord();
                            }
                            return 1;
                        }
                    }
                }
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

// ＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
int YuFa::MultiSentence(int init, int funcType) { //<复合语句>
    ConstState(1);
    VariableState(1); //复合语句里的是临时变量
    if (SentenceList(wordI, funcType) != FAIL) {
        Out("<复合语句>");
        return 1;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
// | char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
void YuFa::ConstDefination(int index) { // <常量定义> //index 区分全局和临时，全局变量为0, 临时变量为1
    NextWord();
    if (wordType == "INTTK") { //int类型
        NextWord();
        if (wordType == "IDENFR") {
            string name = getMin(word);
            NextWord();
            if (wordType == "ASSIGN") { //=
                int number = Integer(); //记录数字类型
                if (number != FAIL) {
                    //midCodeTable.push_back(midCode(ASSIGN,int2string(number),"",name));//常量赋值
                    NextWord();
                    VarIdenfrInsert(name, 1, 2, number, ' ',index,1,0);
                    while (word == ",") {
                        NextWord();
                        if (wordType == "IDENFR") {
                            name = getMin(word); //更新name
                            NextWord();
                            if (wordType == "ASSIGN") { // =
                                number = Integer();
                                if (number != FAIL) { //是数字
                                    //midCodeTable.push_back(midCode(ASSIGN, int2string(number), "", name));//常量赋值
                                    VarIdenfrInsert(name, 1, 2, number,' ', index,1,0);
                                    // 成功 记录一下
                                    NextWord();
                                }
                            }
                        }
                    }
                    BackWord(); //回退一个;
                    Out("<常量定义>");
                }
            }
        }
    }
    else if (wordType == "CHARTK") { //char类型
        NextWord();
        if (wordType == "IDENFR") {
            string name = getMin(word);
            NextWord();
            if (wordType == "ASSIGN") { // =
                NextWord();
                if (wordType == "CHARCON") { //char
                    char tempChar = word[0]; //word 为字符串
                    string charValue = word;
                    VarIdenfrInsert(name, 1, 3, FAIL,tempChar, index,1,0);
                    //midCodeTable.push_back(midCode(ASSIGN, charValue, "", name));
                    NextWord(); //暂时记录一下

                    while (word == ",") {
                        NextWord();
                        if (wordType == "IDENFR") {
                            name = getMin(word);
                            NextWord();
                            if (wordType == "ASSIGN") { // =
                                NextWord();
                                if (wordType == "CHARCON") {
                                    tempChar = word[0];
                                    string charValue = word;
                                    //midCodeTable.push_back(midCode(ASSIGN, charValue, "", name));
                                    VarIdenfrInsert(name, 1, 3, FAIL, tempChar, index,1,0);
                                    NextWord();
                                }
                            }
                        }
                    }
                    BackWord(); //回退一个;
                    Out("<常量定义>");
                }
            }
        }
    }
}

//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
int YuFa::ConstState(int index) { //<常量说明> //修改添加index，若为0则是全局声明，为1则是局部声明
    NextWord();
    if (wordType == "CONSTTK") { //const
        ConstDefination(index); //常量定义
        NextWord();
        if (wordType != "SEMICN") { // ;
            ErrorKLM('k');
        }
        NextWord();
        while (wordType == "CONSTTK") { //const
            ConstDefination(index);
            NextWord();
            if (wordType != "SEMICN") { //缺少;
                ErrorKLM('k');
            }
            NextWord();
        }
        BackWord(); //预读结束，回退一格
        Out("<常量说明>");
        return 1;
    }
    else {
        BackWord();
        return FAIL;
    }
}

int YuFa::NunInteger() { //<无符号整数>
    NextWord();
    if (wordType == "INTCON") {
        Out("<无符号整数>");
        return stoi(word);
    }
    else {
        BackWord();
        return FAIL;
    }
}

/*＜变量定义无初始化＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'
['＜无符号整数＞']''['＜无符号整数＞']'){,(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']' )}*/
int YuFa::VariableDefinationWithoutInit(int init, int index) { //<变量定义无初始化>
    NextWord();
    int type = 0;
    if (wordType == "CHARTK" || wordType == "INTTK") { //类型标识符
        type = (wordType == "CHARTK") ? 3 : 2;
        NextWord();
        if (wordType == "IDENFR") { //标识符
            string name = getMin(word);
            int addr = 1;
            int length = 0; //二维数组长度
            int time = 0;
            NextWord();
            while (wordType == "LBRACK") { //[
                int arrayIndex = NunInteger();
                if (arrayIndex != FAIL) { //无符号整数
                    time++;
                    if (time == 2) {
                        length = arrayIndex; //记录第二维数组长度
                    }
                    addr += addr * arrayIndex; // array = 1*x*y
                    NextWord();
                    if (wordType != "RBRACK") { // ] 缺少
                        ErrorKLM('m');
                    }
                    NextWord();
                }
                else { //下标不为整数
                    Error('i', YuFaLineNumber);
                    while (wordType != "RBRACK") {
                        NextWord();
                    }
                    NextWord();
                }
            }
            while (wordType == "COMMA") { // ,
                NextWord();
                if (wordType == "IDENFR") { //标识符
                    addr = 1;
                    int tempLength = 0;
                    int tempTime = 0;
                    string tempName = getMin(word); //刻意不用name
                    NextWord();
                    while (wordType == "LBRACK") { // [
                        int arrayIndex = NunInteger();
                        if (arrayIndex != FAIL) { //无符号整数
                            tempTime++;
                            if (tempTime == 2) {
                                tempLength = arrayIndex;
                            }
                            addr += addr * arrayIndex;
                            NextWord();
                            if (wordType != "RBRACK") { // ]缺少
                                ErrorKLM('m');
                            }
                            NextWord();
                        }
                        else { //下标不为整数
                            Error('i', YuFaLineNumber);
                            while (wordType != "RBRACK") {
                                NextWord();
                            }
                            NextWord();
                        }
                    }
                    if(tempTime == 1)VarIdenfrInsert(tempName, 4, type, FAIL, ' ', index, addr, tempLength);
                    else if (tempTime == 2)VarIdenfrInsert(tempName, 5, type, FAIL, ' ', index, addr, tempLength);
                    else if (tempTime == 0)VarIdenfrInsert(tempName, 2, type, FAIL, ' ', index, addr, tempLength);
                }
            }
            if (wordType == "SEMICN") { // 是；
                if (time == 1)VarIdenfrInsert(name, 4, type, FAIL, ' ', index, addr, length);
                else if (time == 2)VarIdenfrInsert(name, 5, type, FAIL, ' ', index, addr, length);
                else if (time == 0)VarIdenfrInsert(name, 2, type, FAIL, ' ', index, addr, length);
                BackWord();
                Out("<变量定义无初始化>");
                return 1;
            }
            else {
                string nextWord = wordType;
                if (nextWord == "ASSIGN" || nextWord == "LPARENT" ||
                    nextWord == "LBRACK") { // int a = , int a[, int a(这种时候回退
                    Recover(init);
                    return FAIL;
                }
                else { //缺少;
                    Out("<变量定义无初始化>");
                    BackWord();
                    return 1;
                }
            }
        }
        else {
            Recover(init);
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜常量＞   ::=  ＜整数＞|＜字符＞
int YuFa::Const() { // <常量>
    int tempNumber = Integer();
    if (tempNumber != FAIL) { //数字
        Out("<常量>");
        return tempNumber;
    }
    else {
        NextWord();
        if (wordType == "CHARCON") {
            Out("<常量>");
            return int(word[0]);
        }
        else {
            BackWord();
            return FAIL;
        }
    }
}

/*＜变量定义及初始化＞ ::= ＜类型标识符＞＜标识符＞=＜常量＞|＜类型标识符＞＜标识符＞'['＜无符号整数＞']'
='{'＜常量＞{,＜常量＞}'}'|＜类型标识符＞＜标识符＞'
['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{,＜常量＞}'}'{, '{'＜常量＞{,＜常量＞}'}'}'}'*/
int YuFa::VariableDefinationWithInit(int init, int index) { // <变量定义及初始化>
    NextWord();
    if (wordType == "INTTK" || wordType == "CHARTK") { //类型标识符
        int tempType = (wordType == "INTTK") ? 2 : 3;
        NextWord();
        int time = 0; //记录嵌套次数
        if (wordType == "IDENFR") { //标识符
            string name = getMin(word); //记录变量名
            int length1 = FAIL, length2 = FAIL;//数组下标,初始化为FAIL
            int addr = 1;//用来给数组记录地址
            NextWord();
            while (wordType == "LBRACK") { // [
                time++;
                int tempLength = NunInteger();//数组长度
                if (tempLength != FAIL) {
                    NextWord();
                    addr += addr * tempLength;
                    if (wordType != "RBRACK") { // ]
                        ErrorKLM('m');
                    }
                    if (time == 1) { // 1为1元数组
                        length1 = tempLength;
                    }
                    if (time == 2) { //2 为2元数组
                        length2 = tempLength;
                    }
                    NextWord();
                }
                else {//数组元素的下表不为整数型。
                    Error('i', YuFaLineNumber);
                    while (wordType != "RBRACK") { //跳到下一个]
                        NextWord();
                    }
                    NextWord();
                }
            }
            if (wordType == "ASSIGN") { // = 
                VariableDefinationWithInitOError(tempType);
                int value = Const();
                if (value != FAIL) { //变量非数组赋值
                    VarIdenfrInsert(name, 2, tempType, value, char(value), index,addr,0);
                    midCodeTable.push_back(midCode(ASSIGN, int2string(value), "", name));// 加入赋值
                    Out("<变量定义及初始化>");
                    return 1;
                }
                else {
                    if (time == 1) { //一层括号,一维数组
                        NextWord();
                        if (wordType == "LBRACE") { // {
                            VariableDefinationWithInitOError(tempType);
                            int tempValue = Const(); //取得的值
                            int valueTimes = 0;
                            if (tempValue != FAIL) {
                                midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes), int2string(tempValue), name)); //加入数组
                                valueTimes++; //+次数判断是否匹配
                                NextWord();
                                while (wordType == "COMMA") { // ,
                                    VariableDefinationWithInitOError(tempType);
                                    tempValue = Const();
                                    if (tempValue != FAIL) {
                                        midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes), int2string(tempValue), name)); //加入数组
                                        valueTimes++;
                                        NextWord();
                                    }
                                }
                                if (wordType == "RBRACE") { //}
                                    if (valueTimes != length1) { //长度不匹配
                                        Error('n', YuFaLineNumber);
                                    }
                                    VarIdenfrInsert(name, 4, tempType, FAIL, ' ', index,addr,0);
                                    Out("<变量定义及初始化>");
                                    return 1;
                                }
                            }
                            else if (wordType == "LBRACE") {
                                Error('n', YuFaLineNumber);
                                while (wordType != "SEMICN") {
                                    NextWord();
                                }
                                BackWord();
                                Out("<变量定义及初始化>");
                                return 1;
                            }
                        }
                        else { //缺少 { 缺少某一维的元素
                            Error('n', YuFaLineNumber);
                            Out("<变量定义及初始化>");
                            return 1;
                        }
                    }
                    else if (time == 2) { //2层括号,二维数组
                        NextWord();
                        int valueTimes1 = 0, valueTimes2 = 0; //1为 {}次数，2为每个{}中的个数
                        if (wordType == "LBRACE") { //{
                            NextWord();
                            if (wordType == "LBRACE") { //{
                                valueTimes1++;
                                VariableDefinationWithInitOError(tempType);
                                int tempValue = Const();
                                if (tempValue != FAIL) {
                                    midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                    valueTimes2++;
                                    NextWord();
                                    while (wordType == "COMMA") { // ,
                                        VariableDefinationWithInitOError(tempType);
                                        tempValue = Const();
                                        if (tempValue != FAIL) {
                                            midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                            valueTimes2++;
                                            NextWord();
                                        }
                                    }
                                    if (valueTimes2 != length2) { //数组长度不匹配
                                        Error('n', YuFaLineNumber);
                                    }
                                    if (wordType == "RBRACE") { //}
                                        NextWord();
                                        while (wordType == "COMMA") { // ,
                                            valueTimes1++;
                                            NextWord();
                                            valueTimes2 = 0; //刷新一下
                                            if (wordType == "LBRACE") { //{
                                                VariableDefinationWithInitOError(tempType);
                                                tempValue = Const();
                                                if (tempValue != FAIL) { // 常量
                                                    midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                                    valueTimes2++;
                                                    NextWord();
                                                    while (wordType == "COMMA") { //，
                                                        VariableDefinationWithInitOError(tempType);
                                                        tempValue = Const();
                                                        if (tempValue != FAIL) { //常量
                                                            midCodeTable.push_back(midCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                                            valueTimes2++;
                                                            NextWord();
                                                        }
                                                    }
                                                    if (wordType == "RBRACE") { // }
                                                        if (valueTimes2 != length2) { //数组长度不匹配
                                                            Error('n', YuFaLineNumber);
                                                        }
                                                        NextWord();
                                                    }
                                                }
                                            }
                                        }
                                        if (wordType == "RBRACE") { //}
                                            if (valueTimes1 != length1) {
                                                Error('n', YuFaLineNumber);
                                            }
                                            VarIdenfrInsert(name, 5, tempType, FAIL, ' ', index,addr,length2); //插入变量
                                            Out("<变量定义及初始化>");
                                            return 1;
                                        }
                                    }
                                }
                                else if (wordType == "LBRACE") {
                                    Error('n', YuFaLineNumber);
                                    while (wordType != "SEMICN") {
                                        NextWord();
                                    }
                                    BackWord();
                                    Out("<变量定义及初始化>");
                                    return 1;
                                }
                            }
                            else {//缺少维度
                                Error('n', YuFaLineNumber);
                                NextWord();
                                Out("<变量定义及初始化>");
                                return 1;
                            }
                        }
                        else {
                            Error('n', YuFaLineNumber);
                            return FAIL;
                            Out("<变量定义及初始化>");
                            return 1;
                        }
                    }
                }
            }
            else {
                Recover(init);
                return FAIL;
            }
        }
        else {
            Recover(init);
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

int YuFa::MultiSymbol(int init) { //<乘法运算符>
    NextWord();
    if (wordType == "MULT") {
        return 1;
    }
    if (wordType == "DIV") {
        return 2;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

int YuFa::RelationSymbol() { //<关系运算符>
    NextWord();
    if (wordType == "EQL" || wordType == "NEQ" || wordType == "GEQ" || wordType == "LEQ" || wordType == "LSS" ||
        wordType == "GRE" || wordType == "ASSIGN") {
        return 1;
    }
    else {
        BackWord();
        return FAIL;
    }
}

//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
int YuFa::Term(int init, int isPara,string &value) { //<项> 返回Type
    int time = 0;
    string value1, value2;
    string type1, type2;
    int termType = Factor(wordI, isPara,value1); //因子1
    if (termType != FAIL) {
        int multi = MultiSymbol(wordI); //1是mult，2是div
        while (multi != FAIL) { 
            if ( Factor(wordI, isPara, value2) != FAIL) {
                time++;
               if ((value1[0] <= '9' && value1[0] >= '0' || value1[0] == '+' || value1[0] == '-')
                    && (value2[0] <= '9' && value2[0] >= '0' || value2[0] == '+' || value2[0] == '-'))
                {
                    int v1 = string2int(value1);
                    int v2 = string2int(value2);
                    if (multi == 1) { //是乘法
                        value1 = int2string(v1 * v2);
                    }
                    else if (multi == 2) { //是除法
                        value1 = int2string(v1 / v2);
                    }
                }
                else {
                    string tempId = genId();
                    tempSymTable.insert(make_pair(tempId, SymItem(tempId, 2, 2, tempAddr))); // kind == 2 , type == int
                    tempAddr++;
                    if (multi == 1) midCodeTable.push_back(midCode(MULT, value1, value2, tempId)); //乘法 x= value1,y=value2,tempID=result
                    else  midCodeTable.push_back(midCode(DIV, value1, value2, tempId)); //除法
                    value1 = tempId;
               }
                multi = MultiSymbol(wordI); // 更新multi
            }
            else {
                break;
            }
        }
        if (time >= 1) { //变成整数
            termType = 2;
        }
        value = value1;
        Out("<项>");
        return termType;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'   //TODO
int YuFa::VoidFunctionDefination(int init) { //＜无返回值函数定义＞
    NextWord();
    string wordName;
    if (wordType == "VOIDTK") {
        NextWord();
        if (wordType == "IDENFR") {
            SymItem funcItem;
            wordName = getMin(word); //函数名
            if (globalSymTable.find(wordName) != globalSymTable.end()) { //已经存在
                Error('b', YuFaLineNumber);
            }
            else {
                funcItem = SymItem(wordName, 3, 1, globalAddr);
                globalSymTable.insert(make_pair(wordName, funcItem));
                globalAddr++;
            }
            midCodeTable.push_back(midCode(FUNC, "void", "", wordName));
            NextWord();
            if (wordType == "LPARENT") { // (
                if (ParameterList(wordI, wordName) != FAIL) {
                    NextWord();
                    if (wordType != "RPARENT") { //不为)
                        ErrorKLM('l');
                    }
                    voidList.push_back(wordName);
                    NextWord();
                    if (wordType == "LBRACE") { // {
                        returnTime = 0;
                        if (MultiSentence(wordI, 1) != FAIL) { //注意是void
                            NextWord();
                            if (wordType == "RBRACE") {     //}
                                globalSymTable[wordName].length = tempAddr;
                                initTempTable(wordName); //传入函数名初始化
                                Out("<无返回值函数定义>");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        else {
            Recover(init);
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
int YuFa::ReturnFunctionDefination(int init) { //<有返回值函数定义>
    NextWord();
    string wordName,funType;
    if (wordType == "CHARTK" || wordType == "INTTK") {
        funType = wordType;
        NextWord();
        if (wordType == "IDENFR") {
            wordName = getMin(word);
        }
        Recover(init);
        int funcType = headAnnounce(); //在headAnnounce里定义
        if (funcType != FAIL) {
            NextWord();
            SymItem funcItem;
            midCodeTable.push_back(midCode(FUNC, funType == "CHARTK" ? "char" : "int", "", wordName));
            if (globalSymTable.find(wordName) != globalSymTable.end()) {
                Error('b', YuFaLineNumber);
            }
            else {
                funcItem = SymItem(wordName, 3, funcType, globalAddr);
                globalSymTable.insert(make_pair(wordName, funcItem));
                globalAddr++;
            }
            if (wordType == "LPARENT") { //(
                if (ParameterList(wordI, wordName) != FAIL) {
                    NextWord();
                    if (wordType != "RPARENT") { //)不存在
                        ErrorKLM('l');
                    }
                    NextWord();
                    if (wordType == "LBRACE") { //{
                        returnTime = 0;
                        if (MultiSentence(wordI, funcType) != FAIL) {  //传入返回类型
                            NextWord();
                            if (wordType == "RBRACE") { //}
                                globalSymTable[wordName].length = tempAddr;
                                initTempTable(wordName);//清空临时表
                                Out("<有返回值函数定义>");
                                return 1;
                            }
                        }
                    }
                }
            }
            else { //头部声明以后不为(
                Recover(init);
                return FAIL;
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

int YuFa::AddSymbol(int init) { //<加法运算符>
    NextWord();
    if (wordType == "PLUS" ){
        return 1;
    }
    if (wordType == "MINU") {
        return 2;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

// ＜缺省＞   ::=  default :＜语句＞
int YuFa::Default(int init) { //<缺省>
    NextWord();
    if (wordType == "DEFAULTTK") { //default
        NextWord();
        if (wordType == "COLON") { //:
            if (Sentence(0) != FAIL) {
                Out("<缺省>");
                return 1;
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜读语句＞    ::=  scanf '('＜标识符＞')'
int YuFa::ReadSentence(int init) { //<读语句>
    NextWord();
    SymItem readSym;
    if (wordType == "SCANFTK") { //scanf
        NextWord();
        if (wordType == "LPARENT") {
            NextWord();
            if (wordType == "IDENFR") {
                string name = getMin(word);
                if (tempSymTable.find(name) != tempSymTable.end()) { //找到了
                    readSym = tempSymTable[name];
                    if (readSym.kind == 1) { //const赋值
                        Error('j', YuFaLineNumber);
                    }
                }
                else {
                    if (globalSymTable.find(name) != globalSymTable.end()) { //全局变量里找到了
                        readSym = globalSymTable[name];
                        if (readSym.kind == 1) { //const赋值
                            Error('j', YuFaLineNumber);
                        }
                    }
                    else { //都没有找到 未定义
                        Error('c', YuFaLineNumber);
                    }
                }
                NextWord();
                if (wordType != "RPARENT") { //没接)
                    ErrorKLM('l');
                }
                midCodeTable.push_back(midCode(SCANF, "", "", name));
                Out("<读语句>");
                return 1;
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜语句列＞   ::= ｛＜语句＞｝
int YuFa::SentenceList(int init, int funcType) { //<语句列>
    int times = 0;
    while (Sentence(funcType) != FAIL) {
        times++;
    }
    if (times >= 0) {
        Out("<语句列>");
        return 1;
    }
    else {
        Recover(init);
        return FAIL;
    }
}

// ＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
int YuFa::WriteSentence(int init) { // <写语句>
    NextWord();
    if (wordType == "PRINTFTK") { //printf开头
        NextWord();
        if (wordType == "LPARENT") { //(
            string value = String();
            if (value != int2string(FAIL)) { //是str
                midCodeTable.push_back(midCode(PRINTF, "4", " ", value)); //4 代表输出的是字符串
                strList.push_back(value);
                NextWord();
                if (wordType == "RPARENT") { //) <字符串>
                    midCodeTable.push_back(midCode(PRINTF,  "5"," ", "NextLine")); //5 代表输下一行
                    Out("<写语句>");
                    return 1;
                }
                else if (word == ",") {  // <字符串>,<表达式>
                    string value;
                    int expType = Expression(wordI, 0, value);
                    if (expType != FAIL) { //<表达式>
                        NextWord();
                        if (wordType != "RPARENT") { //)没出现
                            ErrorKLM('l');;
                        }
                        midCodeTable.push_back(midCode(PRINTF, int2string(expType), " ", value)); //输出表达式
                        midCodeTable.push_back(midCode(PRINTF, "5", " ", "NextLine")); //5 代表输下一行
                        Out("<写语句>");
                        return 1;

                    }
                }
                else { //缺少)
                    ErrorKLM('l');
                    Out("<写语句>");
                    return 1;
                }
            }
            else { //<表达式>
                string expValue; //expression 的 value
                int expType = Expression(wordI, 0, expValue); //表达式的type
                if (expType != FAIL) {
                    NextWord();
                    if (wordType != "RPARENT") { //缺少)
                        ErrorKLM('l');
                    }
                    midCodeTable.push_back(midCode(PRINTF, int2string(expType), " ", expValue )); //输出表达式
                    midCodeTable.push_back(midCode(PRINTF, "5"," ", "NextLine" )); //5 代表输下一行
                    Out("<写语句>");
                    return 1;
                }
            }
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞
int YuFa::ValueList(int init, string funcName) { //<值参数表>
    int time = 0;
    SymItem funcSym = globalSymTable[funcName];
    vector<int> typeList;
    vector<int> tempList = funcSym.paraList;
    string value;
    int length = funcSym.paraList.size();
    int type = Expression(wordI, 0,value);
    if (type != FAIL) { //表达式
        midCodeTable.push_back(midCode(USE, "", "", value));
        typeList.push_back(type);
        time++;
        NextWord();
        while (word == ",") {
            type = Expression(wordI, 0,value); //复用value
            if (type != FAIL) {
                midCodeTable.push_back(midCode(USE, "", "", value));
                typeList.push_back(type);
                time++;
                NextWord();
            }
        }
        if (time != length) {
            Error('d', YuFaLineNumber);
        }
        else {
            for (int i = 0; i < time; i++) {
                if (typeList[i] != tempList[i]) {
                    Error('e', YuFaLineNumber);
                    break;
                }
            }
        }
        BackWord();
        Out("<值参数表>");
        return 1;
    }
    else {
        NextWord();
        if (wordType == "RPARENT") { //空的情况 ） 这里是预读不用判
            if (tempList.size() != 0) {
                Error('d', YuFaLineNumber);
            }
            BackWord();
            Out("<值参数表>");
            return 1;
        }
        else { //在此处调用的说明一定为值参数表，只是为空且缺少)
            Recover(init);
            return 1;
        }
    }
}

//＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
int YuFa::ReturnSentence(int init) { //<有返回值函数调用语句>
    NextWord();

    if (wordType == "IDENFR") {
        string funcName = getMin(word);
        SymItem funcSym;
        NextWord();
        if (wordType == "LPARENT") { //(
            if (globalSymTable.find(funcName) == globalSymTable.end()) {//没找到
                Error('c', YuFaLineNumber);
            }
            else {
                funcSym = globalSymTable[funcName];
            }
            if (funcSym.kind == 0) { //没找到
                while (wordType != "SEMICN") { //跳到;
                    NextWord();
                }
                return FAIL;
            }
        }
        BackWord();
        if (funcSym.kind == 3 && funcSym.type != 1) {//确定是函数调用
            if (findString(funcName, ReturnList)) { //在有返回值函数中出现过
                NextWord();
                if (wordType == "LPARENT") { //(
                    if (ValueList(wordI, funcName) != FAIL) {
                        NextWord();
                        if (wordType != "RPARENT") { //没有)
                            ErrorKLM('l');
                        }
                        int returnValue = funcSym.type; //获取函数返回类型
                        midCodeTable.push_back(midCode(CALL, "", "", funcName));
                        Out("<有返回值函数调用语句>");
                        return returnValue;

                    }
                }
            }
        }
        else {
            Recover(init);
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL; //原谅他
    }
}

//＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
int YuFa::VoidSentence(int init) {//<无返回值函数调用语句>
    NextWord();
    if (wordType == "IDENFR") {
        string funcName = getMin(word);
        NextWord();
        SymItem voidSym;
        if (wordType == "LPARENT") { //(
            voidSym = judgeIdenfr(funcName);
            if (voidSym.kind == 0) { //没找到
                while (wordType != "SEMICN") { //跳到;
                    NextWord();
                }
                return FAIL;
            }
        }
        BackWord();
        if (voidSym.type == 1 && voidSym.kind == 3) { //是void
            if (findString(getMin(funcName), voidList)) { //在无返回值函数中出现过
                NextWord();
                if (wordType == "LPARENT") { // (
                    if (ValueList(wordI, funcName) != FAIL) {
                        NextWord();
                        if (wordType != "RPARENT") { //)没有
                            ErrorKLM('l');
                        }
                        midCodeTable.push_back(midCode(CALL, "", "", funcName));
                        Out("<无返回值函数调用语句>");
                        return 1;
                    }
                }
                else {
                    Recover(init); //只有一个IDENFR可能有交集
                    return FAIL;
                }
            }
        }
        else {
            Recover(init);
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜返回语句＞   :: = return['('＜表达式＞')']
int YuFa::BackSentence(int init, int funcType) { //<返回语句>
    NextWord();
    if (wordType == "RETURNTK") { // return
        NextWord();
        if (wordType == "LPARENT") { //（
            NextWord();
            if (funcType == 1) { //void类型
                Error('g', YuFaLineNumber);
            }
            if (wordType == "RPARENT") { //只有()
                if (funcType == 2 || funcType == 3) {
                    Error('h', YuFaLineNumber);
                }
                if (isMain == false) {
                    midCodeTable.push_back(midCode(RET, "", "", int2string(FAIL)));//只有有返回值的函数需要RET
                }
                else if (isMain == true) {
                    midCodeTable.push_back(midCode(EXIT, "", "", 0));
                }
                Out("<返回语句>");
                returnTime++;
                return 1;
            }
            BackWord();
            string value;
            int returnType = Expression(wordI, 0,value); //临时记录变量
            if (returnType != FAIL) {
                NextWord();
                if (returnType != funcType && (funcType == 2 || funcType == 3)) { //h型错误
                    Error('h', YuFaLineNumber);
                }
                if (wordType != "RPARENT") { //) 缺少
                    ErrorKLM('l');
                }
                returnTime++;
                midCodeTable.push_back(midCode(RET, "", "", value));//只有有返回值的函数需要RET
                
                Out("<返回语句>");
                return 1;

            }
        }
        else {
            BackWord();
            if (funcType == 3 || funcType == 2) {
                Error('h', YuFaLineNumber); //h型错误
            }
            returnTime++;
            if (isMain == true) {
                midCodeTable.push_back(midCode(EXIT, "", "", ""));
            }
            else{
                midCodeTable.push_back(midCode(RET, "", "", int2string(FAIL)));
            }
            Out("<返回语句>"); //只有Return 语句
            return 1;
        }
    }
    else { //没有return
        if ((funcType == 2 || funcType == 3) && returnTime == 0) { //没有return的情况
            if (wordType == "RBRACE") { //}
                Error('h', YuFaLineNumber);
            }
        }
        Recover(init);
        return FAIL;
    }
}

// ＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
//返回表达式类型，但是为了获得表达式的值，我们需要传地址value
int YuFa::Expression(int init, int isPara,string &value) { //<表达式>
    NextWord();
    int time = 0;
    if (wordType == "PLUS" || wordType == "MINU") { //-/+
        operation op = wordType == "PLUS" ? PLUS : MINUS;
        string value1,value2;
        int expressType = Term(wordI, isPara,value1);
        if (expressType != 0) {
            string tempId = genId();
            tempSymTable.insert(make_pair(tempId, SymItem(tempId, 2, 2, tempAddr)));// type == int kind == var
            tempAddr++;
            midCodeTable.push_back(midCode(op, "0", value1, tempId));
            value1 = tempId; //重新赋值
            int tempOp = AddSymbol(wordI);//1是加 2是减
            op = tempOp == 1 ? PLUS : MINUS;
            while (tempOp != FAIL) {
                if (Term(wordI, isPara, value2) != FAIL) { //赋值value2
                    time++;
                    if ((value1[0] <= '9' && value1[0] >= '0' || value1[0] == '+' || value1[0] == '-') 
                        && (value2[0] <= '9' && value2[0] >= '0' || value2[0] == '+' || value2[0] == '-')) {
                        int v1 = string2int(value1);
                        int v2 = string2int(value2);
                        if (op == PLUS) {
                            value1 = int2string(v1 + v2);
                        }
                        else {
                            value1 = int2string(v1 - v2);
                        }
                    }
                    else {
                        string tempId = genId();
                        tempSymTable.insert(make_pair(tempId, SymItem(tempId, 2, 2, tempAddr)));// type == int kind == var
                        tempAddr++;
                        midCodeTable.push_back(midCode(op, value1, value2, tempId));
                        value1 = tempId;//重新赋值value1，得到累加的效果
                    }
                    int tempOp = AddSymbol(wordI);//1是加 2是减
                    op = tempOp == 1 ? PLUS : MINUS;
                }
                else {
                    break;
                }
            }
            if (time >= 1) {
                expressType = 2; // 2是int, 3是char
            }
            value = value1;
            Out("<表达式>");
            return expressType;
        }
        else {
            Recover(init); //不是表达式
            return FAIL;
        }
    }
    else {
        BackWord();
        string value1, value2;
        int expressType = Term(wordI, isPara,value1);
        if (expressType != FAIL) {
            int tempOp = AddSymbol(wordI);//1是加 2是减
            operation op = tempOp == 1 ? PLUS : MINUS;
            while (tempOp != FAIL) {
                if (Term(wordI, isPara,value2) != FAIL) {
                    time++;
                    string tempId = genId();
                    tempSymTable.insert(make_pair(tempId, SymItem(tempId, 2, 2, tempAddr)));// type == int kind == var
                    tempAddr++;
                    midCodeTable.push_back(midCode(op, value1, value2, tempId));
                    value1 = tempId;//重新赋值value1，得到累加的效果
                    int tempOp = AddSymbol(wordI);//1是加 2是减
                    op = tempOp == 1 ? PLUS : MINUS;
                }
                else {
                    break;
                }
            }
            if (time >= 1) {
                expressType = 2;
            }
            value = value1;
            Out("<表达式>");
            return expressType;
        }
        else {
            Recover(init); //不是表达式
            return FAIL;
        }
    }
}

//＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|＜标识符＞'
// ['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
//要返回类型 1 为void 2为int 3为char
int YuFa::Factor(int init, int isPara,string &value) { //<因子>
    NextWord();
    SymItem factorSym;
    int factorType = 0;
    if (wordType == "IDENFR") { //第一个单词为标识符
        string name = getMin(word);
        if (isPara == 0) { //不是参数
            factorSym = judgeIdenfr(name);
        }
        else { // 是参数 ToDo
            tempSymTable.insert(make_pair(word, SymItem(name, 2,0,tempAddr))); //只知道是var
            tempAddr++;
        }
        NextWord();
        if (wordType == "LPARENT") { //是(  说明为调用有返回值的语句 //
            Recover(init); //恢复至开始的水平
            factorType = ReturnSentence(wordI); //
            if (factorType != FAIL) {
                string tmpName = genId();
                tempSymTable.insert(make_pair(tmpName, SymItem(tmpName, 2, factorType, tempAddr))); 
                tempAddr++;
                midCodeTable.push_back(midCode(RETVALUE, "", "", tmpName)); //加入RETVALUE
                value = tmpName;
                Out("<因子>");
                return factorType;
            }
        }
        else { //数组 或 标识符
            int arrayTime = 0;
            string arrayY;
            while (word == "[") { //[ 数组
                string value;
                int expressionType = Expression(wordI, 0, value);
                if (expressionType != FAIL) { //表达式
                    if (expressionType != 2) { //下标不为整形
                        Error('i', YuFaLineNumber);
                    }
                    NextWord();
                    if (word != "]") { //]缺少
                        ErrorKLM('m');
                    }
                    arrayTime++; //数组次数+1
                    if (arrayTime == 1) {
                        arrayY = value;
                    }
                    else if (arrayTime == 2) {
                        arrayY = arrayY + " " + value;//i,j坐标
                    }
                    NextWord();
                }
            }
            if (arrayTime == 0) { //只有标识符，则直接复制标识符名字
                value = name;
            }
            if (arrayTime >= 1) {
                string tempId = genId();
                tempSymTable.insert(make_pair(tempId, SymItem(tempId, 2, factorSym.type, tempAddr)));//kind = 2变量
                tempAddr++;
                midCodeTable.push_back(midCode(GETARRAY,  name, arrayY, tempId));
                value = tempId;
            }
            BackWord();
            Out("<因子>");
            factorType = factorSym.type;
            return factorType;
        }

    }
    else if (wordType == "LPARENT") { //第一个单词为(
        factorType = Expression(wordI, 0,value); //直接进入表达式进行分析
        if (factorType != FAIL) {
            NextWord();
            if (wordType != "RPARENT") { //)
                ErrorKLM('l');
            }
            Out("<因子>");
            factorType = 2;//(<表达式>)是int
            return factorType;
        }
    }
    else if (wordType == "CHARCON") { //<字符>
        value = int2string(charCon); // 字符串直接赋值
        Out("<因子>");
        return 3;
    }
    else {
        BackWord();
        int intValue = Integer();
        if (intValue != FAIL) {  //<整数>
            value = int2string(intValue); //整数直接赋值。
            Out("<因子>");
            return 2;
        }
        else {
            Recover(init); // 回退
            return FAIL;
        }
    }

}

//＜条件＞    :: = ＜表达式＞＜关系运算符＞＜表达式＞
int YuFa::Condition(int init,string &result) {  //<条件>
    result = genId();
    tempSymTable.insert(make_pair(result, SymItem(result, tempAddr, 2, 2)));// kind = var, type = int
    tempAddr++;
    string value1, value2;
    int conditionType = Expression(wordI, 0,value1);
    if (conditionType != FAIL) {
        if (conditionType != 2) { //不为整形
            Error('f', YuFaLineNumber);
        }
        if (RelationSymbol() != FAIL) { //是关系运算符
            string symbol = wordType;//保存一下类型
            operation op;
            int conditionType = Expression(wordI, 0,value2);
            if (conditionType != 2) { //如果判断条件不是整形
                Error('f', YuFaLineNumber);
            }
            if (symbol == "LSS") {
                op = LSS;
            }
            else if (symbol == "LEQ") {
                op = LEQ;
            }
            else if (symbol == "GRE") {
                op = GRE;
            }
            else if (symbol == "GEQ") {
                op = GEQ;
            }
            else if (symbol == "EQL") { 
                op = EQL;
            }
            else if (symbol == "NEQ") {
                op = NEQ;
            }
            midCodeTable.push_back(midCode(op, value1, value2, ""));//value1 = x value2 = y
            Out("<条件>");
            return 1;
        }
        else {
            Recover(init); //先保守一下
            return FAIL;
        }
    }
    else { //说明不是条件恢复即可
        Recover(init);
        return FAIL;
    }
}

/*＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;  |＜无返回值函数调用语句＞;
｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'  */
int YuFa::Sentence(int funcType) { //<语句>
    NextWord();
    if (wordType == "SEMICN") { //空语句情况
        Out("<语句>");
        return 1;
    }
    else if (wordType == "LBRACE") { //语句列 {
        if (SentenceList(wordI, funcType) != FAIL) {
            NextWord();
            if (wordType == "RBRACE") { //}
                Out("<语句>");
                return 1;
            }
        }
    }
    else {
        BackWord();
        if (ConditionSentence(wordI, funcType) != FAIL || LoopSentence(wordI) != FAIL ||
            CaseSentence(wordI) != FAIL) { //条件语句/ 条件语句/情况语句
            Out("<语句>");
            return 1;
        }
        else if (ReturnSentence(wordI) != FAIL || VoidSentence(wordI) != FAIL || AssignSentence(wordI) != FAIL ||
            ReadSentence(wordI) != FAIL || WriteSentence(wordI) != FAIL ||
            BackSentence(wordI, funcType) != FAIL) {
            NextWord();
            if (wordType != "SEMICN") { //;
                ErrorKLM('k');
            }
            Out("<语句>");
            return 1;
        }
        else {
            return FAIL;
        }
    }
}

//＜条件语句＞  :: = if '('＜条件＞')'＜语句＞［else＜语句＞］
int YuFa::ConditionSentence(int init, int funcType) { //<条件语句>
    NextWord();
    if (wordType == "IFTK") { //一定要if开头
        NextWord();
        if (wordType == "LPARENT") { //(
            string result;
            if (Condition(wordI,result) != FAIL) {
                NextWord();
                if (wordType != "RPARENT") { //) 缺少的话报错
                    ErrorKLM('l');
                }
                string labelEnd,labelElse;
                labelElse = genLabel("else");
                labelEnd = genLabel("end");
                midCodeTable.push_back(midCode(BZ,  "", "", labelElse)); //不符合条件则跳转
                if (Sentence(0) != FAIL) { //语句
                    NextWord();
                    if (wordType == "ELSETK") { //ELSE
                        midCodeTable.push_back(midCode(GOTO, "", "", labelEnd));
                        midCodeTable.push_back(midCode(LABEL, "", "", labelElse));
                        if (Sentence(0) != FAIL) { //语句
                            midCodeTable.push_back(midCode(LABEL, "", "", labelEnd));
                            Out("<条件语句>");
                            return 1;
                        }
                    }
                    else {
                        midCodeTable.push_back(midCode(LABEL, "", "", labelElse));
                        BackWord();
                        Out("<条件语句>");
                        return 1;
                    }
                }
            }
        }
    }
    else {
        BackWord();
        return FAIL;
    }
}

//＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞|＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞
int YuFa::AssignSentence(int init) { //<赋值语句>
    NextWord();
    if (wordType == "IDENFR") {
        string name = getMin(word);
        SymItem assignSym;
        NextWord();
        int time = 0; //用来判断维度，是变量/1维/2维
        string arrayX; //用来记录数组的下标
        while (word == "[") { //[
            string tempValue;
            int type = Expression(wordI, 0, tempValue);
            if (type != FAIL) { //]
                if (type != 2) { //下标不是整数
                    Error('i', YuFaLineNumber);
                }
                NextWord();
                if (word != "]") { //缺少 ]
                    ErrorKLM('m');
                }
                time++;//计数器加一
                if (time == 1) {
                    arrayX = tempValue;
                }
                else if (time == 2) {
                    arrayX =arrayX+ " " + tempValue;
                }
                NextWord();
            }
        }
        if (wordType == "ASSIGN") {
            if (tempSymTable.find(name) == tempSymTable.end()) { //临时变量里没找到
                if (globalSymTable.find(name) == globalSymTable.end()) { //全局变量里没找到
                    Error('c', YuFaLineNumber);
                }
                else { //全局变量里找到了
                    assignSym = globalSymTable[name];
                    if (assignSym.kind == 1) { //如果是CONST,给CONST赋值报错
                        Error('j', YuFaLineNumber);
                    }
                }
            }
            else { //局部变量里找到了
                assignSym = tempSymTable[name];
                if (assignSym.kind == 1) { //如果是CONST,给CONST赋值报错
                    Error('j', YuFaLineNumber);
                }
            }
            string value; //表达式的值
            if (Expression(wordI, 0,value) != FAIL) {
                if (time == 0 ) { //变量赋值且不是数组给它赋值
                    midCodeTable.push_back(midCode(ASSIGN, value, "", name));//name = value
                }
                else if (time >= 1 ) { //给数组赋值
                    midCodeTable.push_back(midCode(PUTARRAY,arrayX , value, name));// name[arrayX] = value 
                }
                
                Out("<赋值语句>");
                return 1;
            }
        }
        else {
            Recover(init); //只有一个标识符 继续
            return FAIL;
        }
    }
    else {
        Recover(init);
        return FAIL;
    }
}

//＜声明头部＞   :: = int＜标识符＞ | char＜标识符＞
//函数声明均为全局
int YuFa::headAnnounce() { //<声明头部> //若为Int 返回2 若为char 返回3
    NextWord();
    int funcType = 0;
    if (wordType == "INTTK" || wordType == "CHARTK") {
        if (wordType == "INTTK") {
            funcType = 2;
        }
        else if (wordType == "CHARTK") {
            funcType = 3;
        }
        NextWord();
        if (wordType == "IDENFR") { //标识符
            word = getMin(word);
            ReturnList.push_back(word);
            Out("<声明头部>");
            return funcType;
        }
    }
    else {
        BackWord();
        return FAIL;
    }
}

//＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
int YuFa::MainFunction(int init) { //<主函数>
    NextWord();
    if (wordType == "VOIDTK") { //void
        NextWord();
        if (wordType == "MAINTK") { //main
            SymItem mainItem = SymItem("main", 3, 1, globalAddr);
            globalAddr++;
            midCodeTable.push_back(midCode(FUNC, "void", "", "main"));//
            NextWord();
            if (wordType == "LPARENT") { //(
                NextWord();
                if (wordType != "RPARENT") { //应为)
                    ErrorKLM('l');
                }
                NextWord();
                if (wordType == "LBRACE") { //{
                    isMain = true;
                    if (MultiSentence(wordI, 1) != FAIL) {
                        NextWord();
                        if (wordType == "RBRACE") { //}
                            mainItem.length = tempAddr; //设置地址
                            globalSymTable.insert(make_pair("main", mainItem));
                            initTempTable("main");//main函数的临时变量
                            Out("<主函数>");
                            return 1;
                        }
                    }
                }
            }
        }
    }
}

//＜变量定义＞ ::= ＜变量定义无初始化＞|＜变量定义及初始化＞
int YuFa::VariableDefination(int index) { // <变量定义>
    if (VariableDefinationWithInit(wordI, index) != FAIL) {
        Out("<变量定义>");
        return 1;
    }
    else if (VariableDefinationWithoutInit(wordI, index) != FAIL) {
        Out("<变量定义>");
        return 1;
    }
    else {
        return FAIL;
    }
}

//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
void YuFa::VariableState(int index) { //<变量说明>
    int times = 0;
    while (VariableDefination(index) != FAIL) {
        NextWord();
        if (wordType == "SEMICN") { //;
            times++;
        }
        else {
            ErrorKLM('k');
            break;
        }
    }
    if (times >= 1) {
        Out("<变量说明>");
    }
}

//＜程序＞    :: = ［＜常量说明＞］［＜变量说明＞］{ ＜有返回值函数定义＞ | ＜无返回值函数定义＞ }＜主函数＞
void YuFa::ProgramAnalyse() { //<程序>
    ConstState(0); //常量说明
    VariableState(0); //变量说明 //0是全局变量
    initTempTable("global"); //全局变量声明函数
    midCodeTable.push_back(midCode(GOTO, "", "", "main"));
    while (ReturnFunctionDefination(wordI) != FAIL || VoidFunctionDefination(wordI) != FAIL) {
        ;
    }
    if (MainFunction(wordI)) {
        Out("<程序>");
    }
}