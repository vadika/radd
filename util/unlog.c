#include <sys/types.h>
#include <utmp.h>
#include "own_wtmp.h"

char *system_utmp_file_name = "/var/adm/utmp";
char *system_wtmp_file_name = "/var/adm/wtmp";
char *system_ttys_file_name = "/var/adm/ttys";

int main(ac, av)
char *av[];
{
  struct utmp u;

  if (ac != 3) {
    printf("Usage: unlog tty nas\n");
    exit(1);
  }
  bzero(&u, sizeof(u));
  strncpy(u.ut_line, av[1], UT_LINESIZE);
  strncpy(u.ut_host, av[2], UT_HOSTSIZE);
  time(&u.ut_time);
  my_login(&u);
  return 0;
}
