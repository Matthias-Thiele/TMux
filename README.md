# TMux
Arduino Task Multiplexer - not a RTOS

The Arduino Blinky program is the Hello World of embedded users. It is also used by developers who are well versed in C or C++ - it serves as a test for the development environment and connection to the respective microcontroller. For beginners, however, there is a high risk that they will be led down the wrong path early on.

The Blinky program contains two delay() calls with a considerable waiting time:

void loop() {
  digitalWrite(LED_BUILTIN, true);
  delay(500);
  digitalWrite(LED_BUILTIN, false);
  delay(500);
}
