#include "common.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <pthread.h>
using namespace std;

//================================================================
// Packet Header class
//
PacketHdr::PacketHdr()
{ 
  length_ = 0;  
  info_ = new  unsigned char[MAX_HEADER_SIZE] ;
}

int PacketHdr::getIntegerInfo(int position)
{
  int val;
  unsigned char *p = info_ +position;
  val = *(p++);
  val = val << 8 | *(p++);
  val = val << 8 | *(p++);
  val = val << 8 | *(p);
  
  return val;
}

short PacketHdr::getShortIntegerInfo(int position)
{
  short val;
  unsigned char *p = info_ + position;
  val = *(p++);
  val = val << 8 | *(p++);

  return val;
}

void PacketHdr::setIntegerInfo(int a, int position)
{
  unsigned char *p = info_ + position;
  *(p++) =  a >> 24;
  *(p++) = (a >> 16) & 0xFF;
  *(p++) = (a >> 8) & 0xFF;
  *(p++) =  a & 0xFF;
  length_ +=4;
}

void PacketHdr::setShortIntegerInfo(short b, int position)
{
  unsigned char *p = info_ + position;
  *(p++) =  b >> 8;
  *(p++) =  b & 0xFF;
  length_+=2;
}


//=================================================================
//  Packet Class

Packet::Packet()
{
  size_ = 0;
  length_ = DEFAULT_PAYLOAD_SIZE;
  payload_ = new char[DEFAULT_PAYLOAD_SIZE];
  header_ = new PacketHdr();
}

/**
 * Alternate Packet Constructor:
 * Specify a large buffer....
 * This is useful to define a packet receiving buffer.
 */
Packet::Packet(int buffer_length)
{
  size_ = 0;
  length_ = buffer_length;
  payload_ = new char[buffer_length];
  header_ = new PacketHdr();
}

/** 
 * set packet size.
 * As packet already has a default buffer, 
 * there are two ways to determine the payload
 *  - set size only, let the payload be as it is --> SetPayloadSize
 *   -# if necessary, adjust the payload buffer size.
 *  - set size and also fill the payload with speficic data (e.g.  for Audio and Video Applications...) 
 * @see  fillPayload
 */

void Packet::setPayloadSize(int size)
{
  size_ =  size;
  if (size > length_) {
    if (payload_ != NULL) delete [] payload_;
    length_ = (int)(1.5 * size);
    payload_ =  new char[length_];
  }
}

 /** 
  *  A function to fill payload.
  *  user can specify the content of payload for applications like audio/video playback...
  */
int Packet::fillPayload(int size, char *inputstream)
{
  setPayloadSize(size);
  if (memcpy((char *)payload_, (char *)inputstream,  size) == NULL) {
    throw "Fill payload Failed";
  }
  return 0;
}

/**
 *  Assemble header and payload and headersize in a single stream
 */

int Packet::makePacket( char *streambuf )
{
  streambuf[0]= ( header_->getSize() ) & 0xff;
  streambuf[1]=  0x00;
  memcpy(streambuf+1, header_->accessInfo(), header_->getSize());
  memcpy(streambuf+1+header_->getSize(), payload_, size_);
  
  return 1+size_+ header_->getSize();
}

/**
 *  Extract packet  header from the incoming stream
 */
void Packet::extractHeader( char *streambuf )
{
  char* p= streambuf; 
  int a  = *(p++);
  header_->setHeaderSize(a);
  memcpy( header_->accessInfo(), p ,a);   
}


// =========================================================================
// Address Class

Address::Address():port_(-1)
{   
  hostname_[0] = '\0';  
  macaddr_[0] = '\0';
}

Address::Address(const char* hostname, short port)
{ 
   setPort(port); 
   setHostname(hostname);
}

/**
 *Function to convert the input MAC address string to bytes.
 * First, check the MAC address is valid
 * - there are at least 12 Hex characters
 * - there are no other charcter except colon
 */
