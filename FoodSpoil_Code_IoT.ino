#define BLYNK_TEMPLATE_ID "TMPL6yZsq3MWN"
#define BLYNK_TEMPLATE_NAME "esp32"
#define BLYNK_AUTH_TOKEN "Dzp-i1Ef5FurNHz9rWUzCoSsfx7yTD3P"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Sm freeWIFI";
char pass[] = "152439298m";

// Sensor Pins
#define MQ4_PIN     34
#define MQ135_PIN   35
#define DHT_PIN     15
#define RED_LED     26
#define GREEN_LED   27

// DHT Sensor
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// LCD via I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Thresholds
const int MQ4_THRESHOLD = 500;
const int MQ135_THRESHOLD = 500;

BlynkTimer timer;
bool spoilageAlertSent = false;
bool dhtAlertSent = false;

void sendToBlynk() {
  // Read Sensors
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int mq4Value = analogRead(MQ4_PIN);
  int mq135Value = analogRead(MQ135_PIN);

  // Send to Blynk
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humid);
  Blynk.virtualWrite(V2, mq4Value);
  Blynk.virtualWrite(V3, mq135Value);

  // Gas Spoilage Check
  if (mq4Value > MQ4_THRESHOLD || mq135Value > MQ135_THRESHOLD) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Blynk.virtualWrite(V4, "⚠️ Spoiled");

    if (!spoilageAlertSent) {
      Blynk.logEvent("food_spoilage_detected", "⚠️ Food spoilage detected!");
      spoilageAlertSent = true;
    }

    Serial.println("⚠️ Food Spoilage Detected!");
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Blynk.virtualWrite(V4, "✅ Fresh");

    if (spoilageAlertSent) {
      spoilageAlertSent = false;
    }

    Serial.println("✅ Food is Fresh.");
  }

  // DHT Alert Check
  if ((temp > 40 || humid > 80) && !dhtAlertSent) {
    Blynk.logEvent("dht11", "The surrounding can spoil your food");
    dhtAlertSent = true;
  }

  if (temp <= 30 && humid <= 80) {
    dhtAlertSent = false;
  }

  // LCD Output
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C H:");
  lcd.print(humid, 0);
  lcd.print("%  ");

  lcd.setCursor(0, 1);
  lcd.print("G1:");
  lcd.print(mq4Value);


  lcd.print(" G2:");
  lcd.print(mq135Value);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print(" Spoil Alert!");
  lcd.setCursor(0, 1);
  lcd.print(" Connecting...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();
  timer.setInterval(2000L, sendToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}

