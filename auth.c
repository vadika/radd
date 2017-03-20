/*
$Id: auth.c,v 1.3 1998/06/29 11:18:58 vadik Exp vadik $
$Log: auth.c,v $
Revision 1.3  1998/06/29 11:18:58  vadik
rev 1.3

Revision 1.2  1998/06/27 18:22:30  vadik
input done

Revision 1.1  1998/06/26 23:20:42  vadik
Initial revision

*/

#include <stdio.h>
#include <err.h>
#include <string.h>

#define _AUTH_
#include "conf.h"
#include "misc.h"
#include "proto.h"
#include "radd.h"
#include "log.h"

extern int db_auth(char *name, char *passwd, struct procs * ps,...);
extern int radius_auth(char *name, char *passwd, struct procs * ps,...);
extern int attribute_auth(char *name, char *passwd, struct procs * ps,...);
extern int bool_auth(char *name, char *passwd, struct procs * ps,...);
extern int passwd_auth(char *name, char *passwd, struct procs * ps,...);
extern int flags_auth(char *name, char *passwd, struct procs * ps,...);
extern int limits_auth(char *name, char *passwd, struct procs * ps,...);
extern int mysql_auth(char *name, char *passwd, struct procs * ps,...);
extern int vspecgeneric_auth(char *name, char *passwd, struct procs * ps,...);
extern int vspecstring_auth(char *name, char *passwd, struct procs * ps,...);
extern int address_auth(char *name, char *passwd, struct procs *ps,...);


/* type, number of extra arguments, function */
AUTH_TYPE auth_types[] = {
	{"DB", 1, db_auth},
	{"RADIUS", 2, radius_auth},
	{"ATTRADD", 2, attribute_auth},
	{"BOOLDB", 1, bool_auth},
	{"FLAGS", 1, flags_auth},
	{"PASSWD", 1, passwd_auth},
	{"LIMIT", 1, limits_auth},
/*	{"MYSQL", 3, mysql_auth}, */
	{"VSPECGEN", 3, vspecgeneric_auth},
	{"VSPEC", 2, vspecstring_auth},
/*	{"ADDRESS", 4, address_auth}, */
};

int auth_types_num = (sizeof(auth_types) / sizeof(AUTH_TYPE));

REALM realms[MAX_REALMS];
int realm_num = 0;


int auth_byname(char *name)
{
	int i;

	for (i = 0; i < auth_types_num; i++)
		if (strncasecmp(auth_types[i].name, name, strlen(auth_types[i].name)) == 0)
			return i;
	return -1;
}

void auth_init(char *file)
{
	FILE *fin;
	char *str = alloca(STRLEN);
	char *ostr = str;
	int line = 0, new_realm = 1;
	char *words[10];
	int nw;

	extern char *dict_cat(char *, char *);

	if ((fin = fopen(dict_cat(DICT_PATH, file), "r")) == NULL)
		Err("auth:auth_init() can't open %s:%m", file);

	while (!feof(fin)) {
		str = ostr;
		fgets(str, STRLEN - 1, fin);
		line++;

		if (*str == '#' || *str == '\0' || *str == '\n')
			continue;

		nw = lib_sepwords(words, 10, &str);
		if (nw < 1) {
			Warn("auth_init: bad line %d ", line);
			new_realm = 1;
			continue;
		}
		/* first line of realm */
		if (new_realm) {
			if (nw < 2) {
				Warn("auth_init: invalid line %d", line);
				new_realm = 1;
				continue;
			}
			if (words[1][strlen(words[1]) - 1] == '+') {
				words[1][strlen(words[1]) - 1] = '\0';
				new_realm = 0;
			}
#define _ realms[realm_num].
			_   nauths = 0;

			bzero(_ name, 32);
			strcpy(_ name, words[0]);	/* put realm name */
			_   authmethods[_ nauths] = auth_byname(words[1]);

			if (_ authmethods[_ nauths] == -1) {
				Warn("auth:authinit(): invalid auth method %s at line %d",
				     words[1], line);
				continue;
			}
			if ((nw - 2) != auth_types[_ authmethods[_ nauths]].nargs) {
				Warn("auth:auth_init(): invalid number of args for %s at %d",
				     words[1], line);
				continue;
			}
			switch (auth_types[_ authmethods[_ nauths]].nargs) {
			case 4:
				bzero(_ authargs[_ nauths].arg4, ARG_LEN);
				strncpy(_ authargs[_ nauths].arg4, words[5], ARG_LEN);
			case 3:
				bzero(_ authargs[_ nauths].arg3, ARG_LEN);
				strncpy(_ authargs[_ nauths].arg3, words[4], ARG_LEN);
			case 2:
				bzero(_ authargs[_ nauths].arg2, ARG_LEN);
				strncpy(_ authargs[_ nauths].arg2, words[3], ARG_LEN);
			case 1:
				bzero(_ authargs[_ nauths].arg1, ARG_LEN);
				strncpy(_ authargs[_ nauths].arg1, words[2], ARG_LEN);
			}

			_   nauths++;

#undef _
			if (new_realm)
				realm_num++;

/* not the first line */
		} else {

			if (words[0][strlen(words[0]) - 1] == '+') {
				words[0][strlen(words[0]) - 1] = '\0';
				new_realm = 0;
			} else
				new_realm = 1;

#define _ realms[realm_num].

			_   authmethods[_ nauths] = auth_byname(words[0]);

			if (_ authmethods[_ nauths] == -1) {
				Warn("auth:authinit(): invalid auth method %s at line %d",
				     words[1], line);
				continue;
			}
			if ((nw - 1) != auth_types[_ authmethods[_ nauths]].nargs) {
				Warn("auth:auth_init(): invalid number of args for %s at %d",
				     words[0], line);
				continue;
			}
			switch (auth_types[_ authmethods[_ nauths]].nargs) {
			case 4:
				strncpy(_ authargs[_ nauths].arg4, words[4], ARG_LEN);
			case 3:
				strncpy(_ authargs[_ nauths].arg3, words[3], ARG_LEN);
			case 2:
				strncpy(_ authargs[_ nauths].arg2, words[2], ARG_LEN);
			case 1:
				strncpy(_ authargs[_ nauths].arg1, words[1], ARG_LEN);
			}

			_   nauths++;

#undef _

			if (new_realm)
				realm_num++;
		}
	}
	DEBUG("auth_init(): realms %d", realm_num);
}
