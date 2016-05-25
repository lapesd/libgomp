#include "../tsp_mppa.h"
#include "../common_main.h"

static int *comm_buffer;
static int clusters;
static broadcast_t *broad;

void callback_master (mppa_sigval_t sigval);

/* Data exchange statistics. */
size_t data_sent = 0;     /* Number of bytes received. */
unsigned nsend = 0;       /* Number of sends.          */
size_t data_received = 0; /* Number of bytes sent.     */
unsigned nreceive = 0;    /* Number of receives.       */

/*
 * Problem.
 */
struct problem
{
	int nb_towns; /* Number of towns */
};

/* Problem sizes. */
static struct problem tiny     =  { 14 };
static struct problem small    =  { 15 };
static struct problem standard =  { 17 };
static struct problem large    =  { 19 };
static struct problem huge     =  { 20 };

/* Be verbose? */
int verbose = 0;

/* Number of clusters */
int nclusters = 1;
	
/* Seed for random numbers generator. */
int seed = 122;

/* Problem. */           
static struct problem *p = &tiny;

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: tsp [options]\n");
	printf("Brief: Traveler Salesman-problem.\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nclusters <value> Set number of clusters\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - tiny\n");
	printf("                       - small\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("                       - huge\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     	0 /* Read argument.         	*/
	#define SET_NCLUSTERS 	1 /* Set number of clusters. 	*/
	#define SET_CLASS    	2 /* Set problem class.     	*/
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else if (!strcmp(argv[i], "huge"))
						p = &huge;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NCLUSTERS :
					nclusters = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nclusters"))
			state = SET_NCLUSTERS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nclusters < 1)
		usage();
}

int parametersOk() {
    int entries = queue_size(16, nclusters, p->nb_towns, NULL);
    int req_mem = sizeof(job_queue_node_t) * entries;
    if (req_mem > MAX_MEM_PER_CLUSTER) {
        printf("Error, not enough memory. Verify MAX_TOWNS (%d), MIN_JOBS_THREAD (%d), and MAX_MEM_PER_CLUSTER (%d) parameters. Requested memory: %d bytes (should be < %d bytes)\n",
	       MAX_TOWNS, MIN_JOBS_THREAD, MAX_MEM_PER_CLUSTER, req_mem, MAX_MEM_PER_CLUSTER);
        return 0;
    }
    return 1;
}

/*
 * Runs benchmark.
 */
int main (int argc, char **argv) {
	
	readargs(argc, argv);
	
	//mppa_init_time();

    if (parametersOk()) {
        /* Always run with 16 thraeds per cluster by default */
        run_tsp(16, p->nb_towns, seed, nclusters);
    } else {
        printf("Invalid parameters. Terminating execution.\n");
    }

	mppa_exit(0);
	
	return 0;
}

