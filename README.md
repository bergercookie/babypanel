# `BabyPanel`

The goal of this project is to setup a control panel to enable logging
baby-related actions (e.g., feeding, tummy time, diaper changes) in [Baby
Buddy](https://github.com/babybuddy/babybuddy) using physical buttons and using
the Wi-Fi enabled [Huzzah ESP8266 board](https://www.adafruit.com/product/2821).

Here's how the final product looks like:

![BabyPanel](https://github.com/bergercookie/babypanel/blob/master/share/images/with_descriptions.jpg)

![BabyPanel With Battery Installed](https://github.com/bergercookie/babypanel/blob/master/share/images/battery_installation.jpg)

> [!NOTE]
> The current project was built to suit my specific needs so it might not suit
> yours. You should be able to tweak it to your liking but you should be able to
> comfortable to edit and upload the code.

## Code setup

You have to define a series of variables so that the babypanel knows how to:

- Connect to the local Wi-Fi network
- Communicate with the Baby Buddy server
- Communicate with the heartbeat server

Define these settings in a `user-conf.h` file at the root of this repo. A
placeholder file that you can copy and edit exists in
`src/babypanel/user-conf.h`.

## Compilation and upload

I'm using [arduino-cli](https://github.com/arduino/arduino-cli) to compile and
upload the code to the board. You can use the helper script `compile.sh` to do that.

You can also use `picocom` to connect to the board and see the logs with a
command like the following

```bash
picocom -b 115200 /dev/ttyUSB0
```

## Heartbeat setup

In order to detect that the battery on the BabyPanel has dried out, you can
setup a heartbeat server that will receive a message from the BabyPanel every 30
minutes. If the server does not receive a message within a designated interval
(by default 2 hours) it will send a message a [ntfy.sh](https://ntfy.sh/)
channel of your choosing.

To install this heartbeat listener, you can use the `install.sh` script from the
`heartbeat_listener` directory, and pass it the name of the channel to send the
notifications to in case of a missed heartbeat.

```bash
cd heartbeat_listener
sudo ./install.sh favorite_ntfy_channel
```

You can then verify that this channel is embedded in the `heartbeat_listener.sh`
script by looking at `/usr/local/bin/heartbeat_listener.sh` where it should be
installed. You can also verify that the listener is working by querying it with
systemctl

```bash
sudo systemctl status heartbeat_listener
```

## Physical setup

- The babypanel is powered by a 3.7V `Li-ion` battery - With a battery of capacity
  `~3400mAh`, the babypanel can last for about 2 days.
- The shell is made of wood and I'm using arcade buttons, [like
  these](https://www.skroutz.gr/s/44854777/Haitronic-Diakoptis-Mpouton-Kokkino-HS1038R.html).
- The `ESP8266` board is connected to the buttons using jumper wires, in a pull-up
  configuration, i.e., one end of the button is connected to the ground, via a
  resistor and the other end is connected to the corresponding pin on the
  `ESP8266` board.

Here's picture of the back of the panel, showing the wiring of the buttons:

![BabyPanel Wiring](https://github.com/bergercookie/babypanel/blob/master/share/images/schematic.png)

And here's a schematic of the wiring if made on a breadboard:

![BabyPanel Schematic](https://github.com/bergercookie/babypanel/blob/master/share/images/wiring.jpg)

### Pin configuration

Following is the pin configuration that I've for the HUZZAH `ESP8266` board:

- `D0` - Breast Feed
- `D2` - Tummy Time
- `D12` - Diaper Change
- `D13` - Sleep - Green
- `D14` - Formula Feed
