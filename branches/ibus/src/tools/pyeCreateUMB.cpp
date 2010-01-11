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
#include "CreateUMB.h"

const struct option options[] = {
	{"help", 0, NULL, 'h'},
	{"output", 1, NULL, 'o'},
	{"version", 0, NULL, 'v'},
	{NULL, 0, NULL, 0}
};
void print_usage()
{
	printf("Usage: pyeCreateUMB inputfile [-o outputfile]\n"
		 "\t-o <file> --output=<file>\n\t\tplace the output into <file>\n"
		 "\t-h --help\n\t\tdisplay this help and exit\n"
		 "\t-v --version\n\t\toutput version information and exit\n");
}
void print_version()
{
	printf("pyeCreateUMB: 0.1.0\n");
}

int main(int argc, char *argv[])
{
	CreateUMB cumb;
	const char *sfile, *dfile;
	int opt;

	sfile = dfile = NULL;
	while ((opt = getopt_long(argc, argv, "o:hv", options, NULL)) != -1) {
		switch (opt) {
		case 'o':
			dfile = optarg;
			break;
		case 'h':
			print_usage();
			exit(0);
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
		dfile = "user.mb";

	cumb.CreateIndexTree(sfile);
	cumb.WriteIndexTree(dfile);

	return 0;
}

