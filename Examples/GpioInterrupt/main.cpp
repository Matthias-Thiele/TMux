#include <Arduino.h>
#include "TMux.hpp"

HardwareSerial Serial1(PA10, PA9);


class GpioInterrupt: public TMWorker {
  public:
    void setup() {
      pinMode(PA8, INPUT_PULLUP);
      attachWorker(0, PA8, CHANGE);
    }

    void loop() {
      Serial1.println("GPIO state changed.");
    }
} gpioInterrupt;


void loop() {
  tmux.loop();
}

void setup() {
  Serial.begin(115200);  
  Serial.println("Started.");
  Serial1.begin(115200);  
  Serial1.println("Started1.");
  tmux.setup();
}

