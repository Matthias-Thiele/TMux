#ifndef TMUX_HPP
#define TMUX_HPP

#define MAX_WORKER 32
#define MAX_INTERRUPT 3
void tmInterupt1();
void tmInterupt2();
void tmInterupt3();

class TMWorker;

TMWorker *interruptWorker[MAX_INTERRUPT];

class TMux {
private:
  TMWorker *m_workerList[MAX_WORKER];
  uint8_t m_nextFreeSlot = 0;
  uint8_t m_runNext = 0xff;

public:
  void loop();
  void setup();
  uint8_t add(TMWorker *worker);

  void adjustNext(u_int8_t nextSlot) {
    m_runNext = nextSlot;
  }
};

TMux tmux;

class TMWorker {
private:
  unsigned long m_nextRun = 0;
  unsigned long m_delayMillis = __UINT32_MAX__;
  unsigned long m_startup = 0;
  u_int8_t m_mySlot = -1;

public:
  TMWorker() {
    m_mySlot = tmux.add(this);
  };

  void setDelay(unsigned long delay) {
    m_delayMillis = delay;
    m_nextRun = millis() + delay;
  }

  void setStartupDelay(unsigned long startupDelay) {
    m_startup = startupDelay;
  }

  void attachWorker(u_int8_t interruptId, u_int8_t interruptPin, u_int8_t mode) {
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

  void interrupt() {
    m_nextRun = millis();
    tmux.adjustNext(m_mySlot);
  }

  bool checkRun(unsigned long now) {
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

  virtual void loop() {}
  virtual void setup() {};
};

void TMux::loop() {
  unsigned long now = millis();
  if (m_runNext != 0xff) {
    if (m_workerList[m_runNext]->checkRun(now)) {
      m_workerList[m_runNext]->loop();
      m_runNext = 0xff;
      now = millis();
    }
  }

  for (u_int8_t task = 0; task < m_nextFreeSlot; task++) {
    if (m_workerList[task]->checkRun(now)) {
      m_workerList[task]->loop();
      now = millis();
    }
  }
}

void TMux::setup() {
  for (u_int8_t task = 0; task < m_nextFreeSlot; task++) {
    m_workerList[task]->setup();
  }
}


uint8_t TMux::add(TMWorker *worker) {
  if (m_nextFreeSlot < MAX_WORKER) {
    u_int8_t mySlot = m_nextFreeSlot;
    m_workerList[m_nextFreeSlot++] = worker;
    worker->setup();
    return mySlot;
  }

  return 0xff;
}

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


#endif