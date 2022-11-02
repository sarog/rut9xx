# A long time ago, far, far away, under Solaris, you needed to
#    CFLAGS += -xO2 -Xc
#    LDLIBS += -lnsl -lsocket
# To cross-compile
#    CC = arm-linux-gcc
# To use clang
#    CC = clang
# To check for lint
#    CFLAGS += -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow -Wundef \
#     -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
#     -Wstrict-prototypes -Wmissing-prototypes -pedantic

# This was old-school networking code, making the traditional cast between
# struct sockaddr* and struct sockaddr_in*.  Thus a modern gcc needed:
#CFLAGS += -fno-strict-aliasing
# but as of ntpclient-2015, I cleaned up those casts enough to make both
# gcc-4.9.2 and clang-3.5.0 happy without it.

CFLAGS += -D_POSIX_C_SOURCE=199309 -D_BSD_SOURCE
CFLAGS += -W -Wall  # -W is spelled -Wextra these days, but -W is stilli
                    # valid and works with older compilers.
CFLAGS += -O2
CFLAGS += -DPRECISION_SIOCGSTAMP
CFLAGS += -DENABLE_DEBUG
# CFLAGS += -DENABLE_REPLAY
CFLAGS += -DNTP_MAX_SERVERS="(4)"
# CFLAGS += -DUSE_OBSOLETE_GETTIMEOFDAY

LDFLAGS += -lrt -lblobmsg_json -lubox -lubus

all: ntpclient

test: ntpclient
	./ntpclient -d -r <test.dat

ntpclient: src/ntpclient.o src/phaselock.o src/net.o
	$(CC) $(LDFLAGS) -o $@ $^

src/ntpclient.o src/phaselock.o src/net.o: src/ntpclient.h src/net.h

adjtimex: src/adjtimex.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f ntpclient adjtimex src/*.o
