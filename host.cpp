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
		strs << id;
		string temp_str = strs.str();
		char const* foldername = temp_str.c_str();
		mkdir(foldername, S_IRWXU|S_IRGRP|S_IXGRP);
	}

void Host::assign_router(int rid){
		rid_ = rid;
		sendingporttorouter_ = rid * 1000 + 997;
		receivingportfromrouter_ = rid * 1000 + 996;
		routerreceivingport_ = rid * 1000 + 998;
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
		//free this object

		//delete the folder together with all its contents
}

/**
 * @param: argv[1] destination host address
 * 			argv[2] destination port number
 * 			argv[3] listening port number
 * 			argv[4] filename
 */

int sendingporttorouter_;
int receivingportfromrouter_;
int routerreceivingport_;

void Host::send(){
	 try {

	  const char* hname = "localhost";
	  Address * my_tx_addr = new Address(hname, (short)sendingporttorouter_);

	  //configure sending port
	  Address * dst_addr =  new Address(hname, (short)routerreceivingport_);
	  mySendingPort *my_port = new mySendingPort();
	  my_port->setAddress(my_tx_addr);
	  my_port->setRemoteAddress(dst_addr);
	  my_port->init();

	  //configure receiving port to listen to ACK frames
	  Address * my_rx_addr = new Address(hname, (short)receivingportfromrouter_);
	  LossyReceivingPort *my_rx_port = new LossyReceivingPort(0.2);
	  my_rx_port->setAddress(my_rx_addr);
	  my_rx_port->init();

	  //create a single packet
	  Packet * my_packet;
	  my_packet = new Packet();
	  my_packet->setPayloadSize(100);
	  PacketHdr *hdr = my_packet->accessHeader();
	  hdr->setOctet('D',0);
	  hdr->setOctet('A',1);
	  hdr->setOctet('T',2);
	  hdr->setIntegerInfo(1,3);

	  //init a file transfer session
	  Message *m = new Message();
	  Packet *update_packet = m->make_update_packet(1,0);

	  my_port->sendPacket(update_packet);
	  my_port->lastPkt_ = update_packet;
	  cout << " Update packet is sent!" <<endl;
	  my_port->setACKflag(false);
	  //schedule retransmit
	  my_port->timer_.startTimer(2.5);

	  cout << "begin waiting for ACK..." <<endl;
	  Packet *pAck;
	  while (!my_port->isACKed()){
	        pAck = my_rx_port->receivePacket();

	        if (pAck!= NULL)
	        {
		     my_port->setACKflag(true);
		     my_port->timer_.stopTimer();
	        }
	  };

	 } catch (const char *reason ) {
	    cerr << "Exception:" << reason << endl;
	    exit(-1);
	  }
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

	//Test: send to default router
	h.send();


}
