#ifndef DVDBACKUP_H_
#define DVDBACKUP_H_

/*
 * dvdbackup - tool to rip DVDs from the command line
 *
 * Copyright (C) 2002  Olaf Beck <olaf_sc@yahoo.com>
 * Copyright (C) 2008  Benjamin Drung <benjamin.drung@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* libdvdread */
#include <dvdread/dvd_reader.h>

/* Flag for verbose mode */
extern int verbose;
extern int aspect;

typedef enum {
  STRATEGY_ABORT,
  STRATEGY_SKIP_BLOCK,
  STRATEGY_SKIP_MULTIBLOCK
} read_error_strategy_t;

int DVDDisplayInfo(dvd_reader_t * _dvd, char * dvd);
int DVDMirror(dvd_reader_t * _dvd, char * targetdir,char * title_name, read_error_strategy_t errorstrat);
int DVDMirrorTitleSet(dvd_reader_t * _dvd, char * targetdir,char * title_name, int title_set, read_error_strategy_t errorstrat);
int DVDMirrorMainFeature(dvd_reader_t * _dvd, char * targetdir,char * title_name, read_error_strategy_t errorstrat);
int DVDMirrorTitles(dvd_reader_t * _dvd, char * targetdir,char * title_name, int titles);
int DVDMirrorChapters(dvd_reader_t * _dvd, char * targetdir,char * title_name, int start_chapter,int  end_chapter, int titles);
int DVDGetTitleName(const char *device, char *title);

#endif /* DVDBACKUP_H_ */
