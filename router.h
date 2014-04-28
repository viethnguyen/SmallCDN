/*
 * router.h
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#include <vector>
#include <algorithm>
#include <sstream>
#include <sys/stat.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "common.h"
#include "prt.h"
#include "rt.h"

using namespace std;

class Router{
private:
	int rid_;	// router id
	vector<int> nrids_;	//neighbor router ids
	vector<int> sendingportno_;		//sending port to other routers, map to interface
	vector<int> receivingportno_; 	//receiving port to other routers, map to interface
	vector<int> farroutersendingportno_;
	vector<int> farrouterreceivingportno_;
	int sendingporttohost_;
	int receivingportfromhost_;
	int hostlisteningport_;
	int hostsendingport_;
	int hid_;
	boost::mutex prtmutex_;
	boost::mutex rtmutex_;
	PRT *prt_;	//Pending request table
	RT *rt_;		//Routing table
public:
	Router ();
	Router (int rd);
	void set_id(int rid);
	void assign_host(int hid);
	void assign_nr(int rid); 	//assign neighbor router
	void calc_port_no();
	void setup_link();
	void router_send_message(int id, int srcport , int dstport);
	void router_receive_message(int id, int srcport, int dstport);
	void shutdown();
};
#endif /* ROUTER_H_ */
