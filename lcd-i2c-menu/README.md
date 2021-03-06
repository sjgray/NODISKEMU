I²C-LCD-PWM-MENU
================

The code in this directory is meant to run on a second AVR chip,
connected to the one running NODISKEMU over I2C plus one additional line
called IntRequest.

It is only meant as an example.  To adapt the code for a different
display, change config.h as noted in its comments and adjust the
layout in main.c (status line length, positions of the address /
partition fields, if changing the number of lines also edit
`set_part()`, every call to `set_line` and `set_line_part`, `case
DISPLAY_ADDRESS:` branch in `parse_display`).

The two-chip design was chosen to minimize the resource requirements
on the drive side. Although a simple status-only display could be
implemented just on the drive AVR, the ram requirements for a
directory selection menu can be rather high if there are many entries
with a high average length. On an ATmega324 or other 2K ram chips it
should be possible to display 100 entries with an average length of
12, on an ATmega644 the average length is increased to more than 30
for the same number of entries.

The drive side has no knowledge of the presentation method and/or
layout used by the display side, it just sends status information and
menu entries to the display which is free to use or discard any of
the status informations as required. Menus are only shown after the
user interacts with the display in which case the display AVR uses the
IntRequest line to signal the drive side. The drive side does not
remember which data it sent for the menu entries, so it may expect to
read them back exactly as sent for further processing.


Random Protocol Details
-----------------------

The display uses I2C address 0x64 (already shifted, so writing uses
0x65) to communicate. The first byte written is the command number,
everything after that are command-specific parameters. Whenever a
command requires a string parameter, the string is sent in PETSCII
encoding. Strings are NOT zero-terminated unless noted. Partition
numbers are 0-based, to get the user-visible number just add 1.


Menus
-----
The model for menus used in the communication between drive and
display is an ordered list of strings, one for each menu entry. Menu
entries must be stored on the display side, although entries may be
dropped if the local memory is exhausted. The buffered menu entries
are cleared by the `DISPLAY_MENU_RESET` command, entries are added one
by one using the `DISPLAY_MENU_ADD` command. The order of menu entries
should be preserved both for reasonable presentation to the user and
for returning the correct entry number to the drive. Menu entries are
sent as PETSCII, the only character in PETSCII that is not in ASCII
that should really be supported is the left arrow (code 0x5f) because
it is used in directory listings as a shorthand for the parent
directory.


Commands
--------

The format of the command description:
```
<enum name>: <numerical value sent>
Parameter(s): <parameters included>
```

Parameter types are given as "string" (PETSCII character string,
NOT zero-terminated unless noted otherwise) or "byte" (single byte
value, unsigned).


```
DISPLAY_INIT: 0
Parameter: string version
```

This command is issued during startup and should clear the display,
display the version string (if desired) and leave any menu that may
still be active. It is recommended to lower-case the version string
before displaying it.


```
DISPLAY_ADDRESS: 1
Parameter: byte address
```

This command is issued during startup and whenever the IEC address the
device is listening on changes.


```
DISPLAY_FILENAME_READ: 2
Parameters: byte partition, string filename
```

This command is issued when a file is opened for reading. The
partition byte is the partition the file is on, the file name is the
actual file name that is opened with wildcards already resolved. The
file name is always zero-terminated; additional data may be sent after
the terminating zero-byte, but should be ignored.


```
DISPLAY_FILENAME_WRITE: 3
Parameters: byte partition, string filename
```

This command works exactly as `DISPLAY_FILENAME_READ`, but is issued
when a file is opened for writing or appending or if the file is a REL
file.


```
DISPLAY_DOSCOMMAND: 4
Parameter: string command
```

This command is issued when the drive receives a command on the
command channel and transmits the command as-is. Please note that some
dos commands may contain binary data (e.g. `M-W` and `P`), so please
filter the data as required to make sure your output device isn't
confused by random data sent to it. The command string is NOT
zero-terminated.


```
DISPLAY_ERRORCHANNEL: 5
Parameter: string message
```

This command is issued whenever the error channel conents change,
which may be multiple times during the execution of a single drive
operation. The message is sent without the final 0x0d character, but
otherwise exactly as it is stored in the message buffer on the drive
side.


```
DISPLAY_CURRENT_DIR: 6
Parameters: byte partition, string name
```

This command is issued when the current directory on a partition
changes and transmits the affected partition number as well as the
name of the new directory. The name is the same as the string in the
header line of a directory listing.


```
DISPLAY_CURRENT_PART: 7
Parameter: byte partition
```

This command is issued when the current partition is changed using the
dos command "CP".


```
DISPLAY_MENU_RESET: 64
Parameter: nothing
```

This command resets the menu. The display should return to the status
screen (the data for that must be saved internally) and clear out the
menu entry buffer.


```
DISPLAY_MENU_ADD: 65
Parameter: string entry
```

This command adds a new menu entry to the buffer. The entry string is
always zero-terminated. If the menu buffer is full (too many lines or
not enough memory to store another string) it may be ignored. Adding
menu entres while a menu is currently displayed is not required.


```
DISPLAY_MENU_SHOW: 66
Parameter: byte position
```

This command switches from the status display to the menu display
using the menu entries in the buffer. The position parameter specifies
which entry should be selected initially and is 0-based. It is used
for the device address menu to select the current address as default.


```
DISPLAY_MENU_GETSELECTION: 67
Parameter: nothing
```

This command is issued to read the entry number (0-based) of the
currently selected entry. The number is returned as a single byte value
on the next read on address 0x64.


```
DISPLAY_MENU_GETENTRY: 68
Parameter: nothing
```

This command is issued to read the entry string of the currently
selected entry. The string is returned without zero-termination in
PETSCII encoding on the next read on address 0x64. The data in the
menu buffer may be modified to do this as the menu will be reset soon
after this command.


Interrupt Request Line
----------------------

The interrupt request line is used by the display to signal the drive
that it has received user interaction that needs the attention of the
drive. It should be pulled low until an I2C read to the display's
address has been issued. The current display implementation issues an
interrupt request whenever the user pushes the button connected to the
display controller, the drive will either set up a menu or read a menu
selection in response.
