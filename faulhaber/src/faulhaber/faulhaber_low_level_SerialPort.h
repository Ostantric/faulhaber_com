#ifndef __faulhaber_low_level_SerialPort_H__ //define guard
#define __faulhaber_low_level_SerialPort_H__

#include <stdint.h> 
int serialport_setup(const char* serialport, int baud); //setup
int serialport_flush(int fd); //serial flush 
int serialport_close(int fd); //destructor 
int serialport_write(int fd, const char* str); //write
int serialport_read_until(int fd, char* buf, char until, int buf_max,double timeout); // read until timeout
#endif

