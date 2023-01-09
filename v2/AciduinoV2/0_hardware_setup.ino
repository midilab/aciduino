//
// Pinout configuration
//
// Pre-defined released schematic pinout
// for protoboard teensy 3.2+, protoboard avr mega, uOne and mega shield
//
// if you need change pinout, please do
// it under USE_CUSTOM_PINOUT definitions
// and ucomment the line bellow:
//#define USE_CUSTOM_PINOUT
#define USE_PROTOBOARD_PINOUT
//#define USE_UONE_PINOUT
//#define USE_MEGA_SHIELD_PINOUT

//
// USE_CUSTOM_PINOUT
//
#if defined (USE_CUSTOM_PINOUT)

  // going to use changer encoder?
  //#define NAV_ENCODER_DEC_PIN       24
  //#define NAV_ENCODER_INC_PIN       25
  
  #define NAV_SHIFT_PIN             4
  
  #define NAV_FUNCTION1_PIN         6
  #define NAV_FUNCTION2_PIN         7
  #define NAV_GENERAL1_PIN          3
  #define NAV_GENERAL2_PIN          5
  #define NAV_RIGHT_PIN             15
  #define NAV_UP_PIN                14
  #define NAV_DOWN_PIN              9
  #define NAV_LEFT_PIN              16
  
  // going to use changer pot?
  #define CHANGER_POT_PIN           A9

  // any module pinout extension?
  // POT 16
  #define POT_CTRL_PIN1             45
  #define POT_CTRL_PIN2             47
  #define POT_CTRL_PIN3             49
  #define POT_MUX_COMM1             A0
  #define POT_MUX_COMM2             A1

//
// PROTOBOARD pinout for teensy and mega
//
#elif defined(USE_PROTOBOARD_PINOUT)

  //
  // TEENSYDUINO generic
  //
  #if defined(TEENSYDUINO)
  
    // going to use changer encoder?
    //#define NAV_ENCODER_DEC_PIN       24
    //#define NAV_ENCODER_INC_PIN       25
    
    #define NAV_SHIFT_PIN             4
    
    #define NAV_FUNCTION1_PIN         6
    #define NAV_FUNCTION2_PIN         7
    #define NAV_GENERAL1_PIN          3
    #define NAV_GENERAL2_PIN          5
    #define NAV_RIGHT_PIN             15
    #define NAV_UP_PIN                14
    #define NAV_DOWN_PIN              9
    #define NAV_LEFT_PIN              16
    
    // going to use changer pot?
    #define CHANGER_POT_PIN           A9

  //
  // ESP32 family
  //
  #elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)

    // the blue led
    #define LED_BUILTIN               2

    // going to use changer encoder?
    //#define NAV_ENCODER_DEC_PIN       5
    //#define NAV_ENCODER_INC_PIN       18
    
    #define NAV_SHIFT_PIN             12
    
    #define NAV_FUNCTION1_PIN         14
    #define NAV_FUNCTION2_PIN         27
    #define NAV_GENERAL1_PIN          33
    #define NAV_GENERAL2_PIN          32
    #define NAV_RIGHT_PIN             5
    #define NAV_UP_PIN                18
    #define NAV_DOWN_PIN              19
    #define NAV_LEFT_PIN              23
    
    // going to use changer pot?
    // GPIO 0 - ADC2_1
    #define CHANGER_POT_PIN           0
  
  //
  // ARDUINO_ARCH_AVR Mega 2560 generic
  //
  #elif defined(ARDUINO_ARCH_AVR)
  
    // going to use changer encoder?
    #define NAV_ENCODER_DEC_PIN       26
    #define NAV_ENCODER_INC_PIN       28
    
    // or use changer pot?
    #define CHANGER_POT_PIN           A8
    
    #define NAV_SHIFT_PIN             13
    
    #define NAV_FUNCTION1_PIN         12
    #define NAV_FUNCTION2_PIN         11
    #define NAV_GENERAL1_PIN          10
    #define NAV_GENERAL2_PIN          6
    #define NAV_RIGHT_PIN             5
    #define NAV_UP_PIN                4
    #define NAV_DOWN_PIN              3
    #define NAV_LEFT_PIN              2
    
  #endif

//
// Mega 2560 uModular shield
//
#elif defined(USE_MEGA_SHIELD_PINOUT)

  // going to use changer encoder?
  #define NAV_ENCODER_DEC_PIN       26
  #define NAV_ENCODER_INC_PIN       28
  
  // or use changer pot?
  #define CHANGER_POT_PIN           A8
  
  #define NAV_SHIFT_PIN             13
  
  #define NAV_FUNCTION1_PIN         12
  #define NAV_FUNCTION2_PIN         11
  #define NAV_GENERAL1_PIN          10
  #define NAV_GENERAL2_PIN          6
  #define NAV_RIGHT_PIN             5
  #define NAV_UP_PIN                4
  #define NAV_DOWN_PIN              3
  #define NAV_LEFT_PIN              2
  
  // the other controls are handled
  // by touch driver
  
  // Extension modules
  // PUSH 
  #define PUSH_LATCH_PIN            9
  // LED
  #define LED_LATCH_PIN             8
  // POT
  #define POT_CTRL_PIN1             45
  #define POT_CTRL_PIN2             47
  #define POT_CTRL_PIN3             49
  #define POT_MUX_COMM1             A0
  #define POT_MUX_COMM2             A1

//
// uOne board
//
#elif defined(USE_UONE_PINOUT)

  // going to use changer encoder?
  // teensy lc
  #define NAV_ENCODER_DEC_PIN       24
  #define NAV_ENCODER_INC_PIN       25
  // teensy 3.2+
  // on 3.2+ there are no digital port only analog at 24 and 25 pins :(
  //#define NAV_ENCODER_DEC_PIN       A10
  //#define NAV_ENCODER_INC_PIN       A11
  // swap per A6, A7? make nav encoder dec=20(A6) inc=21(A7) on rev 0.2 uone

  #define NAV_SHIFT_PIN             23

  // the other controls are handled
  // by touch driver
  
  // Extension modules
  // LED
  #define LED_LATCH_PIN             17
  // TOUCH
  // 4 and 3 needs a swap, fix it on rev 0.2
  #define TOUCH_CTRL_PIN1           6
  #define TOUCH_CTRL_PIN2           5
  #define TOUCH_CTRL_PIN3           3
  #define TOUCH_CTRL_PIN4           4
  #define TOUCH_MUX_COMM1           A6
  #define TOUCH_MUX_COMM2           A7

#endif
