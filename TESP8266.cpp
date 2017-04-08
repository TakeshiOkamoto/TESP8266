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

#include "TESP8266.h"

// ----------------------------------------------------------------------------
//  Private
// ----------------------------------------------------------------------------
void TESP8266::rxClear() {
    while(m_serial->available() > 0) m_serial->read();
}

bool TESP8266::checkATResponse(String *buf, String target, uint32_t timeout) {
    *buf = "";
    char c;
    const unsigned long start = millis();
    while (millis() - start < timeout) {
        while(m_serial->available() > 0) {
            c = m_serial->read(); // 1 byte
            if(c == '\0') continue;
            *buf += c;
        }
        if (buf->indexOf(target) != -1) return true;
    }
    return false;
}

bool TESP8266::checkATResponse(String target, uint32_t timeout) {
    String buf;
    return checkATResponse(&buf, target, timeout);
}

bool TESP8266::restart() {
    rxClear();
    m_serial->println(F("AT+RST"));
    if(!checkATResponse()) return false;
    delay(2000);
    const unsigned long start = millis();
    while(millis() - start < 3000) {
        if(statusAT()) {
            delay(1500);
            return true;
        }
        delay(100);
    }
    return false;
}

uint8_t TESP8266::ipStatus() {
    String buf;
    rxClear();
    m_serial->println(F("AT+CIPSTATUS"));

    // STATUS:xxx 
    // 2: IPを取得 3: 接続済み 4: 切断済み (5:WiFi接続に失敗)
    checkATResponse(&buf, F("S:"));
    uint32_t index = buf.indexOf(F(":"));
    return buf.substring(index + 1, index + 2).toInt();
}

bool TESP8266::connectTcp(const String& host, uint32_t port) {
    if(connectedTcp()) disconnectTcp();
    String buf;
    uint8_t retry = 10;
    do {
        rxClear();
        m_serial->print(F("AT+CIPSTART=\"TCP\",\""));
        m_serial->print(host);
        m_serial->print(F("\","));
        m_serial->println(port);
        checkATResponse(&buf);
        if(buf.indexOf(F("OK")) != -1 || buf.indexOf(F("ALREADY")) != -1) {
            return true;
        }
        delay(100);
    } while(retry--);
    return false;
}

bool TESP8266::disconnectTcp() {
    rxClear();
    m_serial->println(F("AT+CIPCLOSE"));    
    return checkATResponse();
}

bool TESP8266::connectedTcp() {
  uint8_t status = ipStatus();

  if(status == 3){
      return true;
  }else{
      return false;
  }
  
  // 元のコード(接続のID[0～4]があるのでマルチ接続用)  
  //  uint8_t retry = 5;
  //  do {
  //      if(ipStatus() == 3) return true;
  //     delay(100);
  //  } while(--retry);
  //  return false;
}