void Address::setHWAddrFromColonFormat(const char* colonmac)
{  
  char HexChar;
  //First verify the address
  int Count  = 0;
  int num_mac_char = 0;
  /* Two ASCII characters per byte of binary data */
  bool error_end = false;
  while (!error_end)
    { /* Scan string for first non-hex character.  Also stop scanning at end
         of string (HexChar == 0), or when out of six binary storage space */
      HexChar = (char)colonmac[Count++];
      if (HexChar == ':') continue;     
      if (HexChar > 0x39) HexChar = HexChar | 0x20;  /* Convert upper case to lower */
      if ( (HexChar == 0x00) || num_mac_char  >= (MAC_ADDR_LENGTH * 2) ||
           (!(((HexChar >= 0x30) && (HexChar <= 0x39))||  /* 0 - 9 */
             ((HexChar >= 0x61) && (HexChar <= 0x66))) ) ) /* a - f */ 
	{
	  error_end = true;
	} else 
            num_mac_char++;
    }
  if (num_mac_char != MAC_ADDR_LENGTH * 2 )
    throw "Given Wrong MAC address Format.";

  // Conversion
  unsigned char HexValue = 0x00;
  Count = 0;
  num_mac_char = 0;
  int mac_byte_num = 0;
  while (mac_byte_num < MAC_ADDR_LENGTH )
    {
      HexChar = (char)colonmac[Count++];
      if (HexChar == ':') continue;
      num_mac_char++;  // locate a HEX character
      if (HexChar > 0x39)
        HexChar = HexChar | 0x20;  /* Convert upper case to lower */
      HexChar -= 0x30;
      if (HexChar > 0x09)  /* HexChar is "a" - "f" */
	HexChar -= 0x27;
      HexValue = (HexValue << 4) | HexChar;
      if (num_mac_char % 2  == 0 ) /* If we've converted two ASCII chars... */
        {
          macaddr_[mac_byte_num] = HexValue;
	  HexValue = 0x0;
	  mac_byte_num++;
        }
    }  
  return;   
}


/**
 * Convert HW Address to  Colon Seperated format
 */
char *Address::convertHWAddrToColonFormat()
{
  char *colonformat =  new char[17];
  //printf("HW Address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",u[0], u[1], u[2], u[3], u[4], u[5]);
  sprintf(colonformat,"%02X:%02X:%02X:%02X:%02X:%02X",
          macaddr_[0],macaddr_[1],macaddr_[2],macaddr_[3],macaddr_[4],macaddr_[5]);
  // cout << colonformat << endl;
  return colonformat;

}

/**
 *copy mac address
 */
void Address::setHWAddr(unsigned char* hwaddr)
{
  memcpy(macaddr_, hwaddr , MAC_ADDR_LENGTH*sizeof(unsigned char));
}

/**
 * Compare if two mac address is same or not.
 * use memcmp to compare each byte.
 */
bool Address::isSameMACAddr( Address *addr)
{
  if ( memcmp(macaddr_, addr->getHWAddr(), MAC_ADDR_LENGTH*sizeof(unsigned char))  == 0 )
           return true;
  return false;

}

Address* Address::clone()
{
  Address * ad =  new Address(hostname_, port_);
  ad->setHWAddr(macaddr_);
  return ad;
}

//===============================================================
// Port Class

/**
 * Constructor.
 */

Port::Port():sockfd_(0)
{
}

void Port::setAddress(Address* addr)
{
  setHostname(addr->getHostname());
  setPort(addr->getPort());
}

void Port::setRemoteAddress(Address* daddr)
{
  setRemoteHostname(daddr->getHostname());
  setRemotePort(daddr->getPort());
}

 /**
  * Fill sockaddr_in 'address' structure with information taken from
  * 'addr' and return it cast to a 'struct sockaddr'.
  * It handles following situations:
  * - if hostname is given as empty "", then INADDR_ANY is used in return
  * - if an IP address is given, then address could be set directly
  * - if a hostname is given, call gethostbyname() to find the ip address of the hostname from DNS
  */
struct sockaddr * Port::setSockAddress(Address *addr, struct sockaddr_in *address)
{
  char *hostname;
  int port;
  unsigned int tmp;
  struct hostent *hp;

  hostname = addr->getHostname();
  port = addr->getPort();

  address->sin_family = AF_INET;
  address->sin_port   = htons((short)port);
      
