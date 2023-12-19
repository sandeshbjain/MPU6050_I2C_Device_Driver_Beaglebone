obj-m += mpu6050_driver.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

dt:
	dtc -@ -I dts -O dtb -o bb_mpu6050.dtb bb_mpu6050.dts
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	

