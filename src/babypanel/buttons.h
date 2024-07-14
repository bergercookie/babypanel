/**
 * Setup the physical buttons connected to GPIO pins
 */
#pragma once

#include <AceButton.h>
#include <Arduino.h>

using namespace ace_button;

// TimerButtonConfig -------------------------------------------------------------------------------
/**
 * custom button configuration to encode the timer
 */
class TimerButtonConfig : public ButtonConfig
{
public:
  TimerButtonConfig();

  void setTimerId(int timerId);
  int getTimerId() const;

private:
  int m_timerId;
};

// Note regarding ESP8266 Feather Huzzah:
//
// Don't use GPIO #15 -  It is used to detect boot-mode. It has a pulldown resistor connected to it,
// make sure this pin isn't pulled high on startup.
// Same ofr GPIO #16 - seems it's always set on low and cannot set it to INPUT_PULLUP

// button configuration ----------------------------------------------------------------------------
extern const int BUTTON_PINS[];
extern const char* BUTTON_COLORS[];
extern const char* BUTTON_DESCRIPTIONS[];

extern TimerButtonConfig BUTTON_CONFIGS[];
extern AceButton ACE_BUTTONS[];

constexpr const char* kBreastMilkJSON = "\"method\":\"both breasts\",\"type\":\"breast milk\"";
constexpr const char* kFormulaMilkJSON = "\"method\":\"bottle\",\"type\":\"formula\"";

// callbacks ---------------------------------------------------------------------------------------
void feedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState, const char* feedJSON);
void breastFeedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState);
void formulaFeedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState);
void diaperCb(AceButton* btn, uint8_t eventType, uint8_t buttonState);
void startEndRequestCb(AceButton* btn, uint8_t eventType, uint8_t buttonState, const char* url,
                       const char* jsonExtra = nullptr);
void tummyTimeCb(AceButton* btn, uint8_t eventType, uint8_t buttonState);

void sleepCb(AceButton* btn, uint8_t eventType, uint8_t buttonState);

// more helper functions
// ----------------------------------------------------------------------------
void connectToWifiBasedOnEventType(uint8_t eventType);

void lightSleepBasedOnEventType(uint8_t eventType);

void handleEvent(AceButton* btn, uint8_t eventType, uint8_t buttonState);

// setup GPIO pins ---------------------------------------------------------------------------------
void setupGPIOPins();

// check buttons -----------------------------------------------------------------------------------
void checkButtons();
