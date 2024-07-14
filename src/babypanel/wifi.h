#pragma once

#include "conf.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// supplementary methods and classes ---------------------------------------------------------------

constexpr const char* UserAgent = "BabyBuddyArcadePanel/0.1.0";

extern const String kBabyBuddyTagJson;

/**
 * Connect to the WiFi network
 *
 * @param wifiSSID The SSID of the WiFi network
 * @param wifiPassword The password of the WiFi network
 * @param totalAttempts The number of attempts to connect to the WiFi network.
 * Provide -1 to keep trying until the connection is successful
 */
void connectToWifi(const int totalAttempts = 3);

// HTTP method related -----------------------------------------------------------------------------
/**
 * Enum class to represent the HTTP methods
 */
enum class HTTPMethod
{
  GET = 0,
  POST,
  PUT,
  DELETE
};

/**
 * Get the string representation of the HTTP method
 *
 * @param method The HTTP method
 * @return The string representation of the HTTP method
 */
const char* HTTPMethodStr(HTTPMethod method);

/**
 * array of HTTP method strings
 */
extern const char* HttpMethodStrs[];

// Response class ----------------------------------------------------------------------------------
class Response : public Printable
{
public:
  // set the status, headers and body of the response in the constructor, and add methods to get
  // each one of them
  Response(const char* headers, const char* body);
  Response();

  size_t printTo(Print& p) const override;

  String headers;
  String body;
};

// BBBDClient class -------------------------------------------------------------------------------
/**
 * Utility class to send HTTP requests to the Babybuddy API
 */
class BBBDClient : public WiFiClient
{

public:
  BBBDClient();
  bool connect();
  Response makeRequest(HTTPMethod method, const char* url, const String* jsonBody = nullptr,
                       bool waitForResponse = false);
  int createTimer();
  void decideSendHeartbeat();

private:
  /**
   * Send a heartbeat by sending an empty UDP packet to the
   * HEARTBEAT_SERVER_ADDR:HEARTBEAT_SERVER_PORT
   */
  void sendHeartbeat();
  void connectAndSendHeartbeat();

  int lastMillis = 0;
};

/**
 * Statically initialized WiFiClient instance to use across the application
 */
extern BBBDClient kBBBDClient;
