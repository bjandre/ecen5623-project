EXE_EXTENSION := exe
BIN_EXTENSION := bin
DUMP_EXTENSION := dump

DEPENDS_DIR := _depends
POSTCOMPILE = mv -f $(DEPENDS_DIR)/$*.Td $(DEPENDS_DIR)/$*.d

BUILD_COMPLETE = @echo Build complete: $@ : $(shell date)

BUILD_ARTIFACTS = \
  *.o \
  *.asm \
  *.a \
  *.$(EXE_EXTENSION) \
  *.map \
  *.i \
  *.$(DUMP_EXTENSION) \
  *.$(SREC_EXTENSION) \
  $(DEPENDS_DIR) *.d *.Td \
  doxygen.warnings.log

EDITOR_FILES = \
  *~ \
  *.astyle.orig

