#ifndef QGL_KEY_HEADER
#define QGL_KEY_HEADER

#define QGL_PRESS                  1
#define QGL_REPEAT                 2

#define QGL_HAT_CENTERED           0
#define QGL_HAT_UP                 1
#define QGL_HAT_RIGHT              2
#define QGL_HAT_DOWN               4
#define QGL_HAT_LEFT               8
#define QGL_HAT_RIGHT_UP           (QGL_HAT_RIGHT | QGL_HAT_UP)
#define QGL_HAT_RIGHT_DOWN         (QGL_HAT_RIGHT | QGL_HAT_DOWN)
#define QGL_HAT_LEFT_UP            (QGL_HAT_LEFT  | QGL_HAT_UP)
#define QGL_HAT_LEFT_DOWN          (QGL_HAT_LEFT  | QGL_HAT_DOWN)

#define QGL_KEY_UNKNOWN		-1

#define QGL_KEY_SPACE              32
#define QGL_KEY_APOSTROPHE         39  /* ' */
#define QGL_KEY_COMMA              44  /* , */
#define QGL_KEY_MINUS              45  /* - */
#define QGL_KEY_PERIOD             46  /* . */
#define QGL_KEY_SLASH              47  /* / */
#define QGL_KEY_0                  48
#define QGL_KEY_1                  49
#define QGL_KEY_2                  50
#define QGL_KEY_3                  51
#define QGL_KEY_4                  52
#define QGL_KEY_5                  53
#define QGL_KEY_6                  54
#define QGL_KEY_7                  55
#define QGL_KEY_8                  56
#define QGL_KEY_9                  57
#define QGL_KEY_SEMICOLON          59  /* ; */
#define QGL_KEY_EQUAL              61  /* = */
#define QGL_KEY_A                  65
#define QGL_KEY_B                  66
#define QGL_KEY_C                  67
#define QGL_KEY_D                  68
#define QGL_KEY_E                  69
#define QGL_KEY_F                  70
#define QGL_KEY_G                  71
#define QGL_KEY_H                  72
#define QGL_KEY_I                  73
#define QGL_KEY_J                  74
#define QGL_KEY_K                  75
#define QGL_KEY_L                  76
#define QGL_KEY_M                  77
#define QGL_KEY_N                  78
#define QGL_KEY_O                  79
#define QGL_KEY_P                  80
#define QGL_KEY_Q                  81
#define QGL_KEY_R                  82
#define QGL_KEY_S                  83
#define QGL_KEY_T                  84
#define QGL_KEY_U                  85
#define QGL_KEY_V                  86
#define QGL_KEY_W                  87
#define QGL_KEY_X                  88
#define QGL_KEY_Y                  89
#define QGL_KEY_Z                  90
#define QGL_KEY_LEFT_BRACKET       91  /* [ */
#define QGL_KEY_BACKSLASH          92  /* \ */
#define QGL_KEY_RIGHT_BRACKET      93  /* ] */
#define QGL_KEY_GRAVE_ACCENT       96  /* ` */
#define QGL_KEY_WORLD_1            161 /* non-US #1 */
#define QGL_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define QGL_KEY_ESCAPE             256
#define QGL_KEY_ENTER              257
#define QGL_KEY_TAB                258
#define QGL_KEY_BACKSPACE          259
#define QGL_KEY_INSERT             260
#define QGL_KEY_DELETE             261
#define QGL_KEY_RIGHT              262
#define QGL_KEY_LEFT               263
#define QGL_KEY_DOWN               264
#define QGL_KEY_UP                 265
#define QGL_KEY_PAGE_UP            266
#define QGL_KEY_PAGE_DOWN          267
#define QGL_KEY_HOME               268
#define QGL_KEY_END                269
#define QGL_KEY_CAPS_LOCK          280
#define QGL_KEY_SCROLL_LOCK        281
#define QGL_KEY_NUM_LOCK           282
#define QGL_KEY_PRINT_SCREEN       283
#define QGL_KEY_PAUSE              284
#define QGL_KEY_F1                 290
#define QGL_KEY_F2                 291
#define QGL_KEY_F3                 292
#define QGL_KEY_F4                 293
#define QGL_KEY_F5                 294
#define QGL_KEY_F6                 295
#define QGL_KEY_F7                 296
#define QGL_KEY_F8                 297
#define QGL_KEY_F9                 298
#define QGL_KEY_F10                299
#define QGL_KEY_F11                300
#define QGL_KEY_F12                301
#define QGL_KEY_F13                302
#define QGL_KEY_F14                303
#define QGL_KEY_F15                304
#define QGL_KEY_F16                305
#define QGL_KEY_F17                306
#define QGL_KEY_F18                307
#define QGL_KEY_F19                308
#define QGL_KEY_F20                309
#define QGL_KEY_F21                310
#define QGL_KEY_F22                311
#define QGL_KEY_F23                312
#define QGL_KEY_F24                313
#define QGL_KEY_F25                314
#define QGL_KEY_KP_0               320
#define QGL_KEY_KP_1               321
#define QGL_KEY_KP_2               322
#define QGL_KEY_KP_3               323
#define QGL_KEY_KP_4               324
#define QGL_KEY_KP_5               325
#define QGL_KEY_KP_6               326
#define QGL_KEY_KP_7               327
#define QGL_KEY_KP_8               328
#define QGL_KEY_KP_9               329
#define QGL_KEY_KP_DECIMAL         330
#define QGL_KEY_KP_DIVIDE          331
#define QGL_KEY_KP_MULTIPLY        332
#define QGL_KEY_KP_SUBTRACT        333
#define QGL_KEY_KP_ADD             334
#define QGL_KEY_KP_ENTER           335
#define QGL_KEY_KP_EQUAL           336
#define QGL_KEY_LEFT_SHIFT         340
#define QGL_KEY_LEFT_CONTROL       341
#define QGL_KEY_LEFT_ALT           342
#define QGL_KEY_LEFT_SUPER         343
#define QGL_KEY_RIGHT_SHIFT        344
#define QGL_KEY_RIGHT_CONTROL      345
#define QGL_KEY_RIGHT_ALT          346
#define QGL_KEY_RIGHT_SUPER        347
#define QGL_KEY_MENU               348