  if (strcmp(hostname, "") == 0) {
    address->sin_addr.s_addr = htonl(INADDR_ANY);  
  } 
  else {
    //tmp = inet_addr(hostname);  // If an IP address is given
    tmp = inet_aton(hostname, &(address->sin_addr));
    //if(tmp != (unsigned long) INADDR_NONE){    
    //  address->sin_addr.s_addr = tmp;  
    //}
    if (tmp == 0) 
    {  // if a hostname is passed, call DNS
      if ((hp = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname");
	throw "Error in Resolving hostname!" ;                           
      }
      memcpy((char *)&address->sin_addr, (char *)hp->h_addr, hp->h_length);
    }
  }
  return (sockaddr*)address;
}

/**
 * Function to interpreate hostname and port from the SocketAddress
 *
 */
void Port::decodeSockAddress(Address *addr, struct sockaddr_in *address)
{
  addr->setHostname(inet_ntoa(address->sin_addr));
  addr->setPort(ntohs(address->sin_port));  
}



//==========================================================
// Sending Port Class


SendingPort::SendingPort(): Port(),bcastflag_(0),timer_(this)
{
  setHostname("localhost"); 
  setPort(DEFAULT_SEND_PORT);  
  // setRemoteHostname("internal2");  
}


SendingPort::SendingPort(char *hostname, short port):Port(),bcastflag_(0),timer_(this)
{
  setHostname(hostname);
  setPort(port);
  //setRemotePort(DEFAULT_RECV_PORT); 
}

/** Init Funciton to initialize a socket port.
 *  Init will do
 *  - create socket
 *  - bind socket
 * 
 *  Notes:
 * Bind to local address is one important task in init() 
 * Here source address of node itself (myaddr_) does not really be used by bind function of port.
 * The program use INADDR_ANY as the address filled in address parameters of bind().
 * So, we need an empty hostname with the port number.
 *  
 * 
 */
void SendingPort::init()
{
  //"0" shows the sockfd is uninitialized, -1 means error
  if (sockfd_ != 0) {
    cout << "socket has not been properly initialized!" << endl;
    return;
  }
  if ( myaddr_.isSet() == false) {
    setHostname("localhost");
    setPort(DEFAULT_SEND_PORT);
  }
  //check if itsaddr_ is set
  if ( itsaddr_.isSet() == false)
    throw "Destination address of a sending port is not set!";

  if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    throw "Error while opening a UDP socket";
  }
  Address *emptyAddr = new Address("", myaddr_.getPort());
  struct sockaddr* addr = setSockAddress(emptyAddr, &mySockAddress_);
  if (  bind(sockfd_, addr, sizeof(struct sockaddr_in))  < 0 ){
    perror("bind");
    throw "Scoket Bind Error";
  }
   
  if (bcastflag_ == 1) 
    if (setsockopt(sockfd_,SOL_SOCKET,SO_BROADCAST,&bcastflag_,sizeof(bcastflag_)) == -1   )
         {
           perror("setsockopt");
	   throw "Set broadcast option failed.";
         }; 

  //create sending buffer
  sendingbuf_ = new char[MTU_SIZE+1];
  return; 

}

/** The main send function of UDP Socket Sending Port.
 * 
 *  call sendto()
 */

void SendingPort::sendPacket(Packet* pkt)
{  
   int pktlen = pkt->makePacket(sendingbuf_); 
   Address *dst = getRemoteAddr();
   int  length = sizeof(struct sockaddr_in); 
   struct sockaddr *dest = setSockAddress(dst, &dstSockAddress_);
   int len = sendto(sockfd_, sendingbuf_, pktlen, 0, dest, length); 
   if (len == -1) 
   { 
         perror("send");
         throw "Sending Error.";
   }
}

//====================================================================
// Functions of ReceivingPort class
/**
 * Constructor
 */

ReceivingPort::ReceivingPort(): Port()
{
  pkt_= new Packet(MAXBUFLENGTH);
}

/** Init Funciton to initialize a socket port.
 *  
 * The port binds to its own address, generate a UDP socket.
 *
 * Bind to local address is one important task in init() 
 * Here source address of node itself (myaddr_) does not really be used by bind function of port.
 * The program use INADDR_ANY as the address filled in address parameters of bind().
 * So, we need an empty hostname with the port number.
 * 
 * When the UDP port is recieving, we need to create a default 
 * buffer to store received packet. The data in buffer will be copy to the
 * corresponding flow once the packet's sender address is checked.  
 * 
 */
