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
