#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>

// WiFi coordinates
#define SSID "FRITZ!Box 3272"
#define PASSWORD "Castione0016"

// Web server
WebServer server(80);

void send_file(String file_path, String file_type) {
  if(SPIFFS.exists(file_path)) {
    File content = SPIFFS.open(file_path, "r");
    server.streamFile(content, file_type);
    content.close();
  } else {
    Serial.print(file_path);
    Serial.println(" not found!");
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  Serial.println(WiFi.localIP());

  // Initialize filesystem
  SPIFFS.begin(true);

  // Get page
  // server.on("/", [] () { send_file("/index.html", "text/html"); });
  // server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/", SPIFFS, "/");
  // server.serveStatic("/js", SPIFFS, "/js");
  // server.serveStatic("/images", SPIFFS, "/images");
  
  // Start web server
  server.begin();
}

void loop() {
  server.handleClient();
}