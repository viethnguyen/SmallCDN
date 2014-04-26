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
			m_thread = boost::thread(&linkboostthread::router_receive_message, this,_id,  _srcport, _dstport);
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
				dest << HID;
				dest << "/";
				string foldername = dest.str();
				DIR * dir;
				struct dirent *ent;
				vector<int> CIDs;
				if((dir = opendir(foldername.c_str()))!=NULL){
					while((ent = readdir(dir)) != NULL){
						string filename(ent->d_name);
						string sCID = filename.substr(8, filename.length() - 8);
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
					cout << "[Host " << HID << "]Send a message of type: " << type << " from port " << srcport << " to port " << dstport << "\n";
				}

				usleep(10000);	// Sleep: in microseconds
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

				Message *m = new Message();
				Packet *update_packet = m->make_update_packet(3,1);
				my_tx_port->sendPacket(update_packet);
				usleep(10000);	// Sleep: in microseconds
			}

		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}


void linkboostthread::router_receive_message(int RID, int srcport, int dstport){
	try{
				//cout << "[RECEIVE THREAD]  From: " << _srcport << "(" << (short)_srcport << "). To: " << _dstport << "(" << (short)_dstport << ")\n";
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
					}
				}

			}
			catch(const char *reason ){
			    cerr << "Exception:" << reason << endl;
			    exit(-1);
			}
}
