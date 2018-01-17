#include <stdlib.h>
#include <stdio.h>

#include <omp.h>

int main(void)
{
  unsigned workloads[] = {1, 1, 1, 1};

    unsigned id1 = omp_loop_register("loop-hello");
    unsigned id2 = omp_loop_register("loop-bye");

    omp_set_schedule(omp_sched_binlpt, 0);

    for (int i = 0; i < 4; i++) {
      omp_set_workload(id1, workloads, 4, true);
#pragma omp parallel for schedule(runtime)
      for (size_t i = 0; i < 4; i++) {
        printf("Hello, world from thread %i!\n", omp_get_thread_num());
      }

      omp_set_workload(id2, workloads, 4, false);
#pragma omp parallel for schedule(runtime)
      for (size_t i = 0; i < 4; i++) {
        printf("Bye, world from thread %i!\n", omp_get_thread_num());
      }
    }

    omp_loop_unregister(id1);
    omp_loop_unregister(id2);

    return EXIT_SUCCESS;
}
