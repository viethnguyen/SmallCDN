#include "common.h"
#include "newport.h"
#include "message.h"
#include "host.h"
#include "router.h"
#include "content.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <time.h>
#include <boost/system/error_code.hpp>

using namespace std;

void test_message(){

	Message m;
	Packet * p = m.make_update_packet(3,1);
	cout << m.get_packet_HOPS(p) << "\n";
	cout << m.get_packet_CID(p) << "\n";
	cout << m.get_packet_type(p) << "\n";

}

void gen_content(){
	Content c;
	srand(time(NULL));
	for(int i = 0; i <=255; i++){
		int size = rand()% 1500;
		c.gen_content(i, size);
	}
}

int main(){
	//test_message();
	gen_content();
	return 0;
}
