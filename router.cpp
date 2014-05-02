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
	queuemutex_ = new boost::timed_mutex();
}
Router::Router (int rid){
	rid_ = rid;
	rtmutex_ = new boost::mutex();
	prtmutex_ = new boost::mutex();
	queuemutex_ = new boost::timed_mutex();
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
	int n = nrids_.size();
	for(int i = 0; i < n; i++){
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

	// process message
	boost::thread processthread = boost::thread(&Router::router_process_message, this);

}

void Router::cleaning_tables(int RID, boost::mutex *mutex){
	//scan the routing table and reduce Time to expire of all entries
	while(1){
		cout << "[R" << RID << "]CLEANING TABLES\n";
		vector<RTentry> replica = rt_.export_table();
		vector<RTentry>::iterator it = replica.begin();
		while(it!=replica.end()){
			it->updateTTE();
			if(it->getTTE() <= 0){
				it = replica.erase(it);
			}else{
				it++;
			}
		}
		rt_.import_table(replica);
		boost::this_thread::sleep(boost::posix_time::seconds(30));
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
					//boost::unique_lock<boost::mutex> lock(*rtmutex_);
					//boost::unique_lock<boost::mutex> lock(*rtmutex);
					v = rt_.export_table();
				}
				int n = v.size();
				for(int i = 0; i<n; i++){
					Message *m = new Message();
					Packet *update_packet =m->make_update_packet(v[i].getCID(), v[i].getnHops());
					my_tx_port->sendPacket(update_packet);
					//cout << "[R" << RID << "] Send new update packet CID = "<<  v[i].getCID() << "\n";
				}
				boost::this_thread::sleep(boost::posix_time::seconds(12));
			}

		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}

void Router::router_process_message(){
	try{
		while(1){
			cout << "[R" << rid_ << "] ROUTER PROCESS MESSAGE\n";
			pair<int, Packet> message;
			int queue_size = 0;
			{
				boost::unique_lock<boost::timed_mutex> lock(* queuemutex_ , boost::try_to_lock);
				bool getLock = lock.owns_lock();
				if(!getLock){
					getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
				}
				if(getLock){
					queue_size = message_queue_.size();
					if(queue_size != 0){
						message = message_queue_.front();
						message_queue_.pop();
					}
				}
				boost::timed_mutex *m = lock.release();
				m->unlock();

			}
			cout << "[R" << rid_ << "][In Process] Messages in queue: " << queue_size << "\n";
			if(queue_size == 0){
				boost::this_thread::sleep(boost::posix_time::seconds(1));
				continue;
			}

			int IID = message.first;
			Packet *p = &(message.second);

			Message *m = new Message();
			int type = m->get_packet_type(p);
			int CID = m->get_packet_CID(p);
			cout << "[R" << rid_ << "] [Process message] Type: " << type << "Interface ID: " << IID <<". Content ID: " << CID << "\n";

			switch(type){
			case 2:		{//Message.TYPE_UPDATE
				vector<RTentry> replica = rt_.export_table();
				vector<RTentry>::iterator it = replica.begin();
				cout << "\t\t\t[Current Routing table]\n";
				while(it!=replica.end()){
					cout << "\t\t\tCID = " << it->getCID() << ". IID = " << it->getIID() << ". nHops = " << it->getnHops() << ". TTE: " << it->getTTE()<< "\n";
					it++;
				}
				it = replica.begin();
				while(it!=replica.end()){
					if(it->getCID() == CID){
						break;
					}
					it++;
				}
				if(it!=replica.end()){
					int oldnHops = it->getnHops();
					int newnHops = m->get_packet_HOPS(p) + 1;
					if(newnHops < oldnHops){
						replica.erase(it);
						RTentry newentry(CID, IID, newnHops);
						replica.push_back(newentry);
						cout << "\t[R" << rid_ << "][ADD] CID = "<<  CID << "\n";
					}
					else{
						cout << "\t[R" << rid_ << "][OUTSTANDING]. \n";
					}
				}
				else{
					int newnHops = m->get_packet_HOPS(p) + 1;
					RTentry newentry(CID, IID, newnHops);
					replica.push_back(newentry);
					cout << "\t[R" << rid_ << "][ADD] CID = "<<  CID << ". \n";
				}

				rt_.import_table(replica);

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
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		}

	}catch(const char *reason ){
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

				Packet *p;
				while (1)
				{
					p = my_port->receivePacket();
					int IID = (dstport - 10500) % 1000;
					if(p!=NULL){
						boost::unique_lock<boost::timed_mutex> lock(* queuemutex_ , boost::try_to_lock);
						bool getLock = lock.owns_lock();
						/*
						if(!getLock){
							getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
						}*/
						if(getLock){
							message_queue_.push(make_pair(IID, *p));
							cout << "[R" << RID <<"] Push new message to queue. Messages in queue: " << message_queue_.size() << "\n";
						}
						boost::timed_mutex *m = lock.release();
						m->unlock();
					}
					boost::this_thread::sleep(boost::posix_time::seconds(1));
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
