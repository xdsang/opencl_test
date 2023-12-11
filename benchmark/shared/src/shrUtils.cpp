#include "shrUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

void printBuf(const unsigned char *buf, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (i % 32 == 0)
        {
             printf("\n%5d : ", i);
        }

        if (i % 4 == 0 && i % 32 != 0)
        {
            printf(" ");
        }
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

void populateArray(float *ptr, uint64_t N)
{
    srand((unsigned int)time(NULL));

    for (uint64_t i = 0; i < N; i++)
    {
        ptr[i] = (float)i;
    }
}

std::string outputDigits(long long count, uint length)
{
    std::string digits = std::to_string(count);

    if (digits.length() < length)
    {
        digits.insert(0, length - digits.length(), 'x');
    }
    else
    {
        digits = std::string(digits.length() - length, 'x') + digits.substr(digits.length() - length);
    }

    return digits;
}
