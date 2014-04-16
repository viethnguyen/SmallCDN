#ifndef COMMON_H
#define COMMON_H

#define MAX_HOSTNAME_LENGTH 256
#define MAC_ADDR_LENGTH 6
#define MAX_HEADER_SIZE 256
#define DEFAULT_SEND_PORT 3000
#define DEFAULT_RECV_PORT 4000
#define MAXBUFLENGTH 10000
#define MTU_SIZE 1500

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <ctime>

#ifndef INADDR_NONE
#  define INADDR_NONE (-1)
#endif


/// A Packet Header class
/**
 * Packethdr is an entity which represents the packet header portion of a packet.
 * Important protocol information fields are stored in the header.
 *
 * Based on the functions provided in PacketHdr.
 * You can put three datatype in a header by calling those functions:
   - 16-bit integer.
   - 32-bit integer.
   - a 8-bit character (ASCII code).

 * info_ is defined  "unsigned char" because reading a number byte by byte dose not allow any mistakes in type conversion involving the HSB.
   And both 16-bit and 32-bit data are large enough for any
   sequence number or other signaling appearing in a short test, so we do not need functions for 64-bit number.
 */
class PacketHdr{
 
 public:
  PacketHdr();
  /**
   * Clear all information fileds as empty
   */
  inline void init(){memset(info_,0,MAX_HEADER_SIZE );}
  /**
   * read the information filed at "postion" as a short integer
   */
  short getShortIntegerInfo(int position);
  /**
   * read the information filed at "postion" as a 32-bit integer
   */
  int getIntegerInfo(int position);
  /**
   * Get a pointer to the actual information header .
   */
  inline unsigned char* accessInfo(){return info_;}
  /**
   * get the length(size) of the header
   */
  inline int getSize() { return length_; } 
  /** set a 4-byte(32-bit) information field with an integer 
   *
   */
  void setIntegerInfo(int a, int position);
  /** set a 2-byte(16-bit) information field with an short integer 
   *
   */
  void setShortIntegerInfo(short b, int position);
  /** set one octet as a desired character 
   *  An octet in computer networking is an eight bit quantity
   */
  inline void setOctet( unsigned char c, int position){ *(info_+position)= c; length_++;}
  /**
   * Set the header size
   */
  inline void setHeaderSize(int len){ length_ =len; }
  /**
   * get an octet
   */ 
  inline unsigned char getOctet(int position){ return info_[position];}
  
 protected :
   unsigned char* info_;   ///<pointer to the header's content 
   int length_; ///<length of the header
};


///A Packet class
/**
 * Packet is an entity which contains a set of ordered information bits. 
 * This packet object only carries a pointer to
 * payload information, not any socket address information. 
 */
class Packet {
 
public:
  /**
   *  Default payload size is 512 Bytes
   */
  static const int DEFAULT_PAYLOAD_SIZE = 512;
  Packet();
  Packet(int buffer_length);
  int fillPayload(int size, char *inputstream);  
   /** get a pointer to the payload
    * @return  a char pointer
    */
  inline char* getPayload(){ return payload_;} 
  void setPayloadSize(int size);
  /**get the size of packet buffer where payload is stored.
   *
   */
  inline int getBufferSize(){return length_;}
 /** get the size of the packet 
  *  
  */
  inline int getPayloadSize(){ return size_;}
  /**
   *get the size of the packet header
   */
  inline int getHeaderSize(){ return header_->getSize();}
  /**
   *  Get the packet header
   */
  inline PacketHdr* accessHeader() { return header_;} 
  void extractHeader (char * streambuf);
  int makePacket (  char* streambuf);
 
protected:               
      int size_;  ///< Packet length in Bytes
      int length_; ///< Maximum allocated Size of Payload buffer. it is no less than the size_
      char* payload_; ///< Payload Pointer.
      PacketHdr * header_; ///<Header Pointer;
};

