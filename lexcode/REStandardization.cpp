#include "stdafx.h"
#include "structs.h"
#include "GlobalData.hpp"

using std::stack;
using std::vector;
using std::map;
using std::set;
using std::string;
using std::isalnum;
using std::isalpha;
using std::isdigit;
using std::cout;

static void handle_escape(string& exp, bool in);                                   // 处理\转义字符
static void replace_brace(string& exp, const map<string, string>& reMap);			//处理花括号
static void construct_char_set(set<char> &s, const string &content, bool n);		//根据传入的字符串和全集构造字符集合
static void replace_square_brace(string& exp);										//处理方括号
static void handle_quote(string& exp);												//处理引号
static void replace_dot(string &exp);	                                           //处理点
static void replace_question_and_add(string& exp);                                 //处理问号和加号
static void add_dot(string &exp);                                                  //加点


//替换{X}
void replace_brace(string& exp, const map<string, string>& reMap)  {
	string rename;
	vector<char> charVec;
	bool inBrace = false;          //判断是否在花括号里面
	auto expIt = exp.begin();    // expIt 是exp第一个字符的首地址  *expIt代表exp第一个字符
	while (expIt != exp.end()) {
		if ((*expIt) == '{'&&((expIt) == exp.begin()|| *(expIt-1) != '`')) {   //如果expIt指向{  且  {expIt是exp.begin()  或者expIt指向字符前一个不是 `}
			inBrace = true;
			rename = "";
			++expIt;
			continue;
		}
		else if ((*expIt) == '}' && ((expIt) == exp.begin() || *(expIt - 1) != '`') &&inBrace) {
			inBrace = false;
			auto findIt = reMap.find(rename);
			if (findIt != reMap.end()) {//找到了，把原来的出栈，找到的入栈
				for (const char& c : findIt->second) {
					charVec.push_back(c);
				}
			}
			else {
				throw(""); //UNDONE: 报错，找不到前面的name
			}
			++expIt;
			continue;
		}
		else if (inBrace) {
			rename += *expIt;
			++expIt;
			continue;
		}
		charVec.push_back(*expIt);   //这里代表没有花括号出现的处理 即直接push_back
		++expIt;
	}

	//输出vector的内容
	exp = "";
	for (const auto &e : charVec) {
		exp += e;
	}
	//cout << "REPLACE BRACE:"<<exp << "\n";
}

// 替换[X]
void construct_char_set(set<char> &s, const string &content, bool n)	 {
	string stemp(content);
	handle_escape(stemp, true);
	//处理[a-z]
	auto it = stemp.cbegin();
	set<char> temp;
	while (it != stemp.cend()) {
		if (*it == '-' && ((it) != stemp.cbegin()) && ((it + 1) != stemp.cend()) && isalnum(*(it - 1)) && isalnum(*(it + 1))) {
			//当前字符是-，前后都有字符，且都是字母或数字
			auto sit = ALLSET.find(*(it - 1)) + 1;
			auto eit = ALLSET.find(*(it + 1));
			if (eit >= sit) {
				// UNDONE: 报错：[]内部格式不对
			}
			while (sit != eit) {
				temp.insert(ALLSET[sit]);
				++sit;
			}
			++it;
		}
		else {
			temp.insert(*it);//插入就完事了
			++it;
		}
	}
	if (n) {//集合取反
		for (const char &c : ALLSET) {
			if (temp.find(c) == temp.end()) {
				s.insert(c);
			}
		}
	}
	else {
		s = temp;
	}
}

