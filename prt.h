/*
 * prt.h
 *
 *  Created on: Apr 20, 2014
 *      Author: vietnguyen
 */

#ifndef PRT_H_
#define PRT_H_

#include <vector>

using namespace std;

class PRTentry{
private:
	const static int TIME_TO_REVISIT = 10;
	const static int INITIAL_TTE 	= 1000;
	int _CID;		// Requested content ID
	int _HID;		// Host ID
	int _IID;		// Incoming interface ID
	int _TTE;		// Time to expire
public:
	PRTentry(int CID, int HID, int IID){
		_CID = CID;
		_HID = HID;
		_IID = IID;
		_TTE = INITIAL_TTE;
	}
	void updateTTE();
	int getCID(){
		return _CID;
	}
	int getHID(){
		return _HID;
	}
	int getIID(){
		return _IID;
	}
	int getTTE(){
		return _TTE;
	}
};

class PRT{
private:
	vector<PRTentry> _table;
public:
	void add_entry(PRTentry entry);
	void update_table();
	void print_table();
};
#endif /* PRT_H_ */
