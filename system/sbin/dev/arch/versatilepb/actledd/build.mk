VERSATILEPB_ACTLEDD_OBJS = $(ROOT_DIR)/sbin/dev/arch/versatilepb/actledd/actledd.o

VERSATILEPB_ACTLEDD = $(TARGET_DIR)/$(ROOT_DIR)/sbin/dev/versatilepb/actledd

PROGS += $(VERSATILEPB_ACTLEDD)
CLEAN += $(VERSATILEPB_ACTLEDD_OBJS)

$(VERSATILEPB_ACTLEDD): $(VERSATILEPB_ACTLEDD_OBJS) $(LIB_OBJS)
	$(LD) -Ttext=100 $(VERSATILEPB_ACTLEDD_OBJS) $(LIB_OBJS) -o $(VERSATILEPB_ACTLEDD) $(LDFLAGS)
