#!/usr/bin/env bash
NTFY_CHANNEL=TODO
/usr/local/bin/heartbeat_listener.py --ntfy $NTFY_CHANNEL -vvvv --beat 2h --port 12000
