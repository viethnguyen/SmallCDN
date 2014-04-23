/*
 * sendthread.h
 *
 *  Created on: Apr 23, 2014
 *      Author: vietnguyen
 */

#ifndef SENDTHREAD_H_
#define SENDTHREAD_H_

#include "common.h"
#include "message.h"
#include <pthread.h>

class linkthread {
public:
	linkthread() {/* empty */}
	linkthread(int srcport, int dstport){
		_srcport = srcport;
		_dstport = dstport;
	}
	~linkthread() {/* empty */}

   /** Returns true if the thread was successfully started, false if there was an error starting the thread */
   bool StartInternalThread()
   {
	  return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
   }

   /** Will not return until the internal thread has exited. */
   void WaitForInternalThreadToExit()
   {
	  (void) pthread_join(_thread, NULL);
   }

   /**
    * Set mode for this link thread, 0 = sending, 1 = receiving
    */
   void setmode(int mode){
	   _mode = mode;
   }

protected:
   void send_message();
   void receive_message();
	void InternalThreadEntry();
private:
	int _srcport;
	int _dstport;
	int _mode;	//0: sending, 1: receiving
	static void * InternalThreadEntryFunc(void * This) {((linkthread *)This)->InternalThreadEntry(); return NULL;}
    pthread_t _thread;
};

#endif /* SENDTHREAD_H_ */
