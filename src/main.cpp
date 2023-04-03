#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

// Wifi
#define WIFI_SSID "TP-Link_165C"
#define WIFI_PASSWORD "99368319"

#define MQTT_HOST IPAddress(192, 168, 0, 105)
#define MQTT_PORT 1883

#define MQTT_PUB_LIGHT1 "light1"
#define MQTT_PUB_LIGHT2 "light2"
#define MQTT_PUB_ALARM "alarm"
// lights
const int light1Pin = 5;
int currentLight1Value = LOW;
int lastLight1Value = HIGH;
const int light2Pin = 4;
int currentLight2Value = LOW;
int lastLight2Value = HIGH;
//light status strings
String light1Status = "off";
String light2Status = "off"; 
//alarm
const int alarmPin = 14;
int currentAlarmValue = LOW;
int lastAlarmValue = HIGH;
//alarm status string
String alarmStatus = "off";
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi()
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent)
{
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  // uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  // Serial.print("Subscribing at QoS 2, packetId: ");
  // Serial.println(packetIdSub);

  // uint16_t packetIdPubLight1 = mqttClient.publish(MQTT_PUB_LIGHT1, 1, true, light1Status.c_str());
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPubLight1);
  // uint16_t packetIdPubLight2 = mqttClient.publish(MQTT_PUB_LIGHT2, 1, true, light2Status.c_str());
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPubLight2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(light1Pin, INPUT_PULLUP);
  // lastLight1Value = digitalRead(light1Pin);
  pinMode(light2Pin, INPUT);
  // lastLight2Value = digitalRead(light2Pin);
  pinMode(alarmPin, INPUT);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}

void loop()
{
  currentLight1Value = digitalRead(light1Pin);
  currentLight2Value = digitalRead(light2Pin);
  currentAlarmValue =digitalRead(alarmPin);
  if(currentLight1Value == LOW && lastLight1Value == HIGH){
    if(light1Status == "off"){
      light1Status = "on";
    }
    else{
      light1Status = "off";
    }
    Serial.print("light 1: ");
    Serial.println(light1Status);
    uint16_t packetIdPubLight1 = mqttClient.publish(MQTT_PUB_LIGHT1, 1, true, light1Status.c_str());
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPubLight1);
  }
  lastLight1Value = currentLight1Value;
  if(currentLight2Value == LOW && lastLight2Value == HIGH){
    if(light2Status == "off"){
      light2Status = "on";
    }
    else{
      light2Status = "off";
    }
    Serial.print("light 2: ");
    Serial.println(light2Status);
    uint16_t packetIdPubLight2 = mqttClient.publish(MQTT_PUB_LIGHT2, 1, true, light2Status.c_str());
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPubLight2);
  }
  lastLight2Value = currentLight2Value;
  if(currentAlarmValue == LOW && lastAlarmValue == HIGH){
    if(alarmStatus == "off"){
      alarmStatus = "on";
    }
    else{
      alarmStatus = "off";
    }
    Serial.print("alarm is turned ");
    Serial.println(alarmStatus);
    uint16_t packetIdPubAlarm = mqttClient.publish(MQTT_PUB_ALARM, 1, true, alarmStatus.c_str());
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPubAlarm);
  }
  lastAlarmValue = currentAlarmValue;
}