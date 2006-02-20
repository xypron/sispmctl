#ifndef MAIN_H
#define MAIN_H 1

#define MAXANSWER	8192

#ifndef WEBDIR 
#define WEBDIR ""
#endif


#define HOMEDIR	"/usr/local/httpd/sispm_ctl/doc"

const char*answer(char*in);
void process(int out,char*v,usb_dev_handle *udev);
extern int debug;

#endif /* ! MAIN_H */
