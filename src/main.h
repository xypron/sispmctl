/*
  SisPM.c

  Controls the GEMBIRD Silver Shield PM USB outlet device

  (C) 2004,2005,2006 Mondrian Nuessle, Computer Architecture Group, University of Mannheim, Germany
  (C) 2005, Andreas Neuper, Germany

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


  nuessle@uni-mannheim.de
  aneuper@web.de

*/
#ifndef MAIN_H
#define MAIN_H

#define MAXANSWER	8192

#ifndef WEBDIR
#define WEBDIR ""
#endif


#define HOMEDIR	"/usr/local/share/httpd/sispmctl/doc"

extern int verbose;

const char*answer(char*in);
void process(int out,char*v,struct usb_device*dev,int devnum);
extern int debug;

#endif /* ! MAIN_H */
