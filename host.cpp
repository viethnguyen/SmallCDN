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
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>
#include <istream>
#include <fstream>
#include <iterator>
#include <boost/thread/mutex.hpp>

using namespace std;

Host::Host(int id){
	id_ = id;
	rid_ = -1;
	/* create folder */
	stringstream strs;
	strs << "host";
	strs << id;
	string temp_str = strs.str();
	char const* foldername = temp_str.c_str();
	mkdir(foldername, S_IRWXU|S_IRGRP|S_IXGRP);

	/*create store folder */
	stringstream strs2;
	strs2 << "store";
	strs2 << id;
	string temp_str2 = strs2.str();
	char const* foldername2 = temp_str2.c_str();
	mkdir(foldername2, S_IRWXU|S_IRGRP|S_IXGRP);

	/* init state */
	isWaiting = false;

	/* init mutex */
	to_send_packets_mutex_ = new boost::timed_mutex();
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

	/* copy the content file into the folder */
	Content c;
	string destfile = c.get_content_name_in_host(id_,cid);
	copycontent(c.get_content_name(cid).c_str(), destfile.c_str());
}

void Host::delete_content(int cid){
	std::vector<int>::iterator it;
	it = find(cids_.begin(), cids_.end(), cid);
	if(it!=cids_.begin())
		cids_.erase(it);

	/* Delete the content file */
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


void Host::request_content(int cid){
	/* make a request packet */
	Message *m = new Message();
	Packet *p = m->make_request_packet(cid, id_);

	/* queue the REQUEST packet to the sending_queue */
	{
		boost::unique_lock<boost::timed_mutex> lock(* (to_send_packets_mutex_) , boost::try_to_lock);
		bool getLock = lock.owns_lock();
		if(!getLock){
			getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
		}
		if(getLock){
			to_send_packets_.push_back(p);
			isWaiting = true;
		}

		boost::timed_mutex *m = lock.release();
		m->unlock();
	}
}

void Host::host_send_message(int HID, int srcport, int dstport){
	try{
		//cout << "[HOST SEND THREAD] From: " << srcport << ". To: " << dstport << "\n";
		/* configure a sending port */
		const char* hname = "localhost";
		Address * my_tx_addr = new Address(hname, (short)srcport);
		Address * dst_addr =  new Address(hname, (short)dstport);
		mySendingPort *my_tx_port = new mySendingPort();
		my_tx_port->setAddress(my_tx_addr);
		my_tx_port->setRemoteAddress(dst_addr);
		my_tx_port->init();


		Message *m = new Message();

		int timecount = 0;
		while(1){

			timecount ++;
			//cout << "[H" << id_ << "] [Send] Timecount = " << timecount << "\n";
			/* check if there is any message needs to send ... */
			{
				boost::unique_lock<boost::timed_mutex> lock(* to_send_packets_mutex_ , boost::try_to_lock);
				bool getLock = lock.owns_lock();
				if(!getLock){
					getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
				}
				if(getLock){
					vector<Packet *>::iterator it = to_send_packets_.begin();
					while(it != to_send_packets_.end()){
						my_tx_port->sendPacket(*it);
						//cout << "[H" << id_ << "] [Send] Type = " << m->get_packet_type(*it) << "\n";
						it = to_send_packets_.erase(it);
					}
				}
				boost::timed_mutex *m = lock.release();
				m->unlock();
			}
			boost::this_thread::sleep(boost::posix_time::seconds(1));

			if (timecount < 10) continue;	/* does not have to announce content yet... */

			/* scan to see which contents this host has, then announce ... */
			ostringstream dest;
			dest << "host";
			dest << HID;
			dest << "/";
			string foldername = dest.str();
			struct stat filestat;
			DIR * dir;
			struct dirent *ent;
			vector<int> CIDs;
			if((dir = opendir(foldername.c_str()))!=NULL){
				while((ent = readdir(dir)) != NULL){
					string filename(ent->d_name);
					string filepath = foldername + "/" + filename;
					if(stat(filepath.c_str(), & filestat)) continue;
					if(S_ISDIR(filestat.st_mode))	continue;

					string sCID = filename.substr(8, filename.length() - 8);
					int CID = atoi(sCID.c_str());
					CIDs.push_back(CID);
				}
				closedir (dir);
			}

			/* Send announcement messages */
			Message *m = new Message();
			int n = CIDs.size();
			for(int i = 0; i < n; i++){
				Packet *update_packet = m->make_update_packet(CIDs[i], 0);
				my_tx_port->sendPacket(update_packet);
				//int type = m->get_packet_type(update_packet);
				//cout << "[H" << HID << "]Send a message of type: " << type << ". CID = " << CIDs[i] << ". From port " << srcport << " to port " << dstport << "\n";
				//boost::this_thread::sleep(boost::posix_time::seconds(1));
			}

			timecount = 0;
		}
	}
	catch(const char *reason ){
		cerr << "Exception:" << reason << endl;
		exit(-1);
	}
}

void Host::host_receive_message(int HID, int srcport, int dstport){
	try{
		//cout << "[HOST RECEIVE THREAD]  From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
		//configure receiving port
		const char* hname = "localhost";
		Address * my_addr = new Address(hname, (short)dstport);
		//LossyReceivingPort *my_port = new LossyReceivingPort(0);
		ReceivingPort *my_port = new ReceivingPort();
		my_port->setAddress(my_addr);
		my_port->init();

		Message *m = new Message();
		Packet *p;
		while (1)
		{
			p = my_port->receivePacket();
			if(p!=NULL){
				int type = m->get_packet_type(p);
				//cout << "[H" << id_ << "] [Received] Type: " << type << "\n";
				switch (type){
				/*
				 * Message.TYPE_REQUEST
				 */
				case 0:{
					/* make RESPONSE packet */
					int CID = m->get_packet_CID(p);
					int HID = m->get_packet_HID(p);
					Content c;
					string filename = c.get_content_name_in_host(id_,CID);
					struct stat buffer;
					if(stat(filename.c_str(), &buffer) != 0) break;	/* if this file doesn't exist, do nothing... */
					Packet *p = m->make_response_packet(CID, HID, filename.c_str());

					/* queue the RESPONSE packet to the sending_queue */
					{
						boost::unique_lock<boost::timed_mutex> lock(* to_send_packets_mutex_ , boost::try_to_lock);
						bool getLock = lock.owns_lock();
						if(!getLock){
							getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
						}
						if(getLock){
							to_send_packets_.push_back(p);
						}
						boost::timed_mutex *m = lock.release();
						m->unlock();
					}
					break;
				}
				/*
				* Message.TYPE_RESPONSE
				*/
				case 1:{
					/* if this host is not waiting, discard */
					if(!isWaiting){
						break;
					}

					/* else, store the content to disk  */
					int size = p->getPayloadSize();
					char *payload = p->getPayload();
					int CID = m->get_packet_CID(p);
					Content c;
					ofstream outfile(c.get_content_name_in_store(id_, CID).c_str());
					outfile.write(payload, size);
					outfile.close();
					isWaiting = false;
					cout << "\n[H" << id_ << "]Content CID = " << CID << " has been delivered! \n";
					break;
				}
				}
			}
		}

	}
	catch(const char *reason ){
		cerr << "Exception:" << reason << endl;
		exit(-1);
	}
}

void Host::shutdown(){
	//signaling all the threads
}

void Host::setup_link(){
	boost::thread sthread, rthread;
	sthread = boost::thread(&Host::host_send_message, this, id_, sendingporttorouter_, routerreceivingport_);
	cout << "[CREATE] a thread to send from port " << sendingporttorouter_ << " to port " << routerreceivingport_ << "\n";
	rthread = boost::thread(&Host::host_receive_message, this, id_, routersendingport_, receivingportfromrouter_);
	cout << "[CREATE] a thread to receive from port " << routersendingport_ << " in port " << receivingportfromrouter_ << "\n";

}

int main(){
	int hid, rid, cid;
	cout << "Enter host ID: ";
	cin >> hid;
	cout << "Enter default router ID: ";
	cin >> rid;
	Host h(hid);
	h.assign_router(rid);
	h.setup_link();

	while(1){
		cout << "Request content IDs: ";
		cin >> cid;

		h.request_content(cid);
		//boost::this_thread::sleep(boost::posix_time::seconds(40));

		/*
		if(h.isWaiting = false){
			cout << "Content has been delivered! \n";
		}
		else{
			cout << "Content has not been delivered. Please re-try... \n";
		}
		*/

	}


}
