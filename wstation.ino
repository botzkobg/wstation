#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300      /* Time ESP32 will go to sleep (in seconds) */
#define DHT_PIN 27              // Digital pin connected to the DHT sensor
#define DHT_TYPE DHT22          // DHT 22 (AM2302)
#define LED_PIN 5



const char* ssid = "SSID";
const char* password = "PASS";

const char* domain = "domain.com";
const char* host = "https://domain.com";
const int hostPort = 443;
const char* hostLink = "/wstation.php?lid=%d&t=%.2f&rh=%.2f";
const uint8_t sensro1LocationId = 1;
const int readingPeriod = 5 * 60 * 1000;

RTC_DATA_ATTR int bootCount = 0;


void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));  

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(""); Serial.println("IP Address: "); Serial.println(WiFi.localIP());

  readSensors();
  initDeepSleep();

  
}

void loop() {
}

void initDeepSleep() {
  // This function will disconnect and turn off the WiFi (NVS WiFi data is kept)
  WiFi.disconnect(true, false);

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}

void readSensors() {
  DHT dht(DHT_PIN, DHT_TYPE);
  WiFiClientSecure client;
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();

  dht.begin();

  Serial.print("Temperature: ");
  Serial.println(newT);
  Serial.print("Humidity: ");
  Serial.println(newH);


  client.setInsecure();
  if (! client.connect(domain, hostPort)) {
    Serial.println("Connection failed");
    return;
  }


  Serial.println(String("GET ") + "/wstation.php?lid=" + sensro1LocationId + "&t=" + newT + "&rh=" + newH + " HTTP/1.1\r\n" + "Host: " + domain + "\r\n" + "Connection: close\r\n\r\n");

  // Make a HTTP request:
  client.print("GET "); client.print(host); client.print(String("/wstation.php?lid=") + sensro1LocationId + "&t=" + newT + "&rh=" + newH); client.println(" HTTP/1.0");
  client.print("Host: "); client.println(domain);
  client.println("Connection: close");
  client.println();
  delay(500);
}
