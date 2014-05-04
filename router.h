/*
 * router.h
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#include <vector>
#include <queue>
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
	int rid_;	/* router id */
	vector<int> nrids_;	/* neighbor router ids */
	vector<int> sendingportno_;		/* sending port to other routers, map to interface */
	vector<int> receivingportno_; 	/* receiving port to other routers, map to interface */
	vector<int> farroutersendingportno_;
	vector<int> farrouterreceivingportno_;
	int sendingporttohost_;
	int receivingportfromhost_;
	int hostlisteningport_;
	int hostsendingport_;
	int hid_;

	PRT prt_;	/* Pending request table */
	RT rt_;		/* Routing table */
	queue<pair<int,Packet *> > message_queue_;
	vector<pair<int, Packet *> > to_send_packets_;	/* store packets before sending */
public:
	boost::timed_mutex *queuemutex_;
	boost::timed_mutex *to_send_packets_mutex_;
	Router ();
	Router (int rd);
	void set_id(int rid);
	void assign_host(int hid);
	void assign_nr(int rid); 	/* assign neighbor router */
	void calc_port_no();
	void setup_link();
	void shutdown();

	/* thread functions */
	void cleaning_tables(int id);
	void router_send_message(int id, int srcport , int dstport);
	void router_receive_message(int id, int srcport, int dstport);
	void router_process_message();
};
#endif /* ROUTER_H_ */
