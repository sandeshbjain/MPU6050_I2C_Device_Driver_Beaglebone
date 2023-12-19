#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/proc/mpu6050_driver"

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open the device");
        return -1;
    }

    // Read data from the MPU6050 sensor
    printf("Reading data from MPU6050 sensor...\n");
    int buffer[6];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead == -1) {
        perror("Failed to read from the device");
        close(fd);
        return -1;
    }

    // Print the received data
    printf("Received data from MPU6050 sensor:\n");
    for (int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); ++i) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    // Example: Write data to the MPU6050 sensor (change as needed)
    // struct WriteData {
    //     u8 register_address;
    //     u8 data;
    // };
    // struct WriteData writeData = {0x1A, 0x42}; // Example register address and data
    // ssize_t bytesWritten = write(fd, &writeData, sizeof(struct WriteData));
    // if (bytesWritten == -1) {
    //     perror("Failed to write to the device");
    //     close(fd);
    //     return -1;
    // }

    close(fd);

    return 0;
}

