/* $Id$ */
/*
	Public Domain
	Jared Yanovich 2003
	zzzsh - a minimal shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sysexits.h>

void execute(char *);

struct allowprog {
	char *name;
	char *path;
} allowed[] = {
	{"scp",			"/usr/bin/scp"  },
	{"sftp",		"/usr/bin/sftp" },
	{"cvs",			"/usr/bin/cvs"  },
	{"/usr/bin/scp",	"/usr/bin/scp"  },
	{"/usr/bin/sftp",	"/usr/bin/sftp" },
	{"/usr/bin/cvs",	"/usr/bin/cvs"  },
	{ NULL, NULL }
};

int main(int argc, char *argv[])
{
	char buf[BUFSIZ+1], ch, **t;
	extern char *optarg;

	while ((ch = getopt(argc, argv, "c")) != EOF) {
		switch (ch) {
			case 'c':
				memset(buf, '\0', BUFSIZ);
				if (optarg != NULL) {
					strlcpy(buf, optarg+2, sizeof buf);
					strlcat(buf, " ", sizeof buf);
				}
				/* traverse subsequent arguments */
				for (t = argv + optind; *t != NULL && strlen(buf) < BUFSIZ; t++) {
					strlcat(buf, *t, sizeof buf);
					if (t[1] != NULL)
						strlcat(buf, " ", sizeof buf);
				}
				execute(buf);
				return 0;
			default:
				errx(EX_USAGE, "Unknown option: %c", ch);
		}
	}

#ifdef INTERACTIVE
	for (;;) {
		printf("$ ");
		if (fgets(buf, sizeof buf, stdin) == NULL) {
			/* Overwrite prompt */
		    	printf("\r    \r");
			break;
		}
		/* Remove newline */
		buf[strlen(buf) - 1] = '\0';
		if ((strcmp(buf, "exit") == 0) ||
		    (strcmp(buf, "logout") == 0))
			break;
		execute(buf);
	}
#else
	printf("You cannot log in to this account.\n");
#endif

	return 0;
}

void execute(char *cmd)
{
	char *s;
	struct allowprog *p;

	for (p = allowed; p->name != NULL; p++) {
		if (strncmp(p->name, cmd, strlen(p->name)) == 0) {
			system(cmd);
			return;
		}
	}
	if ((s = strpbrk(cmd, " \t\r\n")) != NULL)
		*s = '\0';
	warnx("%s: Permission denied", cmd);
}
