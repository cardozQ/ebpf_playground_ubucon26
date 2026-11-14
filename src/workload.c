#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void usage(const char *program)
{
    printf("Usage: %s [--iterations N] [--no-child] [--help]\n", program);
    puts("  --iterations N  Number of file write/read rounds (default: 20)");
    puts("  --no-child      Do not execute the small /bin/echo child command");
    puts("  --help          Show this help");
}

static void small_cpu_work(void)
{
    volatile unsigned long value = 1;
    for (unsigned long i = 1; i < 250000UL; ++i)
        value = (value * 33UL) ^ i;
    (void)value;
}

static int run_child_command(void)
{
    pid_t child = fork();

    if (child < 0) {
        perror("fork");
        return -1;
    }

    if (child == 0) {
        execl("/bin/echo", "echo", "hello from the child process", (char *)NULL);
        perror("execl");
        _exit(127);
    }

    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int main(int argc, char **argv)
{
    int iterations = 20;
    int run_child = 1;

    static const struct option options[] = {
        {"iterations", required_argument, NULL, 'i'},
        {"no-child", no_argument, NULL, 'n'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    int option;
    while ((option = getopt_long(argc, argv, "i:nh", options, NULL)) != -1) {
        switch (option) {
        case 'i':
            iterations = atoi(optarg);
            if (iterations < 1 || iterations > 10000) {
                fprintf(stderr, "iterations must be between 1 and 10000\n");
                return EXIT_FAILURE;
            }
            break;
        case 'n':
            run_child = 0;
            break;
        case 'h':
            usage(argv[0]);
            return EXIT_SUCCESS;
        default:
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    char path[128];
    snprintf(path, sizeof(path), "/tmp/ebpf-playground-%ld.txt", (long)getpid());

    printf("[workload] pid=%ld, iterations=%d\n", (long)getpid(), iterations);
    printf("[workload] temporary file: %s\n", path);

    for (int i = 0; i < iterations; ++i) {
        int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (fd < 0) {
            fprintf(stderr, "open for write failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        char buffer[128];
        int length = snprintf(buffer, sizeof(buffer),
                              "bpftrace playground round %d\n", i + 1);
        if (write(fd, buffer, (size_t)length) < 0) {
            perror("write");
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);

        fd = open(path, O_RDONLY);
        if (fd < 0) {
            perror("open for read");
            return EXIT_FAILURE;
        }

        char read_buffer[128];
        (void)read(fd, read_buffer, sizeof(read_buffer));
        close(fd);

        small_cpu_work();

        struct timespec pause = {.tv_sec = 0, .tv_nsec = 30000000L};
        nanosleep(&pause, NULL);
    }

    if (run_child && run_child_command() != 0)
        fprintf(stderr, "[workload] child command did not exit successfully\n");

    if (unlink(path) < 0)
        perror("unlink");

    puts("[workload] finished");
    return EXIT_SUCCESS;
}
