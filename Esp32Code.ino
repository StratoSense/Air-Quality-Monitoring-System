#include <Arduino.h>

// ---------- Blynk Config ----------
#define BLYNK_TEMPLATE_ID "template_id_here"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor"
#define BLYNK_AUTH_TOKEN "auth_token_here"
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <DHT.h>
#include "PMS.h"
#include <HTTPClient.h>
#include <base64.h>
// ---------- Twilio Config ----------
const char* accountSid = "account_id_here";
const char* authToken = "auth_token_here";
const char* smsNumber = "from_number_here";
const char* toNumber = "to_number_here";
// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int display_cycle = 0;
unsigned long lastDisplayChange = 0;
const unsigned long displayInterval = 2500;
// ---------- DHT22 ----------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------- PMS5003 ----------
PMS pms(Serial2);
PMS::DATA data;
// ---------- MQ-7 ----------
#define MQ7_PIN 34
#define RL 10000.0
#define VCC 3.3
#define ADC_MAX 4095.0
#define R0_MQ7 7154603.0
const float m_mq7 = -0.77;
const float b_mq7 = 0.142;
const int NUM_SAMPLES_MQ7 = 5;
float mq7_ppm = 0.0;
// ---------- MQ-135 ----------
#define MQ135_PIN_NEW 35
#define VREF 3.3
#define ADC_RES 4095.0
#define V_CIRCUIT_SUPPLY 5.0
#define RL_KOHM 10.0
#define SAMPLES_MQ135 10
#define SMOOTH_ALPHA 0.12
#define MAX_PPM_CAP 50000.0
#define GAS_A 116.6020682
#define GAS_B -2.769034857
#define HARDCODED_R0_KOHM 1411.35
float baselineR0_MQ135 = HARDCODED_R0_KOHM;
float mq135_ppm = 0.0;
float ppm_smoothed_MQ135 = 0.0;
// ---------- Data ----------
float current_temp = 0.0;
float current_hum = 0.0;
int current_pm1 = 0, current_pm25 = 0, current_pm10 = 0;
int current_AQI = 0;
String current_AQI_category = "";
String mq7_status = "";
String mq135_status = "";
// ---------- Timing ----------

