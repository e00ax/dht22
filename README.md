# C++ dht22 driver
C++ python port that uses pigpiod.<br>

## Prequisites:
- One or more dht22 sensors.
- A MySQL database.
- glibmm2.4, mysql c++ connector installled.
- Pigpiod running.

## Installation:
g++ -Wall -pthread -o dht22 main.cpp DHT22.cpp DHT22Ctl.cpp -lmysqlcppconn -lpigpio -lrt `pkg-config --cflags --libs glibmm-2.4`<br>
Place `<tests/sensor.ini>` in the same dir as the compiled file and fill in the details.<br>

# To run
./dht22
<br>

# As a Deamon
./DHT22_SYS_V start
<br>

# At startup
Move to /etc/init.d/<br>
chmod +x ./DHT22_SYS_V