void replace_square_brace(string& exp) {
	string sbcontent;
	vector<char> charVec;
	bool inSquareBrackes = false;
	auto expIt = exp.begin();
	while (expIt != exp.end()) {
		if ((*expIt) == '[' && ((expIt) == exp.begin() || *(expIt - 1) != '`')) {
			inSquareBrackes = true;
			sbcontent = "";
			++expIt;
			continue;
		}
		else if ((*expIt) == ']' &&inSquareBrackes && ((expIt) == exp.begin() || *(expIt - 1) != '`')) {
			inSquareBrackes = false;
			charVec.push_back('(');
			set<char> s;//保存转换过的字符
			if (sbcontent[0] == '^') { // 非操作
				construct_char_set(s, sbcontent.substr(1, sbcontent.size() - 1), true);
			}
			else {
				construct_char_set(s, sbcontent, false);
			}
			for (const auto &c : s) {
				if(ESCAPEDCHARS.find(c)!= ESCAPEDCHARS.cend())//转义
					charVec.push_back('`');
				charVec.push_back(c);
				charVec.push_back('|');
			}
			charVec.pop_back();
			charVec.push_back(')');
			++expIt;
			continue;
		}
		else if (inSquareBrackes) {
			sbcontent += *expIt;
			++expIt;
			continue;
		}
			
		charVec.push_back(*expIt);
		++expIt;
	}
	//输出vector的内容

	exp = "";
	for (const auto &e : charVec) {
		exp += e;
	}
	//cout << "REPLACE SB:"<<exp << "\n";
}


// 处理引号
void handle_quote(string& exp) {
	bool inQuote = false;
	const auto sexpIt = exp.begin(); //sexpIt是exp第一个字符的首地址
	auto expIt = sexpIt;
	vector<char> charVec;
	while (expIt != exp.end()) {
		if (*expIt == '\"'&&( (expIt != sexpIt && *(expIt - 1) != '\\')|| (expIt == sexpIt))) {// 是",且不是\"
			if (!inQuote) {
				inQuote = true;
				++expIt;
				continue;
			}
			else {
				inQuote = false;
				++expIt;
				continue;
			}
		}
		else if(inQuote && ESCAPEDCHARS.find(*expIt) != ESCAPEDCHARS.cend()){ //转义引号里边可能的操作符需要被转义
			charVec.push_back('`');
		}
		charVec.push_back(*expIt);
		++expIt;
	}
	exp = "";
	for (const auto &c : charVec) {
		exp += c;
	}
	//cout << "HANDLE QUOTE:"<<exp << "\n";
}

//处理\转义字符
void handle_escape(string& exp,bool in) {
	string stemp;
	bool flag = false;
	for (auto &c : exp) {
		if (in) {//转义[]内的所有可能字符
			if (flag) {
				switch (c) {
				case 'n':
					stemp += '\n';
					break;
				case 't':
					stemp += '\t';
					break;
				case 'v':
					stemp += '\v';
					break;
				case 'f':
					stemp += '\f';
					break;
				case '\\':
					stemp += '\\';
					break;
				}
				flag = false;
				continue;
			}
			if (c == '\\') {
				flag = true;
				continue;
			}
		}
		else {/*在[]外，只转义 \"和\\*/
			if (flag) {
				flag = false;
				if (c == '\"' || c == '\\') stemp = stemp.substr(0, stemp.size() - 1);
			}else if (c == '\\') {
				flag = true;
			}
		}
		stemp += c;
	}
	exp = stemp;
	//cout << "HANDLE ESCAPE:" << exp << "\n";
}

//处理. (匹配除了\n之外所有单个字符)
void replace_dot(string &exp) {
	vector<char> charVec;
	auto expIt = exp.begin();
	while (expIt != exp.end()) {
		if (*expIt == '.' && (expIt== exp.begin() || (expIt != exp.begin() && *(expIt-1)!='`'))){
			charVec.push_back('(');
			set<char> s;//保存转换过的字符
			construct_char_set(s, "\n", true);
			for (const auto &c : s) {
				if (ESCAPEDCHARS.find(c) != ESCAPEDCHARS.cend())//转义
					charVec.push_back('`');
				charVec.push_back(c);
				charVec.push_back('|');
			}
			charVec.pop_back();
			charVec.push_back(')');
			++expIt;
			continue;
		}
		charVec.push_back(*expIt);
		++expIt;
	}
	//输出vector的内容

	exp = "";
	for (const auto &e : charVec) {
		exp += e;
	}
	//cout << "REPLACE DOT:" << exp << "\n";
}

