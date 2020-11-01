#include <iostream>
#include <pigpio.h>
#include "DHT22.h"


/**
 * Constructor
 *
 * @param int gpio_0
 * @param DHT22CB_t callback
 * @param int timeout
 * @return void
 */
DHT22::DHT22(
	int gpio_0,
	DHT22CB_t callback,
	int timeout
)
{
	/*
	 *  Instantiate with the gpio,
	 *  the callback function, and the bit timeout in
	 *  milliseconds which indicates the end of a code.
	 *
	 *  The callback is passed the code length in bits and the value.
	 */
	mygpio_0 = gpio_0;
	mycallback = callback;
	mytimeout = timeout;

	bad_CS = 0;
	bad_SM = 0;
	bad_MM = 0;
	bad_SR = 0;

	no_response = 0;
	MAX_NO_RESPONSE = 2;

	hum = -999;
	temp = -999;

	high_tick = 0;
	bits = 40;
}


/**
 * Pigpio callback
 *
 * @param int gpio gpio to use
 * @param int level Edge level
 * @param uint32_t tick ticks measured
 * @return void
 */
void DHT22::_cb(
	int gpio,
	int level,
	uint32_t tick
)
{
	/*
	 * Edge length (tick difference) determines if bit is 1 or 0
	 *
	 * Edge length > 70 microseconds means 1
	 * Edge length > 25 microseconds means 0
	 */

	 // Falling edge
	if (level == 0) /* a falling edge indicates a new bit */
	{
		// Get tick difference
		// Tick = Time since last boot in microseconds
		// This varies roughly verery 72min.
		diff = tick - high_tick;

		// Check tick difference
		if (diff >= 50)
		{
			val = 1;

			// Bad bit?
			if (diff >= 200)
				CS = 256;
		}

		else
		{
			val = 0;
		}

		// [Debug]
		//std::cout << "==================================================" << std::endl;
		//std::cout << "level 0: Falling edge" << std::endl;
		//std::cout << "Tick: " << tick << std::endl;
		//std::cout << "High Tick: " << high_tick << std::endl;
		//std::cout << "Diff: " << diff << std::endl;
		//std::cout << "Bits: " << bits << std::endl;
		//std::cout << "Val: " << val << std::endl;

		// Build bitcode
		//bitcode << val;
		//std::cout << "Bitcode: " << bitcode.str() << std::endl;


		// Message complete
		if (bits >= 40)
		{
			bits = 40;
		}



		/*
		 * Shove each bit in position
		 *
		 * hH = Humidity high
		 * hL = Humidity low
		 * tH = Temperature high
		 * tL = Temperature low
		 */
		 // hH
		if (bits >= 0 && bits < 8)
			hH = (hH << 1) + val;

		// hL
		if (bits >= 8 && bits < 16)
			hL = (hL << 1) + val;

		// tH
		if (bits >= 16 && bits < 24)
			tH = (tH << 1) + val;

		// tL
		if (bits >= 24 && bits < 32)
			tL = (tL << 1) + val;


		/*
		 * Transmission complete
		 *
		 * Check for checksum error and decode
		 * Humididy and temperature.
		 */
		if (bits >= 32)
		{
			// Shove for checksum
			CS = (CS << 1) + val;

			// 40th bit received
			if (bits == 39)
			{
				// Kill watchdog
				gpioSetWatchdog(mygpio_0, 0);

				no_response = 0;

				// Calc checksum
				total = hH + hL + tH + tL;

				// Checksum OK?
				if ((total & 255) == CS)
				{
					// Humidity
					hum = ((hH << 8) + hL) * 0.1;

					// Temperature multiplicator (-)
					if (tH & 128)
					{
						mult = -0.1;
						tH &= 127;
					}

					// Temperature multiplicator (+)
					else
					{
						mult = 0.1;
					}

					// Temperature
					temp = ((tH << 8) + tL) * mult;

					// Timestamp?
					//self.tov = time.time()

					// Pass values to callback
					(mycallback)(bits,
						temp,
						hum,
						bad_CS,
						bad_MM,
						bad_SM,
						bad_SR);

					// [Debug]
					//std::cout << "Humidity: " << hum << std::endl;
					//std::cout << "Temperature: " << temp << std::endl;
				}

				// Checksum failed
				else
				{
					bad_CS += 1;
				}
			}
		}

		// Pass - header bits
		else
		{
			// Do nothing ...
			//std::cout << "header bits: " << bits << std::endl;
		}

		bits += 1;
	}


	// Rising edge
	else if (level == 1)
	{
		// [Debug]
		//std::cout << "==================================================" << std::endl;
		//std::cout << "level 1: Rising edge" << std::endl;
		//std::cout << "Tick: " << tick << std::endl;
		//std::cout << "High Tick: " << high_tick << std::endl;
		//std::cout << "Diff: " << diff << std::endl;
		//std::cout << "Bits: " << bits << std::endl;

		high_tick = tick;

		if (diff > 250000)
		{
			bits = -2;
			hH = 0;
			hL = 0;
			tH = 0;
			tL = 0;
			CS = 0;
		}
	}


	// Specific timeout
	else if (level == PI_TIMEOUT)
	{
		//std::cout << "==================================================" << std::endl;
		//std::cout << "level PI_TIMEOUT: User specific timeout" << std::endl;
		//std::cout << "Bits: " << bits << std::endl;

		// Kill watchdog
		gpioSetWatchdog(mygpio_0, 0);

		// Too few data bits received
		if (bits < 8)
		{
			// Bump missing message count
			bad_MM += 1;

			no_response += 1;

			if (no_response > MAX_NO_RESPONSE)
			{
				no_response = 0;

				// Bump sensor reset count
				bad_SR += 1;
			}
		}

		// Short message receieved
		else if (bits < 39)
		{
			// Bump short message count
			bad_SM += 1;

			no_response = 0;
		}

		else
		{
			no_response = 0;
		}

		// Pass values to callback
		(mycallback)(bits,
			temp,
			hum,
			bad_CS,
			bad_MM,
			bad_SM,
			bad_SR);
	}
}


