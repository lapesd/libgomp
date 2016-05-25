/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * balance.c - Workload balancer.
 */

/*
 * Balances workload.
 */
void balance(int *work, int n, int k)
{
	int i, j;
	
	j = 0;
	
	/* Balance workload. */
	if (n & 1)
	{
		work[0] = 0;
		for (i = 1; i <= (n >> 1); i++)
		{
			work[i] = j;
			work[n - i] = j;
			
			if ((++j) == k)
				j = 0;
		}
	}
	else
	{
		for (i = 0; i < (n >> 1); i++)
		{
			work[i] = j;
			work[n - i - 1] = j;
			
			if ((++j) == k)
				j = 0;
		}
	}
}
