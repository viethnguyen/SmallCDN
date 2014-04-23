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

Router::Router(){
	rid_ = -1;
}
Router::Router (int rid){
	rid_ = rid;
}
void Router::set_id(int rid){
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
		sendingportno_.push_back(10000 + rid_*1000 + nrids_[i]);
		receivingportno_.push_back(10000 + rid_*1000 + 500 + nrids_[i]);
		farroutersendingportno_.push_back(10000 + nrids_[i] * 1000 + rid_ );
		farrouterreceivingportno_.push_back(10000 + nrids_[i] * 1000 + 500 + rid_ );
	}
	sendingporttohost_ = 10000 +rid_ * 1000 + 999;
	receivingportfromhost_ = 10000 + rid_ * 1000 + 998;
	hostsendingport_ = 10000 + rid_ * 1000 + 997;
	hostlisteningport_ = 10000 + rid_ * 1000 + 996;
}

void * send_message(void *threadarg){
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

		//TODO: flood message
		Message *m = new Message();
		Packet *update_packet = m->make_update_packet(1,2);
		my_tx_port->sendPacket(update_packet);
		my_tx_port->lastPkt_ = update_packet;
		cout << "Update packet is sent from port " << sending_port << " to port " << receiving_port << "\n";
		my_tx_port->timer_.startTimer(2.5);

		//TODO: forward packets

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

		Packet *p;
		while (1)
		{
			p = my_port->receivePacket();
			if(p!=NULL){
				cout << "Receive a message from port " << sending_port << " in port " << receiving_port << "\n";
			}
		}

		// TODO: update tables...

	}
	catch(const char *reason ){
	    cerr << "Exception:" << reason << endl;
	    exit(-1);
	}
}

void Router::setup_link(){
	//setup link with host
	pthread_t t_sendtohost;
	struct link_info l1;
	l1.sending_port = sendingporttohost_;
	l1.receiving_port = hostlisteningport_;
	pthread_create(&t_sendtohost, NULL, &send_message, &l1);
	cout << "created a thread to send from port " << l1.sending_port << " to port " << l1.receiving_port << "\n";
	//routerthreads[threadcount ++] = t_sendtohost;

	pthread_t t_receivefromhost;
	struct link_info l2;
	l2.receiving_port = receivingportfromhost_;
	l2.sending_port = hostsendingport_;
	pthread_create(&t_receivefromhost, NULL, &receive_message, &l2);
	cout << "created a thread to receive from port " << l2.sending_port << " in port " << l2.receiving_port << "\n";
	//routerthreads[threadcount ++] = t_receivefromhost;

	//setup links with other neighbor routers
	int n = sendingportno_.size();

	for(int i = 0; i < n; i++){
		pthread_t t_sendtorouter;
		struct link_info lr1;
		lr1.sending_port = sendingportno_[i];
		lr1.receiving_port = farrouterreceivingportno_[i];
		pthread_create(&t_sendtorouter, NULL, &send_message, &lr1 );
		cout << "created a thread to send from port " << lr1.sending_port << " to port " << lr1.receiving_port << "\n";
		//routerthreads[threadcount ++ ] = t_sendtorouter;

		pthread_t t_receivefromrouter;
		struct link_info lr2;
		lr2.sending_port = farroutersendingportno_[i];
		lr2.receiving_port = receivingportno_[i];
		pthread_create(&t_receivefromrouter, NULL, &receive_message, &lr2);
		cout << "created a thread to receive from port " << lr2.sending_port << " in port " << lr2.receiving_port << "\n";
		//routerthreads[threadcount ++] = t_receivefromrouter;
	}
}

void Router::shutdown(){
//
//	for(int i = 0; i < threadcount; i++){
//		pthread_join(routerthreads[i], NULL);
//	}
}

/*
 * FOR TEST
 */
/*
int main(){
	int rid, hid, nrid;
	cout << "Enter router id: ";
	cin >> rid;
	cout << "Enter host id:";
	cin >> hid;
	cout << "Enter neighbor router id: ";
	cin >> nrid;

	Router r(rid);
	r.assign_nr(nrid);
	r.assign_host(hid);
	r.calc_port_no();

	// Test: receive from its host
	r.setup_link();

	while(1){
		//Do nothing, let the processes do their jobs
	}
}
*/
