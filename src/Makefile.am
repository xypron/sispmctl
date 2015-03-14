bin_PROGRAMS = sispmctl 

EXTRA_DIST =  \
	web1/index.html web1/off1.html web1/off3.html web1/on1.html \
	web1/on3.html web1/status0.png web1/style.css web1/logo.png \
	web1/off2.html web1/off4.html web1/on2.html web1/on4.html \
	web1/status1.png web2/index.html web2/off1.html web2/off3.html \
	web2/on1.html web2/on3.html web2/style.css web2/logo.png \
	web2/off2.html web2/off4.html web2/on2.html web2/on4.html \
	web2/off.gif web2/on.gif web2/onoff.gif sispmctl.1 

if WEBLESSCOND
AM_CFLAGS=-Wall -DWEBLESS=@WEBLESS@

sispmctl_SOURCES = sispm_ctl.c main.c sispm_ctl.h main.h
else
AM_CFLAGS=-Wall -DWEBDIR="\"@WEBDIR@\"" -DBINDADDR="\"@BINDADDR@\"" \
-DDATADIR="\"$(datadir)/httpd/$(PACKAGE)/doc\""

pkgdatadir = $(datadir)/httpd/$(PACKAGE)
pkgdata1dir = $(datadir)/httpd/$(PACKAGE)/skin1
pkgdata2dir = $(datadir)/httpd/$(PACKAGE)/skin2

sispmctl_SOURCES = \
	sispm_ctl.c nethelp.c main.c socket.c sispm_ctl.h nethelp.h socket.h \
	main.h

pkgdata1_DATA = \
	web1/index.html web1/logo.png web1/off1.html web1/off2.html \
	web1/off3.html web1/off4.html web1/on1.html web1/on2.html \
	web1/on3.html web1/on4.html web1/status0.png web1/status1.png \
	web1/style.css
pkgdata2_DATA = \
	web2/index.html web2/logo.png web2/off1.html web2/off2.html \
	web2/off3.html web2/off4.html web2/off.gif web2/on1.html \
	web2/on2.html web2/on3.html web2/on4.html web2/on.gif web2/onoff.gif \
	web2/style.css 
endif

man_MANS = sispmctl.1 

AM_CPPFLAGS = $(all_includes)

AM_LDFLAGS = $(all_libraries)

install-data-local:
if WEBLESSCOND
else
	test -z "$(DESTDIR)$(datadir)/httpd/$(PACKAGE)"  \
	|| mkdir -p -- "$(DESTDIR)$(datadir)/httpd/$(PACKAGE)"
	test -z "$(DESTDIR)$(datadir)/httpd/$(PACKAGE)/skin1" \
	|| mkdir -p -- "$(DESTDIR)$(datadir)/httpd/$(PACKAGE)/skin1"
	if test ! -e $(DESTDIR)$(datadir)/httpd/$(PACKAGE)/doc ; then \
	 cd $(DESTDIR)$(datadir)/httpd/$(PACKAGE) && ln -s skin1 doc ; \
	fi 
endif

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/httpd/$(PACKAGE)/doc
	rm -rf $(DESTDIR)$(datadir)/httpd/$(PACKAGE)/skin1
	rm -rf $(DESTDIR)$(datadir)/httpd/$(PACKAGE)/skin2
	rm -rf $(DESTDIR)$(datadir)/httpd/$(PACKAGE)

