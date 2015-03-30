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
    char ** argv;
} coproc;

char **
split_args(char * argv)
{
    if (argv == NULL) {
        return NULL;

    } else {
        int argc = 0;
        char ** args = NULL;
        char * line = argv; // strdup(argv);
        char * arg = strtok(line, " ");

        while (arg != NULL) {
            args = realloc(args, (argc + 1) * sizeof(char *));
            args[argc] = arg;
            ++argc;
            arg = strtok(NULL, " ");
        }

        args = realloc(args, (argc + 1) * sizeof(char *));
        args[argc] = NULL;

        return args;
    }
}

coproc *
alloc_coproc(char * argv)
{
    if (argv == NULL) {
        return NULL;

    } else {
        coproc * cop = calloc(1, sizeof(coproc));
        cop->argv = split_args(argv);
        /*
        cop->cmd = alloc_cmd(line);

        if (cop->cmd != NULL) {
            cop->cmd->argv = realloc(cop->cmd->argv, sizeof(char *));
            cop->cmd->argv[cop->cmd->argc] = NULL;
        }
        */

        return cop;
    }
}

void
free_coproc(coproc * cop)
{
    if (cop != NULL) {
        free(cop->argv);
        free(cop);
    }
}

void
init_coproc(coproc * cop)
{
    pipe(cop->pipe);
    cop->fout = fdopen(cop->pipe[0], "r");
}

void
deinit_coproc(coproc * cop)
{
    if (cop != NULL) {
        fclose(cop->fout);
        close(cop->pipe[0]);
        close(cop->pipe[1]);
    }
}

void
run_coproc(coproc * cop)
{
    if (cop != NULL && cop->argv != NULL) {
        dup2(1, cop->pipe[1]);
        execvp(cop->argv[0], cop->argv);
    }
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
