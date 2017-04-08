// ------------------------------------
//  DHCPでWifi接続 
// ------------------------------------
#include <TESP8266.h>

// Wifi
String SSID = "xxx";
String PASSWORD = "xxx";

// グローバル変数
TESP8266 httpClient(Serial);   

void setup() {      
    // HardwareSerialの通信速度(115.2kbps)
    Serial.begin(115200);

    // ESP-WROOM-02との接続確認(ATコマンドのテスト)
    while(true) {
        if(httpClient.statusAT(true)) { Serial.println("*** ESP-WROOM-02と接続しました。"); break; }
        else Serial.println("*** ESP-WROOM-02と接続できません。");
        delay(1000);
    }

    // アクセスポイントに接続(DHCP)
    while(true) { 
        if(httpClient.connectAP(SSID, PASSWORD)) { Serial.println("*** アクセスポイントに接続しました。"); break; }
        else Serial.println("*** アクセスポイントに接続できませんでした。 再試行中...");
        delay(1000);
    }
 
    // Wifi接続の確認
    while(true) {
        if(httpClient.statusWiFi()) { Serial.println("*** Wifi接続しました。"); break; }
        else Serial.println("*** Wifi接続できません。");
        delay(1000);
    }
    
    // Wifi接続を切断
    // ※このメソッドはTESP8266クラスのデストラクタでも自動的に実行されます。
    // ※但し、TESP8266クラスをグローバル変数にしている場合は、デストラクタは実行されません。
    // httpClient.disconnectAP();
}

void loop() {
    // none
}
