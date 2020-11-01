# dht22
C++ dht22 driver that reads from .ini file and logs to MySQL
<br>

# Requires
One (or more) DHT22.
<br>

# To Build
g++ -Wall -pthread -o dht22 main.cpp DHT22.cpp DHT22Ctl.cpp -lmysqlcppconn -lpigpio -lrt `pkg-config --cflags --libs glibmm-2.4`
<br>

# To run
./dht22
<br>

# As a Deamon
./DHT22_SYS_V start
<br>

# At startup
Move to /etc/init.d/<br>
chmod +x ./DHT22_SYS_V
