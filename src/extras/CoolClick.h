// #include <Arduino.h>

// class CoolClick
// {
// public:
// 	CoolClick(uint8_t reqPin) : PIN(reqPin){
// 		pinMode(PIN, INPUT_PULLUP);
// 		attachInterrupt(PIN, std::bind(&CoolClick::isr, this), FALLING);
// 	};
// 	~CoolClick() {
// 		detachInterrupt(PIN);
// 	}

// 	void IRAM_ATTR isr() {
// 		numberKeyPresses += 1;
// 		pressed = true;
// 	}

// 	void checkPressed() {
// 		if (pressed) {
// 			Serial.printf("CoolClick on pin %u has been pressed %u times\n", PIN, numberKeyPresses);
// 			pressed = false;
// 		}
// 	}

//     volatile uint32_t numberKeyPresses;
// private:
// 	const uint8_t PIN;
//     volatile bool pressed;
// };