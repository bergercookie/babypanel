/**
 * Config file that does a bit of post-processing on the user-provided configuration of conf.h
 */

#pragma once

#include <user-conf.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// join the BABYBUDDY_SERVER_ADDR and BABYBUDDY_SERVER_PORT into a single string
#define BABYBUDDY_SERVER_URL BABYBUDDY_SERVER_ADDR ":" STR(BABYBUDDY_SERVER_PORT)

/* #define HTTP_ALWAYS_WAIT_FOR_RESPONSE_OVERRIDE */

#define DEBUG