#define QGL_KEY_LAST               QGL_KEY_MENU

#define QGL_MOD_SHIFT           0x0001
#define QGL_MOD_CONTROL         0x0002
#define QGL_MOD_SUPER           0x0002
#define QGL_MOD_CAPS_LOCK       0x0010
#define QGL_MOD_NUM_LOCK        0x0020

#define QGL_MOUSE_BUTTON_1         0
#define QGL_MOUSE_BUTTON_2         1
#define QGL_MOUSE_BUTTON_3         2
#define QGL_MOUSE_BUTTON_4         3
#define QGL_MOUSE_BUTTON_5         4
#define QGL_MOUSE_BUTTON_6         5
#define QGL_MOUSE_BUTTON_7         6
#define QGL_MOUSE_BUTTON_8         7
#define QGL_MOUSE_BUTTON_LAST      QGL_MOUSE_BUTTON_8
#define QGL_MOUSE_BUTTON_LEFT      QGL_MOUSE_BUTTON_1
#define QGL_MOUSE_BUTTON_RIGHT     QGL_MOUSE_BUTTON_2
#define QGL_MOUSE_BUTTON_MIDDLE    QGL_MOUSE_BUTTON_3

#define QGL_JOYSTICK_1             0
#define QGL_JOYSTICK_2             1
#define QGL_JOYSTICK_3             2
#define QGL_JOYSTICK_4             3
#define QGL_JOYSTICK_5             4
#define QGL_JOYSTICK_6             5
#define QGL_JOYSTICK_7             6
#define QGL_JOYSTICK_8             7
#define QGL_JOYSTICK_9             8
#define QGL_JOYSTICK_10            9
#define QGL_JOYSTICK_11            10
#define QGL_JOYSTICK_12            11
#define QGL_JOYSTICK_13            12
#define QGL_JOYSTICK_14            13
#define QGL_JOYSTICK_15            14
#define QGL_JOYSTICK_16            15
#define QGL_JOYSTICK_LAST          QGL_JOYSTICK_16

#define QGL_GAMEPAD_BUTTON_A               0
#define QGL_GAMEPAD_BUTTON_B               1
#define QGL_GAMEPAD_BUTTON_X               2
#define QGL_GAMEPAD_BUTTON_Y               3
#define QGL_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define QGL_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define QGL_GAMEPAD_BUTTON_BACK            6
#define QGL_GAMEPAD_BUTTON_START           7
#define QGL_GAMEPAD_BUTTON_GUIDE           8
#define QGL_GAMEPAD_BUTTON_LEFT_THUMB      9
#define QGL_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define QGL_GAMEPAD_BUTTON_DPAD_UP         11
#define QGL_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define QGL_GAMEPAD_BUTTON_DPAD_DOWN       13
#define QGL_GAMEPAD_BUTTON_DPAD_LEFT       14
#define QGL_GAMEPAD_BUTTON_LAST            QGL_GAMEPAD_BUTTON_DPAD_LEFT

#define QGL_GAMEPAD_BUTTON_CROSS       QGL_GAMEPAD_BUTTON_A
#define QGL_GAMEPAD_BUTTON_CIRCLE      QGL_GAMEPAD_BUTTON_B
#define QGL_GAMEPAD_BUTTON_SQUARE      QGL_GAMEPAD_BUTTON_X
#define QGL_GAMEPAD_BUTTON_TRIANGLE    QGL_GAMEPAD_BUTTON_Y

#define QGL_GAMEPAD_AXIS_LEFT_X        0
#define QGL_GAMEPAD_AXIS_LEFT_Y        1
#define QGL_GAMEPAD_AXIS_RIGHT_X       2
#define QGL_GAMEPAD_AXIS_RIGHT_Y       3
#define QGL_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define QGL_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define QGL_GAMEPAD_AXIS_LAST          QGL_GAMEPAD_AXIS_RIGHT_TRIGGER

#endif
