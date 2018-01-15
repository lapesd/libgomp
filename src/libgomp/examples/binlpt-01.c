#include <stdlib.h>
#include <stdio.h>

#include <omp.h>

int main(void)
{
#pragma omp parallel
    {
        unsigned workloads[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int id = omp_set_workload(workloads, 10);
        omp_set_loop(id);
        omp_set_schedule(omp_sched_binlpt, 0);
#pragma omp for schedule(runtime)
        for (size_t i = 0; i < 10; i++) {
            printf("Hello, world from thread %i!\n", omp_get_thread_num());
        }
    }

    return EXIT_SUCCESS;
}
