/* $Id$ */
/*
 * Written by Jared Yanovich
 * This file belongs to the public domain.
 * zzzsh - a minimal shell
 */

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void usage(void);
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
	char buf[BUFSIZ], ch, **t;

	while ((ch = getopt(argc, argv, "c:")) != EOF)
		switch (ch) {
		case 'c':
			(void)strlcpy(buf, optarg, sizeof(buf));
			for (t = argv + optind + 1; *t != NULL; t++) {
				(void)strlcat(buf, " ", sizeof(buf));
				(void)strlcat(buf, *t, sizeof(buf));
			}
			execute(buf);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		default:
			usage();
			/* NOTREACHED */
		}

#ifdef INTERACTIVE
	for (;;) {
		(void)printf("$ ");
		if (fgets(buf, sizeof buf, stdin) == NULL) {
			/* Overwrite prompt. */
		    	(void)printf("\r    \r");
			break;
		}
		/* Remove newline. */
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		if ((strcmp(buf, "exit") == 0) ||
		    (strcmp(buf, "logout") == 0))
			break;
		execute(buf);
	}
#else
	(void)printf("You cannot log in to this account.\n");
#endif

	return 0;
}

void
execute(char *cmd)
{
	struct allowprog *p;
	char *s, **argv;
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
	errno = EPERM;
	warn("%s", cmd);
}

void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-c command ...]\n", __progname);
	exit(EX_USAGE);
}
