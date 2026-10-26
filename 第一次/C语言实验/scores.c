#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNT 5

static int read_score(double *score)
{
    char line[128], *end;
    if (fgets(line, sizeof line, stdin) == NULL)
        return 0;
    errno = 0;
    *score = strtod(line, &end);
    if (end == line || errno != 0 || !isfinite(*score))
        return 0;
    while (*end == ' ' || *end == '\t')
        ++end;
    if (*end != '\n' && *end != '\0')
        return 0;
    return *score >= 0.0 && *score <= 100.0;
}

static void summarize(const double scores[], int n)
{
    double sum = 0.0, max = scores[0];
    int passed = 0;
    for (int i = 0; i < n; ++i) {
        sum += scores[i];
        if (scores[i] > max)
            max = scores[i];
        if (scores[i] >= 60.0)
            ++passed;
    }
    printf("Average: %.2f\nMaximum: %.2f\nPassed: %d/%d\n",
           sum / n, max, passed, n);
}

int main(void)
{
    double scores[COUNT];
    puts("Enter 5 scores, one per line (0-100):");
    for (int i = 0; i < COUNT; ++i) {
        if (!read_score(&scores[i])) {
            fprintf(stderr, "Invalid score at position %d\n", i + 1);
            return 1;
        }
    }
    summarize(scores, COUNT);
    return 0;
}
