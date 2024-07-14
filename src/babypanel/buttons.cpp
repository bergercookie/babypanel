#include "buttons.h"
#include "common.h"
#include "esp.h"
#include "wifi.h"

const int BUTTON_PINS[] = {0, 2, 12, 13, 14};

const char* BUTTON_COLORS[] = {
    "PURPLE", "RED", "BLACK", "GREEN", "YELLOW",
};

const char* BUTTON_DESCRIPTIONS[] = {
    "Breast Feed", "Tummy Time", "Diaper Change", "Sleep", "Formula Feed",
};

TimerButtonConfig BUTTON_CONFIGS[] = {TimerButtonConfig(), TimerButtonConfig(), TimerButtonConfig(),
                                      TimerButtonConfig(), TimerButtonConfig()};
AceButton ACE_BUTTONS[]
    = {AceButton(&BUTTON_CONFIGS[0], BUTTON_PINS[0]), AceButton(&BUTTON_CONFIGS[1], BUTTON_PINS[1]),
       AceButton(&BUTTON_CONFIGS[2], BUTTON_PINS[2]), AceButton(&BUTTON_CONFIGS[3], BUTTON_PINS[3]),
       AceButton(&BUTTON_CONFIGS[4], BUTTON_PINS[4])};

TimerButtonConfig::TimerButtonConfig() : ButtonConfig() {}

void TimerButtonConfig::setTimerId(int timerId) { this->m_timerId = timerId; }
int TimerButtonConfig::getTimerId() const { return m_timerId; }

// feed callbacks ----------------------------------------------------------------------------------
void feedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState, const char* feedJSON)
{
  switch (eventType)
  {
  case AceButton::kEventClicked:
  case AceButton::kEventReleased:
  case AceButton::kEventDoubleClicked:
    break;
  default:
    return;
  }

  // print the button description
  const int btnId = btn->getId();
  DEBUG_PRINT(BUTTON_DESCRIPTIONS[btnId]);

  if (!kBBBDClient.connect())
  {
    return;
  }

  // I don't want to signal both the start and end of the breast feed so on start, I'll create a
  // timer, then immediately use it to make a valid breast feed request

  // create a timer
  const auto timerId = kBBBDClient.createTimer();

  // make breast feed request
  const char* url = "/api/feedings/";

  const String body
      = String("{\"timer\":\"") + timerId + "\"," + feedJSON + "," + kBabyBuddyTagJson + "}\r";
  kBBBDClient.makeRequest(HTTPMethod::POST, url, &body);
}

void breastFeedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  feedCb(btn, eventType, buttonState, kBreastMilkJSON);
}

void formulaFeedCb(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  feedCb(btn, eventType, buttonState, kFormulaMilkJSON);
}

// callback for black ------------------------------------------------------------------------------
void diaperCb(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  const char* diaperContents;
  switch (eventType)
  {
  case AceButton::kEventClicked:
  case AceButton::kEventReleased:
    diaperContents = "\"wet\":\"false\",\"solid\":\"true\"";
    break;
  case AceButton::kEventDoubleClicked:
    diaperContents = "\"wet\":\"true\",\"solid\":\"false\"";
    break;
  default:
    return;
  }

  // print the button description
  const int btnId = btn->getId();
  DEBUG_PRINTLN(BUTTON_DESCRIPTIONS[btnId]);

  if (!kBBBDClient.connect())
  {
    return;
  }

  // make diaper request
  const char* url = "/api/changes/";

  const String body = String("{\"child\":") + STR(BABYBUDDY_CHILD_ID) + "," + diaperContents + ","
                      + kBabyBuddyTagJson + "}\r";
  kBBBDClient.makeRequest(HTTPMethod::POST, url, &body);
}

// supplementary callback for activities with a clear start and end --------------------------------

