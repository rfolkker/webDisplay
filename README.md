# Introduction 
This is a simple web app to allow for web display driving a "MakerFocus ESP8266 OLED WiFi Development Board 0.91 Inch ESP8266 OLED Display CP2102 Support Arduino IDE ESP8266 NodeMCU LUA" that I bought off Amazon on a whim.

# Getting Started
This project was built using VSCode with both Python and Arduino extensions installed.
You will want to add the following manually to be able to build and use
* use the init.bat to initialize the python environment
* add the following library to your VSCode settings for Arduino boards additional urls
<pre>
    ["https://docs.heltec.cn/download/package_heltec_esp32_index.json",
     "https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.1/package_heltec_esp8266_index.json"]
</pre>
* Create a header called wifiSettings.h
* Copy the following information into the file and replace it with your settings
<pre>
    #ifndef STASSID
    #define STASSID "***"
    #define STAPSK  "***"
    #endif
</pre>
* Select the Web-Kit-8 board (in the case of the board I specified)

# Build and Test
To build and upload the arduino portion, simply open the ino file inside VSCode, and choose Verify and/or Upload from the menu.
<br />
To build and run the python portion, open the running.py and press F5, or the run button in the upper right (depending on if you wish to debug or simply execute)


# Contribute
TODO: N/A, this is just a quick run to try to build out a simple web driven display using a raspberry pi and an Arduino. Please feel free to take the code and use it any way you want.  I hope someone finds use in it, I know building it helped me understand a bit more about arduino and web functionality.
