/*
 * testrt.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: vietnguyen
 */

#include "rt.h"
#include <iostream>

using namespace std;
int main(){
	int CID, IID, nHops;
	cout << "Enter CID: ";
	cin >> CID;
	cout << "Enter IID: ";
	cin >> IID;
	cout << "Enter nHops: ";
	cin >> nHops;
	RTentry entry(CID, IID, nHops);
	RT table;
	table.add_entry(entry);
	table.print_table();
	for(int i = 0; i < 101; i++){
		cout << i << "\t";
		table.update_table();
		table.print_table();
	}
	return 0;
}
