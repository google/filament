// REQUIRED DEFINE FROM APP-SIDE: M
// REQUIRED DEFINE FROM APP-SIDE: N
// REQUIRED DEFINE FROM APP-SIDE: P
// REQUIRED DEFINE FROM APP-SIDE: WG_RECT_WIDTH
// REQUIRED DEFINE FROM APP-SIDE: WG_RECT_HEIGHT
// REQUIRED DEFINE FROM APP-SIDE: WG_SQUARE_SIDE
// REQUIRED DEFINE FROM APP-SIDE: WG_LINEAR_SIZE
// REQUIRED DEFINE FROM APP-SIDE: TILE_SQUARE
// REQUIRED DEFINE FROM APP-SIDE: TILE_RECT_M
// REQUIRED DEFINE FROM APP-SIDE: TILE_RECT_N
// REQUIRED DEFINE FROM APP-SIDE: TILE_RECT_P

// A: MxN matrix
// B: NxP matrix
// C: MxP matrix
// AT: A, transposed
// BT: A, transposed
// CT: A, transposed

/////////////////////////////// BASIC IMPLEMENTATIONS - NAIVE MULTIPLICATION ALGORITHM, SEVERAL TRANSPOSITION OPTIONS /////////////////////////////// 

/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_naive(const __global float A[M][N], const __global float B[N][P], __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += A[i][k] * B[k][j];
	}
	C[i][j] = sum;
}

/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
/// >>> Input matrix A transposed
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_transposedA(const __global float AT[N][M], const __global float B[N][P], __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += AT[k][i] * B[k][j];
	}
	C[i][j] = sum;
}

/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
/// >>> Input matrix B transposed
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_transposedB(__global float A[M][N], __global float BT[P][N], __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += A[i][k] * BT[j][k];
	}
	C[i][j] = sum;
}

/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
/// >>> Output matrix (C) transposed
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_transposedC(const __global float A[M][N], const __global float B[N][P], __global float CT[P][M])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += A[i][k] * B[k][j];
	}
	CT[j][i] = sum;
}


/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
/// >>> Input matrix (A) transposed
/// >>> Output matrix (C) transposed
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_transposedAC(const __global float AT[N][M], const __global float B[N][P], __global float CT[P][M])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += AT[k][i] * B[k][j];
	}
	CT[j][i] = sum;
}

/// >>> Naive implementation - each thread calculates one of the output items
/// >>> No local memory
/// >>> Input matrix (B) transposed
/// >>> Output matrix (C) transposed
__attribute((reqd_work_group_size(WG_RECT_WIDTH,WG_RECT_HEIGHT,1)))
__kernel void matmul_transposedBC(__global float A[M][N], __global float BT[P][N], __global float CT[P][M])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0;
	for (int k=0;k<N;++k)
	{
		sum += A[i][k] * BT[j][k];
	}
	CT[j][i] = sum;
}


/////////////////////////////// LINEAR WORKGROUP OPTIONS /////////////////////////////// 

/// >>> Workgroup is a "vertical" line
/// >>> No local memory
/// >>> Input matrix (A) transposed

__attribute((reqd_work_group_size(1,WG_LINEAR_SIZE,1)))
__kernel void matmul_linearwg_AT(const __global float AT[N][M], const __global float B[N][P], __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0.;
	for (int k=0;k<N;++k)
	{
		sum += AT[k][i] * B[k][j];
	}
	C[i][j] = sum;
}

/// >>> Workgroup is a "vertical" line
/// >>> No local memory
/// >>> Input matrix (B) transposed

// Naive implementation - each thread calculates one of the output items. No local. Transposed B. Workgroup forced to linear.
__attribute((reqd_work_group_size(1,WG_LINEAR_SIZE,1)))
__kernel void matmul_linearwg_BT(__global float A[M][N], __global float BT[P][N], __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float sum = 0.;
	for (int k=0;k<N;++k)
	{
		sum += A[i][k] * BT[j][k];
	}
	//C[i * P/4 + j] = sum;
	C[i][j] = sum;
}

