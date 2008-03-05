/*
 * dvdbackup - tool to rip DVD's from the command line
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

#include "dvdbackup.h"

/* C standard libraries */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C POSIX library */
#include <sys/stat.h>
#include <unistd.h>

/* libdvdread */
#include <dvdread/dvd_reader.h>


/* Usage:
	To gather info about the dvd:
		dvdbackup -i /dev/dvd -I

	Gerneral backup information:
		If your backup directory is /my/dvd/backup/dir/
		specified with the "-o" flag, then dvdbackup
		will create a DVD-Video structure under
		/my/dvd/backup/dir/TITLE_NAME/VIDEO_TS.

		Since the title is "unique" you can use the same dir
		for all your DVD backups. If it happens to have a
		generic title dvdbackup will exit with a return value
		of 2. And you will need to specify a title name with
		the -n switch.

		dvdbackup will always mimic the original DVD-Video
		structure. Hence if you e.g. use the -M (mirror) you
		will get an exact duplicate of the original. This
		means that every file will be have the same size as
		the original one. Like wise goes also for the -F
		and the -T switch.

		However the -t and (-t -s/-e) switch is a bit
		different the titles sectors will be written to the
		original file but not at the same offset as the
		original one since they may be gaps in the cell
		structure that we don't fill.


	To backup the whole DVD

		dvdbackup -M -i/dev/dvd -o/my/dvd/backup/dir/

		This action creates a valid DVD-Video structure
		that can be burned to a DVD-/+R(W) with help of
		mkisofs version 1.11a27 or later


	To backup the main feature of the DVD:

		dvdbackup -F -i/dev/dvd -o/my/dvd/backup/dir/

		This action creates a valid DVD-Video structure
		of the feature title set

		dvdbackup defaults to get the 16:9 version of the
		main feature if a 4:3 is also present on the DVD.
		To get the 4:3 version use -a 0.

		dvdbackup makes it best to make a inteligent guess
		what is the main feature of the DVD - in case it fails
		please send a bug report.


	To backup a title set

		dvdbackup -T 2 -i/dev/dvd -o/my/dvd/backup/dir/

		where "-T 2" specifies that you want to backup
		title set 2 i.e. all VTS_02_X.XXX files.

		This action creates a valid DVD-Video structure
		of the specified title set


	To backup a title:

		dvdbackup -t 1 -i/dev/dvd -o/my/dvd/backup/dir

		This action backups all cells that forms the
		specified title. Note that there can be sector
		gaps in between one cell and an other. dvdbackup
		will backup all sectors that belongs to the title
		but will skip sectors that aren't a part of the title.


	To backup a specific chapter or chapters from a title:

		dvdbackup -t 1 -s 20 -e 25 -i/dev/dvd -o/my/dvd/backup/dir

		This action will backup chapter 20 to 25 in title 1, as with
		the backup of a title there can be sector gaps between one chapter
		(cell) and on other.dvdbackup will backup all sectors that belongs
		to the title 1 chapter 20 to 25 but will skip sectors that aren't
		a part of the title 1 chapter 20 to 25.

		To backup a single chapter e.g. chapter 20 do -s 20 -e 20
		To backup from chapter 20 to the end chapter use only -s 20
		To backup to chapter 20 from the first chapter use only -e 20

		You can skip the -t switch and let the program guess the title although
		it's not recommended.

		If you specify a chapter that his higher than the last chapter of the title
		dvdbackup will turncate to the highest chapter of the title.

	Return values:
		0 on success
		1 on usage error
		2 on title name error
		-1 on failure


	Todo - i.e. what's on the agenda.
		Make the main feature guessing algoritm better. Not that it doesn't do
		its job, but its implementation isn't that great. I would also like
		to preserve more information about the main feature since that would
		let me preform better implementations in other functions that depends
		on the titles_info_t and title_set_info_t strcutures.

		Make it possible to extract cells in a title not just chapters (very
		easy so it will definitly be in the next version).

		Make a split mirror (-S) option that divides a DVD-9 to two valid DVD-5
		video structures. This is not a trivial hack and it's my main goal
		the next month or so. It involves writing ifoedit and vobedit
		libraries in order to be able to manipulate both the IFO structures
		and the VOB files. Out of this will most probably also come tscreate
		and vtscreate which will enable you to make a very simple DVD-Video
		from MPEG-1/2 source.

*/


