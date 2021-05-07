# esp8266-iaq

![monitor.jpeg](/images/monitor.jpg)

A simple WiFi-enabled indoor air quality monitor that broadcasts its data using [dweet.io](dweet.io). It consists of source code for an ESP8266 attached to a DHT22 temperature and humidity sensor and a CSS811 CO2 sensor along with a Python script for plotting it. Alternatively, all of the monitoring can be performed through dweet.io themselves, including alerts and long-term logging if you pay for their "locks".

It is designed to pull data that is as accurate as possible from the CCS811, according to its datasheet. To that end, it implements the following:
* a baseline stored in the ESP8266's pseudo-EEPROM that must be calibrated in fresh air on the first run then is updated on-the-fly,
* a mandatory 20 minute run-in period after reset,
* and providing temperature and humidity data to the CCS811 so it can adjust its measurements.

A disclaimer: the CCS811 is *not* an absolute CO2 sensor. It actually measures other compounds people (TVOC) exhale then correlates that to CO2, making a figure it calls "estimated CO2" (eCO2). Additionally, it absolutely needs calibration in fresh air to be effective. Taking both considerations into account, it can still be a good indicator for indoor air quality.

## First-time Usage
0) Upload `esp8266-co2_monitor.ino` with the WiFi SSID and password configured.
1) Open the Serial Monitor in Arduino with the ESP8266 plugged in and sitting in fresh air.
2) Restart the ESP8266 then hold the FLASH button when prompted by the Serial Monitor.
3) Wait 20 minutes. During this time, record your monitor's unique API URL for later.
4) Move the ESP8266 to the desired location.
5) On subsequent resets or power-offs, no more action is necessary. Just wait 20 minutes.