#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <psp2/types.h>
#include <psp2/rtc.h>

#include "neopop.h"

#include "emulate.h"
#include "emumenu.h"

#include "psplib/pl_file.h"
#include <psplib/pl_snd.h>
#include <psplib/image.h>
#include <psplib/pl_psp.h>
#include <psplib/video.h>
#include <psplib/pl_perf.h>
#include <psplib/ctrl.h>
#include <psplib/pl_util.h>

PspImage *Screen;
PspImage *BG3X;

static pl_perf_counter FpsCounter;
static int ScreenX, ScreenY, ScreenW, ScreenH;
static int ClearScreen;
static int TicksPerUpdate;
static u32 TicksPerSecond;
static u64 LastTick;
static u64 CurrentTick;
static unsigned char ReturnToMenu;

extern char *GameName;
extern EmulatorOptions Options;
extern const u64 ButtonMask[];
extern const int ButtonMapId[];
extern struct ButtonConfig ActiveConfig;
extern char *ScreenshotPath;

static void AudioCallback(pl_snd_sample* buf,
                          unsigned int samples,
                          void *userdata);
_u8 system_frameskip_key;

/* Initialize emulation */
int InitEmulation()
{
	/* Initialize BIOS */
	if (!bios_install()) return 0;

  /* Basic emulator initialization */
	language_english = TRUE;
	system_colour = COLOURMODE_AUTO;

  /* Create screen buffer */
  if (!(Screen = pspImageCreateVram(256, 256, GU_PSM_4444)))
    	return 0;

  Screen->Viewport.Width = SCREEN_WIDTH;
  Screen->Viewport.Height = SCREEN_HEIGHT;
  cfb = Screen->Pixels;

  pl_file_path background;
  snprintf(background, sizeof(background) - 1, "%sbg3x.png",
           pl_psp_get_app_directory());
  BG3X = pspImageLoadPng2D(background);

  pl_snd_set_callback(0, AudioCallback, NULL);


  return 1;
}

void system_graphics_update()
{
  pspVideoBegin();

  /* Clear the buffer first, if necessary */
  if (ClearScreen >= 0)
  {
    ClearScreen--;
    pspVideoClearScreen();
  }

  /* Blit screen */
  //sceGuDisable(GU_BLEND);
  pspVideoPutImage(Screen, ScreenX, ScreenY, ScreenW, ScreenH);
  //printf("%d %d %d %d", ScreenX, ScreenY, ScreenW, ScreenH);
  //sceGuEnable(GU_BLEND);


  if(Options.DisplayMode==DISPLAY_MODE_3X_BG&&BG3X){
    pspVideoPutImage(BG3X, 0, 0, SCR_WIDTH, SCR_HEIGHT);
  }
  /* Show FPS counter */
  if (Options.ShowFps)
  {
    static char fps_display[32];
    sprintf(fps_display, " %3.02f", pl_perf_update_counter(&FpsCounter));

    int width = pspFontGetTextWidth(&PspStockFont, fps_display);
    int height = pspFontGetLineHeight(&PspStockFont);

    pspVideoFillRect(SCR_WIDTH - width, 0, SCR_WIDTH, height, PSP_COLOR_BLACK);
    pspVideoPrint(&PspStockFont, SCR_WIDTH - width, 0, fps_display, PSP_COLOR_WHITE);
  }

  pspVideoEnd();

  /* Wait if needed */
  if (Options.UpdateFreq)
  {
    do { sceRtcGetCurrentTick(&CurrentTick); }
    while (CurrentTick - LastTick < TicksPerUpdate);
    LastTick = CurrentTick;
  }

  /* Wait for VSync signal */
  if (Options.VSync)
    pspVideoWaitVSync();

  /* Swap buffers */
  pspVideoSwapBuffers();

}

