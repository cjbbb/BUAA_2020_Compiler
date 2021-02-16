#include <string>
#include<vector>
using namespace std;

#ifndef CIFA_H
#define CIFA_H

class CiFa{
public:

	void CiFaAnalyse(string in);
	vector<string> getWordsType();
	vector<string> getWords();
private:
	void handlCifaElse(char c,string strType);
	void outStr(string word,string wordTpe);
	int isHeld(string in);
	char getLower(char in); // get the lower
	string getToString(string in, int begin, int end); // sub the string which is in[begin,end)
};
//variable
//function



#endif