void startEndRequestCb(AceButton* btn, uint8_t eventType, uint8_t buttonState, const char* url,
                       const char* jsonExtra)
{
  // get the activity description
  const int btnId = btn->getId();
  const char* description = BUTTON_DESCRIPTIONS[btnId];

  switch (eventType)
  {
  case AceButton::kEventClicked:
  case AceButton::kEventReleased:
  {
    DEBUG_PRINT(description);
    DEBUG_PRINTLN(" start");

    // create timer, assign it to the button configuration
    DEBUG_PRINTLN("Creating timer ...");
    if (!kBBBDClient.connect())
    {
      return;
    }

    static_cast<TimerButtonConfig*>(btn->getButtonConfig())->setTimerId(kBBBDClient.createTimer());
    return;
  }
  case AceButton::kEventDoubleClicked:
  {
    DEBUG_PRINT(description);
    DEBUG_PRINTLN(" end");

    // retrieve the timer id from the button configuration
    const int timerId = static_cast<TimerButtonConfig*>(btn->getButtonConfig())->getTimerId();
    if (timerId == 0)
    {
      DEBUG_PRINTLN(
          "No timer found, we probably never started the activity in the first place. Exiting");
      return;
    }

    // set the timer to 0 in the button configuration to mark that there's no active timer
    static_cast<TimerButtonConfig*>(btn->getButtonConfig())->setTimerId(0);

    // make tummy time request end request
    if (!kBBBDClient.connect())
    {
      return;
    }

    String body = String("{\"timer\":\"") + timerId + "\" ";

    if (jsonExtra != nullptr)
    {
      body += ",";
      body += jsonExtra;
    }
    body += "," + kBabyBuddyTagJson + "}\r";

    kBBBDClient.makeRequest(HTTPMethod::POST, url, &body);

    break;
  }
  default:
    return;
  }
}

// callback for red --------------------------------------------------------------------------------
void tummyTimeCb(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  return startEndRequestCb(btn, eventType, buttonState, "/api/tummy-times/", nullptr);
}

// callback for green ------------------------------------------------------------------------------
void sleepCb(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  return startEndRequestCb(btn, eventType, buttonState, "/api/sleep/", nullptr);
}

// helper methods to connect to wifi and go back to sleep ------------------------------------------
void connectToWifiBasedOnEventType(uint8_t eventType)
{
  switch (eventType)
  {
  case AceButton::kEventClicked:
  case AceButton::kEventDoubleClicked:
  case AceButton::kEventReleased:
    DEBUG_PRINTLN("Connecting to the wifi ...");
    connectToWifi();
    break;
  default:
    return;
  }
}

void lightSleepBasedOnEventType(uint8_t eventType)
{
  switch (eventType)
  {
  case AceButton::kEventClicked:
  case AceButton::kEventDoubleClicked:
  case AceButton::kEventReleased:
    DEBUG_PRINTLN("Going back to sleep");
    lightSleep();
    break;
  default:
    return;
  }
}

// generic event handler that delegates to the appropriate callback --------------------------------
void handleEvent(AceButton* btn, uint8_t eventType, uint8_t buttonState)
{
  connectToWifiBasedOnEventType(eventType);

  // get the button id and dispatch accordingly
  if (btn->getId() == 0)
  {
    breastFeedCb(btn, eventType, buttonState);
  }
  else if (btn->getId() == 1)
  {
    tummyTimeCb(btn, eventType, buttonState);
  }
  else if (btn->getId() == 2)
  {
    diaperCb(btn, eventType, buttonState);
  }
  else if (btn->getId() == 3)
  {
    sleepCb(btn, eventType, buttonState);
  }
  else if (btn->getId() == 4)
  {
    formulaFeedCb(btn, eventType, buttonState);
  }

  lightSleepBasedOnEventType(eventType);
}

// -------------------------------------------------------------------------------------------------

void setupGPIOPins()
{
  // common setup for all buttons
  for (int i = 0; i < 5; i++)
  {
    AceButton& b = ACE_BUTTONS[i];
    b.init(BUTTON_PINS[i], HIGH, i);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);

    ButtonConfig* bc = ACE_BUTTONS[i].getButtonConfig();
    bc->setFeature(ButtonConfig::kFeatureDoubleClick);
    bc->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
    bc->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
    bc->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);

    bc->setEventHandler(handleEvent);
  }
}

void checkButtons()
{
  for (int i = 0; i < 5; i++)
  {
    ACE_BUTTONS[i].check();
  }
}
