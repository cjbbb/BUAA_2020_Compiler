#include <fstream>
#include <iostream>
#include <vector>
#include<string>
#include <fstream>
#include <sstream>
#include "Cifa.h"
#include "Extern.h"
#include "Error.h"

using namespace std;
int lineNumber = 1;
vector<int> lineNumberList;

string hold[15] =
{
        "const", "int", "char", "void", "main",
        "if", "else", "switch", "case", "default",
        "while", "for", "scanf", "printf", "return"
};
string holdKey[15] =
{
        "CONSTTK", "INTTK", "CHARTK", "VOIDTK", "MAINTK",
        "IFTK", "ELSETK", "SWITCHTK", "CASETK", "DEFAULTTK",
        "WHILETK", "FORTK", "SCANFTK", "PRINTFTK", "RETURNTK"
};
string replace(string str) {
    string ans = "";
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '\\') {
            ans += "\\\\";
        }
        else {
            ans += str[i];
        }
    }
    return ans;
}
char CiFa::getLower(char in) // get the lower
{
    char ans;
    if (in <= 'Z' && in >= 'A') {
        ans = in - 'A' + 'a';
    }
    else {
        ans = in;
    }

    return ans;
}

string CiFa::getToString(string in, int begin, int end) // sub the string which is in[begin,end)
{
    int tempLength = end - begin + 3;
    char ans[tempLength];
    for (int i = 0; i < end - begin + 3; i++) {
        ans[i] = 0;
    }
    for (int i = begin; i < end; i++) {
        ans[i - begin] = in[i];
    }
    return ans;
}

int CiFa::isHeld(string in)  //判断是否是hold里的元素
{
    for (int j = 0; j < in.size(); j++) {
        in[j] = getLower(in[j]);
    }
    for (int i = 0; i < 15; i++) {
        if (in == hold[i]) {
            return i;
        }
    }
    return -1;
}

void CiFa::outStr(string word, string wordType) {
    words.push_back(word);
    wordsType.push_back(wordType);
    lineNumberList.push_back(lineNumber);
}

void CiFa::handlCifaElse(char c, string in) {
    string s;
    s.push_back(c);
    words.push_back(s);
    wordsType.push_back(in);
    lineNumberList.push_back(lineNumber);
}

vector<string> CiFa::getWords() {
    return words;
}

vector<string> CiFa::getWordsType() {
    return wordsType;
}

