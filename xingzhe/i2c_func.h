#include <linux/types.h>
#include <sys/ioctl.h>

#include <pthread.h>

#ifndef __I2C_FUNC_H__
#define __I2C_FUNC_H__

class I2C_Func {
public:
    I2C_Func(int my_bus);
    I2C_Func();
    int i2c_open(unsigned char bus, unsigned char addr);
    int i2c_write(int handle, unsigned char* buf, unsigned int length);
    int i2c_read(int handle, unsigned char* buf, unsigned int length);
    int i2c_write_read(int handle,
                       unsigned char addr_w, unsigned char *buf_w, unsigned int len_w,
                       unsigned char addr_r, unsigned char *buf_r, unsigned int len_r);
    int i2c_write_ignore_nack(int handle,
                              unsigned char addr_w, unsigned char* buf, unsigned int length);
    int i2c_read_no_ack(int handle, 
                        unsigned char addr_r, unsigned char* buf, unsigned int length);
    int i2c_write_byte(int handle, unsigned char val);
    int i2c_read_byte(int handle, unsigned char* val);

    int i2c_close(int handle);

    int i2c_smbus_write_byte(int file, unsigned char value);
    int i2c_smbus_read_byte_data(int file, unsigned char command);
    int i2c_smbus_read_byte(int file);
    int i2c_smbus_write_byte_data(int file, unsigned char command, unsigned char value);
    int delay_ms(unsigned int msec);
private:
    int i2c_smbus_access(int file, char read_write, unsigned char command, 
                                     int size, union i2c_smbus_data *data);

    int i2c_smbus_write_quick(int, unsigned char);
    int i2c_smbus_read_word_data(int, unsigned char);
    int i2c_smbus_write_word_data(int, unsigned char, short unsigned int);
    int i2c_smbus_process_call(int, unsigned char, short unsigned int);
    int i2c_smbus_read_block_data(int, unsigned char, unsigned char*);
    int i2c_smbus_write_block_data(int, unsigned char, unsigned char, const unsigned char*);
    int i2c_smbus_read_i2c_block_data(int, unsigned char, unsigned char, unsigned char*);
    int i2c_smbus_write_i2c_block_data(int, unsigned char, unsigned char, const unsigned char*);
    int i2c_smbus_block_process_call(int, unsigned char, unsigned char, unsigned char*);

private:
    pthread_mutex_t mutex_0;
    pthread_mutex_t mutex_1;
    pthread_mutex_t mutex_2;

    int bus;
    
};


#endif  /* __I2C_FUNC_H__ */

