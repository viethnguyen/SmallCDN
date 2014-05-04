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
using namespace std;

Router::Router(){
	rid_ = -1;
	queuemutex_ = new boost::timed_mutex();
	to_send_packets_mutex_ = new boost::timed_mutex();
}
Router::Router (int rid){
	rid_ = rid;
	queuemutex_ = new boost::timed_mutex();
	to_send_packets_mutex_ = new boost::timed_mutex();
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
	boost::thread cleaning_thread = boost::thread(&Router::cleaning_tables, this, rid_);

	/* setup link with its host */
	boost::thread sh_thread = boost::thread(&Router::router_send_message, this, rid_, sendingporttohost_, hostlisteningport_);
	boost::thread rh_thread = boost::thread(&Router::router_receive_message,this, rid_, hostsendingport_, receivingportfromhost_);

	/*setup links with other neighbor routers */
	int n = sendingportno_.size();
	for(int i = 0; i<n ; i++){
		boost::thread sthread = boost::thread(&Router::router_send_message, this, rid_, sendingportno_[i], farrouterreceivingportno_[i]);
		boost::thread rthread = boost::thread(&Router::router_receive_message,this, rid_, farroutersendingportno_[i], receivingportno_[i]);
	}

	/* process message */
	boost::thread processthread = boost::thread(&Router::router_process_message, this);

}

void Router::cleaning_tables(int RID){
	//scan the routing table and reduce Time to expire of all entries
	while(1){
		//cout << "[R" << RID << "]CLEANING TABLES\n";

		/* cleaning routing table */
		vector<RTentry> rtcopy = rt_.export_table();
		vector<RTentry>::iterator it = rtcopy.begin();
		while(it!=rtcopy.end()){
			it->updateTTE();
			if(it->getTTE() <= 0){
				it = rtcopy.erase(it);
			}else{
				it++;
			}
		}
		rt_.import_table(rtcopy);

		/* cleaning pending request table */
		vector<PRTentry> prtcopy = prt_.export_table();
		vector<PRTentry>::iterator it2 = prtcopy.begin();
		while(it2!=prtcopy.end()){
			it2->updateTTE();
			if(it2->getTTE() <= 0){
				it2 = prtcopy.erase(it2);
			} else{
				it2++;
			}
		}
		prt_.import_table(prtcopy);

		/* sleep */
		boost::this_thread::sleep(boost::posix_time::seconds(5));
	}
}

