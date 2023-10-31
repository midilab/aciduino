# ACIDUINO V2 User Manual

## Introduction

Up to 16 Tracks TB303/TR808 sequencer with step, pattern and mute automation grid system, MIDI Controller, OLED display and extensible modular design to plug more potentiometers, buttons, MIDI or CV/Gate interfaces.
a friend of mine some time ago had come out with this one:
"Put your DIY super powers to the test building your own Roland’s TB-303 and TB-808 step sequencer! An Arduino based clone project specially hacked with corrosive features for live performances! Save your money building this classic sequencer clone faster than a blink of an eye."



## BEFORE GETTING STARTED

To use aciduino, follow these steps:

1. Read the documentation [here](https://github.com/midilab/aciduino/tree/master/v2/)
2. Build your device [following these steps](https://github.com/midilab/aciduino/tree/master/v2#assembly)
3. Download the latest [software](https://github.com/midilab/aciduino/releases)
4. Check your hardware configuration in `/AciduinoV2/0_hardware_setup.ino`
5. Ajust the sequencer to your taste in `/AciduinoV2/src/sequencer/setup.h`

## USER INTERFACE
If you followed the assembly guide you should have a device like this :

![MIDI Sequencer Interface](images/BasicControls.png)

The GUI is divided into the following main sections:

- **Header bar:** Contains the Track number <span style="color: white; background-color: black;">T1-T5</span> and type (303 or 808), the <span style="color: white; background-color: black;">page name</span> and selected subpage (1 or 2), the <span style="color: white; background-color: black;">clock source</span> and tempo value (40-180).
- **Subpage display:** where the actual tweaking occurs.
- **Function buttons label:** Provides a clear feedback for the 2 function buttons



## WORKFLOW

You can navigate through the different <span style="color: white; background-color: black;">| parameters</span> by clicking the arrow keys `UP`, `DOWN`, `LEFT`, `RIGHT`. The current selected item is <blink>blinking</blink>.
General buttons are to be used aside the potentiometer as increment or increment buttons, depending on the page or selected item they are also used to activate / desactivate steps or patterns, mutes. We will describe them in their corresponding page.

To switch between pages, hold the `Shift ^` button and press `UP` or `DOWN` button. 

1. ## System Page <span style="color: white; background-color: black;">syst</span>
![MIDI Sequencer Interface](images/SystPage1.png)
![MIDI Sequencer Interface](images/SystPage2.png)
 
 - **tempo:** set uClock tempo from 40 to 180,when selected F1= rec and F2 = play
 - **clock:** choose clock source, internal, midi1 or midi2
 - **shuffle:** turn the groove on or off
 - **sig.:** set the swing signature (3 MPC signatures, user signatures can be added)
 - **rec:** input source midi1 or midi2 
 - **mode:** record in real time or stepmode
 - **session:** save your ideas on eeprom or SDCard


2. ## Sequencer page <span style="color: white; background-color: black;">seqr</span>
 ![MIDI Sequencer Interface](images/Seqr303Page1.png)
 ![MIDI Sequencer Interface](images/Seqr303Page2.png)

 

 - **Selected item:** <table>
  <tr>
    <th>Common</th>
    <th>303</th>
    <th>808</th>
  </tr>
  </tr>
     <td> **length:**  </td>
     <td> 0 to whatever you defined in `/AciduinoV2/src/sequencer/setup.h` </td>
     <td> 0 to 64 for a 808 track. Note that each voice of a 808 track can have a different sequence length </td>
  </tr>
  <tr>
    <th> </th>
    <td>**Voice:** set the tonic note</td>
    <td>**Note:** set the drum part midi note number</td>
  </tr>
  <tr>
    <td>**Shift :** offset the whole sequence forward or backward</td>
    <td> </td>
    <td> </td>
  </tr>
   <tr>
    <td> </td>
    <td>**transpose :** transpose the tonic note</td>
    <td>**roll:** set the roll type</td>
  </tr>
  <tr>
  <td> **Fill:** </td>
  <td> </td>
  <td></td>
  </tr>
</table>


3. ## Generator page <span style="color: white; background-color: black;">gene</span>
 ![MIDI Sequencer Interface](images/Gene303Page1.png)
 ![MIDI Sequencer Interface](images/Gene303Page2.png)
 

 - **Header bar:** 

4. ## Pattern page <span style="color: white; background-color: black;">ptrn</span>
 ![MIDI Sequencer Interface](images/PtrnPage1.png)
  ![MIDI Sequencer Interface](images/PtrnPage2.png)

 - **Header bar:** Contains
5. ## MIDI page <span style="color: white; background-color: black;">midi</span>
 ![MIDI Sequencer Interface](images/MidiPage1.png)

 - **Midi CCs** they are sent on the selected Track MIDI channel, CC# and naming can be changed in in `/AciduinoV2/page_midi.ino` : look for     "midiControllerComponent.set303Control("filter", 74);"

 

## Cheatsheet
![MIDI Sequencer Interface](images/cheatsheet.png)

## Support


Check the [official midilab website](https://midilab.co/)  for the latest news


For any inquiries or technical support, please open an [issue](https://github.com/midilab/aciduino/issues)

For some nice talking, ideas or questions you can say hi on our  [discord channel](https://discord.com/channels/1137685010967703582/1137685011416498236)


---
© 2023 Aciduino. All rights reserved.
