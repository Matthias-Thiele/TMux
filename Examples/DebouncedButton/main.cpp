#include <Arduino.h>
#include "TMux.hpp"

HardwareSerial Serial1(PA10, PA9);

/**
 * @brief Configures gpio pin PB_8 as input with pullup.
 * 
 * Checks the button state every 10 milliseconds. When it
 * is LOW for 5 consecutive times the checkPressed method
 * returns true. As long as the button keeps pressed (LOW)
 * the checkPressed method stays true and the output to
 * Serial1 will be sent every 10 milliseconds.
 * 
 * Be aware that not all microcontroller support pullup
 * resistors for gpio inputs. In this case you have to use
 * an external resistor to VCC. The button switches to GND.
 */
class MyButton1: public TMButton {
  private:

  public:
    using TMButton::TMButton;

    void setup() {
    }

    void loop() {
      if (checkPressed()) {
        Serial1.println("Button 1 pressed.");
      }
    }

} button1(PB_8, 5, 10, LOW);

/**
 * @brief Configures gpio pin PB_9 as input with pulldown.
 * 
 * Checks the button state every 10 milliseconds. When it
 * is HIGH for 5 consecutive times the checkOneShot method
 * will return true once. The button has to be released
 * before the checkOneShot can be activated again.
 * 
 * Be aware that not all microcontroller support pulldown
 * resistors for gpio inputs. In this case you have to use
 * an external resistor to GND. The button switches to VCC.
 */
class MyButton2: public TMButton {
  private:

  public:
    using TMButton::TMButton;

    void setup() {
    }

    void loop() {
      if (checkOneShot()) {
        Serial1.println("Button 2 pressed.");
      }
    }

} button2(PB_9, 5, 10, HIGH);


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

