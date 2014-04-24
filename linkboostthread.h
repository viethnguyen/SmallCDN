/*
 * linkboostthread.h
 *
 *  Created on: Apr 24, 2014
 *      Author: vietnguyen
 */

#ifndef LINKBOOSTTHREAD_H_
#define LINKBOOSTTHREAD_H_

#include "common.h"
#include "message.h"
#include <pthread.h>
#include <iostream>

class linkboostthread{
	int _srcport;
	int _dstport;
	int _mode;	//0: sending, 1: receiving
public:
	linkboostthread(int srcport, int dstport, int mode){
		_srcport = srcport;
		_dstport = dstport;
		_mode = mode;
	}
	void run();
	void send_message();
	void receive_message();

};

#endif /* LINKBOOSTTHREAD_H_ */
