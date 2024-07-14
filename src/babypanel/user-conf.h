// Description: User configuration file for the babypanel project

/**
#define BAUD_RATE 115200
#define WIFI_SSID "<name-of-wifi>"
#define WIFI_PASSWORD "<wifi-password>"

# server name or IP that babybuddy is running on
#define BABYBUDDY_SERVER_ADDR "<server-ip>"
# server port that babybuddy is running on
#define BABYBUDDY_SERVER_PORT <port>
# token for babybuddy - see https://docs.baby-buddy.net/api/#authentication
#define BABYBUDDY_TOKEN "<token>"
# ID of the child in babybuddy we're logging events for
#define BABYBUDDY_CHILD_ID 1

# heartbeat configuration
# how often to send a heartbeat to the server to let it know we're still alive (and have not run out
# of battery. See the heartbeat_listener.py script in for the server side of this
#define HEARTBEAT_SERVER_ADDR "<server-ip>"
# port that the heartbeat server is listening for heartbeats on
#define HEARTBEAT_SERVER_PORT 12000

# local port on the ESP8266 to send heartbeats from
#define HEARTBEAT_LOCAL_UDP_PORT 8888
# how often to send a heartbeat in seconds
#define HEARTBEAT_PERIOD_S 1800 // 30 mins
*/
