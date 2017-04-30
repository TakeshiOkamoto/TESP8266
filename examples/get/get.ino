// ------------------------------------
//  HTTP GET
// ------------------------------------
#include <TESP8266.h>

// Wifi
const char* ssid = "xxx";
const char* password = "xxx";

// URL http://www.example.com/iot/get.php?value=1&string=ABCDE
// GET value=1&string=ABCDE
const char* host = "www.example.com";
const char* path = "/iot/get.php?value=1&string=ABCDE";

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
    
    if(raw == "OK") {
        Serial.println("*** GETの成功");
    }else {
        // エラー
        Serial.println("*** エラー：" + raw);
    }         

    // Wifi接続を切断
    // ※このメソッドはTESP8266クラスのデストラクタでも自動的に実行されます。
    // ※但し、TESP8266クラスをグローバル変数にしている場合は、デストラクタは実行されません。
    httpClient.disconnectAP();
}

void loop() {
    // none
}
