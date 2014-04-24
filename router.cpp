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
#include "linkboostthread.h"
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

void Router::setup_link(){

	linkboostthread sh(sendingporttohost_, hostlisteningport_, 0);
	sh.run();
	cout << "[CREATE] a thread to send from port " << sendingporttohost_ << " to port " << hostlisteningport_ << "\n";

	linkboostthread rh(hostsendingport_, receivingportfromhost_, 1);
	rh.run();
	cout << "[CREATE] a thread to receive from port " << hostsendingport_ << " in port " << receivingportfromhost_ << "\n";

	//setup links with other neighbor routers
	int n = sendingportno_.size();
	for(int i = 0; i < n ; i++){
		linkboostthread s(sendingportno_[i], farrouterreceivingportno_[i], 0);
		s.run();
		cout << "[CREATE] a thread to send from port " << sendingportno_[i] << " to port " << farrouterreceivingportno_[i] << "\n";

		linkboostthread r(farroutersendingportno_[i], receivingportno_[i], 1);
		r.run();
		cout << "[CREATE] a thread to receive from port " << farroutersendingportno_[i] << " in port " << receivingportno_[i]<< "\n";
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
