/**
 * hw.h
 * 
 * Hardware access functions (GPIO)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#ifndef INCLUDES_HW_H_
#define INCLUDES_HW_H_
#include "sysLib.h"
#include <subsys/gpio/vxbGpioLib.h>

/**
 * Defines
 */
typedef uint32_t PinNumber;

typedef enum {
	IN  = GPIO_DIR_INPUT,
	OUT = GPIO_DIR_OUTPUT
} ePinMode;

typedef enum  {
	LOW  = GPIO_VALUE_LOW,
	HIGH = GPIO_VALUE_HIGH
} ePinLevel;


/**
 * Configure GPIO pin for OUTput or INnput
 * @param pin	: pin number to configure
 * @param mode	: OUT / IN
 * @return		: STATUS
 */
STATUS pinMode(PinNumber pin, ePinMode mode);

/**
 * Get GPIO pin level (LOW /HIGH)
 * @param pin	: pin number to get level from
 * @return		: value o the PIN LOW / HIGH
 */
ePinLevel pinGet(PinNumber pin);

/**
 * Set GPIO pin level (LOW /HIGH)
 * @param pin	: pin number to set level to
 * @param level	: LOW / HIGH
 * @return		: STATUS
 */
STATUS pinSet(PinNumber pin, ePinLevel level);

/**
 * Free GPIO pin
 * @param pin	: pin number to free
 * @return		: STATUS
 */
STATUS pinFree(PinNumber pin);
#endif /* HW_H */
