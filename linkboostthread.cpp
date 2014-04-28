/*
 * linkboostthread.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: vietnguyen
 */

#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "linkboostthread.h"
#include "newport.h"

void linkboostthread::run(){
	if(_mode == 0){
		if(_nodetype == 0){
			m_thread = boost::thread(&linkboostthread::router_send_message, this, _id, _srcport, _dstport);

		}
		else{
			m_thread = boost::thread(&linkboostthread::host_send_message, this, _id, _srcport, _dstport);
			cout << "In Run: SEND: src = " << _srcport << ", dst = " << _dstport << "\n";
		}
	}else {
		if(_nodetype == 0){
			m_thread = boost::thread(&linkboostthread::router_receive_message, this,_id,  _srcport, _dstport, boost::ref(*rt_), boost::ref(*rtmutex_));
		}
		else{
			m_thread = boost::thread(&linkboostthread::host_receive_message, this, _id, _srcport, _dstport);
			cout << "In Run: RECEIVE: src = " << _srcport << ", dst = " << _dstport << "\n";
		}
	}
}

void linkboostthread::join(){
	m_thread.join();
}

void linkboostthread::host_send_message(int HID, int srcport, int dstport){
	try{
			cout << "[HOST SEND THREAD] From: " << srcport << ". To: " << dstport << "\n";
			//configure a sending port
			const char* hname = "localhost";
			Address * my_tx_addr = new Address(hname, (short)srcport);
			Address * dst_addr =  new Address(hname, (short)dstport);
			mySendingPort *my_tx_port = new mySendingPort();
			my_tx_port->setAddress(my_tx_addr);
			my_tx_port->setRemoteAddress(dst_addr);
			my_tx_port->init();

			while(1){
				// scan to see which contents this host has
				ostringstream dest;
				dest << "host";
				dest << HID;
				dest << "/";
				string foldername = dest.str();
				struct stat filestat;
				//cout << foldername << "\n";
				DIR * dir;
				struct dirent *ent;
				vector<int> CIDs;
				if((dir = opendir(foldername.c_str()))!=NULL){
					while((ent = readdir(dir)) != NULL){
						string filename(ent->d_name);
						//cout << filename << "\n";
						string filepath = foldername + "/" + filename;
						if(stat(filepath.c_str(), & filestat)) continue;
						if(S_ISDIR(filestat.st_mode))	continue;

						string sCID = filename.substr(8, filename.length() - 8);
						cout << sCID << "\n";
						int CID = atoi(sCID.c_str());
						CIDs.push_back(CID);
					}
					closedir (dir);
				}

				Message *m = new Message();
				for(int i = 0; i < CIDs.size(); i++){
					Packet *update_packet = m->make_update_packet(CIDs[i], 0);
					my_tx_port->sendPacket(update_packet);
					int type = m->get_packet_type(update_packet);
					cout << "[H" << HID << "]Send a message of type: " << type << ". CID = " << CIDs[i] << ". From port " << srcport << " to port " << dstport << "\n";
				}

				usleep(5000000);	// Sleep: in microseconds
			}
		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}

void linkboostthread::host_receive_message(int HID, int srcport, int dstport){
	try{
				cout << "[HOST RECEIVE THREAD]  From: " << srcport << "(" << (short)srcport << "). To: " << dstport << "(" << (short)dstport << ")\n";
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
						cout << "[Host " << HID << "]Receive a message of type: " << type << " from port " << srcport << " in port " << dstport << "\n";
					}
				}

			}
			catch(const char *reason ){
			    cerr << "Exception:" << reason << endl;
			    exit(-1);
			}
}

void linkboostthread::router_send_message(int RID, int srcport, int dstport){
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


void linkboostthread::router_receive_message(int RID, int srcport, int dstport, RT rt, boost::mutex rt_mutex){
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
