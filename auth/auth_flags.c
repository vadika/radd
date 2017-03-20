#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "../conf.h"
#include "../proto.h"
#include "../log.h"
#include "../radd.h"
#include "../misc.h"

int flags_auth(char *name, char *passwd, struct procs * ps, char *dbname)
{
	struct stat sb;
	char _path[1024];

		sprintf (_path,"%s/%s",dbname,lib_strtolower(name));

   if (lstat(_path, &sb) == 0)
   {
#ifdef CITYLINE
		Message("duplicate login, user %s  \n", name);
#endif
		return 1;
   }

 return 0;
}
