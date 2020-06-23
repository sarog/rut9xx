include ../../build/module.mk
include ../../build/config.mk
include ../../build/gccconfig.mk

JSONC_CFLAGS = -I$(STAGING_DIR)/usr/include/json-c/
JSONC_LDFLAGS = -llua -lm -ljson-c
JSONC_OBJ = src/jsonc.o
JSONC_LIB = jsonc.so

%.o: %.c
	$(COMPILE) $(LUA_CFLAGS) $(JSONC_CFLAGS) $(FPIC) -c -o $@ $<

compile: $(JSONC_OBJ)
	mkdir -p dist$(LUCI_LIBRARYDIR)
	$(LINK) $(SHLIB_FLAGS) -o dist$(LUCI_LIBRARYDIR)/$(JSONC_LIB) $(JSONC_OBJ) $(JSONC_LDFLAGS) $(LUA_SHLIBS)

clean:
	rm -f *.o *.so
