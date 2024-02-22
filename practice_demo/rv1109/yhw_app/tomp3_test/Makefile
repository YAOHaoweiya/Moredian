# Makefile for RTC ioctl test





# 交叉编译工具链目录
TOOL_CHAIN_DIR=${PWD}/../../prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin
# C编译交叉编译工具链路径
CC=${TOOL_CHAIN_DIR}/arm-linux-gnueabihf-gcc
# C++ 编译交叉编译工具链路径
CXX=${TOOL_CHAIN_DIR}/arm-linux-gnueabihf-g++
# 指定链接器的路径
LD=${TOOL_CHAIN_DIR}/arm-linux-gnueabihf-ld
# 指定用于剥离二进制文件中的符号信息的工具的路径
STRIP=${TOOL_CHAIN_DIR}/arm-linux-gnueabihf-strip
# 指定用于复制二进制文件的工具的路径
OBJCOPY=${TOOL_CHAIN_DIR}/arm-linux-gnueabihf-objcopy


# 编译选项
CFLAGS := -Wall 

# 输出可执行文件,目标文件
TARGET = tomp3_test

#指定除了编译器默认的头文件和库文件的路径之外需要额外引用的头文件路径以及库的路径
INCLUDES := -I./include \
 	   -I/data/yaohaowei/ffmpeg-6.1-amd64-static/includes/ffmpeg_/libavformat
LIBS = -L/${PWD}/libs/

# 源文件
SRCS := ./src/tomp3.c
#SRCS := ./src/v4l2_test.c
#SRCS = $(wildcard ./src/*.c)

# 对象文件
#OBJ := $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
OBJS = $(SRCS:.c = .o)

#编译
%.o: %.c
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@
	
#创建output ,链接规则
$(TARGET):$(OBJS)
	@mkdir -p output
	$(CC) $(OBJS) -o output/$(TARGET)
#	@rm -rf $(OBJS)

# 默认目标
all:  $(TARGET)



# 清理规则
#clean:
#	rm -rf $(BUILD_DIR) $(TARGET)

# 安装到开发板
#install: all
	# 在此添加拷贝到开发板的命令，例如 scp 等

# 默认目标
#.PHONY: all clean install
