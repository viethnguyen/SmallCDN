/*
 * content.h
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */

#ifndef CONTENT_H_
#define CONTENT_H_

class Content{
public:
	void gen_content(int id, int size);
	const char * get_content_name(int id);
};

#endif /* CONTENT_H_ */
