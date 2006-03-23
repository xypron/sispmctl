#ifndef MAIN_H
#define MAIN_H 

#define MAXANSWER	8192

#ifndef WEBDIR 
#define WEBDIR ""
#endif


#define HOMEDIR	"/usr/local/share/httpd/sispmctl/doc"

extern int verbose;

const char*answer(char*in);
void process(int out,char*v,usb_dev_handle *udev,int id);
extern int debug;

#endif /* ! MAIN_H */