void run_tsp (int nb_threads, int nb_towns, int seed, int nb_clusters) {
	assert (nb_threads <= MAX_THREADS_PER_CLUSTER);
	assert (nb_clusters <= MAX_CLUSTERS);

	int rank, status = 0, i;
	int pid;
	int nb_partitions = get_number_of_partitions(nb_clusters);
	int finished_clusters = 0;
	int next_partition = 0;

	printf ("Number of clusters..: %3d\nNumber of partitions: %3d\nNumber of threads...: %3d\nNumber of Towns.....: %3d\nSeed................: %3d\n", 
		nb_clusters, nb_partitions, nb_threads, nb_towns, seed);

	//uint64_t start_comm_time, end_comm_time, comm_time = 0, master_time = 0;
	//uint64_t start = mppa_get_time();
	//end_comm_time = start;

	int comm_buffer_size = (nb_clusters + 1) * sizeof (int);
	comm_buffer = (int *) malloc(comm_buffer_size);
	for (i = 0; i <= nb_clusters; i++) 
		comm_buffer[i] = INT_MAX;

	barrier_t *sync_barrier = mppa_create_master_barrier (BARRIER_SYNC_MASTER, BARRIER_SYNC_SLAVE, nb_clusters);
	barrier_par_t barrier;
	barrier.void_t = sync_barrier;

	broad = mppa_create_broadcast (nb_clusters, BROADCAST_MASK, comm_buffer, comm_buffer_size, TRUE, callback_master);

	rqueue_t *rqueue_partition_request = mppa_create_read_rqueue(2 * sizeof(int), 128, 70, "[0..15]", 71, nb_clusters);
	
	rqueue_t **rqueue_partition_response = (rqueue_t **) malloc (nb_clusters * sizeof(rqueue_t *));
	for(i = 0; i < nb_clusters; i++)
	  rqueue_partition_response[i] = mppa_create_write_rqueue(sizeof(partition_interval_t), i, i + 72, "128", i + 72 + MAX_CLUSTERS, 1);

	char **argv = (char**) malloc(sizeof (char*) * 6);
	for (i = 0; i < 5; i++)
		argv[i] = (char*) malloc (sizeof (char) * 10);
	argv[5] = NULL;

	sprintf(argv[0], "%d", nb_threads); 
	sprintf(argv[1], "%d", nb_towns);
	sprintf(argv[2], "%d", seed);
	sprintf(argv[3], "%d", nb_clusters);

  	for (rank = 0; rank < nb_clusters; rank++) {
	    sprintf(argv[4], "%d", rank);
		pid = mppa_spawn(rank, NULL, "slave", (const char **)argv, NULL);
		assert(pid >= 0);
	}
	
	wait_barrier (barrier); //init barrier

	//Manage partition requests
	while(finished_clusters < nb_clusters) {
		int from[2];
		partition_interval_t partition_interval;

		//start_comm_time = mppa_get_time();
		//master_time += mppa_diff_time(end_comm_time, start_comm_time);
		mppa_read_rqueue (rqueue_partition_request, &from, 2 * sizeof(int));
		//end_comm_time = mppa_get_time();
		//comm_time += mppa_diff_time(start_comm_time, end_comm_time);

		partition_interval = get_next_partition_default_impl(nb_partitions, nb_clusters, &next_partition, from[1]);
		
		if(partition_interval.start == -1) 
			finished_clusters++;

		//start_comm_time = mppa_get_time();
		//master_time += mppa_diff_time(end_comm_time, start_comm_time);
		mppa_write_rqueue (rqueue_partition_response[from[0]], &partition_interval, sizeof(partition_interval_t));
		//end_comm_time = mppa_get_time();
		//comm_time += mppa_diff_time(start_comm_time, end_comm_time);
		
		nsend++;
		data_sent += sizeof(partition_interval_t);
		nreceive++;
		data_received += 2 * sizeof(int);
	}

	wait_barrier (barrier); //end barrier

    for (rank = 0; rank < nb_clusters; rank++) {
		status = 0;
	   	if ((status = mppa_waitpid(rank, &status, 0)) < 0) {
	    	printf("[I/O] Waitpid on cluster %d failed.\n", rank);
	     	mppa_exit(status);
	   	}
	}

	int min = INT_MAX;
	for (i = 0; i < nb_clusters; i++)
		min = (comm_buffer[i] < min) ? comm_buffer[i] : min;

	mppa_close_barrier(sync_barrier);
	mppa_close_broadcast(broad);

	mppa_close_rqueue(rqueue_partition_request);
	for(i = 0; i < nb_clusters; i++)
		mppa_close_rqueue(rqueue_partition_response[i]);

	free(comm_buffer);
	
	for (i = 0; i < 4; i++)
		free(argv[i]);
	free(argv);

	//uint64_t end = mppa_get_time();
   	//uint64_t exec_time = mppa_diff_time(start, end);

	printf("shortest path size = %5d\n", min);
	
	/*printf("timing statistics:\n");
	LOG("  master time........: %f\n", master_time/1000000.0);
	LOG("  comm time..........: %f\n", (exec_time - comm_time - master_time)/1000000.0);
	LOG("  master blocked time: %f\n", comm_time/1000000.0);
	printf("  total time: %f\n", exec_time/1000000.0); */
	printf("data exchange statistics:\n");
	printf("  data sent:            %d\n", data_sent);
	printf("  number sends:         %u\n", nsend);
	printf("  data received:        %d\n", data_received);
	printf("  number receives:      %u\n", nreceive);
}

void new_minimun_distance_found(tsp_t_pointer tsp) {
	printf("SHOULD NOT BE HERE!!!\n");
}

partition_interval_t get_next_partition(tsp_t_pointer tsp) {
	printf("SHOULD NOT BE HERE!!!\n");
	partition_interval_t dummy;
	dummy.start = -1;
	dummy.end = -1;	
	return dummy;
}

void callback_master (mppa_sigval_t sigval) {	
}
