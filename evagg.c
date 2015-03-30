#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stropts.h>
#include <poll.h>

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
    dup2(1, cop->pipe[1]);
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

  pid_t pid;
  for (int n = 0; n < ncoprocs; ++n) {
    pid = fork();
    if (pid == 0) {
      run_coproc(coprocs[n]);
      break;
    }
  }

  if (pid != 0) {
    char * lineptr = NULL;

    nfds_t nfds = ncoprocs;
    struct pollfd fds[ncoprocs];

    for (int n = 0; n < ncoprocs; ++n) {
      fds[n].fd = coprocs[n]->pipe[1];
      fds[n].events = 0;
      fds[n].revents = 0;
    }

    while (1) {
      // -1 == block until event
      int ns = poll(fds, nfds, -1);
      for (int n = 0; n < nfds && ns > 0; ++n) {
        if (fds[n].revents & POLLIN) {
          --ns;
          getline(&lineptr, 0, coprocs[n]->fout);
          printf("lineptr: %s\n", lineptr);
          free(lineptr);
          lineptr = NULL;
        }
      }
    }
  }

  return 0;
}
