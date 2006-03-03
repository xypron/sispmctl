bin_PROGRAMS = sispmctl 

EXTRA_DIST = web1/index.html web1/off1.html web1/off3.html web1/on1.html web1/on3.html web1/status0.png web1/style.css web1/logo.png  web1/off2.html web1/off4.html web1/on2.html web1/on4.html web1/status1.png \
	web3/index.html web3/off1.html web3/off3.html web3/on1.html web3/on3.html web3/style.css web3/logo.png web3/off2.html web3/off4.html web3/on2.html web3/on4.html web2/off.gif \
	web2/index.html web2/off.gif web2/off2.html web2/off4.html web2/on1.html web2/on3.html web2/onoff.gif web2/logoogle.gif web2/off1.html web2/off3.html web2/on.gif web2/on2.html web2/on4.html \
	pmctl.1

if WEBLESSCOND
AM_CFLAGS=-Wall -DWEBLESS=@WEBLESS@
else
AM_CFLAGS=-Wall -DWEBDIR="\"@WEBDIR@\"" -DBINDADDR="\"@BINDADDR@\"" -DDATADIR="\"$(datadir)/httpd/$(PACKAGE)/doc\""
endif

man_MANS = sispmctl.1 

pkgdatadir = $(datadir)/httpd/$(PACKAGE)

pkgdata1dir = $(datadir)/httpd/$(PACKAGE)/skin1

pkgdata2dir = $(datadir)/httpd/$(PACKAGE)/skin2

pkgdata3dir = $(datadir)/httpd/$(PACKAGE)/skin3


if WEBLESSCOND
sispmctl_SOURCES = sispm_ctl.c main.c sispm_ctl.h main.h
else
sispmctl_SOURCES = sispm_ctl.c nethelp.c main.c socket.c sispm_ctl.h nethelp.h socket.h main.h
endif

pkgdata1_DATA = web1/index.html web1/off1.html web1/off3.html web1/on1.html web1/on3.html web1/status0.png web1/style.css web1/logo.png  web1/off2.html web1/off4.html web1/on2.html web1/on4.html web1/status1.png

pkgdata2_DATA = web3/index.html web3/off1.html web3/off3.html web3/on1.html web3/on3.html web3/style.css web3/logo.png web3/off2.html web3/off4.html web3/on2.html web3/on4.html web2/off.gif 

pkgdata3_DATA = web2/index.html web2/off.gif web2/off2.html web2/off4.html web2/on1.html web2/on3.html web2/onoff.gif web2/logoogle.gif web2/off1.html web2/off3.html web2/on.gif web2/on2.html web2/on4.html

INCLUDES= $(all_includes)

AM_LDFLAGS = $(all_libraries)

install-data-local:
	if test ! -e $(datadir)/httpd ; then \
	 mkdir  $(datadir)/httpd ; \
	fi
	if test ! -e $(datadir)/httpd/$(PACKAGE) ; then \
	 mkdir  $(datadir)/httpd/$(PACKAGE) ; \
	fi
	if test ! -e $(datadir)/httpd/$(PACKAGE)/doc ; then \
	 ln -s $(datadir)/httpd/$(PACKAGE)/doc $(datadir)/httpd/$(PACKAGE)/skin1 ; \
	fi 

uninstall-local:
	rm $(datadir)/httpd/$(PACKAGE)/doc
	rm -r $(datadir)/httpd/$(PACKAGE)/skin1
	rm -r $(datadir)/httpd/$(PACKAGE)/skin2
	rm -r $(datadir)/httpd/$(PACKAGE)/skin3
	rm -r $(datadir)/httpd/$(PACKAGE)

