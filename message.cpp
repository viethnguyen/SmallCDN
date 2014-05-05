/*
 * message.cpp
 *
 *  Created on: Mar 2, 2014
 *      Author: vietnguyen
 *
 *      Define message types used
 */

#include "common.h"
#include "newport.h"
#include "message.h"
#include <cstring>
#include <iostream>
#include <fstream>

#define INIT_SIZE 500
#define DATA_SIZE 1200
#define SENDER_ID 15
#define RECEIVER_ID 20

#define INIT_MESSAGE_TYPE 10
#define ACK_MESSAGE_TYPE 11
#define DATA_MESSAGE_TYPE 12
#define NACK_MESSAGE_TYPE 13

//#define TYPE_REQUEST 0
//#define TYPE_RESPONSE 1
//#define TYPE_UPDATE 2


/**
 * Constructor
 */

Message::Message(){
	filename_ =  new char[MAX_FILE_NAME_LEN];
	number_of_packets_ = 0;
	length_ = 0;
}

Packet *Message::make_request_packet(int CID, int HID){
	Packet * request_packet;
	request_packet = new Packet();
	request_packet->setPayloadSize(0);

	//Header
	PacketHdr *hdr = request_packet->accessHeader();
	hdr->setOctet(TYPE_REQUEST, POS_TYPE );
	hdr->setOctet(CID, POS_CID );
	hdr->setOctet(HID, POS_HID );

	// no payload
	return request_packet;
}
Packet *Message::make_response_packet(int CID, int HID, const char *filename){
	Packet * response_packet;
	response_packet = new Packet();

	//get some info of the file
	//read file
	ifstream f1(filename);
	f1.seekg(0, ios::end);
	int len = (int)f1.tellg();
	f1.seekg(0, f1.beg);
	char *datachunk = new char[len];
	f1.read(datachunk, len);
	f1.close();

	//Header
	PacketHdr *hdr = response_packet->accessHeader();
	hdr->setOctet(TYPE_RESPONSE, POS_TYPE );
	hdr->setOctet(CID, POS_CID );
	hdr->setOctet(HID, POS_HID );
	hdr->setShortIntegerInfo(len, POS_SIZE);

	// fill payload by content...
	response_packet->fillPayload(len, datachunk);

	return response_packet;
}

Packet *Message::make_update_packet(int CID, int nhops){
	Packet * update_packet;
	update_packet = new Packet();
	update_packet->setPayloadSize(0);

	//Header
	PacketHdr *hdr = update_packet->accessHeader();
	hdr->setOctet(TYPE_UPDATE, POS_TYPE );
	hdr->setOctet(CID, POS_CID );
	hdr->setOctet(nhops, POS_HOPS );

	// no payload
	return update_packet;
}

int Message::get_packet_type(Packet *p){
	PacketHdr *hdr = p->accessHeader();
	return (int) hdr->getOctet(POS_TYPE);
}

int Message::get_packet_CID(Packet *p){
	PacketHdr *hdr = p->accessHeader();
	return (int) hdr->getOctet(POS_CID);
}

int Message::get_packet_HID(Packet *p){
	PacketHdr *hdr = p->accessHeader();
	return (int) hdr->getOctet(POS_HID);
}

int Message::get_packet_HOPS(Packet *p){
	PacketHdr *hdr = p->accessHeader();
	return (int) hdr->getOctet(POS_HOPS);
}

int Message::get_packet_size(Packet *p){
	PacketHdr *hdr = p->accessHeader();
	return (int) hdr->getShortIntegerInfo(POS_SIZE);

}