void usage(){
	fprintf(stderr,"\nUsage: dvdbackup [options]\n");
	fprintf(stderr,"\t-i device\twhere device is your dvd device\n");
	fprintf(stderr,"\t-v X\t\twhere X is the amount of verbosity\n");
	fprintf(stderr,"\t-I\t\tfor information about the DVD\n");
	fprintf(stderr,"\t-o directory\twhere directory is your backup target\n");
	fprintf(stderr,"\t-n name\t\t(optional) set the title - useful if autodetection fails\n");
	fprintf(stderr,"\t-M\t\tbackup the whole DVD\n");
	fprintf(stderr,"\t-F\t\tbackup the main feature of the DVD\n");
	fprintf(stderr,"\t-T X\t\tbackup title set X\n");
	fprintf(stderr,"\t-t X\t\tbackup title X\n");
	fprintf(stderr,"\t-s X\t\tbackup from chapter X\n");
	fprintf(stderr,"\t-e X\t\tbackup to chapter X\n");
	fprintf(stderr,"\t-a 0\t\tto get aspect ratio 4:3 instead of 16:9 if both are present\n");
	fprintf(stderr,"\t-r {a,b,m}\t\tselect read error handling: a=abort (default), b=skip block, m=skip multiple blocks\n");
	fprintf(stderr,"\t-h\t\tprint a brief usage message\n");
	fprintf(stderr,"\t-?\t\tprint a brief usage message\n\n");
	fprintf(stderr,"\t-i is manditory\n");
	fprintf(stderr,"\t-o is manditory except if you use -I\n");
	fprintf(stderr,"\t-a is option to the -F switch and has no effect on other options\n");
	fprintf(stderr,"\t-s and -e should preferably be used together with -t \n\n");
	exit(1);
}


int main(int argc, char *argv[]){

	/* Args */
	int flags;

	/* Switches */
	int title_set = 0;
	int titles;
	int start_chapter;
	int end_chapter;

	int do_mirror = 0;
	int do_title_set = 0;
	int do_chapter = 0;
	int do_titles = 0;
	int do_feature = 0;
	int do_info = 0;

	read_error_strategy_t errorstrat =
	  STRATEGY_ABORT;


	int return_code = 0;

	/* DVD Video device */
	char * dvd=NULL;

	/* Temp switch helpers */
	char * verbose_temp=NULL;
	char * aspect_temp=NULL;
	char * start_chapter_temp=NULL;
	char * end_chapter_temp=NULL;
	char * titles_temp=NULL;
	char * title_set_temp=NULL;
	char * errorstrat_temp=NULL;


	/* Title of the DVD */
	char title_name[33]="";
	char * provided_title_name=NULL;

	/* Targer dir */
	char * targetdir=NULL;

	/* Temp filename,dirname */
	char targetname[PATH_MAX];
	struct stat fileinfo;


	/* The DVD main structure */
	dvd_reader_t *	_dvd=NULL;



	/* TODO: do isdigit check */

	while ((flags = getopt(argc, argv, "MFI?hi:v:a:o:n:s:e:t:T:r:")) != -1) {
		switch (flags) {
		case 'r':
			if(optarg[0]=='-') usage();
			errorstrat_temp=optarg;
			break;

		case 'i':
      			if(optarg[0]=='-') usage();
      			dvd = optarg;
      			break;
		case 'v':
			if(optarg[0]=='-') usage();
			verbose_temp = optarg;
			break;
		case 'o':
			if(optarg[0]=='-') usage();
			targetdir = optarg;
			break;
		case 'n':
      			if(optarg[0]=='-') usage();
      			provided_title_name = optarg;
      			break;
		case 'a':
			if(optarg[0]=='-') usage();
			aspect_temp = optarg;
			break;
		case 's':
			if(optarg[0]=='-') usage();
			start_chapter_temp = optarg;
			break;
		case 'e':
			if(optarg[0]=='-') usage();
			end_chapter_temp = optarg;
			break;
		case 't':
			if(optarg[0]=='-') usage();
			titles_temp = optarg;
			break;
		case 'T':
			if(optarg[0]=='-') usage();
			title_set_temp = optarg;
			break;
		case 'M':
			do_mirror = 1;
			break;
		case 'F':
			do_feature = 1;
			break;
		case 'I':
			do_info = 1;
			break;

		case '?':
			usage();
			break;
		case 'h':
			usage();
			break;
		default:
			usage();
		}
	}

	if (dvd == NULL) {
		usage();
		exit(1);
	}

	if (targetdir == NULL && do_info == 0) {
		usage();
		exit(1);
	}

	if (errorstrat_temp == NULL || errorstrat_temp[0]=='a') {
		errorstrat=STRATEGY_ABORT;
	} else if (errorstrat_temp[0]=='b') {
	  	errorstrat=STRATEGY_SKIP_BLOCK;
	} else if (errorstrat_temp[0]=='m') {
	  	errorstrat=STRATEGY_SKIP_MULTIBLOCK;
	} else {
	  	usage();
		exit(1);
	}

	if(verbose_temp == NULL) {
		verbose = 0;
	} else {
		verbose = atoi(verbose_temp);
	}

	if (aspect_temp == NULL) {
		/* Deafult to 16:9 aspect ratio */
		aspect = 3;
	} else {
		aspect = atoi(aspect_temp);
	}

	if((aspect != 0) && (aspect != 3) && (do_info == 0)){
		usage();
		exit(1);
	}

	if ( titles_temp != NULL) {
		titles = atoi(titles_temp);
		if ( titles < 1 ) {
			usage();
			exit(1);
		}
	}

	if ( start_chapter_temp !=NULL) {
		start_chapter = atoi(start_chapter_temp);
		if ( start_chapter < 1 || start_chapter > 99 ) {
			usage();
			exit(1);
		}
	}

	if (end_chapter_temp != NULL) {
		end_chapter = atoi(end_chapter_temp);
		if ( end_chapter < 1 || end_chapter > 99 ) {
			usage();
			exit(1);
		}
	}

	if ( end_chapter_temp != NULL || start_chapter_temp != NULL) {
		if( end_chapter_temp == NULL) {
			end_chapter = 99;
		} else if ( start_chapter_temp == NULL) {
			start_chapter = 1;
		}
		if ( end_chapter < start_chapter ) {
			usage();
			exit(1);
		}
	}

	if ( titles_temp != NULL && ((end_chapter_temp != NULL) || (start_chapter_temp != NULL))) {
		do_chapter = 1;
	} else if ((titles_temp != NULL) && ((end_chapter_temp == NULL) && (start_chapter_temp == NULL))) {
		do_titles=1;
	}
	if (do_chapter && (titles_temp == NULL)) {
		titles = 0;
	}

	if ( title_set_temp != NULL ) {
		title_set = atoi(title_set_temp);
		if ( title_set > 99 || title_set < 0 ) {
			usage();
			exit(1);
		}
		do_title_set = 1;
	}

	if (do_info + do_titles + do_chapter + do_feature + do_title_set  + do_mirror > 1 ) {
		usage();
		exit(1);
	} else if ( do_info + do_titles + do_chapter + do_feature + do_title_set  + do_mirror == 0) {
		usage();
		exit(1);
	}
#ifdef DEBUG
	fprintf(stderr,"After args\n");
#endif


	_dvd = DVDOpen(dvd);
	if(!_dvd) exit(-1);


	if (do_info) {
		DVDDisplayInfo(_dvd, dvd);
		DVDClose(_dvd);
		exit(0);
	}


	if(provided_title_name == NULL) {
		if (DVDGetTitleName(dvd,title_name) != 0) {
			fprintf(stderr,"You must provide a title name when you read your DVD-Video structure direct from the HD\n");
			DVDClose(_dvd);
			exit(1);
		}
		if (strstr(title_name, "DVD_VIDEO") != NULL) {
			fprintf(stderr,"The DVD-Video title on the disk is DVD_VIDEO, which is too generic; please provide a title with the -n switch\n");
			DVDClose(_dvd);
			exit(2);
		}

	} else {
		if (strlen(provided_title_name) > 32) {
			fprintf(stderr,"The title name specified is longer than 32 characters; truncating the title name\n");
			strncpy(title_name,provided_title_name, 32);
			title_name[32]='\0';
		} else {
			strcpy(title_name,provided_title_name);
		}
	}



	sprintf(targetname,"%s",targetdir);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,"The target directory is not valid; it may be a ordinary file\n");
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,"Failed creating target directory\n");
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}


	sprintf(targetname,"%s/%s",targetdir, title_name);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,"The title directory is not valid; it may be a ordinary file\n");
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,"Failed creating title directory\n");
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}

	sprintf(targetname,"%s/%s/VIDEO_TS",targetdir, title_name);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,"The VIDEO_TS directory is not valid; it may be a ordinary file\n");
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,"Failed creating VIDEO_TS directory\n");
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}


