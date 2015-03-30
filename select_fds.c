#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

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


        /*
    if (fork() == 0) {
        run_coproc(&coprocs[0]);
    } else {
    */

    // if (pid != 0) {
        // parent

        char * lineptr = NULL;

        fd_set readfds;
        FD_ZERO(&readfds);
        for (int n = 0; n < ncoprocs; ++n) {
            FD_SET(coprocs[n].pipe[1], &readfds);
        }

        while (1) {
            int nfds = select(1, &readfds, NULL, NULL, NULL);
            // select(1, &readfds, NULL, NULL, NULL);

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

    /*
    int fildes[2];

    pipe(fildes);

    pid_t pid = fork();

    if (pid == 0) {
        // child
        // dup2(stdout, fildes[1]);
        // fopen(stdout, "r");
        char * const args[] = { "acpi_listen" };
        dup2(1, fildes[1]);
        execvp(args[0], args);
    } else {
        // parent

        char * lineptr = NULL;
        FILE * pstdout = fdopen(fildes[0], "r");

        // int nfds;
        fd_set readfds;
        // fd_set writefds;
        // fd_set errorfds;
        // struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(fildes[0], &readfds);
        // FD_ZERO(&writefds);
        // FD_ZERO(&errorfds);

        while (1) {
            // int nset = select(1, &readfds, NULL, NULL, NULL);
            select(1, &readfds, NULL, NULL, NULL);
            getline(&lineptr, 0, pstdout);
            printf("lineptr: %s\n", lineptr);
            free(lineptr);
            lineptr = NULL;
        }
    }
    */

    return 0;
}
