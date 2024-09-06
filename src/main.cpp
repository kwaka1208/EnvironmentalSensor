#include <Arduino.h>
#include <M5Stack.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "env.h"
#include "Ambient.h"

Adafruit_BME280 bme; // BME280オブジェクトを作成
Ambient ambient;
WiFiClient client;
const char* ssid = SSID;
const char* password = PASS;
long pastTime;
bool display;

#define INTERVAL_SEC 60 // 送信間隔(秒)

/**
 * 環境データ
 */
struct envData {
  float temperature;
  float humidity;
  float pressure;
};

void getEnv(envData *env);
void showEnv(envData env);
void pushEnv(envData env);

/**
 * セットアップ
 */
void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  // WiFi接続待ち
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.print("\r\nWiFi connected\r\nIP address: ");
  M5.Lcd.println(WiFi.localIP());

  Wire.begin(21, 22); // SDA, SCLのピンを指定
  if (!bme.begin(0x76)) {  // BME280のI2Cアドレスが0x76の場合
    Serial.println("BME280の初期化に失敗しました");
    while (1);
  }
  ambient.begin(CHANNEL_ID, WRITE_KEY, &client); 
  pastTime = millis();
  display = true;
  envData env;
  getEnv(&env);
  pushEnv(env);
}

/**
 * メインループ
*/
void loop() {
  envData env;
  M5.update();
  getEnv(&env);
  if (M5.BtnA.wasReleasefor(50)) {
    // 50ms押したら切り替える。これぐらいの方が誤操作が少ない。
    display = !display;
  }
  if (display) {
    showEnv(env);
  } else {
    M5.Lcd.clear();
  }
  long now = millis();
  if (now > pastTime + INTERVAL_SEC * 1000) {
    pushEnv(env);
    pastTime = now;
  }
}

void getEnv(envData *env) {
  env->temperature = bme.readTemperature();
  env->humidity = bme.readHumidity();
  env->pressure = bme.readPressure() / 100.0F;
}

/**
 * センサーデータを表示
 */
void showEnv(envData env) {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Press A to switch display");
  // センサーデータを出力
  M5.Lcd.print("Temperature = ");
  M5.Lcd.print(env.temperature);
  M5.Lcd.println(" *C");
      
  M5.Lcd.print("Humidity = ");
  M5.Lcd.print(env.humidity);
  M5.Lcd.println(" %");

  M5.Lcd.print("Pressure = ");
  M5.Lcd.print(env.pressure);
  M5.Lcd.println(" hPa");
}

/**
 * センサーデータをAmbientに送信
 */
void pushEnv(envData env) {
  ambient.set(1, env.temperature); // 温度をデータ1にセット
  ambient.set(2, env.humidity); // 湿度をデータ2にセット
  ambient.set(3, env.pressure); // 気圧をデータ3にセット
  ambient.send(); // データをAmbientに送信
}
