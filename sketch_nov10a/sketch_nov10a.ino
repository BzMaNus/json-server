#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

const char *ssid = "Chatree01";
const char *password = "0620565502";
const char *ip = "192.168.1.20";
const int port = 3000;

DHT dht(D4, DHT11);
WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);
  init_wifi(ssid, password);
  dht.begin();
  timeClient.begin();
  timeClient.setTimeOffset(7 * 3600); 
}

void init_wifi(const char *ssid, const char *password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String getFormattedDateTime() {
  time_t now = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&now);

  char dateTime[20];
  strftime(dateTime, sizeof(dateTime), "%d %b %Y %H:%M:%S", timeinfo);

  return String(dateTime);
}

void loop() {
  timeClient.update();
  String dateTime = getFormattedDateTime();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  String data = "{\"datetime\":\"" + dateTime + "\",\"humidity\":" + String(humidity, 2) + ",\"temperature\":" + String(temperature, 2) + "}";

  HTTPClient http;
  String url = "http://" + String(ip) + ":" + String(port) + "/data";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(data);

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_CREATED) {
      Serial.println("Data created successfully!");
    } else {
      Serial.println("HTTP request failed with error code: " + String(httpCode));
    }
  } else {
    Serial.println("HTTP request failed");
  }
  http.end();
  delay(10000);
}
