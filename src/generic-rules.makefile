CC = cc
CXX = c++
AS = as

STD_FLAGS = --std=c99

WARN_FLAGS = -Wall -Werror

GENERAL_CFLAGS = $(STD_FLAGS) $(WARN_FLAGS)

CFLAGS += $(RELEASE_CFLAGS) $(DEFINES) $(GENERAL_CFLAGS)

# NOTE(bja, 2017-03) --warn and --fatal-warnings are only used when calling as
# directly, not when calling with the gcc wrapper.

#ASMFLAGS += --warn --fatal-warnings $(RELEASE_FLAGS)
ASMFLAGS += $(RELEASE_FLAGS)


# flags for generating assembly files from c.
C_ASM_FLAGS = -S

# flags for writing preprocessed output
CPPFLAGS = -E

# flags to automatically generate dependency information
DEPENDS_FLAGS = -MT $@ -MMD -MP -MF $(DEPENDS_DIR)/$*.Td

%.o : %.c
%.o : %.c $(DEPENDS_DIR)/%.d
	$(CC) $(DEPENDS_FLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	$(POSTCOMPILE)

%.i : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c -o $*.i $<

%.asm : %.c
	$(CC) $(C_ASM_FLAGS) $(CFLAGS) $(INCLUDES) -c -o $*.asm $<

%.o : %.S
	$(AS) $(ASMFLAGS) $(INCLUDES) -c -o $@ $<

$(EXE) : $(DEPEND_STARTUP) $(OBJS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) $(CC_LDFLAGS) $(DEBUG_CC_LINK) -o $@ $^ $(DEPEND_STARTUP)

#
# macro for executing TARGET in all SUBDIRS
#
ifdef SUBDIRS
.PHONY : $(SUBDIRS)
$(SUBDIRS) : 
	@if [ -d $@ ]; then \
		$(MAKE) --no-print-directory --directory=$@ $(MAKECMDGOALS); \
	fi
	$(BUILD_COMPLETE)
endif   

$(DEPENDS_DIR)/%.d : ;
.PRECIOUS : $(DEPENDS_DIR)/%.d

-include $(SRCS:%.c=$(DEPENDS_DIR)/%.d)

.PHONY : all
all : $(SUBDIRS) $(LIB) $(STARTUP_LIB) $(EXE) $(BIN)

.PHONY : bin
bin : $(SUBDIRS) $(EXE)

.PHONY : dump
dump : $(SUBDIRS) $(EXE)
ifdef EXE
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(EXE) > $(EXE:%.$(EXE_EXTENSION)=%.$(DUMP_EXTENSION))
endif

.PHONY : clean
clean : $(SUBDIRS)
	@-$(RM) -rf $(EDITOR_FILES) $(BUILD_ARTIFACTS)

.PHONY : clobber
clobber : clean
	@-$(RM) -rf $(EXE)

FORCE :

