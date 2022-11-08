
#ifndef __U_CTRL_MODULES_H__
#define __U_CTRL_MODULES_H__

// wichg hardware version you're using?
// using lite board?
#define USE_LITE_BOARD
// using uOne board?
//#define USE_UONE_BOARD

// specific hardware setup for lite board
#if defined(USE_LITE_BOARD)
  #define USE_DOUT
  // always multiple of 8
  #define USE_DOUT_MAX_PORTS  8
  
  #define USE_DIN
  // always multiple of 8
  #define USE_DIN_MAX_PORTS  16
  
  #define USE_AIN
  #define USE_AIN_MAX_PORTS  1
#endif

// specific hardware setup for uOne board
#if defined(USE_UONE_BOARD)
  #define USE_DOUT
  #define DOUT_SPI_DRIVER
  #define USE_DOUT_MAX_PORTS  24
  
  #define USE_DIN
  #define USE_DIN_MAX_PORTS  8
  
  #define USE_CAP_TOUCH
  #define USE_CAP_TOUCH_MAX_PORTS  32
#endif

#define USE_OLED
#define USE_OLED_U8G2

//#define USE_MIDI

//#define USE_CV_GATE

//#define USE_SDCARD

#define USE_PAGE
#define USE_PAGE_MAX_PAGES  4
#define USE_PAGE_MAX_SUB_PAGES  2
#define USE_PAGE_COMPONENT

#endif
