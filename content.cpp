/*
 * content.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */

#include "content.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

void Content::gen_content(int id, int size){
	srand(time(NULL));
	ofstream file(get_content_name(id).c_str());
	for(int i = 0; i < size; i++){
		int value = rand()%9;
		file << value;
	}
	file.close();
}

string Content::get_content_name(int id){
	ostringstream oss;
	oss << "content_";
	oss << id;
	cout << oss.str()  << "\n";
	return oss.str();
}

string Content::get_content_name_in_host(int hid, int cid){
	Content c;
	ostringstream dest;
	dest << "host";
	dest << hid;
	dest << "/";
	cout << dest.str() << "\n";
	string s = c.get_content_name(cid);
	cout << s  << "\n";
	string destfile = dest.str() + s;
	return destfile;
}
