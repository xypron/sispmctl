project('sispmctl', 'c')
dep = [dependency('libusb-1.0')]
executable('sispmctl', 'src/main.c', 'src/usb.c',
	   dependencies: dep)
