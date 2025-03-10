#CROSS_COMPILE =
#CROSS_COMPILE=/home/scar/buildtools/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
#CROSS_COMPILE = /home/dragon/arm/arm-bcm2708hardfp-linux-gnueabi-4.7.1/bin/arm-bcm2708hardfp-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-bcm2708-linux-gnueabi-4.7.1/bin/arm-bcm2708-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-embedsky-linux-gnueabi-4.6.0/bin/arm-embedsky-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-fsl-linux-gnueabi-4.6.2/bin/arm-fsl-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-none-linux-gnueabi-4.3.3__Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-none-linux-gnueabi-4.4.3/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-none-linux-gnueabi-4.5.1/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/arm-xilinx-linux-gnueabi-4.6.1_Sourcery-CodeBench-Lite/bin/arm-xilinx-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/arm/gcc-linaro-aarch64-linux-gnu-4.9-2014.07_linux/bin/aarch64-linux-gnu-
#CROSS_COMPILE = /home/dragon/arm/gcc-linaro-arm-linux-gnueabihf-4.7.3/bin/arm-linux-gnueabihf-
#CROSS_COMPILE = /home/dragon/arm/gcc-linaro-arm-linux-gnueabihf-4.8.3/bin/arm-linux-gnueabihf-
#CROSS_COMPILE = /home/dragon/arm/gcc-linaro-arm-linux-gnueabihf-4.8.3-raspbian/bin/arm-linux-gnueabihf-
#CROSS_COMPILE = /home/dragon/arm/arm-linux-gnueabihf-4.9.2/bin/arm-linux-gnueabihf-
#CROSS_COMPILE = /home/dragon/arm/arm-none-linux-gnueabi-4.4.1/bin/arm-none-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/sda2/arm/arm-fsl-linux-gnueabi-4.6.2/bin/arm-fsl-linux-gnueabi-
#CROSS_COMPILE = /home/dragon/sda2/arm/aarch64-linux-gnu-4.9.2/bin/aarch64-linux-gnu-
#CROSS_COMPILE = /home/dragon/sda2/arm/aarch64-linux-gnu-4.8.3/bin/aarch64-linux-gnu-
#CROSS_COMPILE = /home/dragon/sda2/arm/mips64el-4.9.3/usr/bin/mips64el-loongson-linux-
CC=$(CROSS_COMPILE)g++
OBJ := main.o tool.o

.PHONY: clean main
all:    clean main

main: $(OBJ)
	$(CC) -o $@ $^ -L./lib  -lpthread -ldl
$(OBJ): main.cpp tool.cpp
	$(CC) -c main.cpp -o main.o
	$(CC) -c tool.cpp -o tool.o
clean:
	rm -f $(OBJ) main

