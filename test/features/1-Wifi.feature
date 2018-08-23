Feature: CoolBoard

# ------- WIFI -------
Scenario: Wifi connection success
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it has a WiFi registered
    Then it connects to the WiFi

Scenario: Wifi connection retry
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it does not connect to the Wifi
    Then it retries several time to connect to the Wifi

Scenario: Wifi connection error
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it does not connect to the Wifi
    Then a message is saved as JSON on SPIFFS

# A excécuter que si celui du dessus l'est aussi
Scenario: Send messages recorded 
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it connects to the Wifi
    And I reset the CoolBoard
    Then messages saved on SPIFFS are sended