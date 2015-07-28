#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "psplib/pl_file.h"
#include "psplib/image.h"
#include "psplib/video.h"

#include "neopop.h"

extern PspImage *Screen;
pl_file_path SaveStatePath;
extern char *GameName;

typedef struct
{
  char label[9];
  char string[256];
}
STRING_TAG;

/* TODO */
const char *FlashDirectory = ".";

static STRING_TAG string_tags[STRINGS_MAX] =
{
  //NOTE: This ordering must match the enumeration 'STRINGS' in neopop.h

  { "SDEFAULT",  "Are you sure you want to revert to the default control setup?" },
  { "ROMFILT",  "Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0" },
  { "STAFILT",  "State Files (*.ngs)\0*.ngs\0\0" },
  { "FLAFILT",  "Flash Memory Files (*.ngf)\0*.ngf\0\0" },
  { "BADFLASH",  "The flash data for this rom is from a different version of NeoPop, it will be destroyed soon." },
  { "POWER",  "The system has been signalled to power down. You must reset or load a new rom." },
  { "BADSTATE",  "State is from an unsupported version of NeoPop." },
  { "ERROR1",  "An error has occured creating the application window" },
  { "ERROR2",  "An error has occured initialising DirectDraw" },
  { "ERROR3",  "An error has occured initialising DirectInput" },
  { "TIMER",  "This system does not have a high resolution timer." },
  { "WRONGROM",  "This state is from a different rom, Ignoring." },
  { "EROMFIND",  "Cannot find ROM file" },
  { "EROMOPEN",  "Cannot open ROM file" },
  { "EZIPNONE", "No roms found" },
  { "EZIPBAD",  "Corrupted ZIP file" },
  { "EZIPFIND", "Cannot find ZIP file" },

  { "ABORT",  "Abort" },
  { "DISCON",  "Disconnect" },
  { "CONNEC",  "Connected." },
};

/*! Used to generate a critical message for the user. After the message
  has been displayed, the function should return. The message is not
  necessarily a fatal error. */

void system_message(char* vaMessage,...)
{
#ifdef PSP_DEBUG
  FILE *debug = fopen("message.txt", "a");

  va_list vl;
  va_start(vl, vaMessage);
  vfprintf(debug, vaMessage, vl);
  va_end(vl);

  fclose(debug);
#endif
}

/*! Get a string that may possibly be translated */

char* system_get_string(STRINGS string_id)
{
  if (string_id >= STRINGS_MAX)
    return "Unknown String";

  return string_tags[string_id].string;
}

/*! Reads a byte from the other system. If no data is available or no
  high-level communications have been established, then return FALSE.
  If buffer is NULL, then no data is read, only status is returned */

BOOL system_comms_read(_u8* buffer)
{
  return 0;
}

/*! Writes a byte from the other system. This function should block until
  the data is written. USE RELIABLE COMMS! Data cannot be re-requested. */

void system_comms_write(_u8 data)
{
  return;
}

/*! Peeks at any data from the other system. If no data is available or
  no high-level communications have been established, then return FALSE.
  If buffer is NULL, then no data is read, only status is returned */

BOOL system_comms_poll(_u8* buffer)
{
  return 0;
}

/*! Reads from the file specified by 'filename' into the given preallocated
  buffer. This is state data. */

BOOL system_io_state_read(char* filename, _u8* buffer, _u32 bufferLength)
{
  /* Open file for reading */
  FILE *f = fopen(filename, "r");
  if (!f) return 0;

  /* Load image into temporary object */
  PspImage *image = pspImageLoadPngFd(f);
  pspImageDestroy(image);

  /* Load state data */
  fread(buffer, bufferLength, sizeof(_u8), f);
  fclose(f);

  return 1;
}

/*! Writes to the file specified by 'filename' from the given buffer.
  This is state data. */

BOOL system_io_state_write(char* filename, _u8* buffer, _u32 bufferLength)
{
  /* Open file for writing */
  FILE *f;
  if (!(f = fopen(filename, "w"))) return 0;

  /* Create copy of screen */
  PspImage *copy;
  if (!(copy = pspImageCreateCopy(Screen)))
  {
    fclose(f);
    return 0;
  }

  /* Correct for color (4444 to 5551) */
  int i, j, r, g, b;
  for (i = copy->Viewport.Y + copy->Viewport.Height; i >= copy->Viewport.Y; i--)
  {
    for (j = copy->Viewport.X + copy->Viewport.Width; j >= copy->Viewport.X; j--)
    {
      _u16 *pixel = &((_u16*)copy->Pixels)[i * copy->Width + j];

      r = ((*pixel & 0xf) * 0xff / 0xf);
      g = (((*pixel >> 4) & 0xf) * 0xff / 0xf);
      b = (((*pixel >> 8) & 0xf) * 0xff / 0xf);

      *pixel = RGB(r,g,b);
    }
  }

  /* Write the screenshot */
  if (!pspImageSavePngFd(f, copy))
  {
    fclose(f);
    pspImageDestroy(copy);
    return 0;
  }

  pspImageDestroy(copy);

  /* Write state data */
  fwrite(buffer, bufferLength, sizeof(_u8), f);
  fclose(f);

  return 1;
}

/*! Reads the "appropriate" (system specific) flash data into the given
  preallocated buffer. The emulation core doesn't care where from. */

BOOL system_io_flash_read(_u8* buffer, _u32 bufferLength)
{
  char *path;
  printf("get_filename %s",SaveStatePath);
  const char *config_name = pl_file_get_filename(GameName);
  path = (char*)malloc(strlen(SaveStatePath) + strlen(config_name) + 8);
  sprintf(path, "%s%s.ngf", SaveStatePath, config_name);
  printf("%s",path);
  FILE* file;
  if ((file = fopen(path, "r")))
  {
    fread(buffer, bufferLength, sizeof(_u8), file);
    fclose(file);
  }

  free(path);
  return file != NULL;
}

/*! Writes the given flash data into an "appropriate" (system specific)
  place. The emulation core doesn't care where to. */

BOOL system_io_flash_write(_u8* buffer, _u32 bufferLength)
{
  char *path;
  const char *config_name = pl_file_get_filename(GameName);
  path = (char*)malloc(strlen(SaveStatePath) + strlen(config_name) + 8);
  sprintf(path, "%s%s.ngf", SaveStatePath, config_name);

  FILE* file;
  if ((file = fopen(path, "w")))
  {
    fwrite(buffer, bufferLength, sizeof(_u8), file);
    fclose(file);
  }

  free(path);
  return file != NULL;
}

/*! Reads as much of the file specified by 'filename' into the given,
  preallocated buffer. This is rom data */

BOOL system_io_rom_read(char* filename, _u8* buffer, _u32 bufferLength)
{
  FILE* file;
  file = fopen(filename, "rb");

  if (file)
  {
    fread(buffer, bufferLength, sizeof(_u8), file);
    fclose(file);
    return TRUE;
  }

  return FALSE;
}

/*! Callback for "sound_init" with the system sound frequency */

void system_sound_chipreset()
{
  /* Initialises sound chips, matching frequncies */
  sound_init(CHIP_FREQUENCY);
}

/*! Clears the sound output. */

void system_sound_silence()
{
}
