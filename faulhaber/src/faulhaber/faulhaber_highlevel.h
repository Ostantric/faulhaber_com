#include "gtest/gtest.h"
#include "units.h"
#include "util.h"
//#include "poll.h" //discussion needed
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include<fstream>
#include<stdio.h>
//#include <boost/asio.hpp> use, if low level must be in c++
#ifdef __cplusplus
extern "C"
{
#endif

#include "faulhaber_low_level_SerialPort.h"

#ifdef __cplusplus
}
#endif
/*********************************FAULHABER STRUCTURE**********************************************/
/*Command Frame

1[Node number] 2Command 3[Argument] 4Carriage-Return(ASCII 13)

- The node number is optional and is only required if several drives are being operated on one interface.
- The command consists of a letter character string.
- The optional argument consists of an ASCII numeric value.
-The end is always a CR character (Carriage Return, ASCII decimal code 13). Space charac-
ters are ignored, and no distinction is made between upper and lower case.
*/

/*Response Frame

1Response 2Carriage-Return(ASCII 13) 3Line-Feed(ASCII 10)

-The response frames do not contain a node number.In bus mode you must therefore ensure that the response of the contacted node is received before a new command is sent.

*/


/*RESPONSE BEHAVIOUR SETTING
set ANSW2 to get response
set ANSW0 to disable responses
*/

/*util error order
	UNKOWN, OK, NULL_PTR, NOT_INITIALIZED, INVALID, TIMEOUT, MEMORY, FAIL
	   0	1	2	      3		  4	   5	   6	  7
*/

/* COMMANDS that I should consider using
-------
LA - absolute target position
LR - relative target position
V - set velocity mode
-------
POS -current position, returns a number
TPOS -target position(based on LR or LA), returns a number
GV   -target velocity(based on V), returns a number
GN  -current velocity, returns a number
GOHOSEQ - homing sequence, if reponse set, returns 'p' ? or just 'OK' confirmation ?
NV - Notify Velocity, returns 'v' when nominal speed reaches if ASNW1 or ANSW2 is set.
M - Move --this will return 'p' if NP set.
OST - Operation status, returns binary;
	0(LSB) - Homing Running
	1      - Program sequence running
	....
	16     - Position attained
ANSW2 - set async reponses and confirmation
NP - notify position
*/


/*###########################IMPORTANT###################################*/
//uncomment this if you don't want to see in between ack messages etc..
//#define FULL_INFO 
namespace faulhaber {
  struct State {
    State(bool _homed, bool _moving, units::Length _position, units::Velocity _velocity) :
      homed(_homed), moving(_moving), position(_position), velocity(_velocity) {};

    bool homed;
    bool moving;

