#define BLYNK_TEMPLATE_ID "TMPL6CoHy1AQr"
#define BLYNK_TEMPLATE_NAME "smart plant"
#define BLYNK_AUTH_TOKEN  "CHnve85cV4FIJEj9gv5qY53x4TfL2mi0"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== KONFIGURASI WIFI =====
char ssid[] = "Ardi Setengah Matang";
char pass[] = "1029384756";

// ===== PIN SETUP =====
const int SENSOR_PIN = A0;
const int RELAY_PIN  = D6;
const int LED_PIN    = LED_BUILTIN;

// ===== VARIABEL =====
int threshold    = 600;
int pumpDuration = 10000;
int minInterval  = 300000;

unsigned long lastPump = 0;
bool pumpActive = false;
unsigned long pumpStart = 0;
bool autoMode   = true;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Smart Watering");                                    j32322
  lcd.setCursor(0,1);
  lcd.print("By Aisyah Team");

  // ===== KONEKSI KE WIFI =====
  WiFi.begin(ssid, pass);

  // Tunggu koneksi max 10 detik
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WiFi Tidak Ada");
    lcd.setCursor(0,1);
    lcd.print("Cek koneksi WiFi");
  }
}

void loop() {
  Blynk.run();

  int kelembapan = analogRead(SENSOR_PIN);
  float tegangan = (kelembapan / 1024.0) * 3.3;

  // Update LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Moist:" + String(kelembapan));
  lcd.setCursor(0,1);
  lcd.print(autoMode ? "AUTO" : "MANUAL");

  // Kirim ke Blynk
  Blynk.virtualWrite(V1, kelembapan);
  Blynk.virtualWrite(V2, autoMode ? "AUTO" : "MANUAL");
  Blynk.virtualWrite(V3, tegangan);

  // Non-blocking pompa
  if (pumpActive && millis() - pumpStart >= pumpDuration) {
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    pumpActive = false;
  }

  // Logika otomatis
  if (autoMode) {
    bool kering = kelembapan > threshold;
    if (kering && !pumpActive && (millis() - lastPump > minInterval)) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      pumpActive = true;
      pumpStart = millis();
      lastPump = millis();
    }
  }
}

// ===== CALLBACKS BLYNK =====
BLYNK_WRITE(V4) {
  int state = param.asInt();
  if (!autoMode) {
    if (state == 1) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LOW);  
      pumpActive = true;
      pumpStart = millis();
      lastPump = millis();
    } else {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      pumpActive = false;
    }
  }
}

BLYNK_WRITE(V5) { autoMode = param.asInt(); }
BLYNK_WRITE(V6) { threshold = param.asInt(); }
BLYNK_WRITE(V7) { pumpDuration = param.asInt(); }
BLYNK_WRITE(V8) { minInterval = param.asInt(); }
