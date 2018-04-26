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

#include <FS.h>

#include <ArduinoJson.h>

#include "CoolMQTT.h"
#include "CoolLog.h"

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
}

/**
 *  CoolMQTT::state():
 *  This method is provided to return the
 *  mqtt client's state.
 *  \return mqtt client state:
 *    -4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within
 *         the keepalive time
 *    -3 : MQTT_CONNECTION_LOST - the network connection was broken
 *    -2 : MQTT_CONNECT_FAILED - the network connection failed
 *    -1 : MQTT_DISCONNECTED - the client is disconnected cleanly
 *     0 : MQTT_CONNECTED - the cient is connected
 *     1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the
 *         requested version of MQTT
 *     2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client
 *         identifier
 *     3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept
 *         the connection
 *     4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were
 *         rejected
 *     5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized
 *         to connect
 */
int CoolMQTT::state() {
  return (this->client.state());
}

void CoolMQTT::printState(int state) {
  switch (state) {
    case MQTT_CONNECTION_TIMEOUT:
      ERROR_LOG("MQTT state: connection timeout");
      break;
    case MQTT_CONNECTION_LOST:
      ERROR_LOG("MQTT state: connection lost");
      break;
    case MQTT_CONNECT_FAILED:
      ERROR_LOG("MQTT state: connection failed");
      break;
    case MQTT_DISCONNECTED:
      ERROR_LOG("MQTT state: disconnected");
      break;
    case MQTT_CONNECTED:
      INFO_LOG("MQTT state: connected");
      break;
    case MQTT_CONNECT_BAD_PROTOCOL:
      ERROR_LOG("MQTT state: connection failed, bad protocol version");
      break;
    case MQTT_CONNECT_BAD_CLIENT_ID:
      ERROR_LOG("MQTT state: connection failed, bad client ID");
      break;
    case MQTT_CONNECT_UNAVAILABLE:
      ERROR_LOG("MQTT state: connection failed, server rejected client");
      break;
    case MQTT_CONNECT_BAD_CREDENTIALS:
      ERROR_LOG("MQTT state: connection failed, bad credentials");
      break;
    case MQTT_CONNECT_UNAUTHORIZED:
      ERROR_LOG("MQTT state: connection failed, client unauthorized");
      break;
    default:
      ERROR_LOG("MQTT state: connection status unknown");
  }
}
/**
 *  CoolMQTT::connect():
 *  This method is provided to connect the client to the server,
 *  publish to the out topic and subscribe to the in topic.
 *
 *  \return mqtt client state
 */
int CoolMQTT::connect() {
  int i = 0;
  char MAC[12];

  INFO_LOG("MQTT connecting...");
  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  tempMAC.toCharArray(MAC, 12);

  while ((!this->client.connected()) && (i < MQTT_RETRY)) {
    if (this->client.connect(MAC)) {
      client.subscribe(this->inTopic);
      INFO_LOG("Subscribed to MQTT input topic");
    } else {
      WARN_LOG("MQTT connection failed, retrying");
    }
    delay(5);
    i++;
  }
  int state = this->state();
  printState(state);
  return (state);
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
  DEBUG_VAR("Message size:", strlen(data));

  byte retries = 0;
  bool published =
      client.publish(this->outTopic, (byte *)data, strlen(data), false);

  while (!published && retries < MQTT_RETRY) {
    published =
        client.publish(this->outTopic, (byte *)data, strlen(data), false);
    if (!published) {
      WARN_LOG("MQTT publish failed, retrying...");
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
  if (published) {
    INFO_LOG("MQTT publish successful");
  } else {
    ERROR_LOG("MQTT publish failed, no more retries left!");
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
    INFO_VAR("Received new MQTT message:", this->msg);
  } else {
    INFO_LOG("No MQTT message to read");
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
  File configFile = SPIFFS.open("/mqttConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /mqttConfig.json");
    return (false);
  } else {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
      ERROR_LOG("Failed to parse MQTT configuration JSON");
      return (false);
    } else {
      DEBUG_JSON("MQTT configi JSON", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

      if (json["mqttServer"].success()) {
        const char *tempmqttServer = json["mqttServer"];
        for (int i = 0; i < 50; i++) {
          mqttServer[i] = tempmqttServer[i];
        }
      } else {
        for (int i = 0; i < 50; i++) {
          this->mqttServer[i] = this->mqttServer[i];
        }
      }
      json["mqttServer"] = this->mqttServer;

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

      configFile.close();
      configFile = SPIFFS.open("/mqttConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("Failed to write to /mqttConfig.json");
        return (false);
      }
      json.printTo(configFile);
      configFile.close();
      DEBUG_LOG("Saved MQTT configuration to /mqttConfig.json");
      return (true);
    }
  }
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
}
