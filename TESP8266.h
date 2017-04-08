/*
  Editor: Takeshi Okamoto
  https://github.com/TakeshiOkamoto/TESP8266/  

  このライブラリはqoosky氏がMITライセンスで公開している

  ---
  Arduino_HttpClient_ESP8266_AT 

  Copyright (c) 2016 Qoosky

  https://github.com/qoosky/Arduino_HttpClient_ESP8266_AT
  ---

  の機能を拡張したライブラリです。
*/

#ifndef __TESP8266_H__
#define __TESP8266_H__

#include <Arduino.h>

class TESP8266{           
  
 private:
    // AT command serial interface for ESP8266
    Stream *m_serial; 
              
    // Clear rx buffer
    void rxClear();

    // Check the response for the last AT command is "OK"
    bool checkATResponse(String target = "OK", uint32_t timeout = 1000);
    bool checkATResponse(String *buf, String target = "OK", uint32_t timeout = 1000); // store the data into buffer

    // Restart (Reset) ESP8266
    bool restart();
    
    // Get IPSTATUS of ESP8266
    //   2: ESP8266 station connected to an AP and has obtained IP
    //   3: ESP8266 station created a TCP or UDP transmission
    //   4: the TCP or UDP transmission of ESP8266 station disconnected
    //   5: ESP8266 station did NOT connect to an AP
    uint8_t ipStatus();

    // Create or Destroy TCP connection
    bool connectTcp(const String& host, uint32_t port);
    bool disconnectTcp();
    bool connectedTcp(); // true if TCP connection exists

    // Common HTTP request interface for GET and POST
    String sendRequest(const String& method,
                     const String& host, uint32_t port, const String& path, uint32_t& filesize,
                     const String& contentType = "", const String& body = "");   

 public:
    TESP8266(HardwareSerial &serial);
    ~TESP8266();

    // Health check of the serial interface
    bool statusAT(bool version = false);

    // Health check of the WiFi connection
    bool statusWiFi();

    // Connect/Disconnect ESP8266 to/from WiFi network
    bool connectAP(const String& ssid, const String& password, 
                   String ip="", String gateway="",String netmask="");
    
    // Disconnect Wifi connection           
    bool disconnectAP();
    
    // Forced disconnection of Wifi connection
    void lastResort();    

    // HTTP GET and POST
    String get(const String& host, const String& path, uint32_t& filesize, uint32_t port = 80);
    String post(const String& host, const String& path, const String& body, uint32_t& filesize,
              const String& contentType = "application/x-www-form-urlencoded", uint32_t port = 80);                                 
};

#endif // #ifndef __TESP8266_H__
