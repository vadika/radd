/*
 $Id$
 $Log$
 */

#include "conf.h"

char *lib_strtolower(char *);

int lib_sepwords(char *[], int, char **);

/*
 *  Return a printable host name (or IP address in dot notation)
 *  for the supplied IP address.
 */
char *lib_hostname(ulong);

/*
 *  Return an IP address in host long notation from a host
 *  name or address in dot notation.
 */
ulong lib_aton(char *);

/*
 *  Return an IP address in standard dot notation for the
 *  provided address in host long notation.
 */
void lib_ntoa(char *, ulong);

/* same shit, but with static buffer */
char *lib_ntoab(ulong);

char *strnchr(char *, char, int);
char *strencode(char *, int);

int lib_notgarbage(char *);
