# 交叉编译工具链前缀，确保这�?针�?�龙�?架构�?  
CROSS_COMPILE := /opt/loongarch64-linux-gnu-2021-12-10-vector/bin/loongarch64-linux-gnu-  
  
# 内核源码�?径和构建�?�?  
KERNEL_DIR := /home/yxc/Documents/linux-4.19-ls2k1000la
KERNEL_BUILD_DIR := $(KERNEL_DIR)  
  
# 编译选项（添加龙�?架构的头文件�?径）  
EXTRA_CFLAGS := -I$(KERNEL_DIR)/include
  
# 编译�?�?  
obj-m := ls2k1000la_3dprinter_motor.o 
  
# 默�?�目�?  
all:  
	$(MAKE) -C $(KERNEL_BUILD_DIR) M=$(PWD) modules ARCH=loongarch CROSS_COMPILE=$(CROSS_COMPILE) $(EXTRA_CFLAGS)
  
# 清理�?�?  
clean:  
	$(MAKE) -C $(KERNEL_BUILD_DIR) M=$(PWD) clean  
