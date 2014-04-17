#include "common.h"
#include "newport.h"
#include "message.h"
#include "host.h"
#include "content.h"
#include <cstring>
#include <iostream>
#include <fstream>

void test_message(){
	/*
	Message m;
	Packet * response = m.make_response_packet(1,1, "c2.txt");
	*/
}

void test_content(){
	Content c;
	c.gen_content(2,125);
	c.gen_content(24,214);
	//cout << c.get_content_name(234) << "\n";
}
void test_host(){
	Host h(1);//create new host
	h.assign_content(24);
	h.assign_content(25);
}
int main(){
	test_content();
	test_host();
	return 0;
}
