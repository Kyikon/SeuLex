// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "structs.h"

using namespace std;

int read_and_parse_lex(string filePath, map<string, string>& m, vector<Rules>& vRules, vector<string>&P1, vector<string>&P4);
void re_standardize(vector<Rules>& reVec, map<string, string>& reMap);
void re_to_suffix(vector<Rules>& rules);
void suffix_to_nfa(vector<Rules>& suffixRules, NFA& finalNfa);
void nfa_to_dfa(const NFA &nfa, DFA &dfa);
void dfa_minimize(const DFA& origindfa, DFA& newdfa);
void dfa_to_array(const DFA& dfa, vector<pair<int*, int> >& arrays, vector<Rules>& endVec);
int generate_c_code(vector<pair<int*, int>>& arrays, vector<Rules>& endVec,vector<string>& part1, vector<string>& part4,int startState,int mode);

static void print(string s, bool printterms, map<string, string> terms, vector<Rules> rules);


vector<Rules> rules;

int main(int argc ,char** argv)
{	
	int mode = -1;

	string input;

	if (argc == 2)
	{
		input = string(argv[1]);
		if (input == "lex") {
			mode = 0;
		}else if (input == "yacc") {
			mode = 1;
		}else{		
			cout << "ERROR: invalid mode!" << endl;
			return -1;
		}
	}
	else {
		cout << "ERROR: invalid arguments number!" << endl;
		return -1;
	}
	map<string, string> terms;
	
	vector<string>part1, part4;

	if (read_and_parse_lex("lex.l", terms, rules, part1, part4)) {
		system("pause");
		return -1;
	}
	cout << "\n\n#####################\n" << "STANDARDIZING RE..." << endl;
	re_standardize(rules,terms);
	cout << "COMPLETED" << endl;

	cout << "\n\n#####################\n" << "CONVERTING RE TO SUFFIXED RE..." << endl;
	re_to_suffix(rules);
	cout << "COMPLETED" << endl;

	NFA nfa;
	cout << "\n\n#####################\n" << "GENERATING NFA..." << endl;
	suffix_to_nfa(rules,nfa);
	cout << "COMPLETED" << endl;

	DFA dfa;
	cout << "\n\n#####################\n" << "CONVERTING TO DFA..." << endl;
	nfa_to_dfa(nfa, dfa);
	cout << "COMPLETED" << endl;

	DFA minidfa;
	cout << "\n\n#####################\n" << "MINIMIZING DFA..." << endl;
	dfa_minimize(dfa, minidfa);
	cout << "COMPLETED" << endl;

	vector<pair<int*, int>> arrays;
	vector<Rules>endVec;
	cout << "\n\n#####################\n" << "GENERATING LEX.C..." << endl;
	dfa_to_array(minidfa,arrays,endVec);
	generate_c_code(arrays, endVec,part1,part4, minidfa.startState,mode);
	cout << "\n\n#####################\n" << "COMPLETED ALL." <<endl;
	return 0;
}



void print(map<string, string> terms, vector<Rules> rules) {

	for (const auto &p : terms) {
		cout << "NAME:" << p.first << "\n" << "EXPRESSION:" << p.second << endl;
	}

	for (const auto &e : rules)
		cout << "PATTERN:" << e.pattern << endl;
}