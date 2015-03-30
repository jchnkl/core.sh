#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stropts.h>
#include <poll.h>
#include <pty.h>
#include <utmp.h>

typedef struct coproc_t {
  int master;
  int slave;
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
    char * line = argv;
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
}

void
deinit_coproc(coproc * cop)
{
  if (cop != NULL) {
    fclose(cop->fout);
  }
}

void
show_coproc(coproc * cop)
{
  printf("argv: {");
  char ** arg = cop->argv;
  while (*arg != NULL) {
    printf(" %s", *arg);
    ++arg;
    if (*arg == NULL) {
      printf(" });\n");
    } else {
      printf(",");
    }
  }
}

void
run_coproc(coproc * cop)
{
  if (cop != NULL && cop->argv != NULL) {
    execvp(cop->argv[0], cop->argv);
  }
}

int
main(int argc, char ** argv)
{
  int ncoprocs = argc - 1;
  coproc * coprocs[ncoprocs];

  for (int n = 1; n < argc; ++n) {
    coprocs[n-1] = alloc_coproc(argv[n]);
    init_coproc(coprocs[n-1]);
  }

  for (int n = 0; n < ncoprocs; ++n) {
    openpty(&coprocs[n]->master, &coprocs[n]->slave, NULL, NULL, NULL);

    if (fork() == 0) {
      login_tty(coprocs[n]->slave);
      close(coprocs[n]->master);
      execvp(coprocs[n]->argv[0], coprocs[n]->argv);

    } else {
      close(coprocs[n]->slave);
      coprocs[n]->fout = fdopen(coprocs[n]->master, "r");
    }
  }


    nfds_t nfds = ncoprocs;
    struct pollfd fds[ncoprocs];

    for (int n = 0; n < ncoprocs; ++n) {
      fds[n].fd = coprocs[n]->master;
      fds[n].events = POLLIN;
      fds[n].revents = 0;
    }

    char buf[BUFSIZ];

    while (1) {
      // -1 == block until event
      int ns = poll(fds, nfds, -1);
      for (int n = 0; n < nfds && ns > 0; ++n) {
        if (fds[n].revents & POLLIN) {
          --ns;
          fgets(buf, BUFSIZ, coprocs[n]->fout);
          printf("%s\n", buf);
        }
      }
    }

  return 0;
}