#ifdef DEBUG
	fprintf(stderr,"After dirs\n");
#endif


	if(do_mirror) {
		if ( DVDMirror(_dvd, targetdir, title_name, errorstrat)  != 0 ) {
			fprintf(stderr, "Mirror of DVD failed\n");
			return_code = -1;
		} else {
			return_code = 0;
		}
	}
#ifdef DEBUG
	fprintf(stderr,"After Mirror\n");
#endif


	if (do_title_set) {
		if (DVDMirrorTitleSet(_dvd, targetdir, title_name, title_set, errorstrat) != 0) {
			fprintf(stderr, "Mirror of title set %d failed\n", title_set);
			return_code = -1;
		} else {
			return_code  = 0;
		}

	}
#ifdef DEBUG
	fprintf(stderr,"After Title Set\n");
#endif



	if(do_feature) {
		if ( DVDMirrorMainFeature(_dvd, targetdir, title_name, errorstrat)  != 0 ) {
			fprintf(stderr, "Mirror of main feature film of DVD failed\n");
			return_code = -1;
		} else {
			return_code = 0;
		}
	}

	if(do_titles) {
		if (DVDMirrorTitles(_dvd, targetdir, title_name, titles) != 0) {
			fprintf(stderr, "Mirror of title  %d failed\n", titles);
			return_code = -1;
		} else {
			return_code  = 0;
		}
	}


	if(do_chapter) {
		if (DVDMirrorChapters(_dvd, targetdir, title_name, start_chapter, end_chapter, titles) != 0) {
			fprintf(stderr, "Mirror of chapters %d to %d in title %d failed\n", start_chapter, end_chapter, titles);
			return_code = -1;
		} else {
			return_code  = 0;
		}
	}


	DVDClose(_dvd);
	exit(return_code);
}
