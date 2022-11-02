include $(TOPDIR)/config.mk

LIB	= lib$(SOC).a

START	=
OBJS	=
SOBJS	=

OBJS	+= mtk_common.o
OBJS	+= mtk_clocks.o
OBJS	+= mtk_dram.o
OBJS	+= mtk_gpio.o
OBJS	+= mtk_serial.o
OBJS	+= mii_mgr.o
OBJS	+= rt2880_eth.o
SOBJS	+= memsetup.o
SOBJS	+= rt2880_init.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS) $(OBJS-0)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
