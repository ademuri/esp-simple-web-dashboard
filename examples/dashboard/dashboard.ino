#ifdef ESP32
#include <ESPmDNS.h>
#include <WiFi.h>

#else  // ESP32
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#endif  // ESP32

#include "dashboard.h"

static const char* ssid = ".";
static const char* password = ".";

Dashboard* dashboard;
AsyncWebServer server(80);

bool test = false;

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println(" done.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef ESP32
  WiFi.setSleep(false);
#endif

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  // TODO: this doesn't work on ESP8266 - it says it starts, but the address
  // never resolves.
  if (MDNS.begin("dashboard")) {
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("Starting server...");
  dashboard = new Dashboard(&server);

  // Note: explicit type is only needed on ESP8266
  dashboard->Add<uint32_t>("Uptime", millis, 5000);
  dashboard->Add("Bool", test, 2000);
  dashboard->Add<float>(
      "Float", []() { return millis() * 1.1; }, 2000);
  server.begin();
  Serial.println("Started server.");
}

void loop() {
  delay(10 * 1000);

#ifdef ESP32
  // Note: this works around
  // https://github.com/espressif/arduino-esp32/issues/3886
  if (!MDNS.begin("dashboard")) {
    Serial.println("Error refreshing MDNS responder!");
  }
#else
  MDNS.update();
#endif

#ifdef ESP32
  if (millis() - dashboard->last_used() < 30 * 1000) {
    WiFi.setSleep(false);
  } else {
    WiFi.setSleep(true);
  }
#endif

  test = !test;
}