/// An address class
/**
 *  Address Class is to handle addresses of unix/linux sockets.
 *  For normal sockets, the address used will be a combination of IP address and port.
 *  In Socket Programming, IP address itself is usually not enough to distinguish an connection, port # is also needed.
 *  For PF_PACKET sockets, the address used is MAC address (HW address). So, we also put macaddr_ as a member variable.
 */

class Address
{
  
 public:
  /// Constructor    
  Address();
  /// Alternative construcor with parameters
  Address(const char* hostname, short port);
   
  /**
   * Check if an address has already been set or remain uninitialized
   *
   */ 
  inline bool isSet() { return (hostname_[0] != '\0' && port_ >= 0); }
  /**
   * set the port # of the Address
   */
  inline void setPort(const short port){ port_ = port; }
  /// get the port #
  inline short getPort(){ return port_; }
  /// set the hostname
  /** use strcpy function to duplicate a string
   *
   */
  inline void setHostname(const char* hostname) { 
    if (hostname == NULL) hostname_[0] = '\0'; else strcpy(hostname_, hostname); }
  /// get the hostname string pointer
  inline char* getHostname() {return hostname_;} 
  /// get the MAC address
  inline   unsigned char* getHWAddr() { return  macaddr_;} 
  void setHWAddr( unsigned char* hwaddr); 
  void setHWAddrFromColonFormat(const char* colon_seperated_macaddr);
  char * convertHWAddrToColonFormat();
  /// function to clone this address
  Address *clone();
  /**
   *Compare whether the two normal "name+port" address is same or not
   */
  inline bool isSame(Address* addr)
  {
    if ( port_==  addr->getPort()) return false;
    if ( strcmp( hostname_, addr->getHostname() ) == 0 ) return true;
    return false;
  } 
  bool isSameMACAddr(Address* addr);

protected:
  char   hostname_[MAX_HOSTNAME_LENGTH]; ///< both hostname and ipaddress format (10.0.0.1) could be given as a string
  short port_;  ///< port number for UDP or TCP (Transport layer)
  char * ipaddr_;  ///<optional use... ignore.... 
  unsigned char   macaddr_[MAC_ADDR_LENGTH]; ///< optional use for Ethernet Socket
  
};

/// Port class abstacts functions of communication interfaces
/** 
 * Port is an abstract class for the interface to send/receive a packet, whether UDP, TCP Socket, or IP raw socket.
 * The common funcitons for a port are defiend here:
 * - init
 * - setSocketAddress
 *
    Port is also associated with a pair of address. One Port send to one Address only,
    no matter the address is unicast or broadcast.
*/


class Port
{
 public:
  /// Constructor
  Port();
  /// Deconstructor
  virtual ~Port(){}
   /**
    * Function to initialize the port
    */
  virtual void init()=0;
  /// set the port's own address
  void setAddress(Address *addr);
  /// set the address of the port at the other end of communication link
  void setRemoteAddress( Address *daddr);
  /// get the address of the port at the other end of communication link
  inline Address *getRemoteAddr(){return &itsaddr_;}
  /// close the port
  inline void closePort(){close(sockfd_);}
 protected:
  /// cast an Address to socket address format
  struct sockaddr* setSockAddress(Address *addr, struct sockaddr_in *address);
  /// cast a socket address to normal address format
  void decodeSockAddress ( Address *addr, struct sockaddr_in *address);
  /// set hostname of local address
  inline void setHostname(const char* hostname){ myaddr_.setHostname(hostname);}
  /// set port of local address
  inline void setPort(const short port){ myaddr_.setPort(port);}
  /// set hostname of remote address
  inline void setRemoteHostname(const char* hostname){itsaddr_.setHostname(hostname);}
  /// set port no of a remote address
  inline void setRemotePort(const short port){itsaddr_.setPort(port); }
  /// get the socket file descriptor
  inline int getSock(){ return sockfd_;}
 
