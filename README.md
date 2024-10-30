# TMux - Arduino delay() considered harmful

Arduino Task Multiplexer - not a RTOS

The Arduino Blinky program is the Hello World of embedded users. It is also used by developers who are well versed in C or C++ - it serves as a test for the development environment and connection to the respective microcontroller. For beginners, however, there is a high risk that they will be led down the wrong path early on.

The Blinky program contains two delay() calls with a considerable waiting time:
```
void loop() {
  digitalWrite(LED_BUILTIN, true);
  delay(500);
  digitalWrite(LED_BUILTIN, false);
  delay(500);
}
```
In this simple case, that's not even wrong or problematic. But it is one of the few special cases in which a delay() can be used sensibly. The special case here: there is only one single plot thread, every second an LED is switched off and on again. So far, so good. But what if a second LED is to be switched on with half the time delay:
```
void loop() {
  digitalWrite(LED_1, true);
  digitalWrite(LED_2, true);
  delay(250);
  digitalWrite(LED_2, false);
  delay(250);
  digitalWrite(LED_1, false);
  digitalWrite(LED_2, true);
  delay(250);
  digitalWrite(LED_2, false);
  delay(250);
  digitalWrite(LED_1, false);
}
```
And that's still easy. I don't even want to write down what it looks like when the time for the second LED is 144 milliseconds. You can see that delay() quickly leads to a dead end if there are several action threads with different time requirements.

A first thought would be to use a real time operating system (RTOS) and several threads. But that's a bigger undertaking. On the one hand, you have to familiarize yourself with the system. On the other hand, each thread takes up considerable resources. The RAM area is often very limited, especially with small microcontrollers, so it's not necessarily worth using.

# A more flexible solution in C

An obvious solution would be, for example, to create a time variable for each action that specifies when it should be executed next.
```
void loop() {
  unsigned long now = millis();

  if (now > nextLed1) {
    nextLed1 = now + 500;
    digitalWrite(LED_1, led1State);
    led1State = !led1State;
  }

  if (now > nextLed2) {
    nextLed2 = now + 250;
    digitalWrite(LED_2, led2State);
    led2State = !led2State;
  }
}
```
Now you can easily execute several storylines with any timing behavior. The delay() function is no longer used at all, so each sub-action is completed in a very short time and is only active again after the specified time has elapsed. In my view, this is a sensible approach for smaller projects and especially for beginner projects. As the project grows, however, you end up with an unsightly chain of if queries in the loop loop.

# A variant for C++

If you have decided not to create the project in C but in C++, you can put the storylines in their own class. The constructor of the class then registers the storyline in the loop loop itself. The action is stored in a method. I have created a framework for this, which can simply be integrated into the program as #include tmux.hpp. For each storyline, you then create a class that contains the respective actions.

The framework for such a thread can be stored as a template in the development environment and can thus be easily inserted:
```
class Storyline: public TMWorker {
  private:

  public:
    Storyline() {
    }

    void action() {
    }

} story1;
```
The class must then be given a meaningful name and the constructor must be filled with the code for initialization (analogous to the Setup() function in Arduino). The commands that should be executed at the desired time are then executed in the action() method. The local status is stored in the private: section. Blinky then looks like this:
```
class LEDBlink: public TMWorker {
  private:
    bool m_LEDstate;

  public:
    LEDBlink() {
      setDelay(500);
    }

    void action() {
      digitalWrite(LED_BUILTIN, m_LEDstate);
      m_LEDstate = !m_LEDstate;
    }

} ledBlink;
```
In the constructor, the waiting time of 500 milliseconds is preset, which should be observed between two calls. In the action method, the LED is then switched on or off, the state is determined via the member variable m_LEDstate.

This is initially significantly more code than in the simple version with the time variables. The advantage of this solution is that the individual storylines are better decoupled from each other and it is immediately clear what belongs to each storyline. And most of the code can be inserted automatically using a template. For simple actions, the classes can be placed directly in the main.cpp file. If the actions become more complex and require significantly more code, it can be useful to split them into separate files. This division also ensures that it is clear more quickly what belongs to the respective action or not.

