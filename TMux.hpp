#ifndef TMUX_HPP
#define TMUX_HPP

#include <Arduino.h>
#include <sys/types.h>

#define MAX_WORKER 32
#define MAX_INTERRUPT 3

class TMWorker;

class TMux {
private:
  TMWorker *m_workerList[MAX_WORKER];
  uint8_t m_nextFreeSlot = 0;
  uint8_t m_runNext = 0xff;

public:
  void loop();
  void setup();
  uint8_t add(TMWorker *worker);
  void adjustNext(u_int8_t nextSlot);
};


class TMWorker {
  friend TMux;

private:
  unsigned long m_nextRun = 0;
  u_int8_t m_mySlot = -1;

public:
  TMWorker();
  TMWorker(unsigned long delay);
  TMWorker(unsigned long delay, unsigned long startup);

  void setDelay(unsigned long delay);
  void setStartupDelay(unsigned long startupDelay);
  void attachWorker(u_int8_t interruptId, u_int8_t interruptPin, u_int8_t mode);
  void interrupt();

protected:
  unsigned long m_delayMillis = __UINT32_MAX__;
  unsigned long m_startup = 0;

  bool checkRun(unsigned long now);
  virtual void loop() {}
  virtual void setup() {};
};

class TMButton: public TMWorker {

private:
  int m_actState;
  int m_stateMask;
  u_int8_t m_mode;
  u_int8_t m_pinNumber;
  bool m_lastButtonState;

public:
  TMButton(int pinNumber, int repeatCount, int delay, int mode);
  bool checkPressed();
  bool checkOneShot();

protected:
  int getMaskFromRepeatCount(u_int8_t repeatCount);
};

extern TMux tmux;

#endif