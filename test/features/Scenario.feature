Feature: CoolBoard

Scenario: Wifi connexion
    Given the CoolBoard is ON
    When it has a WiFi registered
    Then it connects to the WiFi
    #And I reset the CoolBoard

Scenario: Update firmware version
    Given the CoolBoard is connected to a WiFi network
    When the firmware is updated by OTA
    Then it tells me the new version of the firmware

Scenario: Update loginterval
    Given the CoolBoard is connected to a WiFi network
    When the loginterval is updated by OTA
    Then the json file is changed

#Soucis car la carte cherche en boucle à update /!\
#Scenario: Wrong link firmware
#    Given the CoolBoard is connected to a Wifi network
#    When the firmware is updated by OTA with a wrong link
#    Then the CoolBoard is working normally

