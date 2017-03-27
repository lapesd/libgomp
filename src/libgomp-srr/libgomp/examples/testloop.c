#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main(void)
{
#pragma omp parallel for schedule(runtime) num_threads(4)
  for (size_t i = 0; i < 128; i++) {
    printf("Iteration %i from thread %i\n", i, omp_get_thread_num());
  }

  return EXIT_SUCCESS;
}
