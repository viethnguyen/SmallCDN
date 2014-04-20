/*
 * router.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: vietnguyen
 */
#include "router.h"
#include "common.h"
#include "newport.h"
#include "message.h"

using namespace std;

Router::Router (int rid){
	rid_ = rid;
}
void Router::assign_host(int hid){
	hid_ = hid;
}
void Router::assign_nr(int rid){
	nrids_.push_back(rid);
}
void Router::calc_port_no(){
	for(int i = 0; i < nrids_.size(); i++){
		sendingportno_.push_back(rid_*1000 + i);
		receivingportno_.push_back(rid_ * 1000 + 500 + i);

	}
	sendingporttohost_ = rid_ * 1000 + 999;
	receivingportfromhost_ = rid_ * 1000 + 998;
	hostlisteningport_ = rid_ * 1000 + 996;
}

/**
 * @param argv[1] receiving port number
 * 			argv[2] destination address
 * 			argv[3] destination port number
 * 			argv[4] filename to save
 */
void Router::receive(){
	try {
	  const char* hname = "localhost";

	  //configure receiving port
	  Address * my_addr = new Address(hname, (short)receivingportfromhost_);

	  LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
	  my_port->setAddress(my_addr);
	  my_port->init();

	  //configure a sending port to send ACK
	  Address * my_tx_addr = new Address(hname, (short)sendingporttohost_);
	  Address * dst_addr =  new Address(hname, (short)hostlisteningport_);
	  mySendingPort *my_tx_port = new mySendingPort();
	  my_tx_port->setAddress(my_tx_addr);
	  my_tx_port->setRemoteAddress(dst_addr);
	  my_tx_port->init();

	  Message *m = new Message();

	  cout << "begin receiving..." <<endl;
	  Packet *p;
	  int ackedSeqNum = 0;
	  while (1)
	  {
	    p = my_port->receivePacket();
	    if (p !=NULL)
	    {
			int seqNum = p->accessHeader()->getIntegerInfo(m->SEQNUM_POS);
			Packet * ack_packet = m->make_ack_packet(seqNum);
			cout << "receiving a packet of seq num " << seqNum << endl;
			my_tx_port->sendPacket(ack_packet);
			return;
	    }
	  }
	} catch (const char *reason ) {
	    cerr << "Exception:" << reason << endl;
	    exit(-1);
	}

}
int main(){
	int rid, hid;
	cout << "Enter router id: ";
	cin >> rid;
	cout << "Enter host id:";
	cin >> hid;

	Router r(rid);
	r.assign_host(hid);
	r.calc_port_no();

	// Test: receive from its host
	r.receive();
}
