Feature: CoolBoard

# ------- OTA -------
Scenario: Update firmware version
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it connects to the Wifi
    And the firmware is updated by OTA
    Then it tells me the new version of the firmware

Scenario: Update firmware and lost Wifi connection
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it connects to the Wifi
    And the firmware is updated by OTA
    And it does not connect to the Wifi
    Then the CoolBoard is working normally

Scenario: Update loginterval
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it connects to the Wifi
    And the loginterval is updated by OTA
    Then the json file is changed

Scenario: Update firmware when Wifi turned back
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it does not connect to the Wifi
    And the firmware is updated by OTA but failed
    And I reset the CoolBoard
    And it connects to the Wifi
    And I reset the CoolBoard
    Then it tells me the new version of the firmware

# ----- Soucis car la carte cherche en boucle à update /!\ -----
#Scenario: Update firmware with a wrong link
#    Given I reset the CoolBoard
#    And the CoolBoard is connected to a Wifi network
#    When the firmware is updated by OTA with a wrong link
#    Then the CoolBoard is working normally