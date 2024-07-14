#include "core_esp8266_features.h"
#include <user_interface.h>

#include "esp.h"

void lightSleep()
{
  // IMPORTANT! delay to give the wifi thread some time to write any pending data
  delay(1000);

  wifi_station_disconnect();
  wifi_set_opmode_current(NULL_MODE);
  // set sleep type, the above posters wifi_set_sleep_type() didnt seem to work for me although it
  // did let me compile and upload with no errors
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);

  // Enables force sleep
  wifi_fpm_open();

  // GPIO_ID_PIN(2) corresponds to GPIO2 on ESP8266-01 , GPIO_PIN_INTR_LOLEVEL for a logic low, can
  // also do other interrupts, see gpio.h above
  gpio_pin_wakeup_enable(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);

  wifi_fpm_do_sleep(0xFFFFFFF); // Sleep for longest possible time
}
