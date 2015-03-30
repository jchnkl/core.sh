#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

typedef struct cmd_t {
    int argc;
    char ** argv;
} cmd;

typedef struct coproc_t {
    int pipe[2];
    FILE *  fout;
    char *  file;
    int argc;
    char ** argv;
} coproc;

void
init_coproc(coproc * cop, char * file, int argc, char ** argv)
{
    cop->file = file;
    cop->argc = argc;
    cop->argv = argv;
    pipe(cop->pipe);
    cop->fout = fdopen(cop->pipe[0], "r");
}

void
run_coproc(coproc * cop)
{
    int nargs = 2 + cop->argc;
    char * args[nargs];

    memset(args, 0, sizeof(char *) * nargs);
    args[0] = cop->file;

    for (int n = 0; n < cop->argc; ++n) {
        args[n + 1] = cop->argv[n];
    }

    dup2(1, cop->pipe[1]);
    execvp(cop->file, (char * const *)args);
}

cmd *
alloc_cmd(char * line)
{
    if (line == NULL) {
        return NULL;

    } else {
        int argc = 0;
        char ** argv = NULL;

        char * a = strtok(line, " ");

        while (a != NULL) {
            argv = realloc(argv, sizeof(char *));
            argv[argc] = a;
            ++argc;
            a = strtok(NULL, " ");
        }

        cmd * c = calloc(1, sizeof(cmd));
        c->argc = argc;
        c->argv = argv;

        return c;
    }
}

void
free_cmd(cmd * c)
{
    free(c->argv);
    free(c);
}

int
main(int argc, char ** argv)
{
    int ncoprocs = 3;
    coproc coprocs[ncoprocs];

    init_coproc(&coprocs[0], "acpi_listen", 0, NULL);

    char * dbus_monitor[] = { "--profile", "--system" };
    init_coproc(&coprocs[1], "dbus-monitor", 2, dbus_monitor);

    char * udevadm[] = { "monitor", "--u" };
    init_coproc(&coprocs[2], "udevadm", 2, udevadm);

    pid_t pid;
    for (int n = 0; n < ncoprocs; ++n) {
        pid = fork();
        if (pid == 0) {
            run_coproc(&coprocs[n]);
            break;
        }
    }

    if (pid != 0) {
        char * lineptr = NULL;

        fd_set readfds;
        FD_ZERO(&readfds);
        for (int n = 0; n < ncoprocs; ++n) {
            FD_SET(coprocs[n].pipe[1], &readfds);
        }

        while (1) {
            int nfds = select(1, &readfds, NULL, NULL, NULL);

            for (int i = 0; i < nfds; ++i) {
                for (int n = 0; n < nfds; ++n) {
                    if (FD_ISSET(coprocs[n].pipe[1], &readfds)) {
                        getline(&lineptr, 0, coprocs[n].fout);
                        printf("lineptr: %s\n", lineptr);
                        free(lineptr);
                        lineptr = NULL;
                    }
                }
            }
        }
    }

    return 0;
}
