#ifndef LAVA_H_
#define LAVA_H_
//
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------200
//	DEFINE / INCLUDE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------200

	typedef struct
	{
		double x, y, z;

	} THREE_VECTOR;

	typedef struct
	{
		double v, x, y, z;

	} FOUR_VECTOR;

	typedef struct box_str
	{
		// home box
		int x, y, z;
		int number;
		long offset;

		// neighbor boxes
		int nn;
		struct nei_str
		{
			int x, y, z;
			int number;
			long offset;

		}  nei[26];

	} box_str;

	extern void kernel_cpu(box_str *, int, FOUR_VECTOR *, double *, FOUR_VECTOR *, int);

#endif
