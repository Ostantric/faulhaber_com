//POSIX help=>https://www.cmrr.umn.edu/~strupp/serial.html
#include "faulhaber_low_level_SerialPort.h"
#include <unistd.h> 
#include <fcntl.h>    // File control definitions 
#include <string.h> 
#include <termios.h>  // POSIX terminal control definitions 
#include <sys/ioctl.h>
#include <stdio.h>   
#include <errno.h> 
#include <sys/poll.h>

// uncomment this to debugging
//#define DEBUG 

/*
-1=error
-2=timeout
0=good
as well as fd return
*/

// takes the string name of the serial port
// and a baud rate (bps) and connects to that port at that speed and 8N1. (!!DOUBLE CHECK WITH FAULHABER SETTINGS)
// returns valid fd, or -1 on error
int serialport_setup(const char* serialport, int baud)
{
    struct termios options;
    int fd;
    fd = open(serialport, O_RDWR | O_NONBLOCK ); //open serial port
    
    if (fd == -1)  { //check error right away
        perror("Couldn't open port ");
        return -1;
    }
	

    if (tcgetattr(fd, &options) < 0) { //options
        perror("Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // faulhaber supports multiple baudrate. Only 3 common rate is added for now.
    switch(baud) {
    case 9600:   brate=B9600;   break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&options, brate);
    cfsetospeed(&options, brate);
    // setting flags
    // 8N1 - 8 data bits, no parity bit, 1 stop bit
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    // no flow control flag
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    //termios handled in kernel and driver allows read() to return when buff is full or end of file reached.
    //to be able done in between we need VMIN and VTIME but i assume we dont want to change this
    //so no blocking-read for now
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 0;
    
    tcsetattr(fd, TCSANOW, &options);
    if( tcsetattr(fd, TCSAFLUSH, &options) < 0) { //baud,flag etc error
        perror("Couldn't set fd");
        return -1;
    }
    return fd;
}
//flushing
int serialport_flush(int fd)
{
    sleep(2);
    return tcflush(fd, TCIOFLUSH);
}

//close
int serialport_close( int fd )
{
    return close( fd );
}

//write
int serialport_write(int fd, const char* str)
{
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) {
        perror("couldn't write full string\n");
        return -1;
    }
    return 0;
}

//read until ENDLINE RECEIVED or timeout
int serialport_read_until(int fd, char* buf, char until, int buf_max, double timeout)
{
    char b[1];  //give it a 1byte
    int i=0;
    do { 
        int n = read(fd, b, 1);  // read a char at a time - (simple, not the fastest)
        if( n==-1) return -1;    // couldn't read
        if( n==0 ) {
            usleep( 1 * 1000 );  // wait 1 msec try again
            timeout--;
            if( timeout==0 ) return -2;
            continue;
        }
#ifdef DEBUG
        printf("serialport_read_until: i=%d, n=%d b='%c'\n",i,n,b[0]); // debugging
#endif
        buf[i] = b[0]; 
        i++;
    } while( b[0] != until && i < buf_max && timeout>0 );

    buf[i] = 0;  // null terminate the string
    return 0;
}


/*
void poll_read()
{
struct pollfd fds[1];
fds[0].fd = serial_port_setup;
fds[0].events = POLLIN ;
int pollrc = poll( fds, 1, 1000);
if (pollrc < 0)
{
    perror("poll");
}
else if( pollrc > 0)
{
    if( fds[0].revents & POLLIN )
    {
        char buff[256];
        ssize_t rc = read(serial_fd, buff, sizeof(buff) );
        if (rc > 0)
        {
            // we got something from the buffer
        }
    }
}    
}
*/








