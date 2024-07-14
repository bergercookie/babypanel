#!/usr/bin/env bash
# Install the ./heartbeat_listener.* scripts under
# /usr/local/bin and the ./heartbeat_listener.service under /etc/systemd/system
# and enable the service.
# The NTFY channel that we use is passed as the first argument to this script
# and we add it to the heartbeat_listener.sh script before installing it.

set -e

# if we're not root exit
if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root"
  exit 1
fi

# set the first argument to this script to the NTFY variable. if it's not given,
# raise an error
if [ -z "$1" ]; then
  echo "Usage: $0 <ntfy-channel-to-use>"
  exit 1
fi
NTFY_CHANNEL="$1"

# switch to the directory of the script
(
  cd "$(dirname "$0")"

  cat ./heartbeat_listener.sh | sed "s/NTFY_CHANNEL=.*/NTFY_CHANNEL=\"$NTFY_CHANNEL\"/" > /usr/local/bin/heartbeat_listener.sh
  cp -v heartbeat_listener.py /usr/local/bin/
  cp -v heartbeat_listener.service /etc/systemd/system/
)
systemctl enable --now heartbeat_listener.service
