/**
 * hw.c
 * 
 * Hardware access functions (GPIO)
 *
 * @author: Alessandro Mannini <alessandro.mannini@gmail.com>
 * @date: Jan 10, 2023
 */

#include "hw.h"

STATUS pinMode(PinNumber pin, ePinMode mode) {
	return vxbGpioSetDir(pin, mode);
}

ePinLevel pinGet(PinNumber pin) {
	return vxbGpioGetValue(pin);
}

STATUS pinSet(PinNumber pin, ePinLevel level) {
	return vxbGpioSetValue(pin, level);
}


STATUS pinFree(PinNumber pin) {
	return vxbGpioFree(pin);
}
