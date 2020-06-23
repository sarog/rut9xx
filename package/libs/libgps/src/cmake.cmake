SET_PROJECT(libgps)

ADD_LIB("tlt-logger")

COMPILE_LIB(libgps shared "gps.c")

INCLUDE_DIR("include")

COPY_TO_ROUTER_LOC(false /usr/lib 192.168.1.1 root admin01)

CLEAN()
