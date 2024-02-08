/* Libraries */
#include "DHT.h"
#include <LiquidCrystal.h>
#include <FirebaseESP32.h>
#include "time.h"
#include <WiFi.h>
/* ********************* */
/* Macros */
#define WIFI_SSID "ESP32"
#define WIFI_PASSWORD "12345678"
#define dhtpin 13
#define DATABASE_URL "esp32-8010b-default-rtdb.firebaseio.com"
#define LED1 2
#define LED2 4
#define BUZ 15
/* ********************* */
/* Instances*/
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);
DHT dht(dhtpin, DHT11);
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;
/* ********************* */
/* Global Variables*/
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;
bool state = false;
struct tm timeinfo;
/* ********************* */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  /* Pin Modes */
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZ, OUTPUT);
  /* ******************** */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print("Connecting");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;
  Firebase.reconnectWiFi(true);  
  Firebase.begin(&config, &auth);
  //checking firebase
  while(!Firebase.ready())
  {
    Serial.print("Please wait\n");
  }
  dht.begin();
  lcd.begin(16,2);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  lcd.setCursor(0, 0);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float hic = dht.computeHeatIndex(t, h, f);
  
  Firebase.setFloat(firebaseData,"/humidity", h);
  Firebase.setFloat(firebaseData,"/temp", t);
  Firebase.setFloat(firebaseData,"/heat_index", hic);
  
  if (Firebase.RTDB.getBool(&firebaseData, "/state"))
  {
    state = firebaseData.to<bool>();
  }
  delay(500);
  while (t > 40)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature exceed 40");
    digitalWrite(BUZ, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
  }
  digitalWrite(BUZ, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  if (!state)
  {
    lcd.clear();
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.print(" %");
    Serial.print("\nTemperature = ");
    Serial.print(t);
    Serial.print("\n");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print("%");
  }
  else
  {
    if(!getLocalTime(&timeinfo))
    {
      lcd.println("Failed to obtain time");
      return;
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println(&timeinfo, "%A,%B %d* *");
    lcd.setCursor(0,1);
    lcd.println(&timeinfo, "%Y %H:%M:%S - ");
    Serial.println(&timeinfo, "%A,%B %d* *");
    Serial.println(&timeinfo, "%Y %H:%M:%S ^ ");
  }
}
