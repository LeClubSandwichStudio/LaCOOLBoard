#ifndef COOLASYNCEDITOR_H
#define COOLASYNCEDITOR_H
#include <ESPAsyncWebServer.h>
#define PRESENTATION_URL "http://lacool.co"
#define MODEL_NAME "LaCoolBoard"
#define MODEL_URL "http://lacool.co"
#define MANUFACTURER "LaCoolCo"
#define MANUFACTURER_URL "http://lacool.co"

class CoolAsyncEditor : public AsyncWebHandler {
private:
  fs::FS _fs;
  String _username;
  String _password;
  bool _authenticated;
  uint32_t _startTime;

public:
  CoolAsyncEditor(const fs::FS &fs = SPIFFS);
  virtual void write(String patch, String data);
  virtual String read(String patch);
  virtual char *readChars(String patch);
  virtual bool remove(String patch);
  virtual bool addNewWifi(String ssid, String pass);
  virtual void reWriteWifi(String json);
  virtual String getSdpConfig();
  virtual void configSSDP();
 
};

#endif
