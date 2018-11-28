# ESP32

In this code, the esp32 will listen to capture PROBE REQUEST packages and collect the most important data for triangulating points.

In order to keep the ESP32 clock synchronized very inaccurate, an SNTP server is used in which the internal clock synchronization variable has been changed and set to 1 minute.

In order to connect with the SNTP server and perform the capture of the packets you need to be associated with an AP (action that can be done either via the wifi module or as a bluetooth module). In the script it was decided to use the wifi module and consequently for the correct functioning of the code it is necessary to modify the "wifi_config" structure inside the "library.h" file, changing the SSID and password with those of the AP.
