#ifndef NEWPORT_H_
#define NEWPORT_H_


#include "common.h"
#include <iostream>
using namespace std;

class mySendingPort :public SendingPort
{

public:

  inline void setACKflag(bool flag){ackflag_ =flag;}
  inline bool isACKed(){return ackflag_;}
  inline void timerHandler()
  {
    if (!ackflag_)
      {       
       cout << "The last request has not been served yet. Re-send...\n" ;
       sendPacket(lastPkt_);       
       //schedule a timer again
       timer_.startTimer(5);
      }              
  }  
private:
  bool ackflag_;
public:
  Packet * lastPkt_;
};

#endif /*NEWPORT_H_*/
