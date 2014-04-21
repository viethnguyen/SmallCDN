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
	hostsendingport_ = rid_ * 1000 + 997;
	hostlisteningport_ = rid_ * 1000 + 996;
}

void *send_message(void *threadarg){
	try{
		struct link_info *my_link;
		my_link = (struct link_info *)threadarg;
		int sending_port = my_link->sending_port;
		int receiving_port = my_link->receiving_port;

		//configure a sending port
		const char* hname = "localhost";
		Address * my_tx_addr = new Address(hname, (short)sending_port);
		Address * dst_addr =  new Address(hname, (short)receiving_port);
		mySendingPort *my_tx_port = new mySendingPort();
		my_tx_port->setAddress(my_tx_addr);
		my_tx_port->setRemoteAddress(dst_addr);
		my_tx_port->init();

		//TODO: flood message, forward packet...

	}
	catch(const char *reason ){
	    cerr << "Exception:" << reason << endl;
	    exit(-1);
	}
}

void *receive_message(void *threadarg){
	try{
		struct link_info *my_link;
		my_link = (struct link_info *) threadarg;
		int sending_port = my_link->sending_port;
		int receiving_port = my_link->receiving_port;

		//configure receiving port
		const char* hname = "localhost";
		Address * my_addr = new Address(hname, (short)receiving_port);
		LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
		my_port->setAddress(my_addr);
		my_port->init();

		// TODO: update tables...

	}
	catch(const char *reason ){
	    cerr << "Exception:" << reason << endl;
	    exit(-1);
	}
}

void Router::setup_link(){
	//setup link with host
	//TODO: spawn two threads: for sending to host and receiving from host
	pthread_t t_sendtohost;
	struct link_info l1;
	l1.sending_port = sendingporttohost_;
	l1.receiving_port = hostlisteningport_;
	pthread_create(&t_sendtohost, NULL, &send_message, &l1);

	pthread_t t_receivefromhost;
	struct link_info l2;
	l2.receiving_port = receivingportfromhost_;
	l2.sending_port = hostsendingport_;
	pthread_create(&t_receivefromhost, NULL, &receive_message, &l2);

	//setup links with other neighbor routers
	//TODO: spawn two threads: for sending to each neighboring router and receiving from it.

}

void Router::comm_with_host(){
	try {
	  const char* hname = "localhost";

	  //configure receiving port
	  Address * my_addr = new Address(hname, (short)receivingportfromhost_);

	  LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
	  my_port->setAddress(my_addr);
	  my_port->init();

	  //configure a sending port
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
	r.comm_with_host();
}
