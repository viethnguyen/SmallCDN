/*
 * rt.h
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#ifndef RT_H_
#define RT_H_

#include <vector>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

using namespace std;

class RTentry{
private:
	const static int TIME_TO_REVISIT = 30;	// in seconds
	const static int INITIAL_TTE = 100;
	int _CID;		//Content ID
	int _IID;		//Interface ID
	int _nHops;		//number of Hops
	int _TTE;		//Time to expire (in seconds)
public:
	RTentry(int CID, int IID, int nHops){
		_CID = CID;
		_IID = IID;
		_nHops = nHops;
		_TTE = INITIAL_TTE;
	}
	void updateTTE();
	int getTTE(){
		return _TTE;
	}
	int getCID(){
		return _CID;
	}
	int getIID(){
		return _IID;
	}
	int getnHops(){
		return _nHops;
	}
};

class RT{
private:
	vector<RTentry> _table;
public:
	boost::mutex *_mutex;
	RT(){
		_mutex = new boost::mutex();
	}
	vector<RTentry> export_table();
	void add_entry(RTentry entry);
	RTentry *get_entry(int CID);
	void delete_entry(int CID);
	void update_table();
	void print_table();
};
#endif /* RT_H_ */
