// TODO: Script for debugging and testing the faulhaber client.

/*default settings for FAULHABER
baudrate = 9600
port = assuming /dev/ttyS0, but tested with microcontroller at /dev/ttyACM0.
8 data bits, No parity, 1 Stop bit, No flow control

*/

#include <cstdlib>
#include "faulhaber_highlevel.h"
#include <thread> 
#include <string>
#include <memory>
#include <iostream>
#include "getopt.h"
//#include<cstring>
using namespace std;
char port[256] = "/dev/ttyS0";
const char* command = "s" ;
int baudrate=9600; //baudrate
double position=0;
double sync_timeout=60000;//msec
double async_timeout=5000;//msec
bool show_help=true;
int feedback_StartHoming,feedback_Home,feedback_MoveTo,feedback_StartMoveTo,feedback_DefaultValues;//feedbacks
/*
int home1(faulhaber::Client& dev(const char& port1, int& baudrate1))
{
  dev->Home(10000);
}
*/

void help(void)
{
    std::cout<<"Usage: [OPTIONS] -c <Commands>\n"
    "\n"
    "Options:\n"
    "  -b,  --baudrate \t\tBaudrate (default:"<<baudrate<<")\n"
    "  -s,  --serialport \t\tSerial port (default:"<<port<<")\n"
    "  -p,  --position \t\tPosition(default:"<<position<<"mm"<<")\n"	
    "  -a   --async_timeout=millis \tTimeout for asynchronous in millisecs (default: "<<async_timeout<<"ms)\n"
    "  -t   --sync_timeout=millis \tTimeout for synchronous in millisecs (default: "<<sync_timeout<<"ms)\n"
    "Note: Only common baudrates are implemented: 9600,57600,115200!\n"
    "\nRequired to run:\n"
    "\n""   -c   --commands \t\trequired to enter commands\n"
    "\n"
    "Commands:\n"
    "  getsate \t--print current Homed, Moving, Velocity, Position,\n"
    "  moveto \t--sync moving to [Position]mm, NEEDS POSITION OPTION first! otherwise default position set(0mm)\n"
    "  home \t\t--sync home\n"
    "  starthoming \t--async homing\n"
    "  startmoveto \t--async moving to [Position]mm. NEEDS POSITION OPTION first! otherwise default position set(0mm)\n"
    "  defaultvalues\t--load default values. (set ANSW2 and NP)\n"
    "  Note: GETSTATE uses async timeouts!\n"

    "\n  Order for custom values\n"
    "\t-b [baudrate] -s [serialport] -p [position] -a [async_timeout] -t [sync_timeout] -c <required COMMAND>"
    "\n" "  Example:\n"
    "\tsync-moveto 50mm, port at /dev/ttyACM1 with default baudrate:"
    "\n\t./run faulhaber -s /dev/ttyACM1 -p 50 -c moveto"
    "\n";
    "\n";
    exit(EXIT_SUCCESS);
}



int main(int argc, char *argv[]){
   std::unique_ptr<faulhaber::State> Motor_State;
   
   Motor_State=std::unique_ptr<faulhaber::State>(new faulhaber::State(NULL,NULL,NULL,NULL));	
   if (argc==1) {
	help();
    }
   int option_index = 0, opt;
   static struct option loptions[] = {
        {"serialport",       required_argument, 0, 's'},
        {"baud",       required_argument, 0, 'b'},
	{"sync_timeout",       required_argument, 0, 't'},
	{"async_timeout",       required_argument, 0, 'a'},
	{"command",    required_argument, 0, 'c'},
	{"position",    required_argument, 0, 'p'},
        {NULL,         0,                 0, 0}
    };
   while(1)
	{
    	  opt = getopt_long (argc, argv, "b:s:p:a:t:c:",
                           loptions, &option_index);
        if (opt==-1) break;	
		switch(opt){
		case '0': break;
		case 'b': 
			baudrate = strtol(optarg,NULL,10);
			cout<<baudrate;
			show_help=true;
			break;
		case 's': 
         	        strcpy(port,optarg);
			//cout<<port;
			show_help=true;
			break;
		case 'p': 
			position = strtol(optarg,NULL,10);
			//cout<<port;
			show_help=true;
			break;
		case 'a': 
			async_timeout = strtol(optarg,NULL,10);
			//cout<<port;
			show_help=true;
			break;
		case 't': 
			sync_timeout = strtol(optarg,NULL,10);
			//cout<<port;
			show_help=true;
			break;
		case 'c': 
	         	faulhaber::Client device(port,baudrate);// Create device
			command=optarg;
			std::stringstream ss; ss;
			std::string str;
			ss<<command;
			ss>>str;
			if(str=="home")
			{
			 	feedback_Home=device.Home(sync_timeout);
			}
			else if(str=="moveto")
			{
				feedback_MoveTo=device.MoveTo(position,sync_timeout);
			}
			else if(str=="getstate")
			{
			        Motor_State=std::move(device.GetState(async_timeout));
				cout<<"homed:"<<Motor_State->homed<<"\n";
				cout<<"moving:"<<Motor_State->moving<<"\n";
				cout<<"position:"<<Motor_State->position<<"\n";
				cout<<"velocity:"<<Motor_State->velocity<<"\n";
			}
			else if(str=="startmoveto")
			{
			       feedback_StartMoveTo=device.StartMoveTo(position,async_timeout);
			}
			else if(str=="starthoming")
			{
				feedback_StartHoming=device.StartHoming(async_timeout);
			}
			else if (str=="defaultvalues")
			{

				feedback_DefaultValues=device.DefaultValues(async_timeout);
			}
			else
			{
			 cout<<"\nUNKOWN COMMAND!\n\n";
			}
			show_help=false;
			break;
						
		}
		
		
	}
	if(show_help==true)
		{
			help();
		}
	exit(EXIT_SUCCESS);
}


   //device.SendDefault(async_timeout);
   //state=device.GetState(timeout).release();
   //std::cout << Motor_State->homed;
   //feedback_MoveTo=device.StartMoveTo(50,timeout_sync);
   //Motor_State=std::move(device.GetState(timeout_sync));
 	
   //feedback_home=device.Home(timeout_sync);
   //feedback_MoveTo=device.MoveTo(50,timeout_sync);
   //Motor_state->homed=true;
   //std::cout<<Motor_State.get();
