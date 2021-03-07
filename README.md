## ttysh - A dumb terminal dealing with serial communication devices

It is a dumb terminal to connect your machine to a "server"
machine using serial communication ports.

This program addresses a simple problem:

One tries to log in a Linux box using terminal emulation but
curses based and other terminal applications produce a good load 
of mess on the screen. Panik striken - one tries to escape the
application by pressing Fxx function keys.
A lot more of mess - including irreversible data loss...

We know that experts can solve this problem within the hour,
but I'D LIKE TO USE MC HERE AND NOW!

The best way is not using terminal emulation at all.
Here's the tool for this. It does nothing just put stdin/stdout/ttySx
devices into raw-binary mode, and pass all the way in data to the
other sides.
There are no MODEM connect/hangup/dial, nor TERM maintenance.

### Requirements

#### At the "terminal" side** where ttysh runs:

- one free serial port (e.g. /dev/ttyS1 on Linux, \\\\.\\com1 on Windows)
- operable - no getty, mgetty, mice or others on it (check out /etc/inittab, and lsof)
- the default TERM setting for the console - typically **linux** - will do

#### At the "server" side - just for a login prompt

- a serial port (e.g. /dev/ttyS1) hanging on getty/mgetty or similar.

Check out */etc/inittab* for something like this:
```
# Example how to put a getty
# on a serial line (for a terminal)
#
#T0:23:respawn:/sbin/getty -L ttyS0 9600 vt100
 T1:23:respawn:/sbin/getty -h -L ttyS1 115200
```

### Building/Installing

It requires a Linux system, with MINGW installed for Windows port.

#### For Linux OS:
```
make -f Makefile.linux
```
The resulted **ttysh** can be deployed to /usr/bin, or wherever you want.

#### For Windows:
```
make -f Makefile.win32
```
The resulted **ttysh.exe** can be deployed to Program Files, or wherever you want.

### Usage

```
Usage: ttysh device [options]
Notes:
 - device can be a COM port, e.g. /dev/ttyS0
 - defaults for COMx are 115200,8,N,1
 - use Ctrl+Q (cQ) to escape by default
 - do not merge switches (like -io)

Usage: Options:

   -e, -E, --escape <key> .... Escape character is
           cQ, cX, cs2, cA
           cB, cF, cG, cK
           cL, cR
   -n, -N, --num <num> .... Esc in numeric format
   -s, -S, --speed <key> .... Set the port speed to
           50, 75, 110, 134
           150, 200, 300, 600
           1200, 2400, 4800, 9600
           19200, 38400, 57600, 115200
   -p, -P, --parity <key> .... Set the parity to
           none, even, odd
   -t, -T, --stop <key> .... Set stop bits to
           one, two
   -d, -D, --dtrflow .... DTR/DSR flow control
   -r, -R, --rtsflow .... RTS/CTS flow control
   --dtrstat <num> .... Set DTR state 0/1
   --rtsstat <num> .... Set RTS state 0/1
   -b, -B, --break .... Send break
   -i, -I, --ipipe .... Input is piped
   -o, -O, --opipe .... Output is piped
   -c, -C <path> .... Capture to file
   -w, -W <strg> .... Write string and exit
   --bypass .... Bypass errors
   -m, -M, --marker .... Timestamp markers CR
   --version .... Show version and exit
   -h, -H, --help .... Help and exit
```

That's all...

