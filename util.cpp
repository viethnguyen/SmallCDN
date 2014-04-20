#include "common.h"
#include "newport.h"
#include "message.h"
#include "host.h"
#include "content.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <time.h>

void test_message(){
	/*
	Message m;
	Packet * response = m.make_response_packet(1,1, "c2.txt");
	*/
}

void gen_content(){
	Content c;
	srand(time(NULL));
	for(int i = 0; i <=255; i++){
		int size = rand()% 1500;
		c.gen_content(i, size);
	}
}

/*
void test_host(){
	Host h(1);//create new host
	h.assign_content(24);
	h.assign_content(25);
	h.assign_content(2);
	h.delete_content(24);
}
*/
int main(){
	gen_content();
	return 0;
}
