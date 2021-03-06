// ------------------------------------
//  URLにアクセスしてファイルを取得する
// ------------------------------------
#include <TESP8266.h>

// Wifi
const char* ssid = "xxx";
const char* password = "xxx";

// URL http://www.example.com/iot/file.txt
const char* host = "www.example.com";
const char* path = "/iot/file.txt";

// シリアルの種類 (HardwareSerial or SoftwareSerial)
// HardwareSerialとSoftwareSerialを切り替える際には
// ESP-WROOM-02をリセットする必要があります。
// ※リセットはUSB電源(給電)を10秒ぐらいはずせばOKです。

  // HardwareSerial
  TESP8266 httpClient(Serial);  
  
  // SoftwareSerial (RX受信ピン番号,TX送信ピン番号)
  //TESP8266 httpClient(2,3); 

void setup() {      
    // シリアルの通信速度(115.2kbps)
    Serial.begin(115200);

    // ESP-WROOM-02との接続確認(ATコマンドのテスト)
    while(true) {
        if(httpClient.statusAT(true)) { Serial.println("*** ESP-WROOM-02と接続しました。"); break; }
        else Serial.println("*** ESP-WROOM-02と接続できません。");
        delay(1000);
    }

    // アクセスポイントに接続(DHCP)
    while(true) { 
        if(httpClient.connectAP(ssid, password)) { Serial.println("*** アクセスポイントに接続しました。"); break; }
        else Serial.println("*** アクセスポイントに接続できませんでした。 再試行中...");
        delay(1000);
    }
 
    // Wifi接続の確認
    while(true) {
        if(httpClient.statusWiFi()) { Serial.println("*** Wifi接続しました。"); break; }
        else Serial.println("*** Wifi接続できません。");
        delay(1000);
    }

    // HTTP GETリクエストの実行
    uint32_t filesize = 0; // (戻り値)ファイルサイズ 
    String raw  = httpClient.get(host, path, filesize);

    Serial.println("---"); 
        
    // (HTTPヘッダで定義された)ファイルサイズ
    // ※サーバーによってHTTPヘッダに「Content-Length」がない場合は常に0となります。
    // ※その場合はPHPなどのサーバーサイドで「Content-Length:」でサイズを出力すればOKです。
    Serial.println(String(filesize) + " byte");
    // (取得済みの)ファイルサイズ
    Serial.println(String(raw.length()) + " byte");
   
    // ファイルの確認
    if(filesize != raw.length()){
        Serial.println("ファイルが破損しています。");
    }

    // ファイルの出力
    Serial.println("---"); 
    Serial.println(raw);       
    Serial.println("---"); 
    
    // Wifi接続を切断
    // ※このメソッドはTESP8266クラスのデストラクタでも自動的に実行されます。
    // ※但し、TESP8266クラスをグローバル変数にしている場合は、デストラクタは実行されません。
    httpClient.disconnectAP();
}

void loop() {
    // none
}
