#include <Arduino.h>
#include "TMux.hpp"

HardwareSerial Serial1(PA10, PA9);

/*
 * GPIO ports for the red/ yellow/ green LEDs.
*/
int TLED_RED = PA_10, TLED_YELLOW = PA_11, TLED_GREEN = PA_12;

/*
 * Traffic light states (German version with red/yellow indicating
 * that green will light up very soon).
*/
enum TrafficLightStates {
  INIT,
  STOP,
  PREPARE_GO,
  GO,
  PREPARE_STOP
};

class TrafficLight: public TMWorker {
  private:
    TrafficLightStates nextState = INIT;

    void processInit() {
      digitalWrite(TLED_RED, false);
      digitalWrite(TLED_YELLOW, false);
      digitalWrite(TLED_GREEN, false);
      nextState = STOP;
    }

    void processStop() {
      digitalWrite(TLED_RED, true);
      digitalWrite(TLED_YELLOW, false);
      setDelay(5000);
      nextState = PREPARE_GO;
    }

    void processPrepareGo() {
      digitalWrite(TLED_YELLOW, true);
      setDelay(1000);
      nextState = GO;
    }

    void processGo() {
      digitalWrite(TLED_RED, false);
      digitalWrite(TLED_YELLOW, false);
      digitalWrite(TLED_GREEN, true);
      setDelay(4000);
      nextState = PREPARE_STOP;
    }

    void processPrepareStop() {
      digitalWrite(TLED_YELLOW, true);
      digitalWrite(TLED_GREEN, false);
      setDelay(1000);
      nextState = STOP;
    }
    
  public:
    using TMWorker::TMWorker;

    void setup() {
      pinMode(TLED_RED, OUTPUT);
      pinMode(TLED_YELLOW, OUTPUT);
      pinMode(TLED_GREEN, OUTPUT);
    }

    void loop() {
      switch(nextState) {
        case INIT:
          processInit();
          break;

        case STOP:
          processStop();
          break;

        case PREPARE_GO:
          processPrepareGo();
          break;

        case GO:
          processGo();
          break;

        case PREPARE_STOP:
          processPrepareStop();
          break;
      }
    }
} trafficLight(0);


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

