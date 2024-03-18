#查询某个目录是否存在
DIR_TEST = ~/nfs
TEST_DIR_RESULT = $(shell if [ -d $(DIR_TEST) ]; then echo "exist"; else echo "noexist"; fi)

TARGET_NAME := mdalsa
VERSION := 1.0.0
TARGET := lib${TARGET_NAME}.so.${VERSION}

ROOT_DIR=./source
MD_STREAMER=$(PLUG_DIR)/mostreamer
MD_STREAMER_INTERFACE=$(PLUG_DIR)/interface
MD_STREAMER_COMMON=$(MD_STREAMER)/common
MD_STREAMER_MODULE=$(MD_STREAMER)/module


BUILD_DIR :=build
SRC_DIRS :=$(ROOT_DIR)
SRC_DIRS +=$(MD_STREAMER_COMMON)
SRC_DIRS +=$(MD_STREAMER_MODULE)
SRC_DIRS +=$(MD_STREAMER_INTERFACE)

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' )
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))


#plug 第三方库
LIB_FLAGS := -L$(TARGET_DIR)/lib
INC_FLAGS += -I$(PLUG_DIR)/thirdparty/moslog/source
LIBS += -lmoslog

#系统第三方库
CONFIG_LOG4CPP=y
CONFIG_PYBIND11=y
CONFIG_ALSA=y
include ${THIRD_PARTY_DIR}/thirdparty.mk

#vender
INC_FILES += -I$(VENDOR_DIR)/include  -I$(VENDOR_DIR)/include/alsa
LD_FLAGS +=  -lasound  -L$(VENDOR_DIR)/lib 


INC_FLAGS+=$(INC_FILES)
LIB_FLAGS+=$(LIB_FILES) 
LIBS+=$(LD_FLAGS) 


INCLUDS_FLAGS := $(INC_FLAGS) -MMD -MP

LIBS += -lpthread
LDFLAGS:=-shared 

ifeq ($(DEBUG_ENALBE),y)
CXXFLAGS:= -fPIC  -std=c++11 -Wall  -O0  -g -fno-gnu-unique
else
CXXFLAGS:= -fPIC  -std=c++11 -Wall  -O2   -fno-gnu-unique
endif

ifeq ($(CONFIG_PYBIND11),y)
	CXXFLAGS +=-DPYBIND11
endif

$(BUILD_DIR)/$(TARGET):$(OBJS)
	$(CXX) $(LDFLAGS)  $(OBJS) $(LIB_FLAGS) $(LIBS) -o $@
ifneq ($(DEBUG_ENALBE),y)	
	$(STRIP) $(BUILD_DIR)/$(TARGET)
endif


$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDS_FLAGS) $(CXXFLAGS)  -c $< -o $@


$(BUILD_DIR)/%.cpp.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDS_FLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	@echo "clean"
	rm -rf $(BUILD_DIR)

install:
	@echo "Install "
	mkdir -p $(TARGET_DIR)/module
	cp -f $(BUILD_DIR)/${TARGET} $(TARGET_DIR)/module/
ifneq ("$(TEST_DIR_RESULT)", "noexist")
	cp -f $(BUILD_DIR)/${TARGET} ~/nfs/ 
endif

uninstall:
	@echo "Uninstall"
	rm -f $(TARGET_DIR)/module/${TARGET}

-include $(DEPS)

