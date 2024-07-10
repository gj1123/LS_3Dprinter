#!/bin/bash  
  
# 确保脚本在Makefile所在的目录中运行  
# 如果没有，你需要修改下面的Makefile路径或将其设置为相对路径  
MAKEFILE="./Makefile"  
  
 sudo /home/yxc/Desktop/qt/bin/qmake 

# 检查Makefile是否存在  
if [ ! -f "$MAKEFILE" ]; then  
    echo "Error: Makefile does not exist at $MAKEFILE"  
    exit 1  
fi  
  
# 使用sed在第43行末尾添加链接器选项  
# 注意：这里简化了问题，直接添加到行末  
sed -i '43s/$/ -L\/opt\/loongarch64-linux-gnu-2021-12-10-vector\/loongarch64-linux-gnu\/sysroot\/lib64\/ -ldl/' Makefile
  
# 检查sed命令是否成功  
if [ $? -eq 0 ]; then  
    echo "Makefile updated successfully."  
    # 执行sudo make  
    sudo make  
else  
    echo "Error updating Makefile."  
    exit 1  
fi
