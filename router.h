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
using namespace std;

class Router{
	int rid_;	// router id
	vector<int> nrids_;	//neighbor router ids
	vector<int> sendingportno_;		//sending port to other routers, map to interface
	vector<int> receivingportno_; 	//receiving port to other routers, map to interface
	int sendingporttohost_;
	int receivingportfromhost_;
	int hostlisteningport_;
	int hid_;		//host id, -1 if no exist

public:
	Router (int rd);
	void assign_host(int hid);
	void assign_nr(int rid); 	//assign neighbor router
	void calc_port_no();
	void receive();
};
#endif /* ROUTER_H_ */
