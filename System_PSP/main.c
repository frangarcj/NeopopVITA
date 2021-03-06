#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/moduleinfo.h>

#include "psplib/pl_snd.h"
#include "psplib/video.h"
#include "psplib/pl_psp.h"
#include "psplib/ctrl.h"
#include <vita2d.h>

#include "emumenu.h"
#include "revitalize.h"

char *stpcpy(char *dest, const char *src)
{
	register char *d = dest;
	register const char *s = src;

	do {
		*d++ = *s;
	} while (*s++ != '\0');

	return d - 1;
}

PSP2_MODULE_INFO(0,1,PSP_APP_NAME);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void show_splash()
{
	vita2d_start_drawing();
	vita2d_clear_screen();

	vita2d_texture *splash = vita2d_create_empty_texture(960, 544);

	splash = vita2d_load_PNG_buffer(revitalize);

	vita2d_draw_texture(splash, 0, 0);

	vita2d_end_drawing();
	vita2d_swap_buffers();

	sceKernelDelayThread(5000000); // Delay 5 seconds

	vita2d_free_texture(splash);
}

static void ExitCallback(void* arg)
{
  ExitPSP = 1;
}
int main(int argc, char **argv)
{
  /* Initialize PSP */
  pl_psp_init("ux0:/data/NeopopVITA/");
  pl_snd_init(512,1);
  pspCtrlInit();
  pspVideoInit();

//	show_splash();
#ifdef PSP_DEBUG
  FILE *debug = fopen("message.txt", "w");
  fclose(debug);
#endif

  /* Initialize callbacks */
  pl_psp_register_callback(PSP_EXIT_CALLBACK, ExitCallback, NULL);
  pl_psp_start_callback_thread();

  /* Show the menu */
  if (InitMenu())
  {
    DisplayMenu();
    TrashMenu();
  }

  /* Release PSP resources */
  pl_snd_shutdown();
  pspVideoShutdown();
  pl_psp_shutdown();

  return(0);
}
