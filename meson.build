project('sispmctl', 'c',
	version : '5.0-alpha',
	default_options : [
		'c_std=gnu11',
		'warning_level=3'],
	license : 'GPL-2.0')

extra_args = ['-DPACKAGE_VERSION="@0@"'.format(meson.project_version())]
dep = [dependency('libusb-1.0')]

executable('sispmctl', 'src/main.c', 'src/usb.c',
	   dependencies: dep,
	   c_args : extra_args)
