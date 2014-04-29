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
#include <boost/thread/mutex.hpp>
//#include "linkboostthread.h"
using namespace std;

Router::Router(){
	rid_ = -1;
	rtmutex_ = new boost::mutex();
	prtmutex_ = new boost::mutex();
}
Router::Router (int rid){
	rid_ = rid;
	rtmutex_ = new boost::mutex();
	prtmutex_ = new boost::mutex();
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
	boost::thread cleaning_thread = boost::thread(&Router::cleaning_tables, this, rid_, rtmutex_);
	cout << "[IN SETUPLINLK] " << rtmutex_ << "\n";
	boost::thread sh_thread = boost::thread(&Router::router_send_message, this, rid_, sendingporttohost_, hostlisteningport_, rtmutex_);
	boost::thread rh_thread = boost::thread(&Router::router_receive_message,this, rid_, hostsendingport_, receivingportfromhost_, rtmutex_);

	//setup links with other neighbor routers
	int n = sendingportno_.size();
	for(int i = 0; i<n ; i++){
		boost::thread sthread = boost::thread(&Router::router_send_message, this, rid_, sendingportno_[i], farrouterreceivingportno_[i], rtmutex_);
		boost::thread rthread = boost::thread(&Router::router_receive_message,this, rid_, farroutersendingportno_[i], receivingportno_[i], rtmutex_);
	}

}

void Router::cleaning_tables(int RID, boost::mutex *mutex){
	//scan the routing table and reduce Time to expire of all entries
	while(1){
		cout << "[R" << RID << "]CLEANING TABLES - After clean: \n";
		{
			boost::unique_lock<boost::mutex> lock(*mutex);
			rt_.update_table();
			rt_.print_table();
		}
		usleep(30000000);	//in microseconds
	}
}

void Router::router_send_message(int RID, int srcport, int dstport, boost::mutex *rtmutex){
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
				vector<RTentry> v;
				{
					boost::unique_lock<boost::mutex> lock(*rtmutex);
					v = rt_.export_table();
				}
				for(int i = 0; i<v.size(); i++){
					Message *m = new Message();
					Packet *update_packet =m->make_update_packet(v[i].getCID(), v[i].getnHops());
					my_tx_port->sendPacket(update_packet);
					//cout << "[R" << RID << "] Send new update packet CID = "<<  v[i].getCID() << "\n";
				}
				usleep(5000000);	// Sleep: in microseconds
			}

		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}


void Router::router_receive_message(int RID, int srcport, int dstport, boost::mutex *rtmutex){
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
							boost::unique_lock<boost::mutex> lock(*rtmutex);
							RTentry *entry = rt_.get_entry(CID);
							if(entry!=NULL){
								int oldnHops = entry->getnHops();
								int newnHops = m->get_packet_HOPS(p) + 1;
								if(newnHops < oldnHops ){
									rt_.delete_entry(CID);
									int IID = (dstport - 10500) % 1000;
									RTentry newentry(CID, IID, newnHops);
									rt_.add_entry(newentry);
									cout << "\t[R" << RID << "][ADD] CID = "<<  CID << ". New table: \n";
									rt_.print_table();
								}
								else {
									cout << "\t[R" << RID << "][OUTSTANDING]. Old table: \n";
									rt_.print_table();
								}
							}
							else{
								int IID = (dstport - 10500) % 1000;
								int newnHops = m->get_packet_HOPS(p) + 1;
								RTentry newentry(CID, IID, newnHops);
								rt_.add_entry(newentry);
								cout << "\t[R" << RID << "][ADD] CID = "<<  CID << ". New table: \n";
								rt_.print_table();
							}
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
