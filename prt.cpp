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

vector<PRTentry> PRT::export_table(){
	boost::unique_lock<boost::timed_mutex> lock(* _mutex, boost::try_to_lock);
	vector<PRTentry> res;
	bool getLock = lock.owns_lock();
	if(!getLock){
		getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
	}
	if(getLock){
		res = _table;
	}
	boost::timed_mutex *m = lock.release();
	m->unlock();
	return res;
}

void PRT::import_table(vector<PRTentry> newTable){
	boost::unique_lock<boost::timed_mutex> lock(* _mutex, boost::try_to_lock);
	bool getLock = lock.owns_lock();
	if(!getLock){
		getLock = lock.timed_lock(boost::get_system_time() + boost::posix_time::seconds(0.5));
	}
	if(getLock){
		_table = newTable;
	}
	boost::timed_mutex *m = lock.release();
	m->unlock();
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
