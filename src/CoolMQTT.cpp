/**
 *  Copyright (c) 2018 La Cool Co SAS
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include <Arduino.h>
#include <FS.h>

#include <ESP8266WiFi.h>

#include <ArduinoJson.h>

#include "CoolConfig.h"
#include "CoolLog.h"
#include "CoolMQTT.h"
#include "CoolWifi.h"

/**
 *  CoolMQTT::begin():
 *  This method is provided to set the mqtt
 *  client's parameters:  -client
 *        -server
 *        -callback method
 *        -buffer size
 */
void CoolMQTT::begin() {
  client.setClient(espClient);
  client.setServer(mqttServer, 1883);
  client.setCallback([this](char *topic, byte *payload, unsigned int length) {
    this->callback(topic, payload, length);
  });
  client.setBufferSize((unsigned short)bufferSize);
}

/**
 *  CoolMQTT::state():
 *  This method is provided to return the
 *  mqtt client's state.
 *  \return mqtt client state:
 *    -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within
 *the keepalive time -3 : MQTT_CONNECTION_LOST - the network connection was
 *broken -2 : MQTT_CONNECT_FAILED - the network connection failed -1 :
 *MQTT_DISCONNECTED - the client is disconnected cleanly 0 : MQTT_CONNECTED -
 *the cient is connected 1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't
 *support the requested version of MQTT 2 : MQTT_CONNECT_BAD_CLIENT_ID - the
 *server rejected the client identifier 3 : MQTT_CONNECT_UNAVAILABLE - the
 *server was unable to accept the connection 4 : MQTT_CONNECT_BAD_CREDENTIALS
 *- the username/password were rejected 5 : MQTT_CONNECT_UNAUTHORIZED - the
 *client was not authorized to connect
 */
int CoolMQTT::state() {
  DEBUG_VAR("MQTT state:", this->client.state());
  return (this->client.state());
}

/**
 *  CoolMQTT::connect():
 *  This method is provided to connect the client to the server,
 *  publish to the out topic and subscribe to the in topic.
 *
 *  \return mqtt client state
 */
int CoolMQTT::connect() {
  DEBUG_LOG("MQTT connecting...");

  int i = 0;
  String tempMAC = WiFi.macAddress();

  tempMAC.replace(":", "");

  char MAC[12];
  tempMAC.toCharArray(MAC, 12);

  while ((!this->client.connected()) && (i < 5)) {
    // Attempt to connect
    // use the mac as MQTT client ID to assure a unique id
    if (this->client.connect(MAC)) {
      client.subscribe(this->inTopic);
      DEBUG_LOG("Subscribed to MQTT input topic");
      return (this->state());
    } else {
      WARN_LOG("MQTT connection failed, retrying");
    }
    delay(5);
    i++;
  }
  if (state() == 0) {
    DEBUG_LOG("MQTT connected");
  } else {
    ERROR_LOG("MQTT connection failed after 5 retries");
  }
  return (this->state());
}

/**
 *  CoolMQTT::publish(data):
 *  This method is provided to publish data
 *  to the out topic. If the publish fails
 *   it deconnects and reconnects the Wifi
 *   and MQTT. This prevents lost MQTT syndrome
 *   on cheaper Wifi hotspots
 *
 *  \return true if publish successful,
 *  false otherwise
 */
bool CoolMQTT::publish(const char *data) {
  DEBUG_VAR("Message to publish:", data);
  DEBUG_VAR("Message size", strlen(data));

  byte retries = 0;
  bool published = false;
  while (!published && retries < 5) {
    published =
        client.publish(this->outTopic, (byte *)data, strlen(data), false);
    if (!published) {
      WARN_LOG("MQTT publish failed");
      if (wifiManager.state() != 3) {
        WARN_LOG("Wifi offline, reconnecting...");
        wifiManager.disconnect();
        delay(200);
        wifiManager.begin();
        delay(200);
      }
      DEBUG_LOG("Reconnecting to MQTT server...");
      connect();
      delay(100);
    }
    retries++;
  }
  delay(100);
  if (published) {
    DEBUG_LOG("MQTT publish successful");
  } else {
    ERROR_LOG("MQTT publish failed after 5 retries");
  }
  return (published);
}

