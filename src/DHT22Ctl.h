#ifndef DHT22CTL_H
#define DHT22CTL_H

#define DHT22_OVERALL_POLL 2000000
#define MEASUREMENTS 3

#include <string>
#include <cstring>
#include <stdlib.h>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "DHT22.h"


/**
 * Read DHT22 sensor
 *
 */
class DHT22Ctl
{
private:
	// MySQL
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* res;

	// Sensor
	int dht22_gpio;
	int dht22_bits;
	float dht22_temp;
	float dht22_hum;
	uint32_t dht22_bad_CS;
	uint32_t dht22_bad_MM;
	uint32_t dht22_bad_SM;
	uint32_t dht22_bad_SR;

public:
	// Constructor
	DHT22Ctl(
		std::string server,
		std::string username,
		std::string passwd,
		std::string db,
		int gpio
	);

	// Destructor
	virtual ~DHT22Ctl();

	// Callback
	static void clbck(int bits = 0, float temp = -999.0, float hum = -999.0, uint32_t bad_CS = 0, uint32_t bad_MM = 0, uint32_t bad_SM = 0, uint32_t bad_SR = 0);

	// Get sensor data as string
	std::string get_str();

	// Get temp
	float get_temp();

	// Get hum
	float get_hum();

	// Trigger DHT22 with interval
	void trigger_sensor();

	// Write sensor data to mysql
	void sql_write();

	// Cancel Connection
	void sql_cancel();

	// Static reference
	static DHT22Ctl* s_hookObj;
};
#endif
