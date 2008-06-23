"""
  Control a GEMBIRD Silver Shield PM USB outlet device.

  For example:

    import sispm

    print sispm.get_num_devices()    # Print number of devices on the system.

    dev=sispm.Sispm()                # Get first available device.
    dev.set_outlet_enabled(0,True)   # Switch first outlet on.
    dev.set_outlet_enabled(1,False)  # Switch second outlet off.
    print dev.get_outlet_enabled(2)  # Print status of third outlet.

  (c) 2007 Mikael Lindqvist (li.mikael@gmail.com)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""

import usb, struct, sys

VENDOR_ID=0x04B4
PRODUCT_ID_SISPM=0xFD11
PRODUCT_ID_MSISPM_OLD=0xFD10
PRODUCT_ID_MSISPM_FLASH=0xFD12

class SispmException(Exception):
	""" Exception class. """
	pass

def get_num_devices():
	""" Get number of sis-pm devices on the system. """
	cnt=0

	busses=usb.busses()
	for bus in busses:
		for device in bus.devices:
			if device.idVendor==VENDOR_ID \
					and device.idProduct in [PRODUCT_ID_SISPM,
					PRODUCT_ID_MSISPM_OLD,PRODUCT_ID_MSISPM_FLASH]:
				cnt+=1

	return cnt

class Sispm:
	""" The functionality of a Sispm device. """

	def __init__(self, num=0):
		""" Num is the device number on the system (0,1,2...) """
		cnt=0

		busses=usb.busses()
		for bus in busses:
			for device in bus.devices:
				if device.idVendor==VENDOR_ID \
						and device.idProduct in [PRODUCT_ID_SISPM,
						PRODUCT_ID_MSISPM_OLD,PRODUCT_ID_MSISPM_FLASH]:

					if num==cnt:
						self.device=device
						self.deviceHandle=self.device.open()
						return

					else:
						cnt+=1

		raise SispmException("Sispm device not found.")

	def _usb_command(self, b1, b2, dir_in=False):
		""" Send a usb command. """
		if dir_in:
			req=0x01
			reqtype=0x21|0x80;
			buf=2

		else:
			req=0x09
			reqtype=0x21 # USB_DIR_OUT+USB_TYPE_CLASS+USB_RECIP_INTERFACE
			buf=struct.pack("BB",b1,b2)

		buf=self.deviceHandle.controlMsg(reqtype,req,buf,(0x03<<8)|b1,0,500)

		if dir_in:
			return buf[1]!=0

	def _check_outlet(self, num):
		if num<0 or num>=self.get_num_outlets():
			raise SispmException("Outlet %d doesn't exist on this device."%num)

	def set_outlet_enabled(self, outlet, onoff):
		""" Switch outlet on/off. Outlets are numbered 0,1,2 and 3. """
		self._check_outlet(outlet)
		self._usb_command(3*(outlet+1),{False: 0x00, True: 0x03}[onoff])

	def get_outlet_enabled(self, outlet):
		""" Get status of outlet. Outlets are numbered 0,1,2 and 3. """
		self._check_outlet(outlet)
		return self._usb_command(3*(outlet+1), 0x03, True)

	def set_buzzer_enabled(self, onoff):
		""" Set buzzer on/off. """
		self._usb_command(1,{False: 0x00, True: 0x03}[onoff])

	def get_num_outlets(self):
		""" Get number of outlets on this device. """
		if self.device.idProduct in \
				[PRODUCT_ID_MSISPM_OLD,PRODUCT_ID_MSISPM_FLASH]:
			return 1

		elif self.device.idProduct==PRODUCT_ID_SISPM:
			return 4

		else:
			return None

if __name__=="__main__":
	arg=1

	usage="""
Usage: python sispm.py <options>

Option:
  -list             Show list of SIS-PM devices.
  -dev <id>         Select device to control.
  -on <outlet>      Switch outlet on (outlet=0,1,2 or 3).
  -off <outlet>     Switch outlet off.
  -status <outlet>  Show status of outlet.
"""

	if len(sys.argv)<=1:
		print usage
		sys.exit(1)

	dev=None

	while arg<len(sys.argv):
		if sys.argv[arg]=="-list":
			for i in range(0,get_num_devices()):
				dev=Sispm(i)
				print "%d: Gembird device with %d outlet(s). USB device %s."% \
					(i,dev.get_num_outlets(),dev.device.filename)
			sys.exit(0)

		elif sys.argv[arg]=="-dev" and arg<len(sys.argv)-1:
			dev=Sispm(int(sys.argv[arg+1]))
			arg+=2

		elif sys.argv[arg]=="-on" and arg<len(sys.argv)-1:
			if dev is None: dev=Sispm()
			dev.set_outlet_enabled(int(sys.argv[arg+1]),True)
			arg+=2

		elif sys.argv[arg]=="-off" and arg<len(sys.argv)-1:
			if dev is None: dev=Sispm()
			dev.set_outlet_enabled(int(sys.argv[arg+1]),False)
			arg+=2

		elif sys.argv[arg]=="-status" and arg<len(sys.argv)-1:
			if dev is None: dev=Sispm()
			outlet=int(sys.argv[arg+1])
			print "outlet %d is %s"%(outlet,
				{False: "off", True: "on"}[dev.get_outlet_enabled(outlet)])
			arg+=2

		else:
			print usage
			sys.exit(1)
