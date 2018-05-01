include constants.makefile

EXE = laser-game.$(EXE_EXTENSION)

SRCS = \
	main.cpp \
	sequencer.cpp \
	utils.cpp \
	globals.cpp \
	gameobjects.cpp \
	gameutil.cpp \
	plog.cpp

OBJS = $(SRCS:%.cpp=%.o)

CXX_LDLIBS = \
	-Wl,--start-group \
	-lopencv_calib3d \
	-lopencv_contrib \
	-lopencv_core \
	-lopencv_features2d \
	-lopencv_flann \
	-lopencv_gpu \
	-lopencv_highgui \
	-lopencv_imgproc \
	-lopencv_legacy \
	-lopencv_ml \
	-lopencv_objdetect \
	-lopencv_ocl \
	-lopencv_photo \
	-lopencv_stitching \
	-lopencv_superres \
	-lopencv_ts \
	-lopencv_video \
	-lopencv_videostab \
	-Wl,--end-group

include generic-rules.makefile


