/*
 * prt.cpp
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#include <iostream>
#include "prt.h"

using namespace std;

void PRTentry::updateTTE(){
	_TTE = _TTE - TIME_TO_REVISIT;
}

void PRT::add_entry(PRTentry entry){
	_table.push_back(entry);
}

void PRT::update_table(){
	int n = _table.size();
	vector<PRTentry>::iterator it = _table.begin();
	while(it!=_table.end()){
		it->updateTTE();
		if(it->getTTE() <= 0){
			it = _table.erase(it);
		}else{
			it++;
		}
	}
}

void PRT::print_table(){
	int n = _table.size();
	vector<PRTentry>::iterator it = _table.begin();
	cout << "Routing table: \n";
	while(it!=_table.end()){
		cout << "\t" << it->getCID() << "\t|\t" << it->getHID() << "\t|\t" << it->getIID() << "\t|\t" << it->getTTE()  << "\n";
		it++;
	}
}
