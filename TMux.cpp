#include "TMux.hpp"

#ifndef MAX_WORKER
#define MAX_WORKER 32
#endif

/**
 * @brief Definition of the available interrupt sources
 * 
 */
#define MAX_INTERRUPT 3
void tmInterupt1();
void tmInterupt2();
void tmInterupt3();
TMWorker *interruptWorker[MAX_INTERRUPT];

/**
 * @brief Single instance of the task multiplexer
 * 
 * Do not insert additional instances.
 */
TMux tmux;

/**
 * Implementation of the task multiplexer
 */

/**
 * @brief Loop function to be called from the Arduino loop function.
 * 
 * This call will check all registered workers for expired wait
 * times and call the loop function of the worker.
 * 
 * If there are pending interrupt activations then call this
 * worker loop function first.
 */
void TMux::loop() {
  unsigned long now = millis();
  if (m_runInterrupt) {
    if (m_runInterrupt->checkRun(now)) {
      m_runInterrupt->loop();
      now = millis();
    }

    m_runInterrupt = NULL;
  }

  TMWorker *nextWorker = m_WorkerRoot;
  while (nextWorker) {
    if (nextWorker->checkRun(now)) {
      nextWorker->loop();
      now = millis();
    }
    
    nextWorker = nextWorker->m_NextInList;
  }
}

/**
 * @brief Setup function of the task multiplexer, 
 * to be called from the Arduino setup function.
 * 
 * Activates the setup function of all registered workers.
 */
void TMux::setup() {
  TMWorker *nextWorker = m_WorkerRoot;
  while (nextWorker) {
    nextWorker->setup();
    nextWorker = nextWorker->m_NextInList;
  }
}

/**
 * @brief Add one worker to the list of registered workers.
 * 
 * @param worker 
 * @return uint8_t Slot number or 0xff on error.
 */
void TMux::add(TMWorker *worker) {
  worker->m_NextInList = m_WorkerRoot;
  m_WorkerRoot = worker;
}

void TMux::adjustNext(TMWorker *worker) {
    m_runInterrupt = worker;
}

/**
 * Implementation of the worker base class.
 */

/**
 * @brief Default constructor, infinite wait time, no startup delay
 * 
 */
TMWorker::TMWorker() {
  tmux.add(this);
}

/**
 * @brief Constructor with defined wait time, no startup delay
 * 
 * @param delay delay between activations in milliseconds
 */
TMWorker::TMWorker(unsigned long delay) {
  m_delayMillis = delay;
  tmux.add(this);
}

/**
 * @brief Constructor with definied wait time and startup delay
 * 
 * @param delay delay between activations in milliseconds
 * @param startup delay time before first activation in milliseconds
 */
TMWorker::TMWorker(unsigned long delay, unsigned long startup) {
  m_delayMillis = delay;
  m_startup = startup;
  tmux.add(this);
}

/**
 * @brief Sets a new delay duration and adjusts the next activation time.
 * 
 * @param delay 
 */
void TMWorker::setDelay(unsigned long delay) {
  m_delayMillis = delay;
  m_nextRun = millis() + delay;
}

/**
 * @brief Sets the startup delay duration. 
 * 
 * Only useful in the setup method. Will be ignored after
 * the first loop run.
 * 
 * @param startupDelay 
 */
void TMWorker::setStartupDelay(unsigned long startupDelay) {
  m_startup = startupDelay;
}

/**
 * @brief Attach the worker to an interrupt source.
 * 
 * @param interruptId 
 * @param interruptPin 
 * @param mode 
 */
void TMWorker::attachWorker(u_int8_t interruptId, u_int8_t interruptPin, u_int8_t mode) {
  if (interruptId < MAX_INTERRUPT) {
    interruptWorker[interruptId] = this;

    std::function<void ()> func;
    switch (interruptId) {
      case 0:
        func = tmInterupt1;
        break;
    
      case 1:
        func = tmInterupt2;
        break;
    
      case 2:
        func = tmInterupt3;
        break;
    
      default:
      return;
    } 

    attachInterrupt(digitalPinToInterrupt(interruptPin), func, mode);
  }
}

/**
 * @brief Marks this worker for activation after an interrupt occured.
 * 
 * This worker will be activated at the next loop invocation, ignoring
 * the given wait wait delay.
 * 
 */
void TMWorker::interrupt() {
  m_nextRun = millis();
  tmux.adjustNext(this);
}

/**
 * @brief Checks if the worker has an expired wait duration.
 * 
 * The first run will be after the defined startup delay.
 * Any further run will be after the defined delay time.
 * 
 * @param now 
 * @return true delay expired, can be activated
 * @return false wait, do not activate yet
 */
bool TMWorker::checkRun(unsigned long now) {
  if (m_nextRun == 0) {
    Serial1.print("Startup delay ");
    Serial1.println(m_startup);
    m_nextRun = now + m_startup;
  }

  if (m_nextRun <= now) {
    if (m_delayMillis == __UINT32_MAX__) {
      m_nextRun = __UINT32_MAX__;
    } else {
      m_nextRun += m_delayMillis;
    }
    
    if (m_nextRun < now) {
      m_nextRun = now;
    }

    return true;
  } else {
    return false;
  }
}

/* Interrupt callback functions */
void tmInterupt1() {
  if (interruptWorker[0]) {
    interruptWorker[0]->interrupt();
  }
}

void tmInterupt2() {
  if (interruptWorker[1]) {
    interruptWorker[1]->interrupt();
  }
}

void tmInterupt3() {
  if (interruptWorker[2]) {
    interruptWorker[2]->interrupt();
  }
}

/* Implementation of TMButton */

TMButton::TMButton(int pinNumber, int repeatCount, int delay, int mode) {
  m_mode = mode;
  m_pinNumber = pinNumber;
  m_delayMillis = delay;

  m_stateMask = getMaskFromRepeatCount(repeatCount);
  Serial1.print("State Mask: "); Serial1.println(m_stateMask, HEX);

  if (mode == LOW) {
    pinMode(pinNumber, INPUT_PULLUP);
    m_actState = 0xffff;
  } else {
    pinMode(pinNumber, INPUT_PULLDOWN);
    m_actState = 0;
  }
}

bool TMButton::checkPressed() {
  bool actInputState = digitalRead(m_pinNumber);
  m_actState = (m_actState << 1) | (actInputState & 1);
  Serial1.print("Check "); Serial1.println(m_actState, HEX);

  if (m_mode == LOW) {
    return (m_actState & m_stateMask) == 0;
  } else {
    return (m_actState & m_stateMask) == m_stateMask;
  }
}

bool TMButton::checkOneShot() {
  bool actButtonState = checkPressed();
  bool result = actButtonState && !m_lastButtonState;
  
  m_lastButtonState = actButtonState;
  return result;
}

int TMButton::getMaskFromRepeatCount(u_int8_t repeatCount) {
  static int maskList[] = {1, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff};
  if (repeatCount < 16) {
    return maskList[repeatCount];
  } else {
    return 3;
  }
}
