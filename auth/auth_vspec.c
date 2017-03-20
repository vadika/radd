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


/* vendor-specifica attribute */

int vspecgeneric_auth(char *name, char *passwd, struct procs * ps, char *vendor, char *param1, char* param2)
{
 DEBUG("$$$$$ %s, %s %s", vendor, param1, param2);
 return 0;
}

int vspecstring_auth(char *name, char *passwd, struct procs * ps, char *vendor, char* param2)
{
 DEBUG("$$$$$ %s, %s %s", vendor, param2);
 return 0;
}
