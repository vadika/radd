#include <sys/types.h>
#include <db.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <err.h>

#include "../conf.h"
#include "../proto.h"
#include "../log.h"
#include "../radd.h"
#define MAXENCPASSLEN  128

int bool_auth(char *name, char *passwd, struct procs * ps, char *dbname)
{
	DB *passwdb;
	DBT data, key;
	int res;


	/* Get encrypted password from password file */

	passwdb = dbopen(dbname, O_EXLOCK | O_RDONLY, 0, DB_HASH, NULL);

	if (passwdb == NULL) {
		DEBUG("auth_bool: dbopen for <%s> failed\n", dbname);
		return (1);
	}
	key.data = name;
	key.size = strlen(name);

	data.data = alloca(MAXENCPASSLEN);
	data.size = MAXENCPASSLEN;

	if ((res = (passwdb->get) (passwdb, &key, &data, 0))) {
		DEBUG("auth_bool:  -- user not found \n");
		Warn("user %s not found in database\n", name);
#ifdef CITYLINE
		Message("user %s not found in database\n", name);
#endif
		return (1);
	}
	*(char *) ((data.data) + data.size) = 0;

	/* Check it */
	    switch (*(char *)data.data)
        {
        case '0':
#ifdef CITYLINE
		Message("user %s not allowed to login\n", name);
#endif
                return 1;
                break;
        case '1':
                return 0;
                break;
        default:
#ifdef CITYLINE
		Message("user %s not allowed to login\n", name);
#endif
           return 1;
        }

 return 1;
}
