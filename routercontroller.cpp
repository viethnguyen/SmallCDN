/*
 * routercontroller.cpp
 *
 *  Created on: Apr 22, 2014
 *      Author: vietnguyen
 */

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
#include <vector>
#include <map>
#include <sstream>

using namespace std;

class routercontroller{
private:
	vector<pair<int,int> > routerlinks;
	vector<pair<int,int> > routerhostlinks;

public:
	string trim (const string& s){
		  string result( s );
		  result.erase( result.find_last_not_of( " " ) + 1 );
		  result.erase( 0, result.find_first_not_of( " " ) );
		  return result;
	}
	void read_topo(){
		ifstream in("topo1.txt");
		if(in.is_open()){
			int routercount;
			string line0;
			getline(in, line0);
			routercount = atoi(line0.c_str());
			cout << "Count: " << routercount << "\n";
			for(int i = 0; i < routercount; i++){
				Router r;

				// neighbor routers
				string line1;
				getline(in, line1);
				line1 += " ";
				istringstream iss(line1);
				string word;
				int cntr = 0;
				while(getline(iss, word,' ')){
					cout << "word " << ++cntr << ": " << trim( word) << "\n";

					if(cntr == 1){
						r.set_id(atoi(word.c_str()));
					}else{
						r.assign_nr(atoi(word.c_str()));
					}
				}

				// host
				string line2;
				getline(in, line2);
				line2 += " ";
				istringstream iss2(line2);
				cntr = 0;
				while(getline(iss2, word,' ')){
					cout << "word " << ++cntr << ": " << trim( word) << "\n";
					if (cntr == 2){
						r.assign_host(atoi(word.c_str()));
					}
				}
				r.calc_port_no();
				r.setup_link();
			}
			in.close();
		}
		else{
			cout << "Unable to open to network topology file";
		}
	}

};


int main(){
	routercontroller rc;
	rc.read_topo();
	while (1){

	}

	return 0;
}
