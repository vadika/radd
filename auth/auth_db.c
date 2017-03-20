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

int db_auth(char *name, char *passwd, struct procs * ps, char *dbname)
{
	DB *passwdb;
	DBT data, key;
	int res;
	char *crypt();
	char *encpw;
	char *encrypted_pass;


	/* Get encrypted password from password file */

	passwdb = dbopen(dbname,  O_RDONLY, 0, DB_HASH, NULL);

	if (passwdb == NULL) {
		DEBUG("auth_db: dbopen for <%s> failed\n", dbname);
		return (1);
	}
	key.data = name;
	key.size = strlen(name);

	data.data = alloca(MAXENCPASSLEN);
	data.size = MAXENCPASSLEN;

	if ((res = (passwdb->get) (passwdb, &key, &data, 0))) {
		DEBUG("auth_db:  -- user not found ");
		Warn("user %s not found in database", name);
#ifdef CITYLINE
		Message("user %s not found in database", name);
#endif
		return (1);
	}
	*(char *) ((data.data) + data.size) = 0;

	encrypted_pass = data.data;

	/* Run encryption algorythm */
	encpw = crypt(passwd, encrypted_pass);

	DEBUG("auth_db: password is <%s>\n", passwd);
	DEBUG("auth_db: enc_password is <%s>\n", encrypted_pass);
	DEBUG("auth_db: encpw is <%s>\n", encpw);


	/* Check it */
	if (strcmp(encpw, encrypted_pass)) {
		DEBUG("auth_db: password for <%s> failed\n", name);
		Warn("user %s gave us bad passwd <%s>", name, passwd);
#ifdef CITYLINE
		Message("user %s gave us bad passwd <%s>", name, passwd);
#endif
		return 1;
	}

#ifdef CITYLINE
		Message("user %s password ok", name);
#endif 
	return 0;
}
