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
const char* writeKey = WRITE_KEY;
int channelId = CHANNEL_ID;

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
  ambient.begin(channelId, writeKey, &client); 
}

void loop() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  
  M5.Lcd.setCursor(0, 30);
  // センサーデータを出力
  M5.Lcd.print("Temperature = ");
  M5.Lcd.print(temperature);
  M5.Lcd.println(" *C");
  
  M5.Lcd.print("Humidity = ");
  M5.Lcd.print(humidity);
  M5.Lcd.println(" %");

  M5.Lcd.print("Pressure = ");
  M5.Lcd.print(pressure);
  M5.Lcd.println(" hPa");

  // Ambientにデータを送信 
  ambient.set(1, temperature); // 温度をデータ1にセット
  ambient.set(2, humidity); // 湿度をデータ2にセット
  ambient.set(3, pressure); // 気圧をデータ3にセット
  ambient.send(); // データをAmbientに送信
  delay(60000);
}
