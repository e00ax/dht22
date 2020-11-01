# dht22
C++ dht22 driver that reads from .ini file and logs to MySQL


# Requires
One (or more) DHT22.


# To Build
g++ -Wall -pthread -o dht22 main.cpp DHT22.cpp DHT22Ctl.cpp -lmysqlcppconn -lpigpio -lrt `pkg-config --cflags --libs glibmm-2.4`


# To run
./dht22


# As a Deamon
./DHT22_SYS_V start


# At startup
Move to /etc/init.d/
chmod +x ./DHT22_SYS_V

Feature	Status	Version
Notifications (e.g. GPIO changes)	✔️ Fully supported	0.1.1
Reading PWM signals	✔️ Fully supported	0.1.2
Sending PWM signals	✔️ Fully supported	0.3.0
SPI communication	✔️ Fully supported	0.2.0
Hall sensors rotation counter	✔️ Fully supported	0.2.5
Raw requests/response handling	✔️ Fully supported	0.1.1
