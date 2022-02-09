#ifndef GPIOH_H
#define GPIOH_H

#include <stdint.h>

typedef uint8_t gpiopin_t;

// Tried to do this with const variables but the compiler gets angry about discarding const.
#define GPIO_IN   "in"
#define GPIO_OUT  "out"
#define GPIO_BOTH "both"

enum gpio_ErrCode {
	GPIO_OK,
	GPIO_ERR
};

enum gpio_ErrCode gpio_export(const gpiopin_t pin);
enum gpio_ErrCode gpio_setDir(const gpiopin_t pin, char* direction);
enum gpio_ErrCode gpio_readValue(char* buffer, const gpiopin_t pin, const uint32_t size);
enum gpio_ErrCode gpio_writeValue(const gpiopin_t pin, char* buffer);

#endif