    units::Length position;
    units::Velocity velocity;
  };
  
  
  class Client {
    public:
    Client(const char *_serialport, int _baudrate)
    {
	serialport=_serialport;
	baudrate=_baudrate; 
    }
    //Important: Assumed that ANSW2 repsones OK
    util::ErrorT DefaultValues(double timeout)//ANSW2 and NP commands
	{
 	    int feedback1;
	    int feedback2;
	    //allow asynchronous responses
	    feedback1=Async_operation(fd,serialport,baudrate,"ANSW2","OK","\n","ANSW2-Acknowledgement received",timeout); //send ASNW2 command and wait for ack
	    switch(feedback1){
		case 1 : break;//do not exit, keep going for second command
		case -1 : return util::Error::FAIL;
		case -2 : return util::Error::TIMEOUT;
		default : return util::Error::INVALID;
		}
	    feedback2=Async_operation(fd,serialport,baudrate,"NP","OK","\n","NP-Acknowledgement received",timeout); //send ASNW2 command and wait for ack
	    switch(feedback2){
		case 1 : return util::Error::OK;//now we can exit. However same as starthome discussion, thread can be created here. 
		case -1 : return util::Error::FAIL;
		case -2 : return util::Error::TIMEOUT;
		default : return util::Error::INVALID;
		}
	}
    // Start an asynchronous homing operation (kick off homing and return)
    //
    // Timeout is the maximum time to wait until assuming the starting homing
    // operation has failed (not the maximum time to allow for homing).
    //non blocking client
    util::ErrorT StartHoming(double timeout) //GOHOSEQ command
	{
		int feedback;
		feedback=Async_operation(fd,serialport,baudrate,"GOHOSEQ","OK","\n","Home-Acknowledgement received",timeout); //send GOHOSEQ command, only wait for ack
		switch(feedback){
		case 1 : {
		//Personal opinion;
		//Instead of returning right away, we can create a thread and than Getstate can join this thread with linux-poll function. so we can receive "p" without blocking the client. 
		return util::Error::OK;
			};
		case -1 : return util::Error::FAIL;
		case -2 : return util::Error::TIMEOUT;
		default : return util::Error::INVALID;
		}	
	}
    // Start an asynchrounous move to the given position.
    //non blocking client
    util::ErrorT StartMoveTo(units::Length position, double timeout) //LA and M commands
	// ASK THIS PART
	{
		int feedback1;
		int feedback2;
		feedback1=Async_operation(fd,serialport,baudrate,"LA","OK","\n","Load-position Acknowledgement received, ready to send second command",timeout,position);//send LA command, wait for ack 
		switch(feedback1){
		case 1 : break;//do not exit, keep going for second command
		case -1 : return util::Error::FAIL;
		case -2 : return util::Error::TIMEOUT;
		default : return util::Error::INVALID;
		}	
		feedback2=Async_operation(fd,serialport,baudrate,"M","OK","\n","Move-Acknowledgement received",timeout);//send M command, only wait for ack
		switch(feedback1){
		case 1 : return util::Error::OK;//now we can exit. However same as starthome discussion, thread can be created here. 
		case -1 : return util::Error::FAIL;
		case -2 : return util::Error::TIMEOUT;
		default : return util::Error::INVALID;
		}			
	}
    // Return the current state.  It should be possible for users to poll this
    // to see when StartHoming(...), StartMoveTo(...), etc have completed
    // I assumed that getstate gets, current velocity, current position, current OST from the controller
    std::unique_ptr<State> GetState(double timeout) // POS,GN,OST commands
	{
	std::stringstream ss;
	int feedback1;
	int feedback2;
	int feedback3;
	int position;
	int velocity;
	char ost_char[256];
	std::string ost_string;
	char response[256];
	bool postion_attained;
	std::unique_ptr<State> Motor_State;
   	Motor_State=std::unique_ptr<State>(new State(NULL,NULL,NULL,NULL));
	feedback1=Sync_operation(fd,serialport,baudrate,"POS","OK","\n","Get-Position completed",timeout,response,NULL); //send POS command, wait for ack and return
	switch(feedback1){
	case -1 : std::cout<<"Can't get the current state, showing NULL values:\n";return Motor_State;//return state;
	case -2 : std::cout<<"Showing NULL values:\n";return Motor_State;//return state;
	//default : break;
	break; 
	}//feedback1 end	
	ss<<response;
	ss>>position;
	Motor_State->position=position;//update position
	if (position==0)
		{
		Motor_State->homed=true;//homed
		}
	else
		{
		Motor_State->homed=false;//not homed
		}	
	ss.clear();
	memset(response, 0, sizeof(response));//clean response	
	feedback2=Sync_operation(fd,serialport,baudrate,"GN","OK","\n","Get-Velocity completed",timeout,response,NULL);
	switch(feedback1){
	case -1 : std::cout<<"Can't get the current state, showing NULL values:\n";return Motor_State;//return state;
	case -2 : std::cout<<"Showing NULL values:\n";return Motor_State;//return state;
	}	
	ss<<response;
	ss>>velocity;
	Motor_State->velocity=velocity;
	ss.clear();
	memset(response, 0, sizeof(response));//clean response
	feedback3=Sync_operation(fd,serialport,baudrate,"OST","OK","\n","Get-Operating_Status completed",timeout,response,NULL);//send OST command, wait for ack and return
	switch(feedback3){
	case -1 : std::cout<<"Can't get the current state, showing NULL values:\n";return Motor_State;//return state;
	case -2 : std::cout<<"Showing NULL values:\n";return Motor_State;//return state;
	}//feedback3 end
	sprintf(ost_char,response);//create command
	ss<<ost_char;
	ss>>ost_string;
	ss.clear();
	postion_attained=ost_string[16];//update moving
	if(postion_attained==true)
		{
		Motor_State->moving=false;//position attained so motor is not moving
		}
	else
		{
		Motor_State->moving=true;//No position attained so motor is moving
		}
	//memset(response, 0, sizeof(response));//clean response
	return Motor_State;//return state
    }
    // A helper to do a synchronous homing operation that does not return
    // until homing completes (or fails or times out)
    // blocks until receives "p"
    util::ErrorT Home(double timeout)//assume ASNW2 set as default
    {
	int feedback1;
	char response[256];
	feedback1=Sync_operation(fd,serialport,baudrate,"GOHOSEQ","OK","\n","Homing completed",timeout, response, "p");//send GOHOSEQ command, wait for ack and complete
	switch(feedback1){
	case 1 : return util::Error::OK;//do not exit, keep going for second command
	case -1 : return util::Error::FAIL;
	case -2 : return util::Error::TIMEOUT;
	default : return util::Error::INVALID;
	}//feedback1 end	
    }

