extern char *system_ttys_file_name;
extern char *system_wtmp_file_name;
extern char *system_utmp_file_name;
extern char *system_deny_dir;

int  getttyslot __P((const char *, const char *));
int  set_sysadm_status __P(());