/// >>> Workgroup is a "vertical" line
/// >>> No local memory
/// >>> Input matrix (B) transposed
/// >>> VEC4 inputs: more work per inner loop thread

// Basic float4 implementation - each thread calculates one of the output items, but reads are float4. No local. Transposed B. Workgroup forced to linear.
//__attribute((reqd_work_group_size(1,WG_LINEAR_SIZE,1)))
__kernel void matmul_linearwg_vec4(__global float4* A, __global float4* BT, __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	float4 sum = (float4)(0,0,0,0);
	for (int k=0;k<N/4;++k)
	{

		sum += A[i*(N/4)+k] * BT[j*(N/4)+k];
	}
	C[i][j] = sum.x + sum.y + sum.z + sum.w;;
}


/// >>> Workgroup is a "horizontal" line
/// >>> LOCAL MEMORY: Preloading an entire row of A into local memory. *** Expected to suffer badly for large values of N ***
/// >>> Input matrix (B) transposed
/// >>> Input matrices A and B are expressed as float4
#define VECS_PER_ROW (N / 4)
__attribute((reqd_work_group_size(1,WG_LINEAR_SIZE,1)))
__kernel void matmul_linearwg_vec4_local(__global float4* A, __global float4* BT, __global float C[M][P])
{
	int i = get_global_id(0); // 0..M
	int j = get_global_id(1); // 0..P
	int lid = get_local_id(1); // 0..32

	__local float4 localA[VECS_PER_ROW];

	// number of items fetched per instance: VECS_PER_ROW / WG_LINEAR_SIZE = 2
	event_t evt = async_work_group_copy(localA, &(A[VECS_PER_ROW * i]), VECS_PER_ROW, 0);
	wait_group_events(1, &evt);

	barrier(CLK_LOCAL_MEM_FENCE);

	float4 sum = (float4)(0,0,0,0);

	for (int k=0;k<VECS_PER_ROW;++k)
	{
		// k: 0..63
		sum += localA[k] * BT[j*(N/4)+k];
	}
	C[i][j] = sum.x + sum.y + sum.z + sum.w;;
}

/// >>> This algorithm is among the most effective for doing SGEMM on the large Desktop GPUs.
/// >>> Reasonably performing but not the best for mobile GPUs
/// >>> Workgroup is a SQUARE, simplifying a lot of math
/// >>> Calculation is done tile-by-tile, with an entire workgroup, each thread using whatever is needed from a tile of A and B ...
/// >>> ... to calculate part of the result of an item of C, and keeping the results into acc. Then, all threads together ... 
/// >>> ... move to the next tile until done
/// >>> LOCAL MEMORY: An TSMxTSN tile of a and a TSNxTSP (transposed ) tile of and B loaded into local memory
/// >>> Input matrix (B) transposed
#define TS WG_SQUARE_SIDE
//__attribute((reqd_work_group_size(WG_SQUARE_SIDE,WG_SQUARE_SIDE,1)))
__kernel void matmul_tile_square(const __global float A[M][N], const __global float BT[P][N], __global float C[M][P]) 
{
	
	// Thread identifiers
	const int row = get_local_id(0); // Local row ID (max: TS)
	const int col = get_local_id(1); // Local col ID (max: TS)
	const int globalRow = get_global_id(0); // Row ID of C (0..M)
	const int globalCol = get_global_id(1); // Col ID of C (0..P)
 
	// Local memory to fit a tile of TS*TS elements of A and B
	__local float Asub[TS][TS];
	__local float Bsub[TS][TS];
 
	// Initialise the accumulation register
	float acc = 0.0f;
	
	// Loop over all tiles
	const int numTiles = N/TS;
	for (int t=0; t<numTiles; t++) {
 
		// Load one tile of A and B into local memory
		const int tiledRow = TS*t + row;
		const int tiledCol = TS*t + col;
		Asub[row][col] = A[globalRow][tiledCol];
		Bsub[row][col] = BT[globalCol][tiledRow];
 
		// Synchronise to make sure the tile is loaded
		barrier(CLK_LOCAL_MEM_FENCE);
 
		// Perform the computation for a single tile
		for (int k=0; k<TS; k++) {
			acc += Asub[row][k] * Bsub[k][col];
		}
 
		// Synchronise before loading the next tile
		barrier(CLK_LOCAL_MEM_FENCE);
	}
 
	// Store the final result in C
	C[globalRow][globalCol] = acc;
}

