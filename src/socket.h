#ifndef LOCAL_H
#define LOCAL_H

#include <libusb-1.0/libusb.h>

#define LISTENPORT 2638
extern int listenport;
int*socket_init(char*bindaddr);
void l_listen(int*sock,libusb_device*,int devnum);

#endif /* ! LOCAL_H */
