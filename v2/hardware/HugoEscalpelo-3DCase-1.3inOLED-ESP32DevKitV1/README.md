# Hugo Escalpelo 3D printed Case for 1.3 inches OLED Display

This is a 3D printed Case for 1.3 inches OLED Display specifically for the ESP32 DevKit V1. 

This case was designed in Fusion 360 for a prototype perforated PCB. Instructions, images, circuits, modifications and part list are included in this file.

This version has some button misalignment, be aware of that.



## Requirements
You need the following parts. Reference images of parts and dimentions are provided in `img` folder.

| Quantity | Part name |
|----------|------|
| 1 | ESP32 DevKit V1 |
| 1 | 128x64 1.3" I2C OLED Display |
| 9 | 12mm x 12mm Push Button with Cap |
| 1 | Keyes Rotary Encoder Module |
| 1 | Female header |
| 4 | M3x10mm Screw |
| 3 | M3x20mm Screw |
| 1 | M3x25mm Screw |
| 4 | M3 Nut |
| 1 | 90x150mm Universal PCB |

## 3D printed parts
The files are provided in `.stl` for direct 3D printing and `.step` for modifications.

| Quantity | Part name |
|----------|-----------|
| 1 | Base |
| 1 | Top |
| 1 | Faceplate |
| 1 | Wheel |

All the parts can be printed without support, don't forget to rotate them with the most flat part facing down.

This was originally tested on Creality K1 printer and Creality Print Software with both, Normal and High Quality default Settings. PLA filament was used, any rigid filament should do the trick.

If you have an Open Cage Printer, 5 milimiters of brim is needed for Base and Top parts.

Parts need aproximatelly 250gr of filament.

A Blank Faceplate is provided for you to customize. An Audiomáquinas Faceplate is provided to remember this design was first for a workshop in audiomaquinas.com.

## Circuit
This 3D printed case only fits with a very specific placement of the components in the Universal PCB as Shown in the `Circuit` images. Be sure to use the exact same placement. Wiring is more flexible and can be done as you see fit.

An Schematic image and a `.sch` file is provided in the same folder.

## Assembly
You can see images of the assembly in the `Assembly` folder.

## Code Modifications
For the 2.0 version you need to go to the `\aciduino\v2\AciduinoV2\src\ports\esp32\` path and change the following in the `wroom.h` file.

### Navigation Method
Activate encoder navitation.

```
// main navigation
//#define USE_CHANGER_POT
#define USE_CHANGER_ENCODER
```
Activate select pins for ESP32 DevKit V1
```
// going to use changer encoder?
#define NAV_ENCODER_DEC_PIN       35
#define NAV_ENCODER_INC_PIN       34
```
