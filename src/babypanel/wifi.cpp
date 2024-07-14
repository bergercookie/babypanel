#include "wifi.h"
#include "common.h"
#include "esp.h"

/**
 * Statically initialized WiFiClient instance to use across the application
 */
BBBDClient kBBBDClient = BBBDClient();

// clang-format off
const String kBabybuddyRequestHeaderPreamble =
                 String("Host: ") + BABYBUDDY_SERVER_URL + "\n" +
                 "Accept-Encoding: gzip, deflate, br\n" +
                 "Connection: keep-alive\n" +
                 "User-Agent: " + UserAgent + "\n" +
                 "Accept: application/json, */*;q=0.5\n" +
                 "Content-Type: application/json\n" +
                 "Authorization: Token " + BABYBUDDY_TOKEN;
// clang-format on

const String kBabyBuddyChildIdJson = String("{\"child\": ") + STR(BABYBUDDY_CHILD_ID) + "}";
const String kBabyBuddyTagJson = String("\"tags\":\"[\\\"") + UserAgent + +"\\\"]\"";

const char* HttpMethodStrs[] = {"GET", "POST", "PUT", "DELETE"};

const char* HTTPMethodStr(HTTPMethod method) { return HttpMethodStrs[static_cast<int>(method)]; }

void connectToWifi(const int totalAttempts)
{
  // Set WiFi mode to station (client)
  WiFi.mode(WIFI_STA);

  const auto wifiSSID = WIFI_SSID;
  const auto wifiPassword = WIFI_PASSWORD;

  WiFi.begin(wifiSSID, wifiPassword);
  delay(500);

  // Wait until the connection has been confirmed before continuing
  int currAttempt = 0;
  while (WiFi.status() != WL_CONNECTED && currAttempt != totalAttempts)
  {
    delay(5000);

    DEBUG_PRINT(" - Connecting to WiFi ");
    DEBUG_PRINT(wifiSSID);
    DEBUG_PRINT(", attempt #");
    DEBUG_PRINT(currAttempt);
    DEBUG_PRINTLN(" ...");

    currAttempt += 1;
  }

  // Print out information about the connection
  DEBUG_PRINT("Connected to ");
  DEBUG_PRINT(wifiSSID);
  DEBUG_PRINT(" | IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
}

Response::Response(const char* headers, const char* body) : headers(headers), body(body) {}
Response::Response() {}

size_t Response::printTo(Print& p) const
{
  size_t size = 0;
  size += p.print("Headers: \n");
  size += p.print(headers);
  size += p.print("\nBody: \n");
  size += p.print(body);
  return size;
}

BBBDClient::BBBDClient() : WiFiClient() {}
bool BBBDClient::connect()
{
  DEBUG_PRINT("Connecting to ");
  DEBUG_PRINT(BABYBUDDY_SERVER_ADDR);
  DEBUG_PRINTLN(" ...");
  if (!WiFiClient::connect(BABYBUDDY_SERVER_ADDR, BABYBUDDY_SERVER_PORT))
  {
    DEBUG_PRINT("Connection failed");
    return false;
  }

  return true;
}

Response BBBDClient::makeRequest(HTTPMethod method, const char* url, const String* jsonBody,
                                 bool waitForResponse)
{
  // request -------------------------------------------------------------------------------------
  String methodStr = String(HTTPMethodStr(method));
  const int bodyLen = jsonBody->length();
  DEBUG_PRINT("Making HTTP request, method: ");
  DEBUG_PRINT(methodStr);
  DEBUG_PRINT(" | url: ");
  DEBUG_PRINTLN(url);

  this->println(methodStr + " " + url + " HTTP/1.1\n" + kBabybuddyRequestHeaderPreamble + "\n"
                + "Content-Length: " + bodyLen + "\n\n" + *jsonBody);

  DEBUG_PRINTLN("HTTP Request sent");

  Response response;
  String headers;

  // response ------------------------------------------------------------------------------------
#ifndef HTTP_ALWAYS_WAIT_FOR_RESPONSE_OVERRIDE
  if (!waitForResponse)
  {
    DEBUG_PRINTLN("Returning immediately, won't wait for response");

    // aparently I have to read at least a bit from the response for the POST request to go
    // through and have an effect ?!
    readStringUntil('\n');

    return response;
  }
#endif

  String line;
  while (line != "\r")
  {
    line = readStringUntil('\n');
    headers += line;
  }
  response.headers = std::move(headers);
  response.body = std::move(readStringUntil('\n'));
  announce("HTTP Response", response);

  return response;
}

int BBBDClient::createTimer()
{
  String url = "/api/timers/";
  const auto response = makeRequest(HTTPMethod::POST, url.c_str(), &kBabyBuddyChildIdJson, true);

  // parse the response to get the timer id
  JsonDocument doc;
  deserializeJson(doc, response.body);

  return doc["id"];
}

void BBBDClient::decideSendHeartbeat()
{
  if (millis() - lastMillis > HEARTBEAT_PERIOD_S * 1000)
  {
    connectAndSendHeartbeat();
    lastMillis = millis();
  }
}

void BBBDClient::sendHeartbeat()
{
  DEBUG_PRINT("Sending heartbeat to ");
  DEBUG_PRINT(HEARTBEAT_SERVER_ADDR);
  DEBUG_PRINT(":");
  DEBUG_PRINTLN(HEARTBEAT_SERVER_PORT);

  // send the heartbeat --------------------------------------------------------------------------
  WiFiUDP udp;

  // always set the local port regardless whether you want to listen to incoming data or just want
  // to send data
  udp.begin(HEARTBEAT_LOCAL_UDP_PORT);

  // begin
  {
    int resp = udp.beginPacket(HEARTBEAT_SERVER_ADDR, HEARTBEAT_SERVER_PORT);
    if (resp == 0)
    {
      DEBUG_PRINTLN("Failed to begin packet");
      return;
    }
  }

  udp.write("\n");

  // end
  {
    int resp = udp.endPacket();
    if (resp == 0)
    {
      DEBUG_PRINTLN("Failed to end packet");
      return;
    }
  }
}

void BBBDClient::connectAndSendHeartbeat()
{
  connectToWifi();
  sendHeartbeat();
  lightSleep();
}
