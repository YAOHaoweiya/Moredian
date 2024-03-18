#查询某个目录是否存在
DIR_TEST = ~/nfs
TEST_DIR_RESULT = $(shell if [ -d $(DIR_TEST) ]; then echo "exist"; else echo "noexist"; fi)

ROOT_DIR=.
TARGET_NAME := mdalsa_ao_test
TARGET := $(TARGET_NAME)

ROOT_DIR=./source
MD_STREAMER=$(PLUG_DIR)/mostreamer
MD_STREAMER_INTERFACE=$(PLUG_DIR)/interface
MD_STREAMER_COMMON=$(MD_STREAMER)/common
MD_STREAMER_MODULE=$(MD_STREAMER)/module
MD_STREAMER_FACTORY=$(MD_STREAMER)/factory


BUILD_DIR :=build
SRC_DIRS :=$(ROOT_DIR)
SRC_DIRS +=$(MD_STREAMER_COMMON)
SRC_DIRS +=$(MD_STREAMER_MODULE)
SRC_DIRS +=$(MD_STREAMER_INTERFACE)
SRC_DIRS +=$(MD_STREAMER_FACTORY)

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' )
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

#plug 第三方库
LIB_FLAGS := -L$(TARGET_DIR)/lib
INC_FLAGS += -I$(PLUG_DIR)/thirdparty/moslog/source
INC_FLAGS += -I$(PLUG_DIR)/thirdparty/hal_control/source
LIBS += -lmoslog -lhalcontrol


#动态库和包含文件
CONFIG_LOG4CPP=y
CONFIG_PYBIND11=y
CONFIG_ALSA=y
#第三方库配置
include ${THIRD_PARTY_DIR}/thirdparty.mk

INC_FLAGS+=$(INC_FILES)
LIB_FLAGS+=$(LIB_FILES) 
LIBS+=$(LD_FLAGS) 


INCLUDS_FLAGS := $(INC_FLAGS) -MMD -MP


LIBS += -lpthread -ldl -lasound     -L$(VENDOR_DIR)/lib 
CXXFLAGS:=   -std=c++11 -Wall  -O0  -g
ifeq ($(CONFIG_PYBIND11),y)
	CXXFLAGS +=-DPYBIND11
endif



$(BUILD_DIR)/$(TARGET):$(OBJS)
	$(CXX) $(LDFLAGS)  $(OBJS) $(LIB_FLAGS) $(LIBS) -o $@
#	$(STRIP) $(BUILD_DIR)/$(TARGET)	

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDS_FLAGS) $(CXXFLAGS)  -c $< -o $@


$(BUILD_DIR)/%.cpp.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDS_FLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	echo "clean"
	rm -rf $(BUILD_DIR)

install:
	mkdir -p $(TARGET_DIR)/test
	cp -f $(BUILD_DIR)/${TARGET} $(TARGET_DIR)/test/
ifneq ("$(TEST_DIR_RESULT)", "noexist")
	cp -f $(BUILD_DIR)/${TARGET} ~/nfs/ 
endif

uninstall:
	@echo "Uninstall"
	rm -f $(TARGET_DIR)/test/${TARGET}

-include $(DEPS)

