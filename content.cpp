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

using namespace std;

void Content::gen_content(int id, int size){
	srand(time(NULL));
	ofstream file(get_content_name(id));
	for(int i = 0; i < size; i++){
		int value = rand()%9;
		file << value;
	}
	file.close();
}

const char * Content::get_content_name(int id){
	ostringstream oss;
	oss << "c";
	oss << id;
	return oss.str().c_str();
}

