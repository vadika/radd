/*
$Id: conf.h,v 1.1 1998/06/26 23:22:04 vadik Exp vadik $
$Log: conf.h,v $
Revision 1.1  1998/06/26 23:22:04  vadik
Initial revision

*/
#ifndef _CONF_H_
#define _CONF_H_

#define DICT_PATH "/etc/raddb/"
#define DICT_FILE "dictionary"
#define REALM_FILE "realms"
#define CLIENTS_FILE "clients"
#define STATS_FILE "/tmp/radd.stats"

/*
	maximum number of entries in dictionary.
	64 kilobytes shold be enough for anybody
*/
#define MAX_DICT_SIZE	2048
#define MAX_REALMS		64
#define MAX_CLIENTS		32
#define UDP_MAX_DLEN  9216

/*
 pseudo constants :-)
*/
#define STRLEN 256

typedef unsigned long ulong;

#endif				/* _CONF_H_  */
