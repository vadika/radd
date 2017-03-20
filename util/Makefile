SRCS = own_wtmp.c getttyent.c login.c logwtmp.c ttyslot.c
OBJS = $(SRCS:.c=.o)

all:	libsystem.a unlog

libsystem.a:	$(OBJS)
	rm -f libsystem.a
	ar r libsystem.a $(OBJS)
	ranlib libsystem.a

unlog:	unlog.c
	$(CC) $(CFLAGS) -L. -o unlog unlog.c -lsystem

clean:
	rm -f $(OBJS) libsystem.a unlog

