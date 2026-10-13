# Aciduino V2

Put your DIY super powers to the test building your own Roland's TB-303 and/or TB-808 step sequencer! An Arduino based clone project specially hacked with corrosive features to improve live performances! Save your money building the classic and timeless TB-303 and TR-808 step sequencer clone faster than a blink of an eye.

This version is multi-architecture, allowing for the use of different microcontrollers in the build. Currently, it supports Arduino Mega, any Teensy >= 3.2, and ESP32(tested on wroom dev kit only for now).

Aciduino V2 is also the first official [uMODULAR](https://github.com/midilab/uMODULAR) app, leveraging uMODULAR schematics, [uCtrl](https://github.com/midilab/uCtrl) drivers/interface library, and [uClock](https://github.com/midilab/uClock) BPM clock generator.

# Download

The recommended way for aciduino users is to make use of release pack on [Releases area](https://github.com/midilab/aciduino/releases), make sure to download the one named aciduino-v2_xxx.zip, where xxx is the version released.  
  
If you want to get the latest on the edge features and keep in track with development of aciduino you can make use of git version via clone.  
  
As this version utilizes uCtrl and uClock libraries as submodules, please remember to initialize the submodules immediately after cloning the repository.  

```console
you@acidlab:~$ git clone https://github.com/midilab/aciduino.git
you@acidlab:~$ cd aciduino/
you@acidlab:~$ git submodule update --init --recursive
```  

Open v2/AciduinoV2/AciduinoV2.ino and uncomment only one of the following microcontroller ports depending on wich one you're going to use.

```c
...
//
// Select your platform port
//
//#include "src/ports/esp32/wroom.h"
//#include "src/ports/esp32/s3.h"
#include "src/ports/avr/mega.h"
//#include "src/ports/teensy/protoboard.h"
//#include "src/ports/esp32/wroom-ext1.h"
//#include "src/ports/teensy/uone.h"
//#include "src/ports/avr/midilab_mega.h"
//#include "src/ports/avr/afourtrackmind_mega.h"
...
```

# Compile

Know to work environments:  

ESP32: Espressif Arudino Core >= 3.0.5  
Teensy: Teensyduino Version == 1.57.3
s

# SD card

Aciduino can use a microSD card as its main storage. When a card is present and holds aciduino data, it takes priority over the internal EEPROM; without a card everything runs from EEPROM exactly as before.

If you insert a card with a filesystem the device cannot read (or a card that was never formatted), the device shows a **format** prompt at boot: press **F1** to format the card (this wipes everything on it) or **F2** to keep using EEPROM. After formatting, boot continues on the card as usual.

- **Unlimited patterns**: the pattern grid grows on demand. Navigate past the last saved pattern and a new empty pattern slot appears ("new"); it only becomes a pattern once you save it.
- **Sessions**: a session bundles the global settings (generative, MIDI map, track routing, tempo, shuffle and the pattern grid) together with its own set of patterns. Save, fork ("new") and load different sessions from the system page.
- **Migration**: on the first boot with a card that has no aciduino data, whatever lives in EEPROM is automatically copied into a session on the card. EEPROM is left untouched and stays as the fallback when the card is removed.
- **Track remix**: unchanged — you can still load a single track pattern from any pattern into any track, the same way EEPROM storage always worked.

## Preparing the card

Use a microSD card **up to 32GB**, formatted as **FAT32** (cards this size ship pre-formatted as FAT32 and every system below can re-create that format natively). Larger cards work if you force FAT32 with a third party tool, but offer no benefit here — one pattern uses 712 bytes.

**Windows**: right-click the card in Explorer → *Format…* → File system **FAT32** → Start. Windows refuses FAT32 above 32GB; for larger cards use the free portable [FAT32 Format (guiformat)](https://ridgecrop.co.uk/index.htm?guiformat.htm). The [SD Card Formatter](https://www.sdcard.org/downloads/formatter/) from the SD Association is also fine (it formats cards ≤32GB as FAT32).

**macOS**: Disk Utility → select the card → *Erase* → Format **MS-DOS (FAT)**.

**Linux**:

```console
you@acidlab:~$ sudo umount /dev/sdX1
you@acidlab:~$ sudo parted /dev/sdX mklabel msdos
you@acidlab:~$ sudo parted -a optimal /dev/sdX mkpart primary fat32 1MiB 100%
you@acidlab:~$ sudo mkfs.fat -F 32 -s 64 /dev/sdX1
```

(replace `sdX` with your card device — double check the device name, these commands erase whatever they are pointed at). The SD Card Formatter above also runs on Linux.

## On card layout

Aciduino keeps its files under a single folder at the card root, all plain binary files you can copy around as backups:

```
/ACIDUINO/
  S000.BIN   session 0 image: settings + all its patterns
  S000.CFG   session 0 extras: tempo, shuffle and pattern grid
  S001.BIN   session 1 image ...
  S001.CFG   session 1 extras ...
  LAST.BIN   remembers the last active session across reboots
```

A session image (`.BIN`) mirrors the EEPROM layout: a 2 byte layout marker, the session settings block and then one 712 byte slot per pattern, in order. Saving the first pattern that does not exist yet appends a new slot to the file; there is no upper limit other than free card space.

s
## How does it looks like?

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/aciduino_v2_teensy_bb.jpg)

![Image of aciduino teensy on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/aciduino_lite_v2-teensy.png)

![Image of aciduino mega on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/aciduino_v2-avr_mega_bb.png)

![Image of aciduino esp32 wroom on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/aciduino_v2-esp32_bb.png)  

Aciduino v2 brings new acid elements such as 808 step sequencer and a generative engine based on euclidian algorithm for great acid drums ideas.

A oled screen to get access to all parameters that matters in realtime, a easy to edit grid system for 808 and 303.

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/aciduino-v2-808-grid.jpg)

[Click here for more info](https://github.com/midilab/aciduino/tree/master/v2/)

## How to use?

Please reffer to [user manual](https://github.com/midilab/aciduino/tree/master/v2/docs/aciduinoV2-midi_sequencer-user_manual.md)

## Assembly

Place the buttons, knob and powerlines

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/step1.jpg)

You need to increase the size of two of the hole to place the pot base support on top (+) connect grid line.

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/step1_pot_placement.jpg)

Place signal lines for buttons

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/step2.jpg)

# uMODULAR uOne

One of the uMODULAR projects to run your Aciduino V2, soon avaliable.

![Image of aciduino on uone front](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/uone_umodular.jpg)

![Image of aciduino on uone back](https://raw.githubusercontent.com/midilab/aciduino/master/v2/hardware/imgs/uone_back.jpeg)
