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
       cout << "The last sent packet has not been acknowledged yet. Re-send..." <<endl; 
       sendPacket(lastPkt_);       
       //schedule a timer again
       timer_.startTimer(2.5);
      }              
  }  
private:
  bool ackflag_;
public:
  Packet * lastPkt_;
};
