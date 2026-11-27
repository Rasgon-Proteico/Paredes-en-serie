#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>
int m;
// A4 y A5 ocupados por I2C 
const uint8_t SENSOR_PINS[3] = {A0, A1, A2};
const int NUM_ANALOG_SENSORES = 5;
const float V_REF_MV = 5000.0f; 

Adafruit_BME280 bme; 
bool bme_conectado = false;

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,   U8X8_PIN_NONE);

float readTempC(uint8_t pin, int &out_raw, float &out_mv) {
  const int N = 20;
  long sum_raw = 0;

  analogRead(pin);
  delayMicroseconds(200);

  for (int i = 0; i < N; i++) {
    sum_raw += analogRead(pin);
    delay(4);
  }

  float avg_raw = (float)sum_raw / N;
  out_raw = (int)(avg_raw + 0.5f);

  out_mv = (avg_raw * V_REF_MV) / 1024.0f;

  // Curva TMP36: 0°C = 500mV, 10mV/°C
  float tempC = (out_mv - 500.0f) / 10.0f;

  return tempC;
}

void setup() {
  Serial.begin(9600);
  delay(300);

  Serial.println("=========================================");
  Serial.println(" Sistema: 5 TMP + 1 BME280 + OLED SH1106 ");
  Serial.println("=========================================");

  // Iniciar bus 
  Wire.begin();
  u8g2.begin();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(15, 25, "Iniciando...");
    u8g2.drawStr(5, 48, "Monitoreo Temp");
  } while (u8g2.nextPage());

  // Iniciar sensor BME280
  if (bme.begin(0x76)) {
    bme_conectado = true;
    Serial.println("BME280 detectado en direccion 0x76");
  } else if (bme.begin(0x77)) {
    bme_conectado = true;
    Serial.println("BME280 detectado en direccion 0x77");
  } else {
    Serial.println("¡ADVERTENCIA: No se detectó el sensor BME280!");
  }

  delay(1000);
}

void loop() {
  float temps_analog[5];
  int raws[5];
  float mvs[5];

  for (int i = 0; i < NUM_ANALOG_SENSORES; i++) {
    temps_analog[i] = readTempC(SENSOR_PINS[i], raws[i], mvs[i]);
  }

  float bme_temp = 0.0f;
  float bme_hum  = 0.0f;
  float bme_pres = 0.0f;
  if (bme_conectado) {
    bme_temp = bme.readTemperature();
    bme_hum  = bme.readHumidity();
    bme_pres = bme.readPressure() / 100.0F; // hPa
  }

  Serial.println("--- Lecturas ---");
  Serial.println(m++);
  for (int i = 0; i < NUM_ANALOG_SENSORES; i++) {
    Serial.print("Sensor "); Serial.print(i + 1);
    if (i == 4) {
      Serial.print(" (A6) -> ");
    } else {
      Serial.print(" (A"); Serial.print(i); Serial.print(") -> ");
    }
    Serial.print("Raw: "); Serial.print(raws[i]);
    Serial.print(" | mV: "); Serial.print(mvs[i], 1);
    Serial.print(" | Temp: "); Serial.print(temps_analog[i], 2);
    Serial.println(" °C");
  }

  Serial.print("Sensor 6 (BME280) -> ");
  if (bme_conectado) {
    Serial.print("Temp: "); Serial.print(bme_temp, 2); Serial.print(" °C | ");
    Serial.print("Hum: "); Serial.print(bme_hum, 1); Serial.print(" % | ");
    Serial.print("Pres: "); Serial.print(bme_pres, 1); Serial.println(" hPa");
  } else {
    Serial.println("Error de lectura BME280");
  }
  Serial.println();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);

    u8g2.setCursor(0, 9);
    u8g2.print("S1:"); u8g2.print(temps_analog[0]*.56, 1); u8g2.print("C");
    u8g2.setCursor(65, 9);
    u8g2.print("S2:"); u8g2.print(temps_analog[1], 1); u8g2.print("C");

    u8g2.setCursor(0, 21);
    u8g2.print("S3:"); u8g2.print(temps_analog[2]/12, 1); u8g2.print("C");
    
    u8g2.drawHLine(0, 37, 128);

    u8g2.setCursor(0, 49);
    if (bme_conectado) {
      u8g2.print("BME: "); u8g2.print(bme_temp, 1); u8g2.print("C  ");
      u8g2.print(bme_hum, 0); u8g2.print("%");

      u8g2.setCursor(0, 61);
      u8g2.print("P: "); u8g2.print(bme_pres, 1); u8g2.print(" hPa");
    } else {
      u8g2.print("BME: Desconectado");
    }

  } while (u8g2.nextPage());

  delay(1000);
}
