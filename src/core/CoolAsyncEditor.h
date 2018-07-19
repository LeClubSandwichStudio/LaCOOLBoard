#ifndef COOLASYNCEDITOR_H
#define COOLASYNCEDITOR_H
#include <ESPAsyncWebServer.h>

#define SSDP_UUID_SIZE 37

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
  virtual String getSavedWifi(String index);
  virtual String getSavedCredentialFromIndex(uint8_t i, String type);

private:
  virtual String getFlashID();
  virtual String getMAC();
  virtual String getUUID();
};

#endif
