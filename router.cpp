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
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
//#include "linkboostthread.h"
using namespace std;

Router::Router(){
	rid_ = -1;
	prt_ = new PRT();
	rt_ = new RT();
}
Router::Router (int rid){
	rid_ = rid;
	prt_ = new PRT();
	rt_ = new RT();
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
	/*
	linkboostthread sh(rid_, sendingporttohost_, hostlisteningport_, 0, 0);
	sh.router_set_prt(prt_, &prtmutex_);
	sh.router_set_rt(rt_, &rtmutex_);
	sh.run();
	cout << "[CREATE] a thread to send from port " << sendingporttohost_ << " to port " << hostlisteningport_ << "\n";

	linkboostthread rh(rid_, hostsendingport_, receivingportfromhost_, 1, 0);
	sh.router_set_prt(prt_, &prtmutex_);
	sh.router_set_rt(rt_, &rtmutex_);
	rh.run();
	cout << "[CREATE] a thread to receive from port " << hostsendingport_ << " in port " << receivingportfromhost_ << "\n";

	//setup links with other neighbor routers
	int n = sendingportno_.size();
	for(int i = 0; i < n ; i++){
		linkboostthread s(rid_, sendingportno_[i], farrouterreceivingportno_[i],0, 0);
		s.router_set_prt(prt_, &prtmutex_);
		s.router_set_rt(rt_, &rtmutex_);
		s.run();
		cout << "[CREATE] a thread to send from port " << sendingportno_[i] << " to port " << farrouterreceivingportno_[i] << "\n";

		linkboostthread r(rid_, farroutersendingportno_[i], receivingportno_[i], 1, 0);
		r.router_set_prt(prt_, &prtmutex_);
		r.router_set_rt(rt_, &rtmutex_);
		r.run();
		cout << "[CREATE] a thread to receive from port " << farroutersendingportno_[i] << " in port " << receivingportno_[i]<< "\n";
	}*/


	boost::thread sh_thread = boost::thread(&Router::router_send_message, this, rid_, sendingporttohost_, hostlisteningport_);
	boost::thread rh_thread = boost::thread(&Router::router_receive_message,this, rid_, hostsendingport_, hostlisteningport_);

	//setup links with other neighbor routers
	int n = sendingportno_.size();
	for(int i = 0; i<n ; i++){
		boost::thread sthread = boost::thread(&Router::router_send_message, this, rid_, sendingportno_[i], farrouterreceivingportno_[i]);
		boost::thread rthread = boost::thread(&Router::router_receive_message,this, rid_, farroutersendingportno_[i], receivingportno_[i]);
	}

}

void Router::router_send_message(int RID, int srcport, int dstport){
	try{
			cout << "[ROUTER SEND THREAD] From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
			//configure a sending port
			const char* hname = "localhost";
			Address * my_tx_addr = new Address(hname, (short)srcport);
			Address * dst_addr =  new Address(hname, (short)dstport);
			mySendingPort *my_tx_port = new mySendingPort();
			my_tx_port->setAddress(my_tx_addr);
			my_tx_port->setRemoteAddress(dst_addr);
			my_tx_port->init();

			while(1){
				// scan the routing table
				/*
				Message *m = new Message();
				Packet *update_packet = m->make_update_packet(3,1);
				my_tx_port->sendPacket(update_packet);
				*/
				usleep(5000000);	// Sleep: in microseconds
			}

		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}


void Router::router_receive_message(int RID, int srcport, int dstport){
	try{
				cout << "[ROUTER RECEIVE THREAD]  From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
				//configure receiving port
				const char* hname = "localhost";
				Address * my_addr = new Address(hname, (short)dstport);
				LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
				my_port->setAddress(my_addr);
				my_port->init();

				Message *m = new Message();
				Packet *p;
				while (1)
				{
					p = my_port->receivePacket();
					if(p!=NULL){
						int type = m->get_packet_type(p);
						int CID = m->get_packet_CID(p);
						cout << "[R" << RID << "]Receive a message: Type: " << type << ". Content ID: " << CID << ". From port " << srcport << " in port " << dstport << "\n";
						switch(type){
						case 2:		{//Message.TYPE_UPDATE
							/*
							boost::lock_guard<boost::mutex> guard(rt_mutex);
							RTentry *entry = rt.get_entry(CID);
							if(entry!=NULL){
								cout << "Get here 1\n";
								int oldnHops = entry->getnHops();
								cout << "Get here 2\n";
								int newnHops = m->get_packet_HOPS(p);
								cout << "Get here 3\n";
								if(newnHops < oldnHops ){
									rt.delete_entry(CID);

									cout << "Get here 4\n";
									int IID = (dstport - 10500) % 1000;

									cout << "Get here 5\n";
									RTentry newentry(CID, IID, newnHops);

									cout << "Get here 6\n";
									rt.add_entry(newentry);
								}
							}
							*/
							break;
						}
						case 0: 	{//Message.TYPE_REQUEST
							break;
						}
						case 1: 	{//Message.TYPE_RESPONSE
							break;
						}
						default:
							break;
						}
					}
				}

			}
			catch(const char *reason ){
			    cerr << "Exception:" << reason << endl;
			    exit(-1);
			}
}

void Router::shutdown(){
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
