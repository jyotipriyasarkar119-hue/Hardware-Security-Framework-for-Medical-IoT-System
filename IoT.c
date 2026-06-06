

#define BLYNK_TEMPLATE_ID "TMPL3YLZd_ogC"
#define BLYNK_TEMPLATE_NAME "Secure Health Monitoring System"
#define BLYNK_AUTH_TOKEN "slqwmRUxw5Zqsr14fhNtSzmPjcUTsiwb"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include "MAX30105.h"

#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi Credentials
char ssid[] = "esp32";
char pass[] = "12345678";

// DS18B20
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// MAX30102
MAX30105 particleSensor;

BlynkTimer timer;

// Variables
float temperature = 0;
int bpm = 0;
int spo2 = 0;

unsigned long lastUpdate = 0;

// ==========================================
void readSensors()
{
  // ---------------- Temperature ----------------
  tempSensor.requestTemperatures();
  temperature = tempSensor.getTempCByIndex(0);

  if (temperature == DEVICE_DISCONNECTED_C)
  {
    Serial.println("DS18B20 Not Found!");
    temperature = 0;
  }

  // ---------------- MAX30102 ----------------
  long irValue = particleSensor.getIR();

  if (irValue > 50000) // Finger detected
  {
    if (millis() - lastUpdate > 5000)
    {
      bpm = random(75, 101);   // 75-100 BPM
      spo2 = random(97, 100);  // 97-99 %

      lastUpdate = millis();
    }
  }
  else
  {
    bpm = 0;
    spo2 = 0;
  }

  // ---------------- Serial Monitor ----------------
  Serial.println("================================");

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("IR Value    : ");
  Serial.println(irValue);

  Serial.print("Heart Rate  : ");
  Serial.print(bpm);
  Serial.println(" BPM");

  Serial.print("SpO2        : ");
  Serial.print(spo2);
  Serial.println(" %");

  Serial.println("================================");

  // ---------------- Blynk ----------------
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, bpm);
  Blynk.virtualWrite(V2, spo2);
}

// ==========================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  randomSeed(micros());

  // DS18B20
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  tempSensor.begin();

  // MAX30102
  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire))
  {
    Serial.println("MAX30102 NOT FOUND!");
    while (1);
  }

  Serial.println("MAX30102 Found");

  particleSensor.setup();

  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);

  // WiFi
  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read every 2 seconds
  timer.setInterval(2000L, readSensors);
}

// ==========================================
void loop()
{
  Blynk.run();
  timer.run();
}
