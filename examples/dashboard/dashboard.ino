#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "dashboard.h"

static const char* ssid = "........";
static const char* password = "........";

Dashboard *dashboard;
AsyncWebServer server(80);

bool test = false;

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {}
  Serial.println(" done.");
  delay(500);
  WiFi.setSleep(false);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (MDNS.begin("dashboard")) {
    // Add service to MDNS-SD
    MDNS.addService("_http", "_tcp", 80);
    delay(1000);
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("Starting server...");
  dashboard = new Dashboard(&server);
  dashboard->Add("Uptime", millis, 5000);
  dashboard->Add("Bool", test, 2000);
  dashboard->Add<float>("Float", []() { return millis() * 1.1; }, 2000);
  server.begin();
  Serial.println("Started server.");
}

void loop() {
  delay(10 * 1000);

  // Note: this works around https://github.com/espressif/arduino-esp32/issues/3886
  if (!MDNS.begin("dashboard")) {
    Serial.println("Error refreshing MDNS responder!");
  }

  if (millis() - dashboard->last_used() < 30 * 1000) {
    WiFi.setSleep(false);
  } else {
    WiFi.setSleep(true);
  }

  test = !test;
}
