/*
 * host.h
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */

#ifndef HOST_H_
#define HOST_H_

#include <vector>
#include <algorithm>
#include <sstream>
#include <sys/stat.h>
#include "common.h"
using namespace std;

class Host{
	int id_;
	int rid_;	// router id
	vector<int> cids_;	//content id list
	int sendingporttorouter_;
	int receivingportfromrouter_;
	int routerreceivingport_;
	int routersendingport_;
public:
	Host(int id);
	void assign_router(int rid);
	void assign_content(int cid);
	void delete_content(int cid);
	void host_send_message( int id, int srcport, int dstport);
	void host_receive_message(int id, int srcport, int dstport);
	void shutdown();
	void copycontent(const char *infile, const char *outfile);
	void setup_link();
};

#endif /* HOST_H_ */
