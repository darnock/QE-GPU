/*
 * Copyright (C) 2001-2012 Quantum ESPRESSO group
 * Copyright (C) 2010-2011 Irish Centre for High-End Computing (ICHEC)
 *
 * This file is distributed under the terms of the
 * GNU General Public License. See the file `License'
 * in the root directory of the present distribution,
 * or http://www.gnu.org/copyleft/gpl.txt .
 *
 */

#include <cuda.h>
#include <cufft.h>
#include <cuda_runtime.h>
#include "cublas_v2.h"

#ifndef __QE_CUDA_ENVIRONMENT_H
#define __QE_CUDA_ENVIRONMENT_H

#if defined __GPU_NVIDIA_13

#define __CUDA_THREADPERBLOCK__ 256
#define __CUDA_MAXNUMBLOCKS__ 65535

#define __NUM_FFT_MULTIPLAN__ 4

#define __CUDA_TxB_ADDUSDENS_COMPUTE_AUX__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_VLOCPSI_BUILD_PSIC__ 128
#define __CUDA_TxB_VLOCPSI_PSIC__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_VLOCPSI_PROD__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_VLOCPSI_HPSI__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_NEWD_QGM__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_NEWD_DEEPQ__ __CUDA_THREADPERBLOCK__

#elif defined __GPU_NVIDIA_20

#define __CUDA_THREADPERBLOCK__ 512
#define __CUDA_MAXNUMBLOCKS__ 65535

#define __NUM_FFT_MULTIPLAN__ 4

#define __CUDA_TxB_ADDUSDENS_COMPUTE_AUX__ 768
#define __CUDA_TxB_VLOCPSI_BUILD_PSIC__ 128
#define __CUDA_TxB_VLOCPSI_PSIC__ 64
#define __CUDA_TxB_VLOCPSI_PROD__ 128
#define __CUDA_TxB_VLOCPSI_HPSI__ 448
#define __CUDA_TxB_NEWD_QGM__ 512
#define __CUDA_TxB_NEWD_DEEPQ__ 512


#elif defined __GPU_NVIDIA_30

#define __CUDA_THREADPERBLOCK__ 512
#define __CUDA_MAXNUMBLOCKS__ 65535

#define __NUM_FFT_MULTIPLAN__ 4

#define __CUDA_TxB_ADDUSDENS_COMPUTE_AUX__ 128
#define __CUDA_TxB_VLOCPSI_BUILD_PSIC__ 128
#define __CUDA_TxB_VLOCPSI_PSIC__ 256
#define __CUDA_TxB_VLOCPSI_PROD__ 512
#define __CUDA_TxB_VLOCPSI_HPSI__ 256
#define __CUDA_TxB_NEWD_QGM__ __CUDA_THREADPERBLOCK__
#define __CUDA_TxB_NEWD_DEEPQ__ __CUDA_THREADPERBLOCK__

#else

#define __CUDA_THREADPERBLOCK__ 256
#define __NUM_FFT_MULTIPLAN__ 1

#endif

#if defined __MAGMA
#define __SCALING_MEM_FACTOR__ 0.75
#else
#define __SCALING_MEM_FACTOR__ 0.95
#endif

#define MAX_QE_GPUS 8

typedef void* qeCudaMemDevPtr[MAX_QE_GPUS];
typedef size_t qeCudaMemSizes[MAX_QE_GPUS];
typedef int qeCudaDevicesBond[MAX_QE_GPUS];


extern qeCudaMemDevPtr qe_dev_scratch;
extern qeCudaMemDevPtr qe_dev_zero_scratch;

extern qeCudaMemSizes qe_gpu_mem_tot;
extern qeCudaMemSizes qe_gpu_mem_unused;

extern qeCudaDevicesBond qe_gpu_bonded;

extern cudaStream_t  qecudaStreams[ MAX_QE_GPUS ];
extern cublasHandle_t qecudaHandles[ MAX_QE_GPUS ];

