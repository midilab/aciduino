# Hugo Escalpelo 3D printed Case for 2.4 inches OLED Display

This is a 3D printed Case for 2.4 inches OLED Display specifically for the ESP32 DevKit V1. 

This case was designed in Fusion 360 for a prototype universal PCB. Instructions, images, circuits, modifications and part list are included in this file.

This version also includes files for a customized PCB with Gerber and Drill Files.

## Requirements
You need the following parts. Reference images of parts and dimentions are provided in `img` folder.

| Quantity | Part name |
|----------|------|
| 1 | ESP32 DevKit V1 |
| 1 | 128x64 2.4" I2C OLED Display |
| 9 | 5mm x 5mm Push Button |
| 1 | Keyes Rotary Encoder Module |
| 1 | Female header 4 pin |
| 2 | Female header 15 pin |
| 4 | M3x10mm Screw |
| 4 | M3x28mm Screw |
| 4 | M3 secure Nut |
| 2 | M2x8mm Screw |
| 2 | M2 Insert |
| 1 | 90x150mm Universal PCB |

## 3D printed parts
The files are provided in `.stl` for direct 3D printing and `.step` for modifications. Files are in ther respective folder.

| Quantity | Part name |
|----------|-----------|
| 1 | Base |
| 1 | Top |
| 1 | Wheel |
| 1 | Buttons |
| 2 | Separator |

All the parts can be printed without support, don't forget to rotate them with the most flat part facing down.

This was originally tested on Creality K1 printer and Creality Print Software with both, Normal and High Quality default Settings. PLA filament was used, any rigid filament should do the trick.

If you have an Open Cage Printer, 5 milimiters of brim is needed for Base and Top parts.

Parts need aproximatelly 250gr of filament.

This version does not need push buttons with caps, a plate of 3D printed buttons is included. Separators are needed to fit buttons properly along screws.

## Circuit
This 3D printed case only fits with a very specific placement of the components in the Universal PCB as Shown in the `Circuit` images. Be sure to use the exact same placement. Wiring is more flexible and can be done as you see fit.

An Schematic image and a `.sch` file is provided in the same folder. Gerber and Drill Files are provied for a Custom PCB.

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
