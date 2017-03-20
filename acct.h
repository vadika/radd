#define USERSTAT "/stat/statserv/var/users"

typedef struct {
	/* обязательные параметры */
	char uname[128];	/* имя пользователя */
	uint type;		/* start=1/stop=2 */
	uint timestamp;
	uint his_ip;
	uint nas_ip;
	/* параметры ниже -- опционально присутствуют, где */
	/* или же равны нулям */
	uint nasport;
	uint session_time;
	uint bytes_in;
	uint bytes_out;
	uint terminated_by;
	/* User-Request            1 Lost-Carrier            2
	   Lost-Service 3 Idle-Timeout            4 Session-Timeout 5
	   Admin-Reset 6 Admin-Reboot            7 Port-Error 8
	   NAS-Error 9 NAS-Request             10 NAS-Reboot 11
	   Port-Unneeded 12 Port-Preempted          13 Port-Suspended
	   14 Service-Unavailable     15 Callback                16
	   User-Error 17 Host-Request 18 */
}   acctrec;
