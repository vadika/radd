#include <sys/types.h>
#include <db.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <err.h>

#include "../conf.h"
#include "../proto.h"
#include "../log.h"
#include "../radd.h"
#define MAXENCPASSLEN  128

int limits_auth(char *name, char *passwd, struct procs * ps, char *dbname)
{
	DB *passwdb;
	DBT data, key;
	int res;
	int limit;

	passwdb = dbopen(dbname, O_EXLOCK | O_RDONLY, 0, DB_HASH, NULL);

	if (passwdb == NULL) {
		DEBUG("limits_bool: dbopen for <%s> failed\n", dbname);
		return (1);
	}
	key.data = name;
	key.size = strlen(name);

	data.data = alloca(MAXENCPASSLEN);
	data.size = MAXENCPASSLEN;

	if ((res = (passwdb->get) (passwdb, &key, &data, 0))) {
		DEBUG("auth_limits:  -- user not found \n");
		Warn("user %s not found in database\n", name);
#ifdef CITYLINE
		Message("user %s not found in database\n", name);
#endif
		return (1);
	}
	*(char *) ((data.data) + data.size) = '\0';

	/* eliminate dot */
	if (strchr(((char *)data.data),'.') !=NULL)
		*(strchr(((char *)data.data),'.'))= '\0';

	/* Check it */
	  limit=atoi((char *)data.data);
#ifdef CITYLINE
		Message("%s has %d hrs %d min %d sec left\n", 
				name, limit/3600, (limit - (limit/3600)*3600)/60, limit % 60);
#endif
	 limit=htonl(limit);

	 /* 27 stands for Session-Timeout, 4 stands for sizeof(int) */
	 proto_inserttuple(ps->reply,27,&limit,4);

 return 0;
}
