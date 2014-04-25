/*
 * host.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */

#include "common.h"
#include "newport.h"
#include "message.h"
#include "host.h"
#include "content.h"
#include "linkboostthread.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <istream>
#include <fstream>
#include <iterator>

using namespace std;

Host::Host(int id){
		id_ = id;
		rid_ = -1;
		//create folder
		stringstream strs;
		strs << "host";
		strs << id;
		string temp_str = strs.str();
		char const* foldername = temp_str.c_str();
		mkdir(foldername, S_IRWXU|S_IRGRP|S_IXGRP);

	}

void Host::assign_router(int rid){
		rid_ = rid;
		sendingporttorouter_ = 10000 + rid * 1000 + 997;
		receivingportfromrouter_ = 10000 + rid * 1000 + 996;
		routerreceivingport_ = 10000 + rid * 1000 + 998;
		routersendingport_ = 10000 + rid * 1000 + 999;
	}

void Host::assign_content(int cid){
		std::vector<int>::iterator it;
		it = find (cids_.begin(), cids_.end(), cid);
		if(it!=cids_.end())
			return;
		cids_.push_back(cid);

		//copy the content file into the folder
		Content c;
		string destfile = c.get_content_name_in_host(id_,cid);
		copycontent(c.get_content_name(cid), destfile.c_str());
	}

void Host::delete_content(int cid){
		std::vector<int>::iterator it;
		it = find(cids_.begin(), cids_.end(), cid);
		if(it!=cids_.begin())
			cids_.erase(it);

		//Delete the content file
		Content c;
		string filename = c.get_content_name_in_host(id_,cid);
		remove(filename.c_str());
	}

void Host::copycontent(const char *infile, const char *outfile){
	fstream f(infile, fstream::in |fstream::binary );
	if(!f.good())
		return;
	f << noskipws;
	istream_iterator<unsigned char> begin(f);
	istream_iterator<unsigned char> end;

	fstream f2(outfile, fstream::out | fstream::trunc | fstream::binary);
	ostream_iterator<char> begin2(f2);

	copy (begin, end, begin2);
}

void Host::shutdown(){
	//signaling all the threads
}

void Host::setup_link(){
	//linkboostthread s(id_,sendingporttorouter_, routerreceivingport_, linkboostthread.MODE_SEND, linkboostthread.NODE_HOST);
	linkboostthread s(id_,sendingporttorouter_, routerreceivingport_, 0, 1);
	s.run();
	cout << "[CREATE] a thread to send from port " << sendingporttorouter_ << " to port " << routerreceivingport_ << "\n";

	//linkboostthread r(id_, routersendingport_, receivingportfromrouter_, linkboostthread.MODE_RECEIVE, linkboostthread.NODE_HOST);
	linkboostthread r(id_, routersendingport_, receivingportfromrouter_, 1, 1);
	r.run();
	cout << "[CREATE] a thread to receive from port " << routersendingport_ << " in port " << receivingportfromrouter_ << "\n";
}

int main(){
	int hid, rid, cid;
	cout << "Enter host ID: ";
	cin >> hid;
	cout << "Enter default router ID: ";
	cin >> rid;
	cout << "Assign content IDs: ";
	cin >> cid;
	Host h(hid);
	h.assign_router(rid);
	h.assign_content(cid);

	h.setup_link();
	while(1){
		// Do nothing, let the processes do their jobs
	}


}
