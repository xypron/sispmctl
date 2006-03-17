#ifndef LOCAL_H
#define LOCAL_H 

#define LISTENPORT 2638
extern int listenport;
int*socket_init(char*bindaddr);
void l_listen(int*sock, usb_dev_handle *udev,int id);

#endif /* ! LOCAL_H */
