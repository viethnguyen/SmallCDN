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

	//3 types of packets

	static const int POS_TYPE = 0;
	static const int POS_CID = 8;
	static const int POS_HID = 16;
	static const int POS_HOPS = 16;
	static const int POS_SIZE = 24;
	static const int POS_PAYLOAD = 40;

	static const int LEN_TYPE = 8;
	static const int LEN_CID = 8;
	static const int LEN_HID = 8;
	static const int LEN_SIZE = 16;


public:
	static const int MAX_FILE_NAME_LEN = 80;
	Message();

	Packet *make_request_packet(int CID, int HID);
	Packet *make_response_packet(int CID, int HID, char *filename);
	Packet *make_update_packet(int CID, int nhops);
	int get_packet_type(Packet *p);
	int get_packet_CID(Packet *p);
	int get_packet_HID(Packet *p);
	int get_packet_HOPS(Packet *p);
	int get_packet_size(Packet *p);



	// OBSOLETE !!
	//define message format - byte position in the header of each field
	static const int ID_POS = 0;
	static const int TYPE_POS = 4;
	static const int SEQNUM_POS = 6;
	static const int EOF_POS = 10;

	//'make' functions
	int get_number_of_packets(){return number_of_packets_;}
	int get_file_length(){return length_;}
	Packet *make_init_packet(char* filename);
	Packet *make_ack_packet(int seqNum);
	bool prepare_file_to_send(char *filename);
	Packet *make_data_packet(int seqNum);
	bool append_data_to_file(char *filename, char *datachunk, int len);

};

#endif /* MESSAGE_H_ */
