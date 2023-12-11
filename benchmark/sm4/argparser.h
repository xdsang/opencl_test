#ifndef __ARGS_PARSE__
#define __ARGS_PARSE__

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>

class CmdArgs
{
public:
    int loop_num = 1024;
    int groups = 1;
    int per_thread = 128;
    int total_size_MB = 8;
    int platforms = 1;
    int threads = 1;
    int default_platform_idx = -1;
    bool verify_c = false;
};

static struct option long_options[] = {
    {"loop-num",             required_argument,  NULL,  'l' },
    {"groups-buffer",        required_argument,  NULL,  'g' },
    {"per-thread",           required_argument,  NULL,  'x' },
    {"total-size",           required_argument,  NULL,  's' },
    {"platforms",            required_argument,  NULL,  'p' },
    {"threads",              required_argument,  NULL,  't' },
    {"default-platform",     required_argument,  NULL,  'd' },
    {"verify-c",             no_argument,        NULL,  'c' },
    {"help",                 no_argument,        NULL,  'h' },
    {"usage",                no_argument,        NULL,  'h' },
    {0,                      0,                  NULL,   0  }
};

void print_help(const char* name)
{
    printf("Usage: %s\n\n", name);

    printf("  -l, --loop-num\n\tLOOPNUM: number of loops, default 1024.\n");
    printf("  -g, --groups-buffer\n\tUse multiple buffering, default 1.\n");
    printf("  -x, --per-thread\n\tTHRADPERBLOCK[workgroup_size]: number of threads per blcok, default 128.\n");
    printf("  -s, --total-size\n\tSet SM4 process total size(MB), default 8M.\n");
    printf("  -p, --platforms\n\tUse multiple platforms, default 1.\n");
    printf("  -t, --threads\n\tUse multiple threads, default 1.\n");
    printf("  -d, --default-platform\n\tUse platform to run, default platform 0.\n");
    printf("  -c, --verify-c\n\tCompare the sm4 results of the GPU and the C program, default not verify.\n");
    printf("  -h, --help\n\tGive this help list.\n");
}

void parse(int argc, char **argv, CmdArgs *cmdOption)
{
    assert(cmdOption != NULL);

    while (1)
    {
        int option_index = 0;

        int c = getopt_long(argc, argv, "g:cd:l:x:t:s:p:h?",
        long_options, &option_index);
        switch (c) {
        case 'l':
            printf("loop num is: %d\n", atoi(optarg));
            cmdOption->loop_num = atoi(optarg);
            break;

        case 'x':
            printf("thread per block is: %d\n", atoi(optarg));
            cmdOption->per_thread = atoi(optarg);
            break;

        case 's':
            printf("process total size is: %d\n", atoi(optarg));
            cmdOption->total_size_MB = atoi(optarg);
            break;

        case 'g':
            printf("groups buffer is %d\n",atoi(optarg));
            cmdOption->groups = atoi(optarg);
            break;

        case 'p':
            printf("platforms is %d\n",atoi(optarg));
            cmdOption->platforms = atoi(optarg);
            break;

        case 't':
            printf("threads is %d\n",atoi(optarg));
            cmdOption->threads = atoi(optarg);
            break;

        case 'c':
            printf("verify.\n");
            cmdOption->verify_c = true;
            break;

        case 'd':
            printf("default-platform is %d\n", atoi(optarg));
            cmdOption->default_platform_idx = atoi(optarg);
            break;

        case 'h':
            print_help(argv[0]);
            exit(EXIT_SUCCESS);
            break;

        case '?':
            printf("Try '%s --help' or '%s --usage' for more information..\n\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
        if (c == -1)
            break;
    }

    // cmdOption->data_size_inBytes = cmdOption->total_size_MB * 1024 * 1024;

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
        printf("%s ", argv[optind++]);
        printf("\n");
        exit(EXIT_FAILURE);
    }
}

#endif // __ARGS_PARSE__