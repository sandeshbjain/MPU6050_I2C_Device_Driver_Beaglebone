#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#define X_AXIS_ACCL 0x3B
#define Y_AXIS_ACCL 0x3D
#define Z_AXIS_ACCL 0x3F

#define X_AXIS_GYRO 0x43
#define Y_AXIS_GYRO 0x45
#define Z_AXIS_GYRO 0x47

static struct proc_dir_entry *proc_file;
static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int mpu6050_remove(struct i2c_client *client);

struct WriteData {
    u8 register_address;
    u8 data;
};


//Associate mpu6050 as an I2C device
static const struct i2c_device_id mpu6050_i2c_ids[] = {
	{"mpu6050",0},
	{}
};

static struct of_device_id my_driver_ids[]={
		{
				.compatible = "invesense,mpu6050",
		},{}
};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct i2c_client *mpu6050_client;

// Informing the ernel that my module needs
MODULE_DEVICE_TABLE(i2c,mpu6050_i2c_ids);


static struct i2c_driver mpu6050_driver = {
		.driver={
				.name= "mpu6050_driver",
				.owner= THIS_MODULE,
				.of_match_table= my_driver_ids,
		},
		.probe=mpu6050_probe,
		.remove=mpu6050_remove,
		.id_table= mpu6050_i2c_ids,
};

module_i2c_driver(mpu6050_driver);

static int mpu6050_open(struct inode *inode, struct file *file){



	return 0;
}

static ssize_t mpu6050_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
	printk("MPU reading");

	//Read the data
	int buffer[6];

	int ret;

	ret = i2c_smbus_read_word_data(mpu6050_client, X_AXIS_ACCL);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[0] = ret;

	ret = i2c_smbus_read_word_data(mpu6050_client, Y_AXIS_ACCL);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[1] = ret;

	ret = i2c_smbus_read_word_data(mpu6050_client, Z_AXIS_ACCL);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[2] = ret;

	//
	ret = i2c_smbus_read_word_data(mpu6050_client, X_AXIS_GYRO);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[3] = ret;

	ret = i2c_smbus_read_word_data(mpu6050_client, Y_AXIS_GYRO);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[4] = ret;

	ret = i2c_smbus_read_word_data(mpu6050_client, Z_AXIS_GYRO);
	if (ret < 0) {
		printk("mpu6050 - Error reading X-axis accelerometer data from the sensor\n");
		return ret;
	}
	buffer[5] = ret;

	if(copy_to_user(buf, buffer, sizeof(buffer))){
		printk("Error in copying values to buffer");
		return -EFAULT;
	}
	return sizeof(buffer);
}

static ssize_t mpu6050_write(struct file *file,const char __user *buf, size_t count, loff_t *offset){
	printk("MPU6050 - Write function\n");

	// Check if user buffer is valid
	if (!buf) {
		return -EFAULT;
	}

	// Validate the size of the user buffer
	if (count != sizeof(struct WriteData)) {
		printk("MPU6050 - Invalid data size in user buffer\n");
		return -EINVAL;
	}

	// Read the data from user space
	struct WriteData write_data;
	if (copy_from_user(&write_data, buf, sizeof(struct WriteData))) {
		printk("MPU6050 - Error copying data from user space\n");
		return -EFAULT;
	}


	// Process the command and write to the sensor
	i2c_smbus_write_byte_data(mpu6050_client, write_data.register_address, write_data.data);

	return sizeof(struct WriteData); // Return the number of bytes written

}

static int mpu6050_release(struct inode *inode, struct file *file){


	return 0;
}



static const struct file_operations fops={
		.owner = THIS_MODULE,
		.open = mpu6050_open,
		.read = mpu6050_read,
		.write = mpu6050_write,
		.release = mpu6050_release,

};

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id){
	printk("In the Probe Function MPU6050 \n");

	if(client->addr != 0x68){
		printk("Wrong I2C address \n");
		return -1;
	}

	if(client->adapter->nr !=2){
		printk("Error: MPU is not on I2C bus 2");
		return -1;
	}

	//wakeup sensor
	i2c_smbus_write_byte_data(client, 0x6B, 0);
	//Disable FSYNC, 260Hz gyro bandwidth ---------> needed ?????!!!!!
	i2c_smbus_write_byte_data(client, 0x1A, 0);
	//write 8 to register 27 - Gyroscope
	i2c_smbus_write_byte_data(client, 0x1B, 8);
	//write 8 to register 28 - Acclerometer
	i2c_smbus_write_byte_data(client, 0x1C, 8);

	msleep(250);

	proc_file = proc_create("mpu6050_driver", 0666, NULL, &fops);

	if(proc_file== NULL){
		printk("Error creating proc file \n");
		return -ENOMEM;
	}
	return 0;
}

static int mpu6050_remove(struct i2c_client *client){
	proc_remove(proc_file);
	return 0;
}


MODULE_AUTHOR("SANDESH- sandeshbaghmar@gmail.com");
MODULE_DESCRIPTION("I2C DEVICE DRIVER FOR MPU6050 GYROSCOPE MODULE");
MODULE_LICENSE("GPL");

