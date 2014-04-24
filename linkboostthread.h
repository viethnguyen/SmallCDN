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
#include <boost/thread.hpp>
#include <boost/date_time.hpp>


class linkboostthread{
private:
	boost::thread m_thread;
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
	void join();
	void send_message(int srcport, int dstport);
	void receive_message(int srcport, int dstport);

};

#endif /* LINKBOOSTTHREAD_H_ */