/**
 *  CoolMQTT::mqttLoop():
 *  This method is provided to allow the
 *  client to process the data
 *
 *  \return true if successful,false
 *  otherwise
 */
bool CoolMQTT::mqttLoop() {
  unsigned long lastTime = millis();

  while ((millis() - lastTime) < 1000) {
    this->client.loop();
    yield();
  }
  bool connected = this->client.loop();
  DEBUG_VAR("MQTT connection status after listen loop", connected);
  return (connected);
}

/**
 *  CoolMQTT::callback(in topic, incoming message , message length):
 *  This method is provided to handle incoming messages from the
 *  subscribed inTopic.
 *
 *  Arguments are automatically assigned in client.setCallback()
 */
void CoolMQTT::callback(char *topic, byte *payload, unsigned int length) {
  if (this->newMsg == false) {
    char temp[length + 1];
    for (unsigned int i = 0; i < length; i++) {
      temp[i] = (char)payload[i];
    }
    this->newMsg = true;
    temp[length + 1] = '\0';
    this->msg = String(temp);
    this->msg.remove(length, 1);
    DEBUG_VAR("Received new MQTT message:", this->msg);
  } else {
    DEBUG_LOG("No MQTT message to read");
  }
}

/**
 *  CoolMQTT::read():
 *  This method is provided to return the last
 *  read message.
 */
String CoolMQTT::read() {
  if (this->newMsg == true) {
    this->newMsg = false;
    DEBUG_VAR("Last MQTT message received:", this->msg);
    return (this->msg);
  }
  return ("");
}

/**
 *  CoolMQTT::config():
 *  This method is provided to configure
 *  the mqttClient :  -server
 *        -inTopic
 *        -outTopic
 *        -client Id
 *        -buffer size
 *
 *  \return true if successful,false otherwise
 */
bool CoolMQTT::config() {
  CoolConfig config("/mqttConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read MQTT config");
    return (false);
  }
  JsonObject &json = config.get();

  // FIXME: we don't need 50 MQTT servers
  if (json["mqttServer"].success()) {
    const char *tempmqttServer = json["mqttServer"];
    for (int i = 0; i < 50; i++) {
      mqttServer[i] = tempmqttServer[i];
    }
  }
  json["mqttServer"] = this->mqttServer;

  // FIXME: we don't need 50 MQTT input topics
  if (json["inTopic"].success()) {
    const char *tempInTopic = json["inTopic"];
    for (int i = 0; i < 50; i++) {
      inTopic[i] = tempInTopic[i];
    }
  } else {
    String tempMAC = WiFi.macAddress();
    tempMAC.replace(":", "");
    snprintf(inTopic, 50, "$aws/things/%s/shadow/update/delta",
             tempMAC.c_str());
    DEBUG_VAR("Setting outgoing MQTT Channel to:", inTopic);
  }
  json["inTopic"] = this->inTopic;

  // FIXME: we don't need 50 MQTT output topics
  if (json["outTopic"].success()) {
    const char *tempOutTopic = json["outTopic"];
    for (int i = 0; i < 50; i++) {
      outTopic[i] = tempOutTopic[i];
    }
  } else {
    String tempMAC = WiFi.macAddress();
    tempMAC.replace(":", "");
    snprintf(outTopic, 50, "$aws/things/%s/shadow/update", tempMAC.c_str());
    DEBUG_VAR("Setting outgoing MQTT Channel to:", outTopic);
  }
  json["outTopic"] = this->outTopic;

  if (json["bufferSize"].success()) {
    int tempBufferSize = json["bufferSize"];
    bufferSize = tempBufferSize;
  }

  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save MQTT config");
    return (false);
  }
  return (true);
}

/**
 *  CoolMQTT::printConf():
 *  This method is provided to print the
 *  configuration to the Serial Monitor
 */
void CoolMQTT::printConf() {
  INFO_LOG("MQTT configuration");
  INFO_VAR("  MQTT server:", this->mqttServer);
  INFO_VAR("  In topic   :", this->inTopic);
  INFO_VAR("  Out topic  :", this->outTopic);
  INFO_VAR("  Buffer size:", this->bufferSize);
}