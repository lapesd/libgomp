#include <stdlib.h>
#include <stdio.h>

#include <omp.h>

int main(void)
{
		unsigned workloads[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		omp_set_schedule(omp_sched_binlpt, 0);

		for (int i = 0; i < 10; i++)
		{
			int loopid = 0;
			
			if (i == 0)
				loopid = omp_set_workload(workloads, 10);
			else
				omp_set_loop(loopid);

			#pragma omp parallel for schedule(runtime)
			for (size_t i = 0; i < 10; i++) {
				printf("Hello, world from thread %i!\n", omp_get_thread_num());
			}
		}

    return EXIT_SUCCESS;
}