    // A helper to do a synchronous move that does not return until
    // the move completes (or fails or times out)
     // blocks until receives "p"
    util::ErrorT MoveTo(units::Length position, double timeout)//assume ASNW2 set as default
    {
	
	int feedback1;
	int feedback2;
	char response[256];
	feedback1=Async_operation(fd,serialport,baudrate,"LA","OK","\n","Load-position Completed",timeout,position);//send LA command, wait for ack
	switch(feedback1){
	case 1 : break;//do not exit, keep going for second command
	case -1 : return util::Error::FAIL;
	case -2 : return util::Error::TIMEOUT;
	default : return util::Error::INVALID;
	}//feedback1 end	
	feedback2=Sync_operation(fd,serialport,baudrate,"M","OK","\n","Move Completed",timeout,response,"p");//send GOHOSEQ command, wait for ack and complete
	switch(feedback2){
	case 1 : return util::Error::OK;//now we can exit with OK
	case -1 : return util::Error::FAIL;
	case -2 : return util::Error::TIMEOUT;
	default : return util::Error::INVALID;
	}//feedback2 end	
    }
    private:
	 int fd;
    int baudrate;
    const char *serialport;
	int Async_operation(int fd,const char *serialport ,int baudrate,char command[256], char expected_acknowledgement_response[256], char *endoflinechar, char *succes_user_message, double timeout,int optional_argument=0) //1=OK,-1=fail,-2=timeout
  {
	
	int w1; //second descriptor
	char receive[256]; //response
	char send[256];
	const int buf_max = 256;
	std::stringstream ss;
	std::string converted_recieve;
	std::string converted_expected_acknowledgement_response;
	//const char port[256] = serialport->c_str();
	while(1)
	{
	if(fd!=-1) //port is already open
		{	
			serialport_close(fd);//close it for now,
			//std::cout<<"Port was already open ";
		}
	fd=serialport_setup(serialport, baudrate); //create serial and open it
	serialport_flush(fd); // flush it	
	if(fd == -1) {return -1; break;}  //can't open the port
	if(optional_argument==0)
	{
		sprintf(send,"%s\r",command);//create command
#ifdef FULL_INFO  
		std::cout<<"Command:"<<command<<" sending..\n";//inform user that write is a success
#endif
	}
	else
	{
		sprintf(send,"%s%d\r",command,optional_argument);//create command and the argument
#ifdef FULL_INFO  	
		std::cout<<"Command:"<<command<<optional_argument<<" sending..\n";//inform user that write is a success
#endif	
	}
	w1 = serialport_write(fd, send); // send data
	if(w1==-1) //failed sending
	{
		std::cout<<"Failed writing\n";
		return -1;
		break;
	}
#ifdef FULL_INFO  
	std::cout<<"Command successfully sent. Now waiting for response...\n";//inform user that write is a success
#endif
	fd=serialport_read_until(fd, receive , *endoflinechar, buf_max, timeout); //wait until response or times out
	if(fd == -1) //failed reading
	{
		std::cout<<"Failed reading\n";
		return -1; 
		break;
	}
	if(fd == -2) //timesout
	{
		std::cout<<"Reading timeout\n"; 
		return -2;
		break;
	}
#ifdef FULL_INFO  
	std::cout<<"Response:";  //informing receive
	std::cout<<receive;//comes with CR & LF
#endif
	ss<<receive; //convert to stringstream
	ss>>converted_recieve;//convert to string
	ss<<expected_acknowledgement_response;//convert to stringstream
	ss>>converted_expected_acknowledgement_response;//covnert to string
	if(converted_recieve!=converted_expected_acknowledgement_response) //comparing response
	{		
		std::cout<<"FAILURE, RESPONSE IS NOT !!"<<expected_acknowledgement_response<<"\n";  
		return -1;//return if response is not good
		break;
	}
	std::cout<<succes_user_message<<"\n";//completed message 	
	return 1;
	serialport_close(fd);
	break;	
	}
   }
	int Sync_operation(int fd,const char *serialport ,int baudrate,char command[256], char expected_acknowledgement_response[256], char *endoflinechar, char *succes_user_message, double timeout,char response_save[100], char optional_expected_completed_response[256]=NULL)//1=OK,-1=fail,-2=timeout
  {
	int w1;
	int w2;
	char receive[256]; //response
	char send[256];
	const int buf_max = 256;
	std::stringstream ss;
	std::string converted_recieve;
	std::string converted_expected_acknowledgement_response;
	std::string converted_expected_completed_response;
	//const char port[256] = serialport->c_str();
	while(1)
	{
	if(fd!=-1) //port is already open
		{	
			serialport_close(fd);//close it for now,
			//std::cout<<"Port was already open ";
		}
	fd=serialport_setup(serialport, baudrate); //create serial and open it
	serialport_flush(fd); // flush it	
	if(fd == -1) {return -1; break;}  //can't open the port	
	sprintf(send,"%s\r",command);//create command
#ifdef FULL_INFO  
	std::cout<<"Command:"<<command<<" sending..\n";//inform user that write is a success
#endif
	w1 = serialport_write(fd, send); // send data
	if(w1==-1) //failed sending
	{
		std::cout<<"Failed writing\n";
		return -1;
		break;
	}
#ifdef FULL_INFO  
	std::cout<<"Command successfully sent. Now waiting for response...\n";//inform user that write is a success
#endif
	fd=serialport_read_until(fd, receive , *endoflinechar, buf_max, timeout); //wait until response or times out
	if(fd == -1) //failed reading
	{
		std::cout<<"Failed reading\n";
		return -1; 
		break;
	}
	if(fd == -2) //timesout
	{
		std::cout<<"Reading timeout\n"; 
		return -2;
		break;
	}
#ifdef FULL_INFO  
	std::cout<<"Response:";  //informing receive
	std::cout<<receive;//comes with CR & LF
#endif
	ss<<receive; //convert to stringstream
	ss>>converted_recieve;//convert to string
	ss<<expected_acknowledgement_response;//convert to stringstream
	ss>>converted_expected_acknowledgement_response;//covnert to string
	if(converted_recieve!=converted_expected_acknowledgement_response) //comparing response
	{		
		std::cout<<"FAILURE, RESPONSE IS NOT !!"<<expected_acknowledgement_response<<"\n";  
		return -1;//return if response is not good
		break;
	}
	memset(receive, 0, sizeof(receive));//clean recieve-buffer

	fd=serialport_setup(serialport, baudrate); //create serial and open it	
	serialport_flush(fd); // flush it		
	w2=serialport_read_until(fd, receive , *endoflinechar, buf_max, timeout); //wait until response or times out
	if(w2 == -1) //failed reading
	{
		std::cout<<"Failed reading\n";
		return -1; 
		break;
	}
	if(w2 == -2) //timesout
	{
		std::cout<<"Reading timeout\n"; 
		return -2;
		break;
	}
	//response_save=receive;
	sprintf(response_save,receive);//create command
#ifdef FULL_INFO  
	std::cout<<"Response:";  //informing receive
	std::cout<<receive;//comes with CR & LF
#endif
	if(optional_expected_completed_response!=NULL)
	{
	ss.clear();
	ss<<receive; //convert to stringstream
	ss>>converted_recieve;//convert to string
	ss<<optional_expected_completed_response;//convert to stringstream
	ss>>converted_expected_completed_response;//covnert to string
	if(converted_recieve!=optional_expected_completed_response) //comparing response
	{		
		std::cout<<"FAILURE, RESPONSE IS NOT '"<<optional_expected_completed_response<<"' !!\n";  
		return -1;//return if response is not good
		break;
	}
	}
	std::cout<<succes_user_message<<"\n";//completed message 	
	return 1;
	serialport_close(fd);
	break;	
	}
   } 	
  };
}
