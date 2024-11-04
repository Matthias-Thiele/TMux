#ifndef TMUX_HPP
#define TMUX_HPP

#include <Arduino.h>
#include <sys/types.h>

#define MAX_WORKER 32
#define MAX_INTERRUPT 3

class TMWorker;

class TMux {
private:
  TMWorker *m_WorkerRoot = NULL;
  TMWorker *m_nextWorker = NULL;
  TMWorker *m_runInterrupt = NULL;

public:
  void loop();
  void setup();
  void add(TMWorker *worker);
  void adjustNext(TMWorker * worker);
};


class TMWorker {
  friend TMux;

private:
  unsigned long m_nextRun = 0;

public:
  TMWorker(unsigned long delay = __UINT32_MAX__, unsigned long startup = 0, unsigned long userParam = 0);

  void setDelay(unsigned long delay);
  void setStartupDelay(unsigned long startupDelay);
  void attachWorker(u_int8_t interruptId, u_int8_t interruptPin, u_int8_t mode);
  void interrupt();

protected:
  TMWorker *m_NextInList;
  unsigned long m_delayMillis = __UINT32_MAX__;
  unsigned long m_startup = 0;
  unsigned long m_userParam = 0;

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