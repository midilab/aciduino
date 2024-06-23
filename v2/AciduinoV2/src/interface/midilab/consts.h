
typedef enum {
#if defined(USE_CHANGER_ENCODER)
  ENCODER_DEC,
  ENCODER_INC,
#endif
  // the basic minimun ui buttons (9 total)
  SHIFT_BUTTON,
  PAGE_BUTTON_1,
  PAGE_BUTTON_2,
  GENERIC_BUTTON_1,
  GENERIC_BUTTON_2,
  NEXT_BUTTON,
  UP_BUTTON,
  DOWN_BUTTON, 
  PREVIOUS_BUTTON,
#if defined(USE_TRANSPORT_BUTTON)
  TRANSPORT_BUTTON_1,
#endif

#if defined(USE_PUSH_8) || defined(USE_PUSH_24) || defined(USE_PUSH_32) || defined(USE_TOUCH_32)
  // first extension 8 group
  SELECTOR_BUTTON_1,
  SELECTOR_BUTTON_2,
  SELECTOR_BUTTON_3,
  SELECTOR_BUTTON_4,
  SELECTOR_BUTTON_5,
  SELECTOR_BUTTON_6,
  SELECTOR_BUTTON_7,
  SELECTOR_BUTTON_8,
#endif

#if defined(USE_PUSH_24) || defined(USE_PUSH_32) || defined(USE_TOUCH_32)
  // second extension 8 group
  STEP_BUTTON_1,
  STEP_BUTTON_2,
  STEP_BUTTON_3,
  STEP_BUTTON_4,
  STEP_BUTTON_5,
  STEP_BUTTON_6,
  STEP_BUTTON_7,
  STEP_BUTTON_8,

  // third extension 8 group
  STEP_BUTTON_9,
  STEP_BUTTON_10,
  STEP_BUTTON_11,
  STEP_BUTTON_12,
  STEP_BUTTON_13,
  STEP_BUTTON_14,
  STEP_BUTTON_15,
  STEP_BUTTON_16,
#endif

} BUTTONS_INTERFACE_CONTROLS;

typedef enum {
#if defined(USE_BPM_LED)
  BPM_LED,
#endif

#if defined(USE_LED_8) || defined(USE_LED_24)
  // first extension 8 group leds
  SELECTOR_LED_1,
  SELECTOR_LED_2,
  SELECTOR_LED_3,
  SELECTOR_LED_4,
  SELECTOR_LED_5,
  SELECTOR_LED_6,
  SELECTOR_LED_7,
  SELECTOR_LED_8,
#endif

#if defined(USE_LED_24)
  // second extension 8 group
  STEP_LED_1,
  STEP_LED_2,
  STEP_LED_3,
  STEP_LED_4,
  STEP_LED_5,
  STEP_LED_6,
  STEP_LED_7,
  STEP_LED_8,

  // third extension 8 group
  STEP_LED_9,
  STEP_LED_10,
  STEP_LED_11,
  STEP_LED_12,
  STEP_LED_13,
  STEP_LED_14,
  STEP_LED_15,
  STEP_LED_16,
#endif

} LED_INTERFACE_CONTROLS;

#if defined (USE_POT_8) || defined (USE_POT_16) || defined (USE_POT_MICRO)
#define LEARN_ENABLED
#endif
