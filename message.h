/*
 * message.h
 *
 *  Created on: Mar 2, 2014
 *      Author: vietnguyen
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_



class Message {
protected:
	char * filename_;
	int number_of_packets_;
	size_t length_;

public:
	//3 types of packets
	//define message format - byte position in the header of each field
	static const int POS_TYPE = 0;
	static const int POS_CID = 1;
	static const int POS_HID = 2;
	static const int POS_HOPS = 2;
	static const int POS_SIZE = 3;
	static const int POS_PAYLOAD = 5;

	static const int TYPE_REQUEST = 0;
	static const int TYPE_RESPONSE = 1;
	static const int TYPE_UPDATE = 2;

	static const int MAX_FILE_NAME_LEN = 80;
	Message();

	Packet *make_request_packet(int CID, int HID);
	Packet *make_response_packet(int CID, int HID, const char *filename);
	Packet *make_update_packet(int CID, int nhops);
	int get_packet_type(Packet *p);
	int get_packet_CID(Packet *p);
	int get_packet_HID(Packet *p);
	int get_packet_HOPS(Packet *p);
	int get_packet_size(Packet *p);
};

#endif /* MESSAGE_H_ */
