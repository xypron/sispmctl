import sys
from sispmform import sispmform
from qt import *
import  os

def socket_off(num):
    print "turn socket",num,"off"
    print "/usr/local/bin/sispmctl -f "+str(num)
    os.system("/usr/local/bin/sispmctl -f "+str(num))

def socket_on(num):
    print "turn socket",num,"on"
    os.system("/usr/local/bin/sispmctl -o "+str(num))
    

def socket_get_state(num):
    print "get state of socket",num
    #fd=os.popen("/usr/local/bin/sispmctl -g "+str(num))
    result=0
    for line in os.popen("/usr/local/bin/sispmctl -q -n -g "+str(num)).readlines():
        print "line: ",line       
        if line[0]=="0":
            result=1
        elif line[0]=="1":
            result=0
    return result
    
class Mysispmform(sispmform): 
    
    def socket1_clicked(self):
        print "socket1 clicked"
        if self.s1==0:
            socket_off(1)
            self.led1.setColor(QColor(255,0,0))        
        else:
            socket_on(1)
            self.led1.setColor(QColor(0,255,0))
        self.s1=1-self.s1
    
    def socket2_clicked(self):
        print "socket1 clicked"
        if self.s2==0:
            socket_off(2)
            self.led2.setColor(QColor(255,0,0))        
        else:
            socket_on(2)
            self.led2.setColor(QColor(0,255,0))
        self.s2=1-self.s2
    
    def socket3_clicked(self):
        print "socket1 clicked"
        if self.s3==0:
            socket_off(3)
            self.led1.setColor(QColor(255,0,0))        
        else:
            socket_on(3)
            self.led1.setColor(QColor(0,255,0))
        self.s3=1-self.s3
    
    def socket4_clicked(self):
        print "socket4 clicked"
        if self.s4==0:
            socket_off(4)
            self.led4.setColor(QColor(255,0,0))        
        else:
            socket_on(4)
            self.led4.setColor(QColor(0,255,0))
        self.s4=1-self.s4

    def __init__(self, *args):
        apply(sispmform.__init__, (self,) + args)
        self.s1=socket_get_state(1)
        self.s2=socket_get_state(2)
        self.s3=socket_get_state(3)
        self.s4=socket_get_state(4)
        
        if self.s1==1:
            self.led1.setColor(QColor(255,0,0))
        else:
            self.led1.setColor(QColor(0,255,0))
        if self.s2==1:
            self.led2.setColor(QColor(255,0,0))
        else:
            self.led2.setColor(QColor(0,255,0))
        if self.s3==1:
            self.led3.setColor(QColor(255,0,0))
        else:
            self.led3.setColor(QColor(0,255,0))
        if self.s4==1:
            self.led4.setColor(QColor(255,0,0))
        else:
            self.led4.setColor(QColor(0,255,0))
        
        self.connect(self.socket1, SIGNAL("clicked()"), self.socket1_clicked)
        self.connect(self.socket2, SIGNAL("clicked()"), self.socket2_clicked)
        self.connect(self.socket3, SIGNAL("clicked()"), self.socket3_clicked)
        self.connect(self.socket4, SIGNAL("clicked()"), self.socket4_clicked)
        
  
        
        
if __name__ == "__main__":
        a = QApplication(sys.argv)
        QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
        w = Mysispmform()
        a.setMainWidget(w)
        w.show()
        a.exec_loop()
