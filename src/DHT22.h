#ifndef DHT22_H
#define DHT22_H

#include <unistd.h> 
#include <sstream>

// C callback instance
typedef void (*DHT22CB_t)(int, float, float, uint32_t, uint32_t, uint32_t, uint32_t);


/**
 * Class to read DHT22 sensor
 *
 * @see https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
 */
class DHT22
{
protected:
	int mygpio_0, mytimeout, bits;

	DHT22CB_t mycallback;

	// Error checking
	uint32_t bad_CS, bad_SM, bad_MM, bad_SR, CS;

	// Prevent the sensor from hanging
	uint32_t no_response, MAX_NO_RESPONSE;

	// Single bit transferred
	uint32_t val;

	// Bitwise readings for temp/hum
	uint32_t hH, hL, tH, tL, total;

	// Timing
	uint32_t high_tick, diff;

	// Multiplicator
	float mult;

	// Output
	float temp, hum;

	// Bitcode - only testing
	std::stringstream bitcode;


	// Actual c++ callback
	void _cb(int gpio, int level, uint32_t tick);

	/* Need a static callback to link with C. */
	static void _cbEx(int gpio, int level, uint32_t tick, void* user);


public:
	/*
	   This function establishes a DHT22 sensor on gpio_0.

	   ToDo:
	   A gap of timeout milliseconds without a new bit indicates
	   the end of a code.

	   When the code is ended the callback function is called with the code
	   bit length and value.
	*/
	DHT22(int gpio_0, DHT22CB_t callback, int timeout = 200);

	// Trigger sensor
	void trigger();

	// Return temperature
	float get_temp();

	// Return humidity
	float get_hum();

	/*
	   This function releases the resources used by the sensor.
	*/
	void cancel(void);
};
#endif
