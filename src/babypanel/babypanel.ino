#include "buttons.h"
#include "conf.h"
#include "wifi.h"

#include "esp.h"

void setup()
{
  Serial.begin(BAUD_RATE);

  // setup button pins
  setupGPIOPins();

  // go to light sleep and wait for button presses
  lightSleep();
}

void loop() {
  kBBBDClient.decideSendHeartbeat();
  checkButtons(); }
