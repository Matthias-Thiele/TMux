#include <Arduino.h>
#include <TMux.hpp>

HardwareSerial Serial1(PA10, PA9);

/*
 * The LED_BUILTIN starts blinking after 10 seconds with
 * a Period of 600 milliseconds (300 ms on, 300 ms off)
*/
class LEDBlink1: public TMWorker {
  private:
    bool m_LEDstate;

  public:
    using TMWorker::TMWorker;

    void setup() {
      pinMode(LED_BUILTIN, OUTPUT);
    }

    void loop() {
      digitalWrite(LED_BUILTIN, m_LEDstate);
      m_LEDstate = !m_LEDstate;
    }

} ledBlink1(300ul, 10000);

/*
 * The LED at port PA_10 starts blinking immediately with
 * a period of 2 seconds.
*/
class LEDBlink2: public TMWorker {
  private:
    bool m_LEDstate;

  public:
    using TMWorker::TMWorker;

    void setup() {
      pinMode(PA_10, OUTPUT);
    }

    void loop() {
      digitalWrite(PA_10, m_LEDstate);
      m_LEDstate = !m_LEDstate;
    }

} ledBlink2(1000ul);

/**
 * @brief Arduino loop rerouted to tmux loop.
 * 
 * The loop method of every worker will be called.
 */
void loop() {
  tmux.loop();
}

/**
 * @brief Arduino setup rerouted to tmux setup.
 * 
 * The tmux setup calls the setup method of
 * every worker.
 */
void setup() {
  Serial.begin(115200);  
  Serial.println("Started.");
  Serial1.begin(115200);  
  Serial1.println("Started1.");
  tmux.setup();
}