// 替换？和+ (同时处理括号)
void replace_question_and_add(string& exp){
	vector<char> charVec;
	auto expIt = exp.begin();
	while (expIt != exp.end()) {
		if ((*expIt == '+' || *expIt == '?') && (expIt == exp.begin() || (expIt != exp.begin() && *(expIt - 1) != '`'))) {
			int counter = 0;
			if ((expIt) != exp.begin() && *(expIt - 1) == ')' && (((expIt-1) != exp.begin()&& *(expIt - 2)!='`')|| (expIt - 1) == exp.begin())){//如果前面是)且不是`），找到整个括号的部分
				const auto preExpIt = expIt;//记录之前+/?的位置	
				--expIt;
				charVec.pop_back();
				++counter;
				do{
					charVec.pop_back();
					--expIt;
					if (*expIt == '(' && (((expIt) != exp.begin() && *(expIt - 1) != '`') || (expIt) == exp.begin())) {//是(且不是`(
						--counter;
					}
					else if (*expIt == ')' && (((expIt) != exp.begin() && *(expIt - 1) != '`') || (expIt) == exp.begin())) {//是)且不是`)
						++counter;
					}
				} while (counter > 0);
				if (*preExpIt == '?') { //a? -> (@|a)
					charVec.push_back('(');
					charVec.push_back('@');
					charVec.push_back('|');
					while (expIt != preExpIt) {
						charVec.push_back(*expIt);
						++expIt;
					}
					charVec.push_back(')');
				}
				else { //a+ -> aa*
					const auto startIt = expIt;
					for (int i = 0; i < 2; i++) {
						expIt = startIt;
						while (expIt != preExpIt) {
							charVec.push_back(*expIt);
							++expIt;
						}
					}
					charVec.push_back('*');
				}
			}
			else {
				char c = charVec.back();
				if (*expIt == '?') { //a? -> (@|a)
					charVec.pop_back();
					charVec.push_back('(');
					charVec.push_back('@');
					charVec.push_back('|');
					charVec.push_back(c);
					charVec.push_back(')');
				}
				else {//a+ -> aa*
					charVec.push_back(c);
					charVec.push_back('*');
				}
			}
			++expIt;
			while (expIt != exp.end()) {
				charVec.push_back(*expIt);//后面的直接接上去
				++expIt;
			}
			exp = "";
			for (const auto &e : charVec) {
				exp += e;
			}
			charVec.clear();
			expIt = exp.begin();//从头开始
			continue;
		}
		charVec.push_back(*expIt);
		++expIt;
	}
	//输出vector的内容

	exp = "";
	for (const auto &e : charVec) {
		exp += e;
	}
	//cout << "REPLACE QS&A:" << exp << "\n";
}


// 加点
void  add_dot(string &exp) {
	string oldExp = exp;
	string dotedExp;
	for (auto oldExpIt = oldExp.cbegin(); oldExpIt != oldExp.cend(); ++oldExpIt) {
		dotedExp += (*oldExpIt);
		if (*oldExpIt == '`')continue;// 是转义字符，直接跳过
		if ((*oldExpIt == '(' || *oldExpIt == '|' )&& (oldExpIt == oldExp.cbegin() || *(oldExpIt - 1) != '`'))
			continue;// |或(,且前面不是`
		if ((oldExpIt + 1) == oldExp.cend()) continue; //最后一个字符，不加点
		if (*(oldExpIt + 1) == '|' || *(oldExpIt + 1) == '*' ||  *(oldExpIt + 1) == ')') continue; //下一个字符已经是操作符或右括号，不加点
		dotedExp += '.';//加点
	}
	exp = dotedExp;
	//cout << "ADD DOT:" << exp << "\n";
}

void re_standardize(vector<Rules>& reVec, map<string, string>& reMap) {    // 规范化
	//先处理map里面的
	for (auto & p : reMap) {
		handle_quote(p.second);                           //处理引号
		replace_brace(p.second, reMap);					 //替换花括号
	}

	//再处理Vector里面的
	for (auto & e : reVec) {
		handle_quote(e.pattern);						//处理引号
		replace_brace(e.pattern, reMap);				//替换花括号
		replace_square_brace(e.pattern);				//替换方括号
		replace_dot(e.pattern);							//替换点
		replace_question_and_add(e.pattern);			//替换问号和加号
		handle_escape(e.pattern, false);				//处理转义字符
		add_dot(e.pattern);								//加点
	}
	
}