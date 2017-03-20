#include <sys/types.h>
#include <db.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <err.h>

#include "../conf.h"
#include "../proto.h"
#include "../log.h"
#include "../radd.h"

int passwd_auth(char *name, char *passwd, struct procs * ps, char *password)
{
 DEBUG("$$$$$ %s, %s ?= %s", name, passwd, password);
 if(strncmp(passwd,password,strlen(passwd))==0)
 	return 0;
 return 1;
}
