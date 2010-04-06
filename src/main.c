/*
 * dvdbackup - tool to rip DVDs from the command line
 *
 * Copyright (C) 2002  Olaf Beck <olaf_sc@yahoo.com>
 * Copyright (C) 2008-2009  Benjamin Drung <benjamin.drung@gmail.com>
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

#include <config.h>
#include "dvdbackup.h"

/* internationalisation */
#include "gettext.h"
#define _(String) gettext(String)

/* C standard libraries */
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C POSIX libraries */
#include <sys/stat.h>
#include <unistd.h>

/* libdvdread */
#include <dvdread/dvd_reader.h>

/* other libraries */
#include <getopt.h>


/* String containing name the program is called with. */
const char* program_name;


static void print_version() {
	printf("%s\n", PACKAGE_STRING);

	/* It is important to separate the year from the rest of the message,
	 * as done here, to avoid having to retranslate the message when a new
	 * year comes around.
	 */
	printf(_("Copyright (C) 2002 Olaf Beck <olaf_sc@yahoo.com>\n\
Copyright (C) %s Benjamin Drung <benjamin.drung@gmail.com>\n\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
Homepage: %s\n"), "2008-2009", "http://dvdbackup.sourceforge.net/");
}


static void print_help() {
	/* TRANSLATORS: --help output 1 (synopsis) */
	printf(_("Usage: %s [OPTION]...\n"), program_name);

	/* TRANSLATORS: --help output 2 (brief description)
	   no-wrap */
/*	fputs (_("\
Print a friendly, customizable greeting.\n"), stdout); */
	printf("\n");

	/* TRANSLATORS: --help output 3: options 1/2
	   no-wrap */
	printf(_("\
  -h, --help         display this help and exit\n\
  -V, --version      display version information and exit\n\n"));

	/* TRANSLATORS: --help output 4: options 2/2
	   no-wrap */
	printf(_("\
  -I, --info         prints information about the DVD\n\
  -M, --mirror       backup the whole DVD\n\
  -F, --feature      backup the main feature of the DVD\n\
  -T, --titleset=X   backup title set X\n\
  -t, --title=X      backup title X\n\
  -s, --start=X      backup from chapter X\n\
  -e, --end=X        backup to chapter X\n\n"));

	printf(_("\
  -i, --input=DEVICE       where DEVICE is your DVD device\n\
                           if not given /dev/dvd is used\n\
  -o, --output=DIRECTORY   where directory is your backup target\n\
                           if not given the current directory is used\n"));
	printf(_("\
  -v, --verbose            print more information about progress\n\
  -n, --name=NAME          set the title (useful if autodetection fails)\n\
  -a, --aspect=0           to get aspect ratio 4:3 instead of 16:9 if both are\n\
                           present\n\
  -r, --error={a,b,m}      select read error handling: a=abort, b=skip block,\n\
                           m=skip multiple blocks (default)\n\
  -p, --progress           print progress information while copying VOBs\n\n"));

	printf(_("\
  -a is option to the -F switch and has no effect on other options\n\
  -s and -e should preferably be used together with -t\n"));

	printf("\n");
	/* TRANSLATORS: --help output 5 (end)
	   TRANSLATORS: the placeholder indicates the bug-reporting address
	   for this application.  Please add _another line_ with the
	   address for translation bugs. */
	printf (_("Report bugs on Launchpad: https://bugs.launchpad.net/dvdbackup\n"));
}


void init_i18n() {
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
}


int main(int argc, char* argv[]) {

	/* Args */
	int flags;
	bool lose = false;

	/* Switches */
	int title_set = 0;
	int titles = 0;
	int start_chapter = 0;
	int end_chapter = 0;

	int do_mirror = 0;
	int do_title_set = 0;
	int do_chapter = 0;
	int do_titles = 0;
	int do_feature = 0;
	int do_info = 0;

	/* Because of copy protection you normally want to skip
	 * the defect sectors. To speed things up we skip multiblocks.
	 */
	read_error_strategy_t errorstrat = STRATEGY_SKIP_MULTIBLOCK;

	int return_code = 0;

	/* DVD Video device */
	char* dvd = "/dev/dvd";

	/* Temp switch helpers */
	char* aspect_temp = NULL;
	char* start_chapter_temp = NULL;
	char* end_chapter_temp = NULL;
	char* titles_temp = NULL;
	char* title_set_temp = NULL;
	char* errorstrat_temp = NULL;


	/* Title of the DVD */
	char title_name[33] = "";
	char* provided_title_name = NULL;

	/* Targer dir */
	char* targetdir = ".";

	/* Temp filename,dirname */
	char targetname[PATH_MAX];
	struct stat fileinfo;

	/* The DVD main structure */
	dvd_reader_t* _dvd = NULL;

	/* the long and the short options */
	static const struct option longopts[] = {
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'V'},

		{"info", no_argument, NULL, 'I'},
		{"mirror", no_argument, NULL, 'M'},
		{"feature", no_argument, NULL, 'F'},
		{"titleset", required_argument, NULL, 'T'},
		{"title", required_argument, NULL, 't'},
		{"start", required_argument, NULL, 's'},
		{"end", required_argument, NULL, 'e'},

		{"input", required_argument, NULL, 'i'},
		{"output", required_argument, NULL, 'o'},
		{"verbose", no_argument, NULL, 'v'},
		{"name", required_argument, NULL, 'n'},
		{"aspect", required_argument, NULL, 'a'},
		{"error", required_argument, NULL, 'r'},
		{"progress", no_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	const char* shortopts = "hVIMFT:t:s:e:i:o:vn:a:r:p";

	init_i18n();
	program_name = argv[0];

	/* TODO: do isdigit check */

	while((flags = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(flags) {
		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
			break;
		case 'V':
			print_version();
			exit(EXIT_SUCCESS);
			break;

		case 'I':
			do_info = 1;
			break;
		case 'M':
			do_mirror = 1;
			break;
		case 'F':
			do_feature = 1;
			break;
		case 'T':
			title_set_temp = optarg;
			break;
		case 't':
			titles_temp = optarg;
			break;
		case 's':
			start_chapter_temp = optarg;
			break;
		case 'e':
			end_chapter_temp = optarg;
			break;

		case 'i':
			dvd = optarg;
			break;
		case 'o':
			targetdir = optarg;
			break;
		case 'v':
			verbose = 10;
			break;
		case 'n':
			provided_title_name = optarg;
			break;
		case 'a':
			aspect_temp = optarg;
			break;
		case 'r':
			errorstrat_temp=optarg;
			break;
		case 'p':
			progress = 1;
			break;

		default:
			lose = true;
			break;
		}
	}

	if(lose || optind < argc) {
		/* Print error message and exit. */
		if (optind < argc) {
			fprintf(stderr, _("%s: extra operand: %s\n"), program_name, argv[optind]);
		}
		fprintf(stderr, _("Try `%s --help' for more information.\n"), program_name);
		exit (EXIT_FAILURE);
	}

	if(errorstrat_temp != NULL) {
		if(errorstrat_temp[0]=='a') {
			errorstrat=STRATEGY_ABORT;
		} else if(errorstrat_temp[0]=='b') {
			errorstrat=STRATEGY_SKIP_BLOCK;
		} else if(errorstrat_temp[0]=='m') {
			errorstrat=STRATEGY_SKIP_MULTIBLOCK;
		} else {
			print_help();
			exit(1);
		}
	}

	if (aspect_temp == NULL) {
		/* Default to 16:9 aspect ratio */
		aspect = 3;
	} else {
		aspect = atoi(aspect_temp);
	}

	if((aspect != 0) && (aspect != 3) && (do_info == 0)){
		print_help();
		exit(1);
	}

	if ( titles_temp != NULL) {
		titles = atoi(titles_temp);
		if ( titles < 1 ) {
			print_help();
			exit(1);
		}
	}

	if ( start_chapter_temp !=NULL) {
		start_chapter = atoi(start_chapter_temp);
		if ( start_chapter < 1 || start_chapter > 99 ) {
			print_help();
			exit(1);
		}
	}

	if (end_chapter_temp != NULL) {
		end_chapter = atoi(end_chapter_temp);
		if ( end_chapter < 1 || end_chapter > 99 ) {
			print_help();
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
			print_help();
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
			print_help();
			exit(1);
		}
		do_title_set = 1;
	}

	if (do_info + do_titles + do_chapter + do_feature + do_title_set + do_mirror > 1 ) {
		print_help();
		exit(1);
	} else if ( do_info + do_titles + do_chapter + do_feature + do_title_set + do_mirror == 0) {
		print_help();
		exit(1);
	}
#ifdef DEBUG
	fprintf(stderr,"After args\n");
#endif


	_dvd = DVDOpen(dvd);
	if (!_dvd) {
		fprintf(stderr,_("Cannot open specified device %s - check your DVD device\n"), dvd);
		exit(-1);
	}

	if (do_info) {
		DVDDisplayInfo(_dvd, dvd);
		DVDClose(_dvd);
		exit(0);
	}


	if(provided_title_name == NULL) {
		if (DVDGetTitleName(dvd,title_name) != 0) {
			fprintf(stderr,_("You must provide a title name when you read your DVD-Video structure direct from the HD\n"));
			DVDClose(_dvd);
			exit(1);
		}
		if (strstr(title_name, "DVD_VIDEO") != NULL) {
			fprintf(stderr,_("The DVD-Video title on the disk is DVD_VIDEO, which is too generic; please provide a title with the -n switch\n"));
			DVDClose(_dvd);
			exit(2);
		}

	} else {
		if (strlen(provided_title_name) > 32) {
			fprintf(stderr,_("The title name specified is longer than 32 characters; truncating the title name\n"));
			strncpy(title_name,provided_title_name, 32);
			title_name[32]='\0';
		} else {
			strcpy(title_name,provided_title_name);
		}
	}



	sprintf(targetname,"%s",targetdir);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,_("The target directory is not valid; it may be an ordinary file.\n"));
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,_("Failed creating target directory %s\n"), targetname);
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}


	sprintf(targetname,"%s/%s",targetdir, title_name);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,_("The title directory is not valid; it may be an ordinary file.\n"));
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,_("Failed creating title directory\n"));
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}

	sprintf(targetname,"%s/%s/VIDEO_TS",targetdir, title_name);

	if (stat(targetname, &fileinfo) == 0) {
		if (! S_ISDIR(fileinfo.st_mode)) {
			fprintf(stderr,_("The VIDEO_TS directory is not valid; it may be an ordinary file.\n"));
		}
	} else {
		if (mkdir(targetname, 0777) != 0) {
			fprintf(stderr,_("Failed creating VIDEO_TS directory\n"));
			perror("");
			DVDClose(_dvd);
			exit(-1);
		}
	}


