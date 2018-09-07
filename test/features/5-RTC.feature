Feature: CoolBoard

# ------- RTC -------
Scenario: True timestamp
    Given the CoolBoard is ON
    And I reset the CoolBoard
    When it connects to the Wifi
    Then the timestamp is true