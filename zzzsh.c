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
#include <sysexits.h>
#include <err.h>

void execute(char *);

struct allowprog {
	char *ap_name;
	char *ap_path;
} allowed[] = {
	{ "cvs",		"/usr/bin/cvs" },
	{ "/usr/bin/cvs",	"/usr/bin/cvs" },
	{ NULL, NULL }
};

char *nenvp[] = {
	NULL
};

int
main(int argc, char *argv[])
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
				/* Traverse subsequent arguments. */
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

void
execute(char *cmd)
{
	char *s, **argv;
	struct allowprog *p;
	int len, pos;

	while ((*cmd == ' ' || *cmd == '\t' || *cmd == '\n') &&
	       *cmd != '\0')
		cmd++;

	if (*cmd == '\0')
		return;

	for (p = allowed; p->ap_name != NULL; p++) {
		pos = strlen(p->ap_name);
		if (strncmp(p->ap_name, cmd, strlen(p->ap_name)) == 0 &&
		    (cmd[pos] == ' ' || cmd[pos] == '\0')) {
			switch (fork()) {
			case -1:
				warnx("fork");
				break;
			case 0:
				/* XXX: quotes, escape, etc. */
				for (s = cmd, len = 1;
				     (s = strpbrk(s, " \t\r\n")) != NULL;
				     len++, s++)
					;

				if ((argv = malloc(len * sizeof(char *))) == NULL)
					err(EX_OSERR, "malloc");

				for (pos = 1, argv[0] = cmd;
				     (s = strpbrk(cmd, " \t\r\n")) != NULL;
				     pos++) {
					*s = '\0';
					argv[pos++] = s + 1;
				}

				argv[pos] = NULL;

				execve(p->ap_path, argv, nenvp);
				err(EX_OSERR, "exec");
				/* NOTREACHED */
			default:
				wait((int *)NULL);
				return;
				/* NOTREACHED */
			}
		}
	}
	if ((s = strpbrk(cmd, " \t\r\n")) != NULL)
		*s = '\0';
	warnx("%s: Permission denied", cmd);
}
