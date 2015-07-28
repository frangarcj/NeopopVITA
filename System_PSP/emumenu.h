/* NeoPop - Menu definitions */

#ifndef _EMUMENU_H
#define _EMUMENU_H

int  InitMenu();
void DisplayMenu();
void TrashMenu();

#define DISPLAY_MODE_UNSCALED    0
#define DISPLAY_MODE_FIT_HEIGHT  1
#define DISPLAY_MODE_FILL_SCREEN 2
#define DISPLAY_MODE_2X 3
#define DISPLAY_MODE_3X 4
#define DISPLAY_MODE_3X_BG 5

#define JOY 0x100
#define SPC 0x200

#define CODE_MASK(x) (x & 0xff)

#define SPC_MENU               1

#define MAP_BUTTONS            18

#define MAP_ANALOG_UP          0
#define MAP_ANALOG_DOWN        1
#define MAP_ANALOG_LEFT        2
#define MAP_ANALOG_RIGHT       3
#define MAP_BUTTON_UP          4
#define MAP_BUTTON_DOWN        5
#define MAP_BUTTON_LEFT        6
#define MAP_BUTTON_RIGHT       7
#define MAP_BUTTON_SQUARE      8
#define MAP_BUTTON_CROSS       9
#define MAP_BUTTON_CIRCLE      10
#define MAP_BUTTON_TRIANGLE    11
#define MAP_BUTTON_LTRIGGER    12
#define MAP_BUTTON_RTRIGGER    13
#define MAP_BUTTON_SELECT      14
#define MAP_BUTTON_START       15
#define MAP_BUTTON_LRTRIGGERS  16
#define MAP_BUTTON_STARTSELECT 17

typedef struct
{
  int ShowFps;
  int ControlMode;
  int ClockFreq;
  int DisplayMode;
  int VSync;
  int UpdateFreq;
  int Frameskip;
} EmulatorOptions;

struct ButtonConfig
{
  unsigned int ButtonMap[MAP_BUTTONS];
};

#endif // _EMUMENU_H
