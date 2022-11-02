PROG   = rsa_pkey_tool
CXX    = g++ -O2 -Wall -Wno-write-strings -Wno-unused-variable -Wno-unused-but-set-variable
CXX_C  = gcc -O2 -Wall -Wno-write-strings -Wno-unused-variable -Wno-unused-but-set-variable
LIB    = -lssl -lcrypto
RM     = rm -f
CFLAGS = -c -I ../../

OBJS = \
	rsa_pkey_tool.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIB)

rsa_pkey_tool.o: rsa_pkey_tool.c
	$(CXX) $(CFLAGS) rsa_pkey_tool.c

clean:
	-$(RM) $(PROG) $(OBJS)
