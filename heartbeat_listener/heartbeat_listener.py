#!/usr/bin/env python3

"""
The heartbeat listener at the start creates a simple server that listens for heartbeat messages
from an arbitrary client on a periodic basis. If it doesn't receive the said heartbeat message
within the designated time it will send a notification to a specific ntfy channel so that the
user is aware of this.
"""


import logging
import threading
import argparse
import socket
import datetime
from typing import Literal, Mapping, Sequence
import requests
import sys

# flag set if we receive a heartbeat message --------------------------------------------------
# Always acquire the lock before accessing this variable
_prog_name = sys.argv[0].split("/")[-1]


NtfyPriorityT = Literal["max", "high", "default", "low", "min"]


class HeartbeatMonitor:
    """A heartbeat monitoring class to check for heartbeats."""

    DEFAULT_VERBOSITY_LVL = logging.WARNING

    def __init__(
        self,
        port: int,
        ntfy_channel: str,
        verbosity_lvl: int,
        client_description: str,
        heartbeat_check_interval: datetime.timedelta = datetime.timedelta(hours=5),
    ):
        """
        Initialize the heartbeat monitor.

        :param port: The port to listen on.
        :param ntfy_channel: The ntfy_channel channel to send notifications to
        in case the heartbeat_check_interval is exceeded.
        :param heartbeat_check_interval: The interval to check for heartbeats.
        """

        self.port = port
        self.ntfy_channel = ntfy_channel
        self.heartbeat_check_interval = heartbeat_check_interval
        self.logger = logging.getLogger(self.__class__.__name__)
        self.client_description = client_description

        # heartbeat related configuration
        self.last_heartbeat_time = datetime.datetime.now()
        self.heartbeat_received_lock = threading.Lock()
        self.heartbeat_timer: threading.Timer = self._create_new_timer()

        # store whether the last heartbeat was missed
        self.last_heartbeat_missed = False

        # setup logging
        self._setup_logger(verbosity_lvl)

        self.logger.debug("Initialized HeartbeatMonitor.")

    def _create_new_timer(self) -> threading.Timer:
        """Create a new timer."""
        return threading.Timer(
            self.heartbeat_check_interval.total_seconds(),
            self._check_heartbeat_or_notify,
        )

    def start(self) -> None:
        """Start the heartbeat monitor.

        - Block indefinitely on the control loop - abort if ctrl-c is pressed.
        - Start a timer to check for heartbeats every heartbeat_check_interval.
        """
        self.logger.debug("Calling HeartbeatMonitor.start() ...")

        self.logger.debug("Starting control loop thread to check for heartbeats ...")
        control_loop_thread = threading.Thread(target=self._run_control_loop)
        control_loop_thread.start()
        self.logger.debug("Control loop thread started.")

        # start a timer to check for heartbeats every heartbeat_check_interval
        # if a heartbeat is not received within this interval, send a notification to the ntfy channel
        self.logger.debug(
            "Starting a timer to inform the user in case of missed heartbeats | Heartbeat check interval"
            f" {self.heartbeat_check_interval} ..."
        )
        self.heartbeat_timer.start()
        self.logger.debug("Heartbeat timer started.")

        self.logger.info("All set, waiting for heartbeats. Press Ctrl-c to exit.")
        try:
            control_loop_thread.join()
        except KeyboardInterrupt:
            self.logger.info("Ctrl-c pressed, exiting.")
            self.heartbeat_timer.cancel()

    def _run_control_loop(self):
        """Run the control loop.

        Block until a heartbeat message is received.
        When a heartbeat message is received, set the heartbeat_received flag.
        """
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        server_socket.bind(("", self.port))

        while True:
            message, address = server_socket.recvfrom(1024)
            del address
            del message

            with self.heartbeat_received_lock:
                self.logger.debug(
                    "Received heartbeat message at %s, resetting last heartbeat time.",
                    datetime.datetime.now(),
                )
                self.last_heartbeat_time = datetime.datetime.now()

    def _check_heartbeat_or_notify(self):
        """Check if a heartbeat message was received within the heartbeat_check_interval.

        If a heartbeat message was not received, send a notification to the ntfy channel.
        This function is executed from the main function every heartbeat_check_interval using a
        threading timer.
        """
        try:
            with self.heartbeat_received_lock:
                time_since_last_heartbeat = (
                    datetime.datetime.now() - self.last_heartbeat_time
                )
                if time_since_last_heartbeat < self.heartbeat_check_interval:
                    # if we had missed the last heartbeat, reset the flag and inform the user that all is good now
                    if self.last_heartbeat_missed:
                        self.last_heartbeat_missed = False
                        logging.warning(
                            f"Connection to client restored. Last heartbeat received at {self.last_heartbeat_time}."
                        )

                        self.send_to_ntfy(
                            msg=(
                                f"* Connection to client restored\n"
                                f"* Client app: {self.client_description}\n"
                                f"* Last heartbeat received at {self.last_heartbeat_time.strftime('%Y%m%d %H:%M:%S')}"
                            ),
                            title=f"Heartbeat restored - {self.client_description}",
                            priority="default",
                            tags=["green_heart"],
                        )
                    return

                # heartbeat delta exceeded - send a notification --------------------------------------
                self.last_heartbeat_missed = True
                logging.warning(
                    f"Delta for heartbeat exceeded!\n\n"
                    f"- Last heartbeat received at {self.last_heartbeat_time}.\n"
                    f"- Time since last heartbeat: {time_since_last_heartbeat}.\n"
                    "\n"
                    f"Sending notification to ntfy.sh channel -> {self.ntfy_channel} ..."
                )

                # send a notification to the ntfy channel `ntfy_channel`
                self.send_to_ntfy(
                    msg=(
                        f"* Heartbeat delta of {self.heartbeat_check_interval} exceeded\n* Client app: "
                        f"{self.client_description}\n* Last heartbeat received at {self.last_heartbeat_time.strftime('%Y%m%d %H:%M:%S')}"
                    ),
                    title=f"Heartbeat missed - {self.client_description}",
                    priority="high",
                    tags=["rotating_light", "heartbeat"],
                )

        finally:
            self.heartbeat_timer = self._create_new_timer()
            self.heartbeat_timer.start()

    def send_to_ntfy(
        self, msg: str, title: str, priority: NtfyPriorityT, tags: Sequence[str]
    ):
        """Send a message to the ntfy channel."""
        resp = requests.post(
            f"https://ntfy.sh/{self.ntfy_channel}",
            data=msg,
            headers={
                "Title": title,
                "Priority": priority,
                "Tags": ",".join(tags),
            },
        )

        if not resp.ok:
            logging.error(
                f"Failed to send notification to ntfy.sh channel {self.ntfy_channel}. | "
                f"Status code: {resp.status_code} | "
                f"Response text: {resp.text}"
            )

    @staticmethod
    def _setup_logger(logging_level: int):
        """Setup the logger."""
        # Use the HH:MM:SS <logging-level format
        logging.basicConfig(
            format="%(asctime)s | %(levelname)-8s | %(message)s",
            level=logging_level,
            datefmt="%Y%m%d %H:%M:%S",
        )


