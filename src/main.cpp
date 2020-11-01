#define CONFIG_FILE "sensor.ini"

#include <glibmm.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "DHT22Ctl.h"

/**
 * Deamon code
 *
 * @param int argc number of args
 * @param char argv cmd args
 * @return void
 */
int main(int argc, char* argv[])
{
	Glib::KeyFile gkf;
	std::string gpio;
	std::string measurement_intervall;
	std::string sql_server, sql_username, sql_passwd, sql_db;
	std::stringstream ss;

	try
	{
		gkf.load_from_file(CONFIG_FILE);
	}
	catch (const Glib::Error & ex)
	{
		std::cerr << "Exception while loading key file: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	/*
	 * Store groups as vector
	 *
	 * Standard type for Glib::KeyFile::get_groups() is
	 * Glib::ArrayHandle<Glib::ustring> ...
	 * But you can use every container like std::vector or std::map
	 * wich is much more handy!
	 */
	std::vector<std::string> groups = gkf.get_groups();
	//int size = groups.size();

	// Get settings from ini file
	try
	{
		// Loop over groups and spawn a process for each group
		for (auto itr = groups.begin(); itr != groups.end(); ++itr)
		{
			// [Debug]
			//std::cout << "Group: " << itr[0] << std::endl;
			
			// Store keys as vector
			std::vector<std::string> keys = gkf.get_keys(itr[0]);

			// Loop over keys
			for (auto it = keys.begin(); it != keys.end(); ++it)
			{
				// Get value as string
				std::string val = gkf.get_value(itr[0], it[0]);

				if (it[0] == "gpio")
					gpio = val;

				if (it[0] == "measurement_intervall")
					measurement_intervall = val;

				if (it[0] == "sql_server")
					sql_server = val;

				if (it[0] == "sql_username")
					sql_username = val;

				if (it[0] == "sql_passwd")
					sql_passwd = val;

				if (it[0] == "sql_db")
					sql_db = val;

				// [Debug]
				//std::cout << "Key: " << it[0] << " Value: " << val << std::endl;
			}
		}
	}
	catch (const Glib::KeyFileError & kfex)
	{
		std::cerr << "Exception reading setting from ini file: " << kfex.what() << std::endl;
		exit(-1);
	}

	// Initialise DHT22 sensor
	DHT22Ctl dht22_ctl(
		sql_server,
		sql_username,
		sql_passwd,
		sql_db,
		stoi(gpio)
	);

	while (true)
	{
		// Trigger sensor
		dht22_ctl.trigger_sensor();

		// Formatted output
		std::cout << dht22_ctl.get_str() << std::endl;

		// Write data to MySQL database
		dht22_ctl.sql_write();

		sleep(stoi(measurement_intervall));
	}

	return EXIT_SUCCESS;
}