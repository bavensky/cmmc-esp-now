#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "data_type.h"
#include <Wire.h>
// #include "SSD1306.h"

CMMC_PACKET_T pArr[60];
int pArrIdx = 0;
esp_now_peer_info_t slave;
uint8_t currentSleepTimeMinuteByte = 5;
uint32_t msAfterESPNowRecv = millis();
uint32_t counter = 0;
bool dirty = false;

// // OLED pins to ESP32 GPIOs via this connecthin:
// // OLED_SDA -- GPIO4
// // OLED_SCL -- GPIO15
// // OLED_RST -- GPIO16
// #define OLED_SDA 4
// #define OLED_SCL 15
// #define OLED_RST 16
// SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

#define DEBUG_PACKET true
#define LED 13

void setup()
{
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // pinMode(OLED_RST, OUTPUT);
  // digitalWrite(OLED_RST, LOW); // set GPIO16 low to reset OLED
  // delay(50);
  // digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO16 in high

  // display.init();
  // display.flipScreenVertically();
  // display.setContrast(255);
  // display.setFont(ArialMT_Plain_10);

  WiFi.mode(WIFI_AP_STA);
  Serial.printf("STA MAC: %s\r\n", WiFi.macAddress().c_str());
  Serial.printf(" AP MAC: %s\r\n", WiFi.softAPmacAddress().c_str());
  delay(100);

  WiFi.disconnect();

  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESPNow Init Success\n");
  }
  else
  {
    Serial.println("ESPNow Init Failed\n");
    ESP.restart();
  }

  esp_now_register_send_cb([&](const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.println("rev data...");
  });

  esp_now_register_recv_cb([&](const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    int pArrIdxCurr = pArrIdx;
    pArrIdx = (pArrIdx + 1) % 30;
    printf("=====================\n");
    printf("RECV ESPNow Data pArrIdx=%d\n", pArrIdx);
    memcpy(&slave.peer_addr, mac_addr, 6);
    CMMC_PACKET_T wrapped;
    CMMC_SENSOR_DATA_T packet;

    memcpy(&packet, data, sizeof(packet));
    memcpy(&wrapped.data, &packet, sizeof(packet));
    wrapped.sleepTime = (uint32_t)currentSleepTimeMinuteByte;
    // packet.field9 = counter;
    wrapped.ms = millis();
    wrapped.sum = counter;

    pArr[pArrIdxCurr] = wrapped;

    digitalWrite(LED, !digitalRead(LED));
    delay(200);
    digitalWrite(LED, !digitalRead(LED));

    // printf("\nhave data: %lu", packet.field1);

    esp_now_send(mac_addr, &currentSleepTimeMinuteByte, 1);
    printf("sending back sleepTime=%lu", currentSleepTimeMinuteByte);
    printf("\n=====================\n");

#if (DEBUG_PACKET)
    printf("+++++ PACKET for %d +++++\n", pArrIdxCurr);
    for (int i = 0; i < sizeof(wrapped); i++)
    {
      printf("%02x", ((uint8_t *)&wrapped)[i]);
    }
    printf("\n+++++++++++++++++++++++\n", pArrIdxCurr);
#endif
    esp_err_t addStatus = esp_now_add_peer(&slave);
    if (addStatus == ESP_OK)
    {
      printf("\n=====================");
      printf("\nADD PEER status=0x%02x OK", ESP_OK);
      printf("\n=====================\n");
    }
    else
    {
      printf("\n=====================");
      printf("\nADD PEER status=0x%02x", addStatus - ESP_ERR_ESPNOW_BASE);
      printf("\n=====================\n");
    }

    uint8_t time = 1;
    esp_err_t result = esp_now_send(mac_addr, &time, 1);
    // printf("\nresult:
    counter++;
    msAfterESPNowRecv = millis();
    dirty = true;
  });
}

void loop()
{
  // display.clear();
  // display.setTextAlignment(TEXT_ALIGN_LEFT);
  // display.setFont(ArialMT_Plain_16);
  // display.drawString(0, 0, "ESP NOW");
  // // display.drawString(50, 30, String(10));
  // display.display();
  delay(1);
}