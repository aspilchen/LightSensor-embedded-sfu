// static int initI2cBus(char* bus, int address)
// {
// 	int i2cFileDesc = open(bus, O_RDWR);
// 	if (i2cFileDesc < 0) {
// 		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
// 		perror("Error is:");
// 		exit(-1);
// 	}
// 
// 	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
// 	if (result < 0) {
// 		perror("Unable to set I2C device to slave address.");
// 		exit(-1);
// 	}
// 	return i2cFileDesc;
// }