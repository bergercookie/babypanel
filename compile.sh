#!/usr/bin/env bash
set -ex


(
  cd "$(dirname "$0")"
  cd src/babypanel

  # build compile_commands.json to use it with clangd for easier development
  arduino-cli compile --fqbn esp8266:esp8266:huzzah --only-compilation-database --build-path ../../build/ --library ../../ babypanel.ino

  arduino-cli compile --fqbn esp8266:esp8266:huzzah --library ../../ babypanel.ino
  arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:huzzah babypanel.ino
)