/**
 * Need a static callback to link with C
 *
 * @param int gpio gpio to use
 * @param int level Edge level
 * @param uint32_t tick ticks to measure
 * @param void user
 * @return void
 */
void DHT22::_cbEx(int gpio, int level, uint32_t tick, void* user)
{
	DHT22* mySelf = (DHT22*)user;

	mySelf->_cb(gpio, level, tick); /* Call the instance callback. */
}


/**
 * Trigger sensor
 *
 * @return void
 */
void DHT22::trigger()
{
	// Testing
	//gpioSetMode(24, PI_OUTPUT);
	//gpioWrite(24, 1);

	// Kill watchdog on gpio
	gpioSetWatchdog(mygpio_0, 0);

	// Pull down gpio starts the transfer initiation
	gpioSetPullUpDown(mygpio_0, PI_PUD_DOWN);

	// Set gpio to LOW
	gpioWrite(mygpio_0, 0);

	// sleep for exactly 18 milliseconds
	usleep(18000);

	// Set gpio to input
	gpioSetMode(mygpio_0, PI_INPUT);

	// Set watchdog with timeout
	gpioSetWatchdog(mygpio_0, mytimeout);

	// Connect callback
	gpioSetAlertFuncEx(mygpio_0, _cbEx, this);
}


/**
 * Get temperature
 *
 * @return float temperature
 */
float DHT22::get_temp()
{
	return temp;
}


/**
 * Get humidity
 *
 * @return float humidity
 */
float DHT22::get_hum()
{
	return hum;
}


/**
 * Cancel the DHT22 sensor
 *
 * @return void
 */
void DHT22::cancel(void)
{
	gpioSetAlertFuncEx(mygpio_0, 0, this);
}