 protected:
  Address myaddr_;  ///< The default address of mine 
  Address itsaddr_; ///< The default address of the other end of communication link.
  struct sockaddr_in mySockAddress_;  ///<IN UDP. this will be INADDR_ANY, not really my IP addr
  struct sockaddr_in dstSockAddress_;  ///< for every outgoing packet of a connection, the destination address.
  int sockfd_; ///<socket file descriptor
};

class SendingPort;

/// A timer to schedule a later event/transmission occured in a port
/** The timer is associated with a SendingPort object.
 *  when the timer expires, the SendingPort::timerHandler() will be called.
 *
 * 
The intenal design of this class is a little tricky. Usually, 
LinuxThreads does not support a thread function as a member function of C++ class.
I designed timerProc as a static function, and give the class pointer as the 4th argument of the pthread_create
 */

class TxTimer{
friend class SendingPort;
 public:
    TxTimer(SendingPort *txport);
    /**
     * Function to start a timer which will expire after a certain delay
     * @param delay: the timing delay in seconds.
     */
    void startTimer(float delay);
     /**
      * Function to stop a timer
      */
    void stopTimer();
 public:
    /** Function to create a seperate thread for this timer
     * it will call timerHandler() function of the port_
     */
    static void *timerProc(void *arg); 
 protected:
    /**
     * port the timer belongs to
     */
    SendingPort *port_;
    /**
     * delay variable used by nanosleep()
     */
    struct timespec tdelay_;
    /**
     * thread id variable
     */
    pthread_t tid_;
};



/// SendingPort is an subclass of Port for sending purpose
/** SendingPort is an subclass of Port for sending purpose
 *
 */
class SendingPort : public Port
{
 public:
  SendingPort();
  /// Another constructor with local address given
  SendingPort(char* hostname, short port);
  ///Deconstructor
  virtual ~SendingPort(){}
   /**
    * Function to initialize the port
    */
  void init();
   /**
    * Function to send a packet.
    * The default socket file descriptor will always be used for send()
    * only a sockfd used by the port.
    */
  void sendPacket(Packet *pkt);
  /**
   * toggle broadcast option on
   */
  inline void setBroadcast(){bcastflag_ = 1;}
  /**
   * toggle broadcast option off
   */
  inline void setBroadcastOff(){bcastflag_ = 0;}
  /** 
   * TimerHandler is called when the TxTimer expires.
   * This function is virtual. So another child class has to be derived from this 
   * base class to use the timer.
   */
  virtual void timerHandler()=0;


 protected:
  /// this flag indicates the port ought to broadcast or unicast a packet. 
  /** 
   * If broadcast, an broadcast IP address (192.168.255.255, etc) need to be supplied
   */
  int bcastflag_; 
  /**
   * Sending buffer
   */
  char *sendingbuf_;

 public:
  /**
   * The timer used to schedule future events in a sending port.
   * When this timer expires, the timerHandler will be called.
   */
   TxTimer timer_;
};

/// A Receiving Port Class
/** 
 * ReceivingPort is an abstract class for the interface to receive a packet.
 * The main function for the receiving port:
 * - initialize
 * - Receive Packet
 *
 */

class ReceivingPort : public Port
{
 public:
  ReceivingPort();
  virtual ~ReceivingPort(){}
  void init();
  ///The main receive function of receiving port to receive a single packet.
  Packet* receivePacket(); 
  
 protected:
   /**  This pointer points to the packet.
    *   This packet is just received.
    */
   Packet *pkt_;   
   char *tmpBuffer_;  ///<temporary buffer for packets
};

///A receiving port simulating link loss and delay
/**
 * A receiving port which would random drop packets and delay packet reception in 1 second.
 */
class LossyReceivingPort: public ReceivingPort
{
 public:
  LossyReceivingPort(float lossyratio);
  ~LossyReceivingPort(){}
  Packet* receivePacket();
 protected:
  float loss_ratio_; ///<how probable a packet will get dropped in receiving
  int secdelay_;     ///<how large is the link propagation delay.
};

#endif


