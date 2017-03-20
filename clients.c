/*
 $Id: clients.c,v 1.1 1998/06/29 11:19:55 vadik Exp vadik $
 $Log: clients.c,v $
 Revision 1.1  1998/06/29 11:19:55  vadik
 Initial revision

*/
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <netdb.h>

#include "conf.h"
#include "misc.h"
#include "proto.h"
#include "radd.h"
#include "log.h"

extern int h_errno;

CLIENTS clients[MAX_CLIENTS];
int clients_num = 0;

void clnt_init(char *file)
{
	FILE *fin;
	char str[512];
	char name[64], key[64];
	int line = 0;
	extern char *dict_cat(char *, char *);

	if ((fin = fopen(dict_cat(DICT_PATH, file), "r")) == NULL)
		Err("clnt_init:fopen():%s:%m", file);

	while (!feof(fin)) {
		fgets(str, 512, fin);
		line++;
		if (*str == '#' || *str == '\0' || *str == '\n')
			continue;

		if (sscanf(str, "%s%s", name, key) != 2) {
			Warn("clnt_init(): invalid line %d", line);
			continue;
		}
		strncpy(clients[clients_num].name, name, 64);
		strncpy(clients[clients_num].key, key, 64);

		DEBUG("%s", name);
		clients_num++;
	}
	DEBUG("clnts_init(): clients %d", clients_num);
}

char *clnt_getkey(uint addr)
{
	int i;
	uint haddr;

	for (i = 0; i < clients_num; i++) {
		if ((haddr = lib_aton(clients[i].name)) == 0)
			return NULL;
		if (haddr == addr)
			return clients[i].key;
	}
	return NULL;
}
