/***************************************************************************
 *   Copyright (C) 2009 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "../include/sys.h"
#include "ParseUMB.h"

const struct option options[] = {
	{"help", 0, NULL, 'h'},
	{"length", 1, NULL, 'l'},
	{"output", 1, NULL, 'o'},
	{"reset", 0, NULL, 'r'},
	{"version", 0, NULL, 'v'},
	{NULL, 0, NULL, 0}
};
void print_usage()
{
	printf("Usage: pyeParseUMB inputfile [-o outputfile]\n"
		 "\t-h --help\n\t\tdisplay this help and exit\n"
		 "\t-l <n> --length=<n>\n\t\tthe length of the shortest phrase\n"
		 "\t-o <file> --output=<file>\n\t\tplace the output into <file>\n"
		 "\t-r --reset\n\t\treset all phrase frequencies\n"
		 "\t-v --version\n\t\toutput version information and exit\n");
}
void print_version()
{
	printf("pyeParseUMB: 0.1.0\n");
}

int main(int argc, char *argv[])
{
	ParseUMB pumb;
	const char *sfile, *dfile;
	int length, opt;
	bool reset;

	sfile = dfile = NULL;
	length = 1;
	reset = false;
	while ((opt = getopt_long(argc, argv, "hl:o:rv", options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			print_usage();
			exit(0);
		case 'l':
			length = atoi(optarg);
			break;
		case 'o':
			dfile = optarg;
			break;
		case 'r':
			reset = true;
			break;
		case 'v':
			print_version();
			exit(0);
		default:
			print_usage();
			exit(1);
		}
	}
	if (optind >= argc) {
		print_usage();
		exit(1);
	}
	sfile = argv[optind];
	if (!dfile)
		dfile = "user.txt";

	pumb.CreateIndexTree(sfile);
	pumb.WriteIndexTree(dfile, length, reset);

	return 0;
}
