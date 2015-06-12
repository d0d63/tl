/*
   $dodge: tl.h,v 1.17 2014/12/20 11:54:36 dodge Exp $
   $Name:  $
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <limits.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#ifdef SOLARIS
#include <sys/loadavg.h>
#endif /* SOLARIS */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif /* MAXHOSTNAMELEN */

#define TL_VERSION_NAME "$Name:  $"
#define TL_VERSION_ID	"$Id: tl.h,v 1.17 2014/12/20 11:54:36 dodge Exp $"
#define SPIN_CHARS "|/-\\\x00"
