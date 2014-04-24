/*
 * linkboostthread.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: vietnguyen
 */

#include <iostream>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include "linkboostthread.h"
#include "newport.h"

void linkboostthread::run(){
	if(_mode == 0){
		boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&linkboostthread::send_message, this)));
	}
	else {
		boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&linkboostthread::receive_message, this)));
	}
}

void linkboostthread::send_message(){
	try{
			cout << "[SEND THREAD] From: " << _srcport << "(" << (short)_srcport << "). To: " << _dstport << "(" << (short)_dstport << ")\n";

			//configure a sending port
			const char* hname = "localhost";
			Address * my_tx_addr = new Address(hname, (short)_srcport);
			Address * dst_addr =  new Address(hname, (short)_dstport);
			mySendingPort *my_tx_port = new mySendingPort();
			my_tx_port->setAddress(my_tx_addr);
			my_tx_port->setRemoteAddress(dst_addr);
			my_tx_port->init();

			//TODO: flood message
			Message *m = new Message();
			Packet *update_packet = m->make_update_packet(1,2);
			my_tx_port->sendPacket(update_packet);
			my_tx_port->lastPkt_ = update_packet;
			cout << "Update packet is sent from port " << _srcport << " to port " << _dstport << "\n";
			my_tx_port->timer_.startTimer(2.5);

			//TODO: forward packets

		}
		catch(const char *reason ){
		    cerr << "Exception:" << reason << endl;
		    exit(-1);
		}
}

void linkboostthread::receive_message(){
	try{
				cout << "[RECEIVE THREAD]  From: " << _srcport << "(" << (short)_srcport << "). To: " << _dstport << "(" << (short)_dstport << ")\n";

				//configure receiving port
				const char* hname = "localhost";
				Address * my_addr = new Address(hname, (short)_dstport);
				LossyReceivingPort *my_port = new LossyReceivingPort(0.2);
				my_port->setAddress(my_addr);
				my_port->init();

				Packet *p;
				while (1)
				{
					p = my_port->receivePacket();
					if(p!=NULL){
						cout << "Receive a message from port " << _srcport << " in port " << _dstport << "\n";
					}
				}

				// TODO: update tables...

			}
			catch(const char *reason ){
			    cerr << "Exception:" << reason << endl;
			    exit(-1);
			}
}