String TESP8266::sendRequest(const String& method,
                                        const String& host, uint32_t port, const String& path, uint32_t& filesize,
                                        const String& contentType, const String& body) {
    // Create TCP connection
    connectTcp(host, port);

    // HTTP Request parts
    const uint8_t nGetRequest = 3;
    String getRequest[] = {
        F("GET "),
        F(" HTTP/1.1\r\nHost: "),
        F("\r\nUser-Agent: Arduino ESP8266\r\nConnection: close\r\n\r\n"),
    };
    const uint8_t nPostRequest = 6;
    String postRequest[] = {
        F("POST "),
        F(" HTTP/1.1\r\nHost: "),
        F("\r\nContent-Type: "),
        "\r\nContent-Length: " + String(body.length()),
        F("\r\nUser-Agent: Arduino ESP8266\r\nConnection: close\r\n\r\n"),
        F("\r\n"),
    };
    uint32_t len = path.length() + host.length() + contentType.length() + body.length();
    if(method == F("GET")) for(uint8_t i = 0; i < nGetRequest; ++i) len += getRequest[i].length();
    else for(uint8_t i = 0; i < nPostRequest; ++i) len += postRequest[i].length();

    // prepare to send the request data
    uint8_t retry = 15;
    do {
        String buf;
        rxClear();
        m_serial->print(F("AT+CIPSEND="));
        m_serial->println(len);
        if(checkATResponse(&buf, F("> "))) break;
        if(!(--retry)) {
            return "TIMEOUT ERROR";
        }
    } while(true);

    // send data
    uint32_t sentLen = 0;
    for(uint8_t i = 0; (method == F("GET") && i < nGetRequest) || (method == F("POST") && i < nPostRequest); ++i) {
        for(uint32_t j = 0; (method == F("GET") && j < getRequest[i].length()) || (method == F("POST") && j < postRequest[i].length()); ++j) {
            if(++sentLen % 64 == 0) delay(20); // Some Arduino's default serial buffer size is 64 bytes (Wait for it to be empty again.)
            m_serial->write(method == F("GET") ? getRequest[i][j] : postRequest[i][j]);
        }
        for(uint32_t j = 0; i == 0 && j < path.length(); ++j) {
            if(++sentLen % 64 == 0) delay(20);
            m_serial->write(path[j]);
        }
        for(uint32_t j = 0; i == 1 && j < host.length(); ++j) {
            if(++sentLen % 64 == 0) delay(20);
            m_serial->write(host[j]);
        }
        for(uint32_t j = 0; method == F("POST") && i == 2 && j < contentType.length(); ++j) {
            if(++sentLen % 64 == 0) delay(20);
            m_serial->write(contentType[j]);
        }
        for(uint32_t j = 0; method == F("POST") && i == 4 && j < body.length(); ++j) {
            if(++sentLen % 64 == 0) delay(20);
            m_serial->write(body[j]);
        }
    }

    // Start to buffer serial data fast!! to avoid serial buffer overflow.    
    const unsigned long start = millis();
 
    String raw = "";    
    bool file_get_flg = false;
    bool contenttype_flg = false;
    filesize = 0;
    while (millis() - start < 2000) {
        String line="";

        if(m_serial->available() > 0) {
            
            // ファイルの取得
            if(file_get_flg){
                raw = m_serial->readStringUntil("*CLOSED");                
                raw.replace("CLOSED\r\n","");                
                return raw; 
            }else{
                // シリアルバッファから一行を読み込む
                line = m_serial->readStringUntil('\r\n');
                line.toUpperCase();
            }
                        
            // ヘッダの終端
            if(line.length() == 1 && contenttype_flg){
                file_get_flg = true;
            }
            
            // ファイルサイズの取得
            // ※サーバーによってヘッダに「Content-Length」がない場合があるので注意
            if(line.indexOf("CONTENT-LENGTH:") != -1){             
                line.replace("CONTENT-LENGTH:","");
                line.trim();
                filesize = line.toInt(); 
            }
            
            // コンテントタイプの確認
            if(line.indexOf("CONTENT-TYPE:") != -1){             
                contenttype_flg = true;
            }
            
            // デバッグ用 
            // Serial.print(String(line.length()) + " - " + line);   
        }
    }
    return "FILE READ ERROR";
}

// ----------------------------------------------------------------------------
//  Public
// ----------------------------------------------------------------------------
TESP8266::TESP8266(HardwareSerial &serial) :
    m_serial(&serial){
}

TESP8266::~TESP8266() {
    disconnectAP();
}

bool TESP8266::statusAT(bool version) {
    rxClear();
    m_serial->println(F("AT"));
    bool result = checkATResponse();
    
    if(version && result){
      
      // バージョンの表示
      String buf;
      rxClear();
      m_serial->println(F("AT+GMR"));
      checkATResponse(&buf); 
      
      // ゴミの削除 
      buf.replace("AT+GMR","");  
      buf.replace("\r\r\n","");        
      buf.replace("\r\nOK","");              
      buf.setCharAt(0,'*');
      buf.replace("*",""); 
      m_serial->println(buf);
    }
        
    return result; 
}

bool TESP8266::statusWiFi() {
  uint8_t status = ipStatus();

  if(!(status == 4  || status == 5)){
      return true;
  }else{
      return false;
  }
    
  // 元のコード(接続のID[0～4]があるのでマルチ接続用)
  //  uint8_t checkCnt = 5;
  //  do {
  //      if(ipStatus() == 5) return false;
  //      delay(100);
  //  } while(--checkCnt);
  //  return true;
  //
}

