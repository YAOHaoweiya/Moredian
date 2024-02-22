#查询某个目录是否存在
DIR_TEST = ~/nfs
TEST_DIR_RESULT = $(shell if [ -d $(DIR_TEST) ]; then echo "exist"; else echo "noexist"; fi)


TARGET=yhw_vivotest

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


#plug thirdparty
PLUG_THIRDPARTY_MOSLOG=y
PLUG_THIRDPARTY_HAL_CONTROL=y
PLUG_THIRDPARTY_MOSMISC=y
include ${PLUG_DIR}/thirdparty/thirdparty.mk

#动态库和包含文件
CONFIG_LOG4CPP=y
CONFIG_CURL=y
CONFIG_OPENSSL=y
CONFIG_ZLIB=y
#第三方库配置
include ${THIRD_PARTY_DIR}/thirdparty.mk

INC_FLAGS+=$(INC_FILES)
LIB_FLAGS+=$(LIB_FILES) 
LIBS+=$(LD_FLAGS) 


INCLUDS_FLAGS := $(INC_FLAGS) -MMD -MP


LIBS += -lpthread -ldl 
CXXFLAGS:=   -std=c++11 -Wall  -O0  -g 



$(BUILD_DIR)/$(TARGET):$(OBJS)
	$(CXX) $(LDFLAGS)  $(OBJS) $(LIB_FLAGS) $(LIBS) -o $@


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