/// >>> This algorithm is a generalization of the above, allowing non-square tiles
/// >>> Calculation is done tile-by-tile, with an entire workgroup, each thread using whatever is needed from a tile of A and B ...
/// >>> ... to calculate part of the result of an item of C, and keeping the results into acc. Then, all threads together ... 
/// >>> ... move to the next tile until done
/// >>> LOCAL MEMORY: Tiles for both A and B loaded into local memory
/// >>> Input matrix (B) transposed
/// >>> TILE_RECT_N must be defined and a multiple of BOTH TILE_RECT_M AND TILE_RECT_P

#define TSM TILE_RECT_M
#define TSN TILE_RECT_N
#define TSP TILE_RECT_P

// How many items must each instance fetch? (A, B items)
#define FETCHES_A (TSN/TSP)
#define FETCHES_BT (TSN/TSM)

// Tiled and coalesced version
__attribute((reqd_work_group_size(TSM,TSP,1)))
__kernel void matmul_tile_rect(const __global float A[M][N], const __global float BT[P][N], __global float C[M][P])
{
	
	// Thread identifiers
	const int row = get_local_id(0); // Local row ID (max: TS)
	const int col = get_local_id(1); // Local col ID (max: TS)
	const int globalRow = get_global_id(0); // Row ID of C (0..M)
	const int globalCol = get_global_id(1); // Col ID of C (0..P)
 
	// Local memory to fit a tile of TS*TS elements of A and B
	__local float Asub[TSM][TSN];
	__local float BTsub[TSP][TSN];
 
	// Initialise the accumulation register
	float acc = 0.0f;
	
	// Loop over all tiles
	const int numTiles = N/TSN;
	for (int t=0; t<numTiles; t++) {
 
		// Load one tile of A and B into local memory
		const int tiledColFetch = TSN*t + col;
		const int tiledRowFetch = TSN*t + row;
		for (int m=0;m<FETCHES_A;++m)
		{
			Asub[row][col + m * TSP] = A[globalRow][tiledColFetch + m * TSP];
		}
		for (int p=0;p<FETCHES_BT;++p)
		{
			BTsub[col][row + p * TSM] = BT[globalCol][tiledRowFetch + p * TSM];
		}
 
		// Synchronise to make sure the tile is loaded
		barrier(CLK_LOCAL_MEM_FENCE);
 
		// Perform the computation for a single tile
		for (int k=0; k<TSN; ++k) {
			acc += Asub[row][k]   * BTsub[col][k];
		}
 
		// Synchronise before loading the next tile
		barrier(CLK_LOCAL_MEM_FENCE);
	}
 
	// Store the final result in C
	C[globalRow][globalCol] = acc;
}


/// >>> This algorithm is a version of the above, only with vec4 items
/// >>> Calculation is done tile-by-tile, with an entire workgroup, each thread using whatever is needed from a tile of A and B ...
/// >>> ... to calculate part of the result of an item of C, and keeping the results into acc. Then, all threads together ... 
/// >>> ... move to the next tile until done
/// >>> LOCAL MEMRTile for both A and B loaded into local memory
/// >>> Input matrices A and B are expressed as float4
/// >>> Input matrix (B) transposed
/// >>> TILE_RECT_N must be defined and a multiple of BOTH TILE_RECT_M AND TILE_RECT_P

