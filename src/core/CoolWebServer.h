#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#define http_username "admin"
#define http_password "admin"


class CoolWebServer{
    public:
    bool begin(const char* currentSSID, const char* currentPASS);
    void end();
    void requestConfiguration();
    bool isRunnig = false;
    private:
    void doWithSta(const char* ssid, const char* pass);

};