void CiFa::CiFaAnalyse(string in) {
    int i = 0;
    while (i < in.size()) {
        //for (int i =0;i<in.size();i++){
        char c = in[i];
        if (c == ' ' || c == '\t' || c == '\n') {
            if (c == '\n') {
                lineNumber++;
            }
            i++;
            continue;
        }
        else if (isalpha(c) != 0 || c == '_')  //is alpha
        {
            int tempi = i;
            if (i + 1 < in.size()) {
                i++;
                char d = in[i];
                while (isalpha(d) != 0 || isalnum(d) != 0 || d == '_')  //number or alpha
                {
                    if (i + 1 < in.size()) {
                        i++;
                        d = in[i];
                    }
                    else {
                        break;
                    }
                }
                string tempWord = getToString(in, tempi, i);
                string tempWordType;
                int isheld = isHeld(tempWord);
                if (isheld == -1) {
                    tempWordType = "IDENFR";
                    //wordsType.push_back("IDENFR");
                }
                else {
                    tempWordType = holdKey[isheld];
                    //wordsType.push_back(holdKey[isheld]);
                }
                outStr(tempWord, tempWordType);
                continue;
            }
        }
        else if (isalnum(c) != 0)  //is number
        {
            int tempi = i;
            if (i + 1 < in.size()) {
                i++;
                char d = in[i];
                while (isalnum(d) != 0) {
                    if (i + 1 < in.size()) {
                        i++;
                        d = in[i];
                    }
                    else {
                        break;
                    }
                }
                string tempWord = getToString(in, tempi, i);
                outStr(tempWord, "INTCON");
                //words.push_back(tempWords);
                //wordsType.push_back("INTCON");
                continue;
            }

        }
        else if (c == '"')  //string const
        {
            int tempi = i;
            if (i + 1 < in.size()) {
                i++;
                char d = in[i];

                if (d == '"') { //空串 Error(A)
                    Error('a', lineNumber);
                }

                while (d != '"') {
                    if (!(d == 32 || d == 33 || (d <= 126 && d >= 35))) { //不合法 字符串非法字符
                        Error('a', lineNumber);
                    }
                    if (i + 1 < in.size()) {
                        i++;
                        d = in[i];
                    }
                    else {
                        break;
                    }

                }
                string tempWord = getToString(in, tempi + 1, i); //remove "
                tempWord = replace(tempWord);

                i++;
                outStr(tempWord, "STRCON");
                //words.push_back(tempWords);
                //wordsType.push_back("STRCON");
            }

        }
        else if (c == '\'')  //char const
        {
            if (i + 2 < in.size()) {
                int tempi = i;
                i++;
                char d = in[i];
                if (!(d == '+' || d == '-' || d == '*' || d == '/' || isalnum(d) || isalpha(d) || d == '_')) {
                    Error('a', lineNumber); //a 类错误 字符不为 + - 字母 数字
                }
                i++;
                char e = in[i];
                if (e == '\'') {
                    string tempWord = getToString(in, tempi + 1, i);
                    outStr(tempWord, "CHARCON");
                    //words.push_back(tempWord);
                    //wordsType.push_back("CHARCON");
                    i++;
                }
            }
        }
        else if (c == '!') //!=
        {
            if (i + 1 < in.size()) {
                int tempi = i;
                i++;
                char d = in[i];
                if (d == '=') {
                    i++;
                    string tempWord = getToString(in, tempi, i);
                    outStr(tempWord, "NEQ");
                    //words.push_back(tempWord);
                    //wordsType.push_back("NEQ");
                }
            }
        }
        else if (c == '<') // < | <=
        {
            if (i + 1 < in.size()) {
                int tempi = i;
                i++;
                char d = in[i];
                if (d == '=') {
                    i++;
                    string tempWord = getToString(in, tempi, i);
                    outStr(tempWord, "LEQ");
                    //words.push_back(tempWords);
                    //wordsType.push_back("LEQ");
                }
                else {
                    handlCifaElse(c, "LSS");
                    //wordsType.push_back("LSS");
                }
            }
            else {
                handlCifaElse(c, "LSS");
                //wordsType.push_back("LSS");
                i++;
            }
        }
        else if (c == '>') // >= | >
        {
            if (i + 1 < in.size()) {
                int tempi = i;
                i++;
                char d = in[i];
                if (d == '=') {
                    i++;
                    string tempWord = getToString(in, tempi, i);
                    outStr(tempWord, "GEQ");
                    //words.push_back(tempWords);
                    //wordsType.push_back("GEQ");
                }
                else {
                    handlCifaElse(c, "GRE");
                    //wordsType.push_back("GRE");
                }
            }
            else {
                handlCifaElse(c, "GRE");
                //wordsType.push_back("GRE");
                i++;
            }
        }
        else if (c == '=') // == | =
        {
            if (i + 1 < in.size()) {
                int tempi = i;
                i++;
                char d = in[i];
                if (d == '=') {
                    i++;
                    string tempWord = getToString(in, tempi, i);
                    outStr(tempWord, "EQL");
                    //words.push_back(tempWords);
                    //wordsType.push_back("EQL");
                    continue;
                }
                else {
                    handlCifaElse(c, "ASSIGN");
                    //wordsType.push_back("ASSIGN");
                }
            }
            else {
                handlCifaElse(c, "ASSIGN");
                //wordsType.push_back("ASSIGN");
                i++;
            }
        }
        else if (c == '+') {
            i++;
            handlCifaElse(c, "PLUS");
            //wordsType.push_back("PLUS");
        }
        else if (c == '-') {
            i++;
            handlCifaElse(c, "MINU");
            //wordsType.push_back("MINU");
        }
        else if (c == '*') {
            i++;
            handlCifaElse(c, "MULT");
            //wordsType.push_back("MULT");
        }
        else if (c == '/') {
            i++;
            handlCifaElse(c, "DIV");
            //wordsType.push_back("DIV");
        }
        else if (c == ';') {
            i++;
            handlCifaElse(c, "SEMICN");
            //wordsType.push_back("SEMICN");
        }
        else if (c == ',') {
            i++;
            handlCifaElse(c, "COMMA");
            //wordsType.push_back("COMMA");
        }
        else if (c == '(') {
            i++;
            handlCifaElse(c, "LPARENT");
            //wordsType.push_back("LPARENT");
        }
        else if (c == ')') {
            i++;
            handlCifaElse(c, "RPARENT");
            //wordsType.push_back("RPARENT");
        }
        else if (c == '[') {
            i++;
            handlCifaElse(c, "LBRACK");
            //wordsType.push_back("LBRACK");
        }
        else if (c == ']') {
            i++;
            handlCifaElse(c, "RBRACK");
            //wordsType.push_back("RBRACK");
        }
        else if (c == '{') {
            i++;
            handlCifaElse(c, "LBRACE");
            //wordsType.push_back("LBRACE");
        }
        else if (c == '}') {
            i++;
            handlCifaElse(c, "RBRACE");
            //wordsType.push_back("RBRACE");
        }
        else if (c == ':') {
            i++;
            handlCifaElse(c, "COLON");
            //wordsType.push_back("COLON");
        }
        else {
            i++;
        }
    }
}
