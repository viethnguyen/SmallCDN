/*
 * content.h
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */

#ifndef CONTENT_H_
#define CONTENT_H_

#include <string>

using namespace std;
class Content{
public:
	void gen_content(int id, int size);
	string get_content_name(int id);
	string get_content_name_in_host(int hid, int cid);
	string get_content_name_in_store(int hid, int cid);
};

#endif /* CONTENT_H_ */