void system_input_update()
{
  _u8 *input_ptr = &ram[0x6F82];

  /* Reset input */
  *input_ptr = 0;

  static SceCtrlData pad;

  /* Check the input */
  if (pspCtrlPollControls(&pad))
  {
#ifdef PSP_DEBUG
    if ((pad.buttons & (PSP_CTRL_SELECT | PSP_CTRL_START))
      == (PSP_CTRL_SELECT | PSP_CTRL_START))
        pspUtilSaveVramSeq(ScreenshotPath, "game");
#endif

    /* Parse input */
    int i, on, code;
    for (i = 0; ButtonMapId[i] >= 0; i++)
    {
      code = ActiveConfig.ButtonMap[ButtonMapId[i]];
      on = (pad.buttons & ButtonMask[i]) == ButtonMask[i];

      /* Check to see if a button set is pressed. If so, unset it, so it */
      /* doesn't trigger any other combination presses. */
      //if (on) pad.buttons &= ~ButtonMask[i];

      if (code & JOY)
      {
        if (on) *input_ptr |= CODE_MASK(code);
        continue;
      }
      else if (code & SPC)
      {
        switch (CODE_MASK(code))
        {
        case SPC_MENU:
          ReturnToMenu = on; break;
        }
      }
    }
  }
}

/*! Called at the start of the vertical blanking period */
void system_VBL(void)
{
	/* Update Graphics */
  if (frameskip_count == 0)
    system_graphics_update();

	/* Update Input */
	system_input_update();

	/* Sound update performed in the callback */
}

/* Run emulation */
void RunEmulation()
{
  float ratio;

  /* Recompute screen size/position */
  switch (Options.DisplayMode)
  {
  default:
  case DISPLAY_MODE_UNSCALED:
    ScreenW = Screen->Viewport.Width;
    ScreenH = Screen->Viewport.Height;
    break;
  case DISPLAY_MODE_FIT_HEIGHT:
    ratio = (float)SCR_HEIGHT / (float)Screen->Viewport.Height;
    ScreenW = (float)Screen->Viewport.Width * ratio;
    ScreenH = SCR_HEIGHT;
    break;
  case DISPLAY_MODE_FILL_SCREEN:
    ScreenW = SCR_WIDTH;
    ScreenH = SCR_HEIGHT;
    break;
  case DISPLAY_MODE_2X:
      ScreenW = Screen->Viewport.Width*2;
      ScreenH = Screen->Viewport.Height*2;
      break;
  case DISPLAY_MODE_3X:
  case DISPLAY_MODE_3X_BG:
      ScreenW = Screen->Viewport.Width*3;
      ScreenH = Screen->Viewport.Height*3;
      break;
  }



  ScreenX = (SCR_WIDTH / 2) - (ScreenW / 2);
  ScreenY = (SCR_HEIGHT / 2) - (ScreenH / 2);

  /* Init performance counter */
  pl_perf_init_counter(&FpsCounter);

  /* Recompute update frequency */
  TicksPerSecond = sceRtcGetTickResolution();
  if (Options.UpdateFreq)
  {
    TicksPerUpdate = TicksPerSecond
      / (Options.UpdateFreq - (Options.Frameskip));
    sceRtcGetCurrentTick(&LastTick);
  }


	system_frameskip_key = Options.Frameskip + 1; /* 1 - 7 */
  ReturnToMenu = 0;
  ClearScreen = 2;


  /* Initiate sound */
  pl_snd_resume(0);

  /* Wait for V. refresh */
  pspVideoWaitVSync();

  /* Emulation loop */
  while (!ExitPSP && !ReturnToMenu)
    emulate();

  /* Stop sound */
  pl_snd_pause(0);

}

static void AudioCallback(pl_snd_sample* buf,
                          unsigned int samples,
                          void *userdata){

  int length_bytes = samples << 2; /* 4 bytes per stereo sample */

  /* If the sound buffer's not ready, render silence */
  if (ExitPSP || ReturnToMenu) memset(buf, 0, length_bytes);
	else sound_update_stereo((_u16*)buf, length_bytes);
}



/* Release emulation resources */
void TrashEmulation()
{
  pspImageDestroy(Screen);
  if(BG3X)
    pspImageDestroy(BG3X);
}
