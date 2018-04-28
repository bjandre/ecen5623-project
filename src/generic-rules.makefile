CXX = c++

AS = as

AR = ar

LD = ld

SIZE = size
SIZEFLAGS = --format=sysv -x

OBJDUMP = objdump
OBJDUMP_FLAGS = --disassemble-all --line-numbers --source --full-contents --all-headers

STD_FLAGS = --std=c++14

WARN_FLAGS = -Wall -Werror

GENERAL_CFLAGS = $(STD_FLAGS) $(WARN_FLAGS)

CFLAGS += $(RELEASE_CFLAGS) $(DEFINES) $(GENERAL_CFLAGS)

CXXFLAGS += $(RELEASE_CFLAGS) $(DEFINES) $(GENERAL_CFLAGS)

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

LDFLAGS = $(LDFLAGS) \
        -dynamic-linker \
        -Map=$*.map

LDLIBS = $(LDLIBS) \
        -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed \

CXX_LDFLAGS =
CXX_LDLIBS = -lpthread -lrt

#
# Generic rule to generate various targets
#
$(shell mkdir -p $(DEPENDS_DIR))

%.o : %.cpp
%.o : %.cpp $(DEPENDS_DIR)/%.d
	echo $(CXX) $(DEPENDS_FLAGS) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<
	$(CXX) $(DEPENDS_FLAGS) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<
	$(POSTCOMPILE)

%.i : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INCLUDES) -c -o $*.i $<

%.asm : %.cpp
	$(CXX) $(C_ASM_FLAGS) $(CXXFLAGS) $(INCLUDES) -c -o $*.asm $<

%.o : %.S
	$(AS) $(ASMFLAGS) $(INCLUDES) -c -o $@ $<

$(EXE) : $(DEPEND_STARTUP) $(OBJS) $(DEPEND_LIBS)
	$(CXX) $(CXXFLAGS) $(CXX_LDFLAGS) $(DEBUG_CC_LINK) -o $@ $^ $(DEPEND_STARTUP) $(CXX_LDLIBS)

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

-include $(SRCS:%.cpp=$(DEPENDS_DIR)/%.d)

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