In the constructor, the base class TMWorker ensures that each instance of this class is stored in a list. This is created as an array and not as a list of variable length, since I do not want to use a heap. The array only contains pointers to the instances, so it is not particularly large and can be extended if necessary. The list is stored in a variable tmux, which is automatically created by including the hpp file. However, an understanding of these internals is not necessary to use the class.

On the Arduino side, only the tmux.process() method needs to be called in the loop. This runs through the list of registered processes and executes them one after the other.
```
void loop() {
  tmux.process();
}
```

# Additional functions

If the times between calls are not constant, the next execution can be set explicitly in the action() method. The changed waiting time then applies to all subsequent calls until the next change.
```
class LEDBlink: public TMWorker {
  private:
    bool m_LEDstate;
    int  m_actDelay = 250;

  public:
    LEDBlink() {
      setDelay(m_actDelay);
    }

    void action() {
      digitalWrite(LED_BUILTIN, m_LEDstate);
      m_LEDstate = !m_LEDstate;

      // Wartezeit bei jedem Durchlauf um 100 Millisekunden verlängern
      m_actDelay += 100;
      setDelay(m_actDelay);
    }

} ledBlink;
```
You can also set a start delay, which ensures that the first call of the action takes place later. This can be useful, for example, if the state of the system should stabilize before a measurement is carried out. The setStartupDelay() method is available for this:
```
    LEDBlink() {
      setDelay(250);
      setStartupDelay(3000);
    }
```

# Integrating external events via interrupt

The Arduino framework supports the use of interrupts to react quickly to external events. However, the interrupt routine should be kept as short as possible, otherwise there may be disruptions in other parts of the program or the framework. If the action is a bit more complex, it can therefore be useful to create the action as a TMWorker class (more precisely - a class derived from TMWorker) with an 'infinite' waiting time. In the interrupt routine, only the execution is triggered, so it is completed again in a very short time. The coupling is done via the attachWorker() method, which connects the worker object with an interrupt source.
```
class InterruptButton: public TMWorker {
  public:
    InterruptButton() {
      pinMode(PA8, INPUT_PULLUP);
      attachWorker(0, PA8, CHANGE);
    }

    void action() {
      Serial1.println("Button 2 changed.");
    }
} interruptButton;
```
In the constructor, the PA8 pin is defined as an input with a pull-up resistor. Using attachWorker, this object is registered as interrupt source 0 for port PA8. CHANGE defines that the interrupt should be executed at every level change. Three interrupt sources (0, 1 and 2) are predefined; if more are required, the hpp file must be adjusted. In the constructor, there is no call to setDelay() - this means that the default waiting time of 'infinite' is used. The action should only be executed when a port change occurs.

So that the action for an interrupt is executed as quickly as possible, the tmux object saves the action thread with the last active interrupt and executes it first in the next loop, even if it would not be next in the normal order.

Using a state machine

In the simplest case, an action thread periodically executes an action over and over again. In practice, however, it is often more complicated. Depending on the previous history, different actions should be executed. Finite state machines are often used for this purpose. They are easy to implement and can map chains of actions clearly and easily. Such a state machine is essentially made up of two pieces of information: which states are there and which transitions between the states are possible.

A simple way of implementing this is to use a switch statement for all states. To make it easier to read, you can define an enum that gives all states meaningful names. In the following, I would like to demonstrate this using a simple traffic light control. The enum maps the states Wait, Ready to Drive, Drive, Wait to Stop. There is also an initialization, and it is a matter of taste whether you put this code in the constructor or in a separate state. If there are transitions that require re-initialization (e.g. a reset in the event of an error), then I would always take the initialization into the state machine.
```
enum TrafficLightStates {
  INIT,
  STOP,
  PREPARE_GO,
  GO,
  PREPARE_STOP
};
```
In the TrafficLight class, a local member variable is created which stores the current state - or more precisely: the state that is to be executed in the next step. It is assigned INIT, which then sets the traffic light to its basic state.

