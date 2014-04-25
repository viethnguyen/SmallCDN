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
	int _id;			//id of the node corresponding to this link
	int _srcport;
	int _dstport;
	int _mode;	//0: sending, 1: receiving
	int _nodetype;	//0:router, 1:host
public:
	static const int MODE_SEND = 0;
	static const int MODE_RECEIVE = 1;
	static const int NODE_ROUTER = 0;
	static const int NODE_HOST = 1;
	linkboostthread(int id, int srcport, int dstport, int mode, int nodetype){
		_id = id;
		_srcport = srcport;
		_dstport = dstport;
		_mode = mode;
		_nodetype = nodetype;
	}
	void run();
	void join();
	void host_send_message( int id, int srcport, int dstport);
	void host_receive_message(int id, int srcport, int dstport);
	void router_send_message(int id, int srcport , int dstport);
	void router_receive_message(int id, int srcport, int dstport);
};

#endif /* LINKBOOSTTHREAD_H_ */
