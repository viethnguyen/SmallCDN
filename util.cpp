#include "common.h"
#include "newport.h"
#include "message.h"
#include "host.h"
#include "content.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

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

void setup_topo(){
	ifstream in("networktopo.txt");
	if(in.is_open()){
		int linkcount;
		in >> linkcount;
		vector<pair<int, int> > routerlinks;
		for(int i = 0; i < linkcount; i++){
			int node1, node2;
			in >> node1 >> node2;
			routerlinks.push_back(make_pair(node1, node2));
		}
		int nr;		//number of routers with host
		in >> nr;
		vector<pair<int, int> > routerhostlinks;	// link
		for(int i = 0; i < nr; i++){
			int router, host;
			in >> router >> host;
			routerhostlinks.push_back(make_pair(router,host));
		}
		in.close();
	}
	else{
		cout << "Unable to open to network topology file";
	}

}

int main(){
	gen_content();
	setup_topo();
	return 0;
}
