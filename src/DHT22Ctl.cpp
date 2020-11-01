#include <iostream>
#include <sstream>
#include <pigpio.h>
#include "DHT22Ctl.h"

// Static object hook
DHT22Ctl* DHT22Ctl::s_hookObj;

/**
 * Constructor
 *
 * @param string server
 * @param string username
 * @param string passwd
 * @param string db
 * @return void
 */
DHT22Ctl::DHT22Ctl(
	std::string server,
	std::string username,
	std::string passwd,
	std::string db,
	int gpio
)
{
	// MySQL connector
	try
	{
		// Convert std:string because mysql driver expects arg[] as const char*
		const char* dht22_server = server.c_str();
		const char* dht22_username = username.c_str();
		const char* dht22_passwd = passwd.c_str();
		const char* dht22_db = db.c_str();

		// Create con
		driver = get_driver_instance();
		con = driver->connect(
			dht22_server,
			dht22_username,
			dht22_passwd
		);

		// Connect to db
		con->setSchema(dht22_db);
	}

	catch (sql::SQLException & e)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}

	// Sensor
	dht22_gpio = gpio;
	dht22_bits = 0;
	dht22_temp = -999.0;
	dht22_hum = -999.0;
	dht22_bad_CS = 0;
	dht22_bad_MM = 0;
	dht22_bad_SM = 0;
	dht22_bad_SR = 0;

	// Reference static object hook to this
	s_hookObj = this;
}


/**
 * Destructor
 *
 * @return void
 */
DHT22Ctl::~DHT22Ctl()
{
}


/**
 * Callback
 *
 * @param int bits bits transferred
 * @param float temp Sensor temperature
 * @param float hum humidity
 * @param uint32_t bad_CS bad checksum
 * @param uint32_t bad_MM
 * @param uint32_t bad_SM
 * @param uint32_t bad_SR
 * @return void
 */
void DHT22Ctl::clbck(int bits, float temp, float hum, uint32_t bad_CS, uint32_t bad_MM, uint32_t bad_SM, uint32_t bad_SR)
{
	// Callback data
	s_hookObj->dht22_bits = bits;
	s_hookObj->dht22_temp = temp;
	s_hookObj->dht22_hum = hum;
	s_hookObj->dht22_bad_CS = bad_CS;
	s_hookObj->dht22_bad_MM = bad_MM;
	s_hookObj->dht22_bad_SM = bad_SM;
	s_hookObj->dht22_bad_SR = bad_SR;
}


/**
 * Get sensor data as string
 *
 * @return string
 */
std::string DHT22Ctl::get_str()
{
	std::stringstream ss;
	ss << "bits=" << dht22_bits << " Temp=" << dht22_temp << " Hum=" << dht22_hum << " " << dht22_bad_CS << " " << dht22_bad_MM << " " << dht22_bad_SM << " " << dht22_bad_SR << std::endl;

	return ss.str();
}


/**
 * Get temperature
 *
 * @return float
 */
float DHT22Ctl::get_temp()
{
	return dht22_temp;
}


/**
 * Get humidity
 *
 * @return float
 */
float DHT22Ctl::get_hum()
{
	return dht22_hum;
}


/**
 * Trigger dht22 with inverval
 *
 * @return void
 */
void DHT22Ctl::trigger_sensor()
{
	// Initialise pigpio
	if (gpioInitialise() < 0)
	{
		std::cout << "Pigpio init failed!" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Initialise sensor
	DHT22 dht22(dht22_gpio, clbck);

	// Perform 3 messurements and take only the last one as data for more accuracy
	for (int i = 0; i < MEASUREMENTS; i++)
	{
		// Trigger sensor
		dht22.trigger();

		// Prevent the sensor from hanging
		usleep(DHT22_OVERALL_POLL);
	}

	// Cancel dht22 sensor
	dht22.cancel();

	// Terminate gpio
	gpioTerminate();
}


/**
 * Write sensor data to mysql
 *
 * @return void
 */
void DHT22Ctl::sql_write()
{
	std::stringstream ss_query;

	try
	{
		// Create statement
		stmt = con->createStatement();

		ss_query << "INSERT INTO `dht22` (temp, hum, bad_CS, bad_MM, bad_SM, bad_SR) VALUES ('"
			<< dht22_temp << "', '"
			<< dht22_hum << "', '"
			<< dht22_bad_CS << "', '"
			<< dht22_bad_MM << "', '"
			<< dht22_bad_SM << "', '"
			<< dht22_bad_SR << "')"
			<< std::endl;

		// Execute query
		stmt->execute(ss_query.str());
	}

	catch (sql::SQLException & e)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}


/**
 * Cancel connection details
 *
 * @return void
 */
void DHT22Ctl::sql_cancel()
{
	delete res;
	delete stmt;
	delete con;
}
