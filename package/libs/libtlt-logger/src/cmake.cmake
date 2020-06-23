SET_PROJECT(libtlt_logger)

ADD_LIB("uci")

COMPILE_LIB(libtlt-logger shared "tlt_logger.c")
SET_BUILD_INCLUDE_DIR("libtlt_logger")
INCLUDE_DIR("include")

COPY_TO_ROUTER_LOC(false /usr/lib 192.168.1.1 root Fasada123)

CLEAN()
