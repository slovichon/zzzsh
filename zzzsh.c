/* $Id$ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

void delegate(char *);
void report(char *, ...);

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
	char buf[BUFSIZ+1], prog[BUFSIZ], ch, **t;
	extern char *optarg;

	while ((ch = getopt(argc, argv, "c")) != EOF) {
		switch (ch) {
			case 'c':
				bzero(buf, BUFSIZ);
				if (optarg != NULL) {
					strncpy(buf, optarg+2, BUFSIZ);
					buf[BUFSIZ] = '\0';
					strncat(buf, " ", BUFSIZ-strlen(buf));
					buf[BUFSIZ] = '\0';
				}
				/* traverse subsequent arguments */
				for (t = argv+optind; *t != NULL && strlen(buf) < BUFSIZ; t++) {
					strncat(buf, *t, BUFSIZ-strlen(buf));
					buf[BUFSIZ] = '\0';
					if (t[1] != NULL) {
						strncat(buf, " ", BUFSIZ-strlen(buf));
						buf[BUFSIZ] = '\0';
					}
				}
				delegate(buf);
				return 0;
			default:
				errx("Unknown option: %c", ch);
		}
	}

//	while (fgets(buf, BUFSIZ, stdin) != NULL)
//		delegate(buf);
	printf("You cannot log in to this account.\n");

	return 0;
}

void delegate(char *cmd)
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
	report("%s: Permission denied", cmd);
}

void report(char *msg, ...)
{
	va_list p;
	va_start(p, msg);
	printf("zzzsh: ");
	vprintf(msg, p);
	printf("\n");
	va_end(p);
}
