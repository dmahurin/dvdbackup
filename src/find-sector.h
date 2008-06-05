#ifndef FIND_SECTORS_H
#define FIND_SECTORS_H

#include <glib/gslist.h>
void create_titleset_range_list(dvd_reader_t *dvd, int titleset,  GSList **range_list);
int find_next_sectors(GSList *range_list, int offset);

#endif
