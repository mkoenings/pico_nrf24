#ifndef __I2C_BUS_H__
#define __I2C_BUS_H__

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void i2c_bus_create(void);

esp_err_t i2c_bus_write_bytes(i2c_bus_handle_t bus, int addr, uint8_t* reg, int reglen, uint8_t* data, int datalen);

/// This method is used
esp_err_t i2c_bus_read_bytes(i2c_bus_handle_t bus, int addr, uint8_t* reg, int reglen, uint8_t* outdata, int datalen);

esp_err_t i2c_bus_delete(void);

#ifdef __cplusplus
}
#endif

#endif // __I2C_BUS_H__