void Router::router_send_message(int RID, int srcport, int dstport){
	try{
		//cout << "[ROUTER SEND THREAD] From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
		/* configure a sending port */
		const char* hname = "localhost";
		Address * my_tx_addr = new Address(hname, (short)srcport);
		Address * dst_addr =  new Address(hname, (short)dstport);
		mySendingPort *my_tx_port = new mySendingPort();
		my_tx_port->setAddress(my_tx_addr);
		my_tx_port->setRemoteAddress(dst_addr);
		my_tx_port->init();

		/* calculate the interface id of this sending thread */
		int IID = (srcport - 10000) % 1000;
		if(IID > 256){
			IID = 498;
		}

		Message *m = new Message();

		int timecount = 0;

		while(1){
			timecount ++;
			//cout << "[R" << rid_ << "] [Send] Timecount = " << timecount << "\n";
			/* send request/ response packet, if any */
			{

				//cout << "\t\t[R" << RID <<"][Send]IID = " << IID << ". srcport =" << srcport << "\n";
				/* try to acquire the to send message queue lock, then send the packets with destination port of this sending thread */
				boost::unique_lock<boost::timed_mutex> lock(* to_send_packets_mutex_ , boost::try_to_lock);
				bool getLock = lock.owns_lock();
				if(!getLock){
					getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
				}
				if(getLock){
					vector<pair<int, Packet *> >::iterator it = to_send_packets_.begin();
					while(it != to_send_packets_.end()){
						cout << "\t\t[R" << RID <<"][Send] IID = " << it->first << ".Type : " << m->get_packet_type(it->second) << ". CID = " << m->get_packet_CID(it->second) << "\n";
						if(it->first == IID ){
							my_tx_port->sendPacket(it->second);
							it = to_send_packets_.erase(it);
						}
						else{
							it++;
						}
					}
				}
				boost::timed_mutex *mu = lock.release();
				mu->unlock();
			}
			boost::this_thread::sleep(boost::posix_time::seconds(1));

			if (timecount < 12) continue;

			/* send update packet */
			if(IID == 498) {
				timecount = 0;	/* Don't send to host */
				continue;
			}
			vector<RTentry> v;
			{
				v = rt_.export_table();
			}
			int n = v.size();
			for(int i = 0; i<n; i++){
				if(v[i].getIID() == IID) continue;
				Message *m = new Message();
				Packet *update_packet =m->make_update_packet(v[i].getCID(), v[i].getnHops());
				my_tx_port->sendPacket(update_packet);
				cout << "[R" << RID << "] Send new update packet CID = "<<  v[i].getCID() << "to IID = " << IID << "\n";
			}

			timecount = 0;
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
			//cout << "[R" << rid_ << "] ROUTER PROCESS MESSAGE\n";
			pair<int, Packet *> message;
			int queue_size = 0;

			{
				/* try to acquire the receive message queue lock, then take one message out */
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

			//cout << "[R" << rid_ << "][In Process] Messages in queue: " << queue_size << "\n";
			if(queue_size == 0){
				boost::this_thread::sleep(boost::posix_time::seconds(1));
				continue;
			}

			int IID = message.first;
			Packet *p = message.second;

			Message *m = new Message();
			int type = m->get_packet_type(p);
			int CID = m->get_packet_CID(p);
			cout << "[R" << rid_ << "] [Process message] Type: " << type << ". IID: " << IID <<". Content ID: " << CID << "\n";

			switch(type){
			/*
			 * Message.TYPE_UPDATE
			 */
			case 2:		{
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
						//cout << "\t[R" << rid_ << "][ADD] CID = "<<  CID << "\n";
					}
					else{
						//cout << "\t[R" << rid_ << "][OUTSTANDING]. \n";
					}
				}
				else{
					int newnHops = m->get_packet_HOPS(p) + 1;
					RTentry newentry(CID, IID, newnHops);
					replica.push_back(newentry);
					//cout << "\t[R" << rid_ << "][ADD] CID = "<<  CID << ". \n";
				}

				rt_.import_table(replica);
				break;
			}

			/*
			 * Message.TYPE_REQUEST
			 */
			case 0: 	{
				vector<PRTentry> prtcopy = prt_.export_table();
				int HID =  m->get_packet_HID(p);
				vector<PRTentry>::iterator it = prtcopy.begin();
				while(it!=prtcopy.end()){
					if(it->getHID()==HID && it->getCID() == CID){
						break;
					}
					it ++;
				}

				/*if new request, add it to pending request table */
				if(it==prtcopy.end()){
					PRTentry newentry( CID,  HID,  IID);
					prtcopy.push_back(newentry);
					prt_.import_table(prtcopy);
				}

				/* look up routing table for this content id */
				vector<RTentry> rtcopy = rt_.export_table();
				vector<RTentry>::iterator rt_it = rtcopy.begin();
				while(rt_it!=rtcopy.end()){
					if(rt_it->getCID() == CID){
						break;
					}
					rt_it++;
				}
				if(rt_it != rtcopy.end()){
					int interface_to_send = rt_it->getIID();


					{
						/* try to acquire the to send message queue lock, and add this message to the queue */
						boost::unique_lock<boost::timed_mutex> lock(* to_send_packets_mutex_ , boost::try_to_lock);
						bool getLock = lock.owns_lock();
						if(!getLock){
							getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
						}
						if(getLock){
							to_send_packets_.push_back(make_pair(interface_to_send, p));
						}
						boost::timed_mutex *mu = lock.release();
						mu->unlock();
					}

				}
				break;
			}

			/**
			 * Message.TYPE_RESPONSE
			 */
			case 1: 	{
				/* match with pending request table */

				vector<PRTentry> prtcopy = prt_.export_table();
				int HID =  m->get_packet_HID(p);
				vector<PRTentry>::iterator it = prtcopy.begin();
				while(it!=prtcopy.end()){
					if(it->getHID()==HID && it->getCID() == CID){
						break;
					}
					it ++;
				}

				if(it!= prtcopy.end() ){
					int interface_to_send = it->getIID();
					{
						/* try to acquire the to send message queue lock, and add this message to the queue*/
						boost::unique_lock<boost::timed_mutex> lock(* to_send_packets_mutex_ , boost::try_to_lock);
						bool getLock = lock.owns_lock();
						if(!getLock){
							getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
						}
						if(getLock){
							cout << "[R" << rid_ << "]Push back: Type = " << m->get_packet_type(p) << ". CID = " << m->get_packet_CID(p) << "\n";
							to_send_packets_.push_back(make_pair(interface_to_send, p));
						}
						boost::timed_mutex *mu = lock.release();
						mu->unlock();
					}

				}

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


void Router::router_receive_message(int RID, int srcport, int dstport){
	try{
				//cout << "[ROUTER RECEIVE THREAD]  From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
				//configure receiving port
				const char* hname = "localhost";
				Address * my_addr = new Address(hname, (short)dstport);
				LossyReceivingPort *my_port = new LossyReceivingPort(0);
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
						}
						*/
						if(getLock){
							message_queue_.push(make_pair(IID, p));
							//cout << "[R" << RID <<"] Push new message to queue. Messages in queue: " << message_queue_.size() << "\n";
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

