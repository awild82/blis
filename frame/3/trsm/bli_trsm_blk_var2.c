/*

   BLIS    
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas at Austin nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "blis.h"

void bli_trsm_blk_var2
     (
       obj_t*  a,
       obj_t*  b,
       obj_t*  c,
       cntx_t* cntx,
       cntl_t* cntl,
       thrinfo_t* thread
     )
{
	obj_t b1, c1;

	dir_t direct;

	dim_t i;
	dim_t b_alg;
	dim_t my_start, my_end;

	// Determine the direction in which to partition (forwards or backwards).
	direct = bli_l3_direct( a, b, c, cntx );

	// Prune any zero region that exists along the partitioning dimension.
	bli_l3_prune_unref_mparts_n( a, b, c, cntx );

	// Determine the current thread's subpartition range.
	bli_thread_get_range_ndim
	(
	  direct, thread, a, b, c, cntl, cntx,
      &my_start, &my_end
	);

	// Partition along the n dimension.
	for ( i = my_start; i < my_end; i += b_alg )
	{
		// Determine the current algorithmic blocksize.
		b_alg = bli_determine_blocksize( direct, i, my_end, b,
		                                 bli_cntl_bszid( cntl ), cntx );

		// Acquire partitions for B1 and C1.
		bli_acquire_mpart_ndim( direct, BLIS_SUBPART1,
		                        i, b_alg, b, &b1 );
		bli_acquire_mpart_ndim( direct, BLIS_SUBPART1,
		                        i, b_alg, c, &c1 );

		// Perform trsm subproblem.
		bli_trsm_int
		(
		  &BLIS_ONE,
		  a,
		  &b1,
		  &BLIS_ONE,
		  &c1,
		  cntx,
		  bli_cntl_sub_node( cntl ),
		  bli_thrinfo_sub_node( thread )
		);
	}
}