// Pre-loaded data-structure
extern int * preloaded_nlsm_D, * preloaded_nls_D;

extern long ngpus_detected;
extern long ngpus_used;
extern long ngpus_per_process;

extern "C" size_t initCudaEnv();
extern "C" void closeCudaEnv();
extern "C" void preallocateDeviceMemory(int);
extern "C" void initPhigemm(int);

/*
 * These routines are exactly the same in "cutil_inline_runtime.h" but,
 * replicating them here, we remove the annoying dependency to CUTIL & SDK (Filippo)
 *
 * We define these calls here, so the user doesn't need to include __FILE__ and __LINE__
 * The advantage is the developers gets to use the inline function so they can debug
 */

#define qecudaGenericErr(err, routine, msg)  __qecudaGenericErr(err, routine, msg, __FILE__, __LINE__)
#define qecudaSafeCall(err)  __qecudaSafeCall(err, __FILE__, __LINE__)
#define qecudaGetLastError(msg)  __qecudaGetLastError(msg, __FILE__, __LINE__)
#define qecheck_cufft_call(err) __qecheck_cufft_call(err, __FILE__, __LINE__)


inline void __qecudaGenericErr( cudaError_t err, char* routine, char* msg, const char *file, const int line )
{
	if(err != cudaSuccess) {
		printf("[%s:%d] qecudaGenericErr() Runtime %s : %s (%s).\n",
					file, line, routine, msg, cudaGetErrorString( err) ); fflush(stdout);
		exit(EXIT_FAILURE);
	}
}

inline void __qecudaSafeCall( cudaError_t err, const char *file, const int line )
{
    if( cudaSuccess != err) {
		printf("[%s:%d] qecudaSafeCall() Runtime API error : %s.\n",
					file, line, cudaGetErrorString( err) ); fflush(stdout);
		exit(EXIT_FAILURE);
    }
}

inline void __qecudaGetLastError(const char *errorMessage, const char *file, const int line )
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err) {
		printf("[%s:%d] qecudaGetLastError() error : %s : %s.\n",
				file, line, errorMessage, cudaGetErrorString( err) ); fflush(stdout);
		exit(EXIT_FAILURE);
    }
}

inline void __qecheck_cufft_call(  cufftResult cufft_err, const char *file, const int line )
{
	switch ( cufft_err ) {

	case CUFFT_INVALID_PLAN :
		fprintf( stderr, "\n[%s:%d] The plan parameter is not a valid handle! Program exits... \n", file, line );
		break;

	case CUFFT_INVALID_VALUE :
		fprintf( stderr, "\n[%s:%d] The idata, odata, and/or direction parameter is not valid! Program exits... \n", file, line );
		break;

	case CUFFT_EXEC_FAILED :
		fprintf( stderr, "\n[%s:%d] CUFFT failed to execute the transform on GPU! Program exits... \n", file, line );
		break;

	case CUFFT_SETUP_FAILED :
		fprintf( stderr, "\n[%s:%d] CUFFT library failed to initialize! Program exits... \n", file, line );
		break;

	case CUFFT_INVALID_SIZE :
		fprintf( stderr, "\n[%s:%d] The nx parameter is not a supported size! Program exits... \n", file, line );
		break;

	case CUFFT_INVALID_TYPE :
		fprintf( stderr, "\n[%s:%d] The type parameter is not supported! Program exits... \n", file, line );
		break;

	case CUFFT_ALLOC_FAILED :
		fprintf( stderr, "\n[%s:%d] Allocation of GPU resources for the plan failed! Program exits... \n", file, line );
		break;

	case CUFFT_SUCCESS:
		break;

	default:
		fprintf( stderr, "\n[%s:%d] CUFFT returned not recognized value! %d\n", file, line, cufft_err );
		break;
	}

	if (cufft_err != CUFFT_SUCCESS) {
		exit(EXIT_FAILURE);
	}
}

#endif // __QE_CUDA_ENVIRONMENT_H
