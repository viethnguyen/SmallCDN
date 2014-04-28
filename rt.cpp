/*
 * rt.cpp
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#include "rt.h"
#include <iostream>

using namespace std;

void RTentry::updateTTE(){
	_TTE = _TTE - TIME_TO_REVISIT;
}

void RT::add_entry(RTentry entry){
	_table.push_back(entry);
}

RTentry *RT::get_entry(int CID){
	vector<RTentry>::iterator it = _table.begin();
	while(it!=_table.end()){
		if(it->getCID() == CID){
			return &(*it);
		}
	}
	return NULL;
}

void RT::delete_entry(int CID){
	vector<RTentry>::iterator it = _table.begin();
	while(it!= _table.end()){
		if(it->getCID() == CID){
			it = _table.erase(it);
			break;
		}
	}
}

void RT::update_table(){
	vector<RTentry>::iterator it = _table.begin();
	while(it!=_table.end()){
		it->updateTTE();
		if(it->getTTE() <= 0){
			it = _table.erase(it);
		}else{
			it++;
		}
	}
}

void RT::print_table(){
	vector<RTentry>::iterator it = _table.begin();
	cout << "Routing table: \n";
	while(it!=_table.end()){
		cout << "\t" << it->getCID() << "\t|\t" << it->getIID() << "\t|\t" << it->getnHops() << "\t|\t" << it->getTTE()  << "\n";
		it++;
	}
}
