/*
  $Id$
  $Log$

  Global todo -- should I define modularity for logging capabilities?

  Indeed, we need three logging levels for generic radius operations:
  debug, warnings and fatal errors.
  Additionaly, we need separate logging for authentication/accounting modules.

*/
#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include "syslog.h"

/* these are for  normal radius operations */
extern int debug;

void init_log()
{
	int flags = LOG_PID | LOG_NDELAY;

	if (debug)
		flags |= LOG_PERROR;

	openlog("radd", flags, LOG_LOCAL7);
}

void Err(char *str,...)
{
	va_list va;
	char buff[1024];

	va_start(va, str);
	vsprintf(buff, str, va);
	syslog(LOG_ERR|LOG_LOCAL7, buff);
	va_end(va);
	exit(1);
}

void Warn(char *str,...)
{
	va_list va;
	char buff[1024];

	va_start(va, str);
	vsprintf(buff, str, va);
	syslog(LOG_WARNING|LOG_LOCAL7, buff);
	va_end(va);
}


void Message(char *str,...)
{
	va_list va;
	char buff[1024];

	va_start(va, str);
	vsprintf(buff, str, va);
	mysyslog(LOG_INFO | LOG_LOCAL6, buff);
	va_end(va);
}
