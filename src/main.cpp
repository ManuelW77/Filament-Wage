#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <HX711.h>
#include <SH1106Wire.h>
#include <OLEDDisplayFonts.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

const int buttonTare = D4;

String esp_id = "filwage"; // ESP Name

const int spule = 150.0; //Gewicht Spule in g

const char* ssid = "iApfel";
const char* password = "druffmann77";
const char* mqtt_server = "192.168.1.5";
const char* ota_password = "druffmann77";

String esp = "esp_" + String(esp_id);
char* esp_name = &esp[0u];

String print_topic = "esp_filwage";
const char* mqtt_topic = &print_topic[0u];

WiFiClient espClient;
PubSubClient client(espClient);

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D6;
const int SDC_PIN = D7;
// change if your are using SSD1306
SH1106Wire        display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
//SSD1306Wire      display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);

// Scale Settings
const int SCALE_DOUT_PIN = D2;
const int SCALE_SCK_PIN = D1;
HX711 scale(SCALE_DOUT_PIN, SCALE_SCK_PIN);

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  wifi_station_set_hostname(esp_name);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //WiFi.config(IPAddress(192,168,1,20), IPAddress(192,168,1,1), IPAddress(255,255,255,0), IPAddress(192,168,1,1));
  //Serial.println(WiFi.status());

 // wait for a connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();

  pinMode(buttonTare, INPUT);

  setup_wifi();

  client.setServer(mqtt_server, 1883);

  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.setHostname(esp_name);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
  });
  ArduinoOTA.begin();

  scale.set_scale(1497135.00 / 0.755);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  //scale.tare();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(esp_name)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
  ArduinoOTA.handle();

  if (digitalRead(buttonTare) == LOW) {
    display.drawString(50, 0, String("LOW"));
    display.display();
    //scale.tare();
  }

  float weight = scale.get_units(5)*1000-4446-spule;
  // Nullen der Wage mit ca. Trommelgewicht
  // Muss auskommentiert werden zum Eichen !!!
  if (weight < 0) weight = 0;
  int gewicht = weight;

  int laenge = (gewicht)/3.0;
  if (laenge < 0) laenge = 0;

  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
  display.drawString(60, 3, String(gewicht) + "g");
  display.drawString(60, 19, "---------------");
  display.drawString(60, 36, String(laenge) + "m");
  display.display();

  delay(500);
}