unsigned long lastSend = 0;
const unsigned long sendInterval = 5000;
unsigned long lastAlert = 0;
const unsigned long alertInterval = 600000;
// ---------- Warmup ----------
unsigned long warmupStartTime = 0;
const unsigned long WARMUP_DURATION = 10UL * 60UL * 1000UL; // 10 min
bool warmupDone = false;
// ---------- Thresholds ----------
int AQI_Threshold = 150;
const int MQ7_ALERT_THRESHOLD = 200;
const int MQ135_ALERT_THRESHOLD = 2000;
// ---------- Function Prototypes ----------
void sendSMS(String message);
void sendToBlynkBulk(float, float, int, int, int, int, String, float, float);
void readMQ7();
void readMQ135();
float readRs_MQ135();
float getGasPPM_MQ135(float);
int readAvgADC_MQ135();
void updateOLED();
// ---------- Blynk Input ----------
BLYNK_WRITE(V8) {
 AQI_Threshold = param.asInt();
 Serial.printf(" New AQI Threshold set to %d\n", AQI_Threshold);
}
// ---------- AQI ----------
int calculateAQI(float pm25, float pm10) {
 int aqi_pm25, aqi_pm10;
 if (pm25 <= 12) aqi_pm25 = map(pm25, 0, 12, 0, 50);
 else if (pm25 <= 35.4) aqi_pm25 = map(pm25, 13, 35, 51, 100);
 else if (pm25 <= 55.4) aqi_pm25 = map(pm25, 36, 55, 101, 150);
 else if (pm25 <= 150.4) aqi_pm25 = map(pm25, 56, 150, 151, 200);
 else if (pm25 <= 250.4) aqi_pm25 = map(pm25, 151, 250, 201, 300);
 else aqi_pm25 = 500;

 if (pm10 <= 54) aqi_pm10 = map(pm10, 0, 54, 0, 50);
 else if (pm10 <= 154) aqi_pm10 = map(pm10, 55, 154, 51, 100);
 else if (pm10 <= 254) aqi_pm10 = map(pm10, 155, 254, 101, 150);
 else if (pm10 <= 354) aqi_pm10 = map(pm10, 255, 354, 151, 200);
 else if (pm10 <= 424) aqi_pm10 = map(pm10, 355, 424, 201, 300);
 else aqi_pm10 = 500;
 return max(aqi_pm25, aqi_pm10);
}
String getAQICategory(int aqi) {
 if (aqi <= 50) return "Good";
 else if (aqi <= 100) return "Moderate";
 else if (aqi <= 150) return "Sensitive";
 else if (aqi <= 200) return "Unhealthy";
 else if (aqi <= 300) return "V. Unhealthy";
 else return "Hazardous";
}
// ---------- Twilio ----------
void sendSMS(String message) {
 if (WiFi.status() == WL_CONNECTED) {
 HTTPClient http;
 String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(accountSid) +
"/Messages.json";
 http.begin(url);
 String auth = base64::encode(String(accountSid) + ":" + String(authToken));
 auth.replace("\n", "");
 http.addHeader("Authorization", "Basic " + auth);
 http.addHeader("Content-Type", "application/x-www-form-urlencoded");
 String postData = "To=" + String(toNumber) + "&From=" + String(smsNumber) + "&Body="
+ message;
 int code = http.POST(postData);
 Serial.printf(" SMS Sent [%d]\n", code);
 http.end();
 }
}
// ---------- Blynk ----------
void sendToBlynkBulk(float temp, float hum, int pm1, int pm25, int pm10, int aqi, String cat,
float mq7, float mq135) {

 if (WiFi.status() != WL_CONNECTED) return;
 WiFiClientSecure client;
 client.setInsecure();
 HTTPClient http;
 String url = "https://blynk.cloud/external/api/batch/update?token=" +
String(BLYNK_AUTH_TOKEN);
 String msg = "AQI:%20" + String(aqi) + "%20" + cat;
 msg.replace(" ", "%20");
 url += "&V0=" + String(temp);
 url += "&V1=" + String(hum);
 url += "&V2=" + String(mq7, 1);
 url += "&V3=" + String(mq135, 0);
 url += "&V4=" + String(pm1);
 url += "&V5=" + String(pm25);
 url += "&V6=" + String(pm10);
 url += "&V7=" + msg;
 http.begin(client, url);
 int code = http.GET();
 if (code == 200) Serial.println(" Sent to Blynk");
 else Serial.printf(" Blynk Error %d\n", code);
 http.end();
}
// ---------- MQ-7 ----------
String getMQ7Status(float ppm) {
 if (ppm <= 9) return "Good";
 else if (ppm <= 35) return "Moderate";
 else if (ppm <= 100) return "Sensitive";
 else if (ppm <= 200) return "Unhealthy";
 else if (ppm <= 400) return "V. Unhealthy";
 else return "Hazardous";
}
void readMQ7() {
 long sum = 0;
 for (int i = 0; i < NUM_SAMPLES_MQ7; i++) {
 sum += analogRead(MQ7_PIN);
 delay(2);
 }
 float avg = sum / (float)NUM_SAMPLES_MQ7;
 float Vout = (avg / ADC_MAX) * VCC;

 if (Vout < 0.01) Vout = 0.01;
 float Rs = RL * ((VCC / Vout) - 1);
 float ratio = Rs / R0_MQ7;
 float logPPM = (log10(ratio) - b_mq7) / m_mq7;
 mq7_ppm = pow(10, logPPM);
 mq7_status = getMQ7Status(mq7_ppm);
}
// ---------- MQ-135 ----------
float readRs_MQ135() {
 long s = 0;
 for (int i = 0; i < SAMPLES_MQ135; i++) {
 s += analogRead(MQ135_PIN_NEW);
 delay(2);
 }
 float avg = (float)s / SAMPLES_MQ135;
 float vOut = avg * (VREF / ADC_RES);
 if (vOut < 1e-6) vOut = 1e-6;
 float Rs = ((V_CIRCUIT_SUPPLY - vOut) * RL_KOHM) / vOut;
 return Rs;
}
float getGasPPM_MQ135(float Rs) {
 if (baselineR0_MQ135 <= 0.0) return 0.0;
 float ratio = Rs / baselineR0_MQ135;
 float ppm = GAS_A * pow(ratio, GAS_B);
 return ppm;
}
String getMQ135Status(float ppm) {
 if (ppm >= 10000) return "HAZARD";
 else if (ppm >= 5000) return "V.HIGH";
 else if (ppm >= 2000) return "HIGH";
 else if (ppm >= 1000) return "POOR";
 else if (ppm >= 600) return "MOD";
 else return "GOOD";
}
void readMQ135() {
 float Rs = readRs_MQ135();
 float raw = getGasPPM_MQ135(Rs);
 if (!isfinite(raw) || raw < 0) raw = 0;

 if (raw > MAX_PPM_CAP) raw = MAX_PPM_CAP;
 if (ppm_smoothed_MQ135 <= 0) ppm_smoothed_MQ135 = raw;
 else ppm_smoothed_MQ135 = (SMOOTH_ALPHA * raw) + ((1.0 - SMOOTH_ALPHA) *
ppm_smoothed_MQ135);
 mq135_ppm = ppm_smoothed_MQ135;
 mq135_status = getMQ135Status(mq135_ppm);
}
// ---------- OLED ----------
void updateOLED() {
 display.clearDisplay();
 display.setTextSize(1);
 display.setFont();
 display.setTextColor(SSD1306_WHITE);
 display.setCursor(0, 0);
 if (display_cycle == 0) {
 display.setTextColor(SSD1306_WHITE);
 display.setCursor(0,10);
 display.setTextSize(1);
 display.printf("TEMPERATURE : %.1f C\n\n\nHUMIDITY :
%.1f%%",current_temp,current_hum);
 } else if (display_cycle == 1) {
 display.setCursor(15,0);
 display.println("PARTICULATE MATTER\n");
 display.printf("PM1: %d ug/m3\nPM2.5: %d ug/m3\nPM10: %d ug/m3", current_pm1,
current_pm25, current_pm10);
 } else if (display_cycle == 2) {
 display.setCursor(15,0);
 display.println("AIR QUALITY INDEX");
 display.setTextSize(2);
 display.setCursor(15,18);
 display.printf("AQI:%d\n", current_AQI);
 display.setCursor(15,35);
 display.printf("%s",current_AQI_category.c_str());
 } else if (display_cycle == 3) {
 display.setCursor(25,0);
 display.println("GENERAL GAS\n");
 display.setTextSize(2);
 display.printf("%s\n", warmupDone ? String(mq135_status).c_str() : "WARMING");
 if (warmupDone) display.printf("%.0fppm", mq135_ppm);

 } else if (display_cycle == 4) {
 display.setCursor(20,0);
 display.println("CARBON MONOXIDE\n");
 display.setTextSize(2);
 display.printf("%s\n", warmupDone ? String(mq7_status).c_str() : "WARMING");
 if (warmupDone) display.printf("%.1fppm", mq7_ppm);
 }
 // show warmup timer at bottom
 if (!warmupDone) {
 unsigned long elapsed = millis() - warmupStartTime;
 unsigned long remain = (WARMUP_DURATION > elapsed) ? (WARMUP_DURATION -
elapsed) : 0;
 int mins = remain / 60000;
 int secs = (remain % 60000) / 1000;
 display.setTextSize(1);
 display.setCursor(0, 56);
 display.printf("Warmup %02d:%02d", mins, secs);
 }
 display.display();
}
// ---------- Setup ----------
void setup() {
 Serial.begin(115200);
 Wire.begin(21, 22);
 if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
 Serial.println(" OLED failed");
 for (;;);
 }
 display.clearDisplay();
 display.setFont(&FreeMonoOblique9pt7b);
 display.setTextColor(SSD1306_WHITE);
 display.setTextSize(1);
 display.setCursor(3, 30);
 display.println("StratoSense");
 display.display();
 delay(2000);

 dht.begin();
 Serial2.begin(9600, SERIAL_8N1, 16, 17);
 analogSetWidth(12);
 analogSetPinAttenuation(MQ7_PIN, ADC_11db);
 analogSetPinAttenuation(MQ135_PIN_NEW, ADC_11db);
 display.clearDisplay();
display.setTextSize(1);
display.setFont();
display.setTextColor(SSD1306_WHITE);
display.setCursor(15, 20);
display.println("Connecting WiFi...");
display.display();
WiFiManager wm;
if (!wm.autoConnect("StratoSense-Setup")) {
 display.clearDisplay();
 display.setCursor(15, 20);
 display.setFont();
 display.println("WiFi Failed!");
 display.display();
 Serial.println(" WiFi not connected");
} else {
 display.clearDisplay();
 display.setCursor(15, 20);
 display.setFont();
 display.println("WiFi Connected!");
 display.display();
 delay(1000);
}
 if (WiFi.status() == WL_CONNECTED) {
 Blynk.config(BLYNK_AUTH_TOKEN);
 Blynk.connect();
 }
 warmupStartTime = millis();
 Serial.println(" MQ7 & MQ135 warmup started (10min)");
}

