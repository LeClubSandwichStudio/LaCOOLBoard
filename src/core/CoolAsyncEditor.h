#ifndef COOLASYNCEDITOR_H
#define COOLASYNCEDITOR_H
#include <ESPAsyncWebServer.h>

class CoolAsyncEditor: public AsyncWebHandler {
  private:
    fs::FS _fs;
    String _username;
    String _password; 
    bool _authenticated;
    uint32_t _startTime;
  public:
    CoolAsyncEditor(const fs::FS& fs=SPIFFS);
    virtual void write(String patch, String data);
    virtual String read(String patch);
    virtual bool remove(String patch);
    virtual bool addNewWifi(String ssid, String pass);
    virtual void reWriteWifi(String json);
};

#endif
