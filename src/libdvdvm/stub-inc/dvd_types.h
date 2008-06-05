/* stub for libdvdvm compile from unmodified dvdnav sources. */
#ifndef DVD_TYPES
#define DVD_TYPES

#include <inttypes.h>

typedef enum {
  DVD_MENU_Escape     = 0,
  DVD_MENU_Title      = 2,
  DVD_MENU_Root       = 3,
  DVD_MENU_Subpicture = 4,
  DVD_MENU_Audio      = 5,
  DVD_MENU_Angle      = 6,
  DVD_MENU_Part       = 7
} DVDMenuID_t;

#endif