```
class TrafficLight: public TMWorker {
  private:
    TrafficLightStates nextState = INIT;
```
In the constructor, the microcontroller outputs for the LEDs are set. At this point, the state machine should move on to the next state without delay; setDelay(0) takes over this task.

```
    TrafficLight() {
      pinMode(TLED_RED, OUTPUT);
      pinMode(TLED_YELLOW, OUTPUT);
      pinMode(TLED_GREEN, OUTPUT);
      setDelay(0);
    }
```
In the action() method there is then the switch statement over the nextState.
```
    void action() {
      switch(nextState) {
```
For each state from the TrafficLightStates there is then a case label within the switch statement. In the INIT state all lights are switched off and the next state is Stop (STOP: Red).
```
        case INIT:
          digitalWrite(TLED_RED, false);
          digitalWrite(TLED_YELLOW, false);
          digitalWrite(TLED_GREEN, false);
          nextState = STOP;
          break;
```
In the STOP state, the red LED is switched on. Since this state normally comes from the yellow phase, the yellow LED must also be switched off. If you want to be on the safe side, you can also switch off the green LED, but it is always off at this point.
```
        case STOP:
          Serial1.println("RED");
          digitalWrite(TLED_RED, true);
          digitalWrite(TLED_YELLOW, false);
          setDelay(5000);
          nextState = PREPARE_GO;
          break;
```
The red phase should last 5 seconds, this is achieved using setDelay(5000). After these 5 seconds, the traffic light should change to the red-yellow phase, so the nextState is set to PREPARE_GO. Important: don't forget the break at the end of each state!

This is how all states are gradually implemented. The complete state machine could then look like this:
```
int TLED_RED = PA_10, TLED_YELLOW = PA_11, TLED_GREEN = PA_12;

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

  public:
    TrafficLight() {
      pinMode(TLED_RED, OUTPUT);
      pinMode(TLED_YELLOW, OUTPUT);
      pinMode(TLED_GREEN, OUTPUT);
      setDelay(0);
    }

    void action() {
      switch(nextState) {
        case INIT:
          digitalWrite(TLED_RED, false);
          digitalWrite(TLED_YELLOW, false);
          digitalWrite(TLED_GREEN, false);
          nextState = STOP;
          break;

        case STOP:
          digitalWrite(TLED_RED, true);
          digitalWrite(TLED_YELLOW, false);
          setDelay(5000);
          nextState = PREPARE_GO;
          break;

        case PREPARE_GO:
          digitalWrite(TLED_YELLOW, true);
          setDelay(1000);
          nextState = GO;
          break;

        case GO:
          digitalWrite(TLED_RED, false);
          digitalWrite(TLED_YELLOW, false);
          digitalWrite(TLED_GREEN, true);
          setDelay(4000);
          nextState = PREPARE_STOP;
          break;

        case PREPARE_STOP:
          digitalWrite(TLED_YELLOW, true);
          digitalWrite(TLED_GREEN, false);
          setDelay(1000);
          nextState = STOP;
          break;
      }
    }
} trafficLight;
```
Writing the actions directly into the switch statement only makes sense if the number of states remains small and the respective actions are manageable. Otherwise you get a long switch statement that is confusing and quickly leads to errors. In this case you should outsource each state action to a separate method and only call the respective method from the switch statement.

```
    void action() {
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
```
Now you have written additional code again, but the advantage is that the actions of the states are more clearly separated. In real life, you should of course also provide the individual parts with documentation. The complete traffic light control then looks like this:
```
int TLED_RED = PA_10, TLED_YELLOW = PA_11, TLED_GREEN = PA_12;

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
    TrafficLight() {
      pinMode(TLED_RED, OUTPUT);
      pinMode(TLED_YELLOW, OUTPUT);
      pinMode(TLED_GREEN, OUTPUT);
      setDelay(0);
    }

    void action() {
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
} trafficLight;
```
