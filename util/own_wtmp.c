#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ttyent.h>
#include <string.h>

#include <utmp.h>

char *system_ttys_file_name = "/var/adm/ttys";
char *system_wtmp_file_name = "/var/adm/wtmp";
char *system_utmp_file_name = "/var/adm/utmp";

int init_utmp(reason)
char *reason;
{
  struct utmp u;
  int f;
  struct ttyent *t;

  f = open(system_utmp_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0664);
  if (f < 0) return 1;

  bzero(&u, sizeof(u));
  time(&u.ut_time);
  if (setttyent())
    while (t = getttyent())
      write(f, &u, sizeof(u));
  else return 1;
  close(f); endttyent();

  logwtmp("~", reason, "");
  return 0;
}
