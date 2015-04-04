## core.sh - event aggregator and dispatcher for bash

### Motivation

Linux desktop computers provide many event sources. For example, changing
between power sources (AC and battery) triggers an event, docking and undocking
a laptop triggers another event. Even switching on an external monitor causes an
udev event.

Several command line tools for monitoring those events exist. For instance there
are `acpi_listen`, `dbus-monitor` and `udevadm` with the `monitor` option. Each
of these programs will block until an event occurs and then print it out, one
per line. Using shell scripts and regular expressions it is possible to capture
the output and run other actions.

This is a more versatile and simpler approach than to use the according APIs
themselves. It might be however limiting to what one wants to achieve. However,
it is suffficient for simple task, for example to automatically dim the screen
when AC power changes to battery.

### Installation

Run `make install`. The default `PREFIX` is `/usr`, which can be overriden by
running `PREFIX=/my/install/path`. I'm using `~/.local`.

This will install the `evagg` binary to `${PREFIX}/lib/evagg` and the `core.sh`
script to `${PREFIX}/bin/core.sh`.

### Usage

To run `core.sh` the shell environment variables `SRCS_D` and `SINKS_D` need to
be set.

`SRCS_D`: A directory with executable files (`chmod +x`) which wait on an event
source and print out events to `stdout`. For example a shell script which simply
calls `acpi_listen`.

`SINKS_D`: A directory with executable files (`chmod +x`) which will be called
with every event as argument. For example, consider the scripts `10-a.sh`,
`10-b.sh` and `10-c.sh`. An event (`some event`) produced by one of the sources
in `SRCS_D` will then cause a call to `10-a.sh some event`, `10-b.sh some event`
and `10-c.sh some event`.

### Examples

TBD