// ---------- Loop ----------
void loop() {
 if (WiFi.status() == WL_CONNECTED) Blynk.run();
 current_temp = dht.readTemperature();
 current_hum = dht.readHumidity();
 if (!warmupDone && millis() - warmupStartTime >= WARMUP_DURATION) {
 warmupDone = true;
 Serial.println(" MQ7 & MQ135 Warmup complete!");
 }
 if (pms.read(data)) {
 current_pm1 = data.PM_AE_UG_1_0;
 current_pm25 = data.PM_AE_UG_2_5;
 current_pm10 = data.PM_AE_UG_10_0;
 current_AQI = calculateAQI(current_pm25, current_pm10);
 current_AQI_category = getAQICategory(current_AQI);
 }
 if (warmupDone) {
 readMQ7();
 readMQ135();
 } else {
 mq7_ppm = 0;
 mq135_ppm = 0;
 mq7_status = "WARMING";
 mq135_status = "WARMING";
 }
 if (millis() - lastDisplayChange > displayInterval) {
 display_cycle = (display_cycle + 1) % 5;
 updateOLED();
 lastDisplayChange = millis();
 }
 if (millis() - lastSend > sendInterval) {
 sendToBlynkBulk(current_temp, current_hum, current_pm1, current_pm25, current_pm10,
 current_AQI, current_AQI_category, mq7_ppm, mq135_ppm);
 lastSend = millis();
 }
 if (!warmupDone) return;
 if (millis() - lastAlert > alertInterval) {
 if (current_AQI > AQI_Threshold)
 sendSMS(" High AQI: " + String(current_AQI) + " (" + current_AQI_category + ")");
 else if (mq7_ppm > MQ7_ALERT_THRESHOLD)
 sendSMS(" High CO: " + String(mq7_ppm, 1) + " ppm");
 else if (mq135_ppm > MQ135_ALERT_THRESHOLD)
 sendSMS(" High Gas: " + String(mq135_ppm, 0) + " ppm");
 lastAlert = millis();
 }
}