#ifdef DEBUG
	fprintf(stderr,"After dirs\n");
#endif


	if(do_mirror) {
		if ( DVDMirror(_dvd, targetdir, title_name, errorstrat) != 0 ) {
			fprintf(stderr, _("Mirror of DVD failed\n"));
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
			fprintf(stderr, _("Mirror of title set %d failed\n"), title_set);
			return_code = -1;
		} else {
			return_code = 0;
		}

	}
#ifdef DEBUG
	fprintf(stderr,"After Title Set\n");
#endif



	if(do_feature) {
		if ( DVDMirrorMainFeature(_dvd, targetdir, title_name, errorstrat) != 0 ) {
			fprintf(stderr, _("Mirror of main feature film of DVD failed\n"));
			return_code = -1;
		} else {
			return_code = 0;
		}
	}

	if(do_titles) {
		if (DVDMirrorTitles(_dvd, targetdir, title_name, titles) != 0) {
			fprintf(stderr, _("Mirror of title %d failed\n"), titles);
			return_code = -1;
		} else {
			return_code = 0;
		}
	}


	if(do_chapter) {
		if (DVDMirrorChapters(_dvd, targetdir, title_name, start_chapter, end_chapter, titles) != 0) {
			fprintf(stderr, _("Mirror of chapters %d to %d in title %d failed\n"), start_chapter, end_chapter, titles);
			return_code = -1;
		} else {
			return_code = 0;
		}
	}


	DVDClose(_dvd);
	exit(return_code);
}
