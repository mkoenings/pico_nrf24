#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Definitions for error constants. */
#  define ESP_OK          0       /*!< esp_err_t value indicating success (no error) */
#  define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure */
#  define ESP_ERR_INVALID_ARG 1
typedef int esp_err_t;
typedef void *i2c_bus_handle_t;

#ifdef __cplusplus
}
#endif

#endif // __TYPES_H__