void ReceivingPort::init()
{ 
  if (sockfd_ != 0) {
    return;
  }
  if ( myaddr_.isSet() == false) {
    setHostname("localhost");
    setPort(DEFAULT_RECV_PORT);
  }
  
  if ((sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw "Error while opening UDP socket of a receiver";
  }
  Address *emptyAddr = new Address("", myaddr_.getPort());
  struct sockaddr* addr = setSockAddress(emptyAddr, &mySockAddress_);
  if (  bind(sockfd_, addr, sizeof(struct sockaddr_in))  < 0 ){
    throw "Scoket Bind Error occured in an UDP receiver";
  }
  //cout << "binding to port: " << myaddr_.getPort() << "......" << endl;
  // needs a dummy buffer for storing packets
  tmpBuffer_ =  new char[MAXBUFLENGTH];
}

/** The main receive function of a receiving port.
 * First,check addresses. Then  call recvfrom() to get a packet.
 * after this, pkt_ variable stores information of the packet and tmpSockAddr stores the sender information.
 * then, recast tmpSockAddr to itsaddr_.
 * As the socket is not set as non-blocking, the recvfrom() call blocks usually, but if the main program use select() to do 
 * synchronized I/O Multiplexing, this call will not block.
 * 
 * The design structure allows a future implementation improvement.
 *
 * packet size is the maximum allowed packet above UDP or buffer size
 * 
 */

Packet* ReceivingPort::receivePacket()
{
  struct sockaddr_in tmpSockAddr;
  int length = sizeof(struct sockaddr);
  int len = (int)recvfrom(sockfd_, tmpBuffer_, MAXBUFLENGTH, 0, (struct sockaddr*)&tmpSockAddr,(socklen_t *)&length); 
  if (len == -1) 
  {
           perror("recvfrom");
           return false;
  }         
  decodeSockAddress( &itsaddr_, &tmpSockAddr);
  pkt_->extractHeader(tmpBuffer_);
  //tmpBuffer_ pointer shall not be moved/shifted,it will reused by the receiving port.
  pkt_->fillPayload(len-1-pkt_->getHeaderSize(), tmpBuffer_+pkt_->getHeaderSize()+1 );
  return pkt_;
}
 
//===================================================================
//    LossyReceivingPort class
/**
 *  Constructor with parameter (drop probability p)
 *  Using a fixed link delay: 1 second 
 */
LossyReceivingPort::LossyReceivingPort(float lossyratio): ReceivingPort(), loss_ratio_(lossyratio),secdelay_(1)
{
}

/**
 * Simulate link delay of 1 seconds. Drop packets with a propabilty equal to loss_ratio 
 */
Packet* LossyReceivingPort::receivePacket()
{   
  Packet *p = ReceivingPort::receivePacket();
  //simulate some delay
  sleep(secdelay_); //delay
  float  x;
  // Set evil seed (initial seed)
  srand( (unsigned)time( NULL ) );
  x = (double) rand()/RAND_MAX;
  //cout << x << endl;
  if ( x <= loss_ratio_) 
    return NULL;
  else
    return p;
}


//====================================================
///         TxTimer class
//===================================================

TxTimer::TxTimer(SendingPort *txport)
{
  port_ = txport;  
  tdelay_.tv_nsec = 0;
  tdelay_.tv_sec =  0; 
}

void *TxTimer::timerProc(void *arg) {
  TxTimer *th = (TxTimer *)arg;
  nanosleep(&(th->tdelay_), NULL);
  th->port_->timerHandler(); 
  return NULL;
}

void TxTimer::startTimer(float delay)
{
  tdelay_.tv_nsec = (long int)((delay - (int)delay)*1e9);
  tdelay_.tv_sec =  (int)delay; 
  int error = pthread_create(&tid_, NULL, &timerProc, this );
  if (error) 
    throw "Timer thread creation failed...";
  
}
void TxTimer::stopTimer()
{
  pthread_cancel(tid_);
}
