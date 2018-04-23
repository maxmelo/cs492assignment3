/*
 * Assignment 3
 * Max Melo and Michael Iacona
 * April 17th, 2018
 * I pledge my honor that I have abided by the Stevens Honor System
 */

#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string.h>

using namespace std;

vector<vector <string> > read_in(string filename, char separator) {
	ifstream instream(filename);
	vector<vector <string> > out;	
	string instring;

	if (!instream) throw 1;

	//Read file line-by-line
	while (getline(instream, instring)) {
		stringstream line(instring);
		string segment;
		vector<string> seglist;

		if (instring.size() == 0) continue;

		//Read line string-by-string, separated by given separator
		while(getline(line, segment, separator)) seglist.push_back(segment);
		
		out.push_back(seglist);
	}
	return out;
}


int main(int argc, char **argv) {
	vector<vector <string> > dir_list;
	try {
		dir_list = read_in("file_list.txt", ' ');
	} catch (int e) {
		cout << "Error reading directory list. Make sure there is a file named dir_list.txt in this directory" << endl;
		return 1;
	}
	cout << "HELLO?" << endl;
	for (vector<vector <string> >::iterator it = dir_list.begin(); it != dir_list.end(); it++) {
		string str = "";

		for (int i = 0; i < it->size(); i++) str += it->at(i) + "; ";

		cout << str << endl;
	}
	
	return 0;
}