bool TESP8266::connectAP(const String& ssid, const String& password, String ip,String gateway,String netmask) {
    rxClear();
    m_serial->println(F("AT+CWMODE_DEF=1")); // 1: station(client/子機) mode, 2: softAP(server/親機) mode, 3: 1+2
    if(!(checkATResponse() && restart())) return false; // change "DEF"ault cwMode and restart
    
    // 電源投入時の自動接続の有効/無効
    // 0:無効 1:有効(デフォルト)
    //m_serial->println(F("AT+CWAUTOCONN=0")); 
    //if(!checkATResponse()) return false;
    
    // DHCPの有効/無効 
    // 第一引数  0: softAP(server/親機) 1: station(client/子機) 2: 0+1
    // 第二引数  0: DHCP無効 1: DHCP有効
    if(ip == ""){
      m_serial->println(F("AT+CWDHCP_CUR=1,1")); 
    }else{
      m_serial->println(F("AT+CWDHCP_CUR=1,0")); 
    }
    rxClear();
    if(!checkATResponse()) return false;
    
    // IP関連の指定
    // ip,gateway,netmask 
    if(ip != ""){
      rxClear();
      m_serial->println("AT+CIPSTA_CUR=\"" + ip +"\",\""+ gateway +"\",\"" + netmask + "\""); 
      if(!checkATResponse()) return false;
    }
    
    uint8_t retry = 5;
    do {
        // Connect to an AP
        rxClear();
        delay(500);
        m_serial->print(F("AT+CWJAP_DEF=\""));
        m_serial->print(ssid);
        m_serial->print(F("\",\""));
        m_serial->print(password);
        m_serial->println(F("\""));
        
        if(checkATResponse(F("OK"), 10000)){
            // IP情報を取得する
            rxClear();
            m_serial->println(F("AT+CIPSTA_CUR?")); 
            String buf;
            checkATResponse(&buf);   
            
            // ゴミの削除 
            buf.replace("AT+CIPSTA_CUR?\r\r\n","");  
            buf.replace("\0","");  
            buf.replace("\r\n\r\nOK","");              
            buf.setCharAt(0,'*');
            buf.setCharAt(1,'*');
            buf.replace("*","");  
            
            // ip,gateway,netmask 
            m_serial->println(buf);       
            return true;
        }
        
    } while(--retry);
    return false;
}

bool TESP8266::disconnectAP() {
    // 本来はこの「ATコマンド」で切断できるはずですが、ごく稀に切断できない場合があります。
    // その場合は苦肉の策のlastResort()メソッドを使用してください。
    rxClear();
    m_serial->println(F("AT+CWQAP"));
    return checkATResponse();
}

// Wifi接続の強制切断用(苦肉の策)
void TESP8266::lastResort(){
    
    // 適当なSSID、パスワード、IPアドレスなどで接続を試みます。
    // これによりWifi接続が切断されます。アドレスが被ったら適宜、変更してください。
    m_serial->print(F("AT+CWJAP_DEF=\""));
    m_serial->print("ssid");
    m_serial->print(F("\",\""));
    m_serial->print("password");
    m_serial->println(F("\""));
    checkATResponse();
    
    m_serial->println(F("AT+CWDHCP_CUR=1,0")); 
    checkATResponse();

    m_serial->println("AT+CIPSTA_CUR=\"255.255.255.0\",\"255.255.254.0\",\"255.255.253.0\""); 
    checkATResponse();
}

// [GET版]URLにアクセスしてファイルを取得する
// ---
// host      : ホスト(例: www.example.com)
// path      : パス(例: /)
// filesize  : ファイルサイズ(戻り値) 
//             ※「Content-Length」が出力されないサーバーは常に0です。
// port      : ポート(省略可能した場合は80となる)   
// ---
// 戻り値    : [成功]ファイルのRAWデータ [エラー] "TIMEOUT ERROR" or "FILE READ ERROR"
// ---
// 備考      : 64byte迄の「テキストファイル」「CSVファイル」などを想定しています。
//             ※大きいファイルはデータの破損が発生して文字化けする場合があります。
//             ※SSL/TLSはサポートしていません。
String TESP8266::get(const String& host, const String& path, uint32_t& filesize, uint32_t port) {
    return sendRequest(F("GET"), host, port, path, filesize);
}

// [POST版]URLにアクセスしてファイルを取得する
// ---
// host      : ホスト(例: www.example.com)
// path      : パス(例: /)
// filesize  : ファイルサイズ(戻り値) 
//             ※「Content-Length」が出力されないサーバーは常に0です。
// port      : ポート(省略可能した場合は80となる)   
// ---
// 戻り値    : [成功]ファイルのRAWデータ [エラー] "TIMEOUT ERROR" or "FILE READ ERROR"
// ---
// 備考      : 64byte迄の「テキストファイル」「CSVファイル」などを想定しています。
//             ※大きいファイルはデータの破損が発生して文字化けする場合があります。
//             ※SSL/TLSはサポートしていません。
String TESP8266::post(const String& host, const String& path, const String& body,
                      uint32_t& filesize, const String& contentType, uint32_t port) {
    return sendRequest(F("POST"), host, port, path, filesize, contentType, body);
}

