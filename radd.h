/*
$Id: radd.h,v 1.1 1998/06/26 23:24:26 vadik Exp vadik $
$Log: radd.h,v $
Revision 1.1  1998/06/26 23:24:26  vadik
Initial revision

*/
#include "conf.h"

typedef struct dict_attr {
	char name[32];
	unsigned int value;
	int type;
}   DICT_ATTR;

#define T_STR	0
#define T_INT	1
#define T_IP	2
#define T_DATE	3

typedef struct dict_val {
	int attr;
	char name[32];
	unsigned int value;
}   DICT_VAL;

typedef struct auth_type {
	char name[32];
	int nargs;
	int (*func) (char *, char *, struct procs *,...);
}   AUTH_TYPE;

#define MAX_AUTH_METHODS	32
#define ARG_LEN   64

typedef struct auth_args {
	char arg1[ARG_LEN];
	char arg2[ARG_LEN];
	char arg3[ARG_LEN];
	char arg4[ARG_LEN];
}   AUTH_ARGS;

typedef struct realm {
	char name[64];
	int nauths;
	int authmethods[MAX_AUTH_METHODS];
	AUTH_ARGS authargs[MAX_AUTH_METHODS];
}   REALM;

#ifndef _AUTH_
extern REALM realms[];
extern int realm_num;
extern AUTH_TYPE auth_types[];

#endif

typedef struct clients {
	char name[64];
	char key[64];
}   CLIENTS;

#ifndef _MAIN_
extern int debug;
extern int auth_sock, acct_sock;

#endif

#define DEBUG if(debug)Warn


/* globals */

/* 1. dictionary */
int dict_attrbyname(char *);
void dict_init(char *);
char *dict_attrbynum(int);
char *dict_valbynum(char *, int);
char *dict_valbyinum(int, int);
int dict_valbynam(char *, char *);
int dict_valbyinam(int, char *);
int dict_typebyname(char *);
int dict_typebyval(int);

/* 2. realms */
void auth_init(char *);

/* 3. clients */
void clnt_init(char *);
char *clnt_getkey(uint);

/* 4. protocol */
void proto_init();

/* 5. statistics */
void stats_init(char *);
