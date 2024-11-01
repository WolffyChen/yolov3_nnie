# Hisilicon Hi3519A Makefile
CC:=arm-himix200-linux-g++ -std=c++11 -mcpu=cortex-a53 -mfloat-abi=softfp -mfpu=neon-vfpv4

INC_FLAGS := -I$(PWD)/sdk/include \
				-I$(PWD)/../common \
				-I$(PWD)/../../common
INC_FLAGS += -I$(PWD)/src
LIB_FLAGS := -L$(PWD)/sdk/lib \
				-L$(PWD)

CFLAGS += -Wall -g -O3 $(INC_FLAGS) $(LIB_FLAGS) -fpermissive

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

TARGET := hello

.PHONY : clean all

all: $(TARGET)
$(TARGET):$(OBJS)
	@$(CC) -o $(TARGET) $(OBJS) $(CFLAGS) libbg_nnie.a libhis.a -lpthread -lm -ldl
$(OBJS):$(SRCS)
	@$(CC) -c $(CFLAGS) $< -o $@

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