_verbosity_count_to_logging_level = {
    0: logging.WARNING,
    1: logging.INFO,
    2: logging.DEBUG,
}


def verbosity_count_to_logging_level(verbosity_count: str) -> int:
    """Convert the verbosity count to a logging level."""
    count = int(verbosity_count)
    if count < 0:
        raise ValueError(f"Verbosity count cannot be negative, got {verbosity_count}.")
    if count > 2:
        count = 2

    return _verbosity_count_to_logging_level.get(count, logging.WARNING)


def parse_time_interval(time_interval: str) -> datetime.timedelta:
    """Parse a time interval string and return a timedelta object."""
    if not time_interval:
        raise ValueError("Time interval cannot be None or empty.")

    time_interval = time_interval.strip().lower()
    if time_interval[-1] not in ["s", "m", "h", "d"]:
        raise ValueError(
            f"Invalid time interval format, expected one of [s, m, h, d], got {time_interval[-1]}."
        )

    time_value = int(time_interval[:-1])
    time_unit = time_interval[-1]

    if time_unit == "s":
        return datetime.timedelta(seconds=time_value)
    if time_unit == "m":
        return datetime.timedelta(minutes=time_value)
    if time_unit == "h":
        return datetime.timedelta(hours=time_value)
    if time_unit == "d":
        return datetime.timedelta(days=time_value)

    raise ValueError(f"Invalid time unit {time_unit}.")


# run -----------------------------------------------------------------------------------------
class Formatter(argparse.ArgumentDefaultsHelpFormatter, argparse.RawTextHelpFormatter):
    pass


def main():
    parser = argparse.ArgumentParser(
        description="Heartbeat listener", formatter_class=Formatter
    )

    parser.add_argument(
        "--ntfy",
        dest="ntfy_channel",
        type=str,
        help="The ntfy channel to send notifications to",
        default="kalimera",
        required=True,
    )
    parser.add_argument(
        "--port", type=int, help="The local port to listen on", default=12000
    )
    # verbosity argument. If the user passes -v It's INFO, -vv is DEBUG. make it a count argument
    parser.add_argument(
        "-v",
        "--verbose",
        dest="verbose",
        action="count",
        help="Increase verbosity of the logger",
    )

    parser.add_argument(
        "--beat",
        "--heartbeat-check-interval",
        dest="heartbeat_check_interval",
        type=str,
        help="The interval to check for heartbeats",
        default="5h",
    )
    parser.add_argument(
        "--client-description",
        type=str,
        help="The description of the client application for which the heartbeat is being monitored",
        default="client",
    )

    # print some sample usage examples
    epilog_examples: Mapping[str, str] = {
        "Use defaults for everything": f"{_prog_name}",
        "Set the ntfy channel": f"{_prog_name} --ntfy ntfy_channel",
        "Set the ntfy channel and port": f"{_prog_name} --ntfy ntfy_channel --port 12000",
        "Set the ntfy channel and a heartbeat check interval": f"{_prog_name} --ntfy ntfy_channel --heartbeat-check-interval 5h",
        "Set a heartbeat check interval of 5 minutes": f"{_prog_name} --heartbeat-check-interval 5m",
        "Set a heartbeat check interval of 10 seconds": f"{_prog_name} --heartbeat-check-interval 10s",
    }
    longest_epilog_example_key = max(map(len, epilog_examples.keys()))

    parser.epilog = f'Example usages:\n{"=" * 30}\n\n' + "\n".join(
        # indent all the examples by the length of the longest key + 2
        f"  {key.ljust(longest_epilog_example_key)} : {value}"
        for key, value in epilog_examples.items()
    )

    args = parser.parse_args()
    if args.verbose is None:
        args.verbose = HeartbeatMonitor.DEFAULT_VERBOSITY_LVL
    else:
        args.verbose = verbosity_count_to_logging_level(args.verbose)

    HeartbeatMonitor(
        port=args.port,
        ntfy_channel=args.ntfy_channel,
        verbosity_lvl=args.verbose,
        heartbeat_check_interval=parse_time_interval(args.heartbeat_check_interval),
        client_description=args.client_description,
    ).start()


if __name__ == "__main__":
    main()