// DEFINITIONS
// How many vectors in the intermediate direction (N) 
// Must be at least as wide as both dimensions of the WG, and a multiple of both
#define TILE_WIDTH_VEC (TSN/4)

// Number of tiles for the N direction
#define NTILES ((N/4)/TILE_WIDTH_VEC)

// Tiles are different sizes for each matrix:
// For a results tile  ( C )	of size						TSM x TSP				float
// We will need:	A	: Will use a (horizontal) strip of	TSM x TILE_WIDTH_VEC	float4
//					BT	: Will use a (horizontal) strip of	TSP x TILE_WIDTH_VEC	float4

// We will be doing this one tile at a time:
// Fetch one tile of A and one tile of B - accumulate all the necessary value for each thread
// Move both directions to the next tile.

//__attribute((reqd_work_group_size(TSM,TSP,1)))
__kernel void matmul_tile_rect_vec4(const __global float4 A4[M][N/4], const __global float4 BT4[P][N/4], __global float C[M][P])
{
	
	// Thread identifiers
	const int row = get_local_id(0); // Local row ID (max: TS)
	const int col = get_local_id(1); // Local col ID (max: TS)
	const int globalRow = get_global_id(0); // Row ID of C (0..M)
	const int globalCol = get_global_id(1); // Col ID of C (0..P)

	// Local memory to fit a tile of elements A and B
	__local float4 Asub[TSM][TILE_WIDTH_VEC];
	__local float4 BTsub[TSP][TILE_WIDTH_VEC];

	// Initialise the accumulation register
	float4 acc = (float4)(0.0f,0.0f,0.0f,0.0f);
	
	// Loop over all tiles - there are the same number of tiles in both A and B matrices as N direction matches (obviously)
	for (int t=0; t<NTILES; t++)
	{

		// Load one tile of A and B(T) into local memory. Use all instances to do this:
		
		// Reading from A - each instance will bring an item for every time the WIDTH of the workgroup can 
		// fit in the width of the tile

// Number of A fetches per thread:    Number of vectors in local tile, divided by number of instance columns in workgroup (TSP)
// FOR Tile-width 8 (16 floats wide), TSP 4: Fetches = 2 vec4
#define FETCHES_A4 ((TILE_WIDTH_VEC/TSP))

// Number of B(T) fetches per thread: Number of vectors in local tile, divided by number of instance rows in workgroup (TSM)
// FOR Tile-width 8 (16 floats wide), TSM 8: Fetches = 1 vec4
#define FETCHES_BT4 ((TILE_WIDTH_VEC/TSM))

		const int tiledColFetch = TILE_WIDTH_VEC*t + col;
		for (int m=0;m<FETCHES_A4;++m) // for WG_X = 8, WG_Y = 4, WG_INTERMEDIATE MIN: 4x4 = 16      = 4 vecs - should be (1,2)
		{
			Asub[row][col + m * TSP] = A4[globalRow][tiledColFetch + m * TSP];
		}

		// Reading from A - each instance will bring an item for every time the HEIGHT of the workgroup can 
		// fit in the width of the tile

		const int tiledRowFetch = TILE_WIDTH_VEC*t + row;
		for (int p=0;p<FETCHES_BT4;++p) // for WG_X = 8, WG_Y = 4, WG_INTERMEDIATE MIN: 8x4 = 32 / 4 = 8 vecs - should be (1,2)
		{
			BTsub[col][row + p * TSM] = BT4[globalCol][tiledRowFetch + p * TSM];
		}

		// Synchronise to make sure the tile is loaded
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform the computation for a single tile
		for (int k=0; k<TILE_WIDTH_VEC; k++) {
			acc += (Asub[row][k] * BTsub[col][k]);
		}

		// Synchronise before loading the next tile
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	// Store the final result in C
	C[globalRow][globalCol] = acc.x + acc.y + acc.z + acc.w;
}
