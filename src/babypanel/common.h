/**
 * Functionality common to all modules.
 */
#pragma once

// debug printing functions
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

// headers
#define H1 "========================"
#define H2 "------------------------"

/**
 * @name Message announcing fns
 */
//@{
template <typename T>
void announce(const char* title, const T& obj)
{
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN(H1);
  DEBUG_PRINT(title);
  DEBUG_PRINTLN(": ");
  DEBUG_PRINTLN("");

  DEBUG_PRINTLN(obj);
  DEBUG_PRINTLN(H1);
}

template <typename T>
void announce(const char* title, const T* obj)
{
  announce(title, *obj);
}
