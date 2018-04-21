camera = unset
strip_camera := $(strip $(camera))
ifeq ($(strip_camera), unset)
  CAMERA_DEV := /dev/video0
else
  CAMERA_DEV := $(strip_camera)
endif

# subdirectories that will be built.
SUBDIRS = \
        src

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

test-camera :
	camorama --device $(CAMERA_DEV) --width 620 --height 480

.PHONY : astyle
astyle : FORCE
	astyle --options=.astylerc --recursive *.c *.h

.PHONY : clean
clean : $(SUBDIRS)
	@-$(RM) -rf $(EDITOR_FILES) $(BUILD_ARTIFACTS)

FORCE :
