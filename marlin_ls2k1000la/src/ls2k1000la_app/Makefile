
HEADER_FILE = $(wildcard *.h)
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
DEST = 3dprinter_app

CROSS_COMPILE = mipsel-linux-
CXX = gcc

# 函数lround()需要用参数-std=c99编译，但是函数usleep()又不能使用，最后发现参数-std=gnu99两者能兼顾
#CFLAGS += -Wall -std=c99
CFLAGS += -Wall -std=gnu99
LDFLAGS += -lm -lpthread

all:$(DEST)
	
$(DEST):$(OBJ)
	$(CROSS_COMPILE)$(CXX) $(LDFLAGS) $^ -o $@
	cp $@ /nfsramdisk/LS1xrootfs-demo/test/


$(OBJ):$(SRC) $(HEADER_FILE)
	$(CROSS_COMPILE)$(CXX) $(CFLAGS) -c $^

clean:
	rm -f *.o $(DEST) *.h.gch

