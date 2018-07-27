#ifndef COOLASYNCEDITOR_H
#define COOLASYNCEDITOR_H
#include <ESPAsyncWebServer.h>

#define SSDP_UUID_SIZE 37

class CoolAsyncEditor : public AsyncWebHandler {
private:
  fs::FS _fs;
  bool _authenticated;
  uint32_t _startTime;

public:
  String HTTPuserName;
  String HTTPpassword;
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
  virtual bool beginAdminCredential();
  virtual bool configureAdminCredential(String userName, String password);
private:
  virtual bool resetAdminCredential();
  virtual String getFlashID();
  virtual String getMAC();
  virtual String getUUID();
};

#endif
