#include <mpi.h>
#include <complex.h>
#include <fftw3-mpi.h>
#include <iostream>
#include "getopt.h"
#include "seconds.h"
#include "timing.h"
#include "cmult-sse2.h"
#include "exmpiutils.h"

using namespace std;
using namespace utils;
using namespace fftwpp;


void init3r(double *f,
	    const int local_n0, const int local_n0_start,
	    const int N1, const int N2)
{
  for (int i = 0; i < local_n0; ++i) {
    for (int j = 0; j < N1; ++j) {
      for (int k = 0; k < N2; ++k) {
	f[(i*N1 + j) * (2*(N2/2+1)) + k] =
	  10 * (i + local_n0_start) + j + 0.1*k;
      }
    }
  }
}

void show3r(const double *f, const int local_n0, const int N1, const int N2)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  for(int r = 0; r < size; ++r) {
    MPI_Barrier(MPI_COMM_WORLD);
    if(r == rank) {
      cout << "process " << r << endl;
      for (int i = 0; i < local_n0; ++i) {
	for (int j = 0; j < N1; ++j) {
	  for (int k = 0; k < N2; ++k) {
	    cout << f[(i*N1 + j) * (2*(N2/2+1)) + k] << " ";
	  }
	  cout << endl;
	}
	cout << endl;
      }
    }
  }
}

void show3c(const fftw_complex *F, const int local_n0,
	    const int N1, const int N2p)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  for(int r = 0; r < size; ++r) {
    MPI_Barrier(MPI_COMM_WORLD);
    if(r == rank) {
      cout << "process " << r << endl;
      double *Fd = (double*) F;
      for (int i = 0; i < local_n0; ++i) {
  	for (int j = 0; j < N1; ++j) {
  	  for (int k = 0; k < N2p; ++k) {
  	    int pos = i * N1 * N2p + j * N2p + k;
	    cout << "(" << Fd[2 * pos] << ","  << Fd[2 * pos + 1] << ") ";
	  }
	  cout << endl;
	}
      }
    }
  }
}

int main(int argc, char **argv)
{
  int N=4;
  int m=4;
  int nthreads=1; // Number of threads
#ifdef __GNUC__	
  optind=0;
#endif	
  for (;;) {
    int c = getopt(argc,argv,"N:m:T:");
    if (c == -1) break;
    
    switch (c) {
      case 0:
        break;
      case 'N':
        N=atoi(optarg);
        break;
      case 'm':
        m=atoi(optarg);
        break;
      case 'T':
        nthreads=atoi(optarg);
        break;
    }
  }

  const unsigned int m0 = m;
  const unsigned int m1 = m;
  const unsigned int m2 = m;
  const unsigned int N0 = m0;
  const unsigned int N1 = m1;
  const unsigned int N2 = m2;

  const unsigned int N2p = N2 / 2 + 1;
  
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  int threads_ok = provided >= MPI_THREAD_FUNNELED;
  
  int mpirank;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);

  int mpisize;
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

  if(threads_ok)
    threads_ok = fftw_init_threads();
  fftw_mpi_init();
  
  if(threads_ok)
    fftw_plan_with_nthreads(nthreads);
  else 
    if(mpirank ==0) cout << "threads not ok!" << endl;
  
  /* get local data size and allocate */
  ptrdiff_t local_n0;
  ptrdiff_t local_n0_start;
  ptrdiff_t alloc_local=fftw_mpi_local_size_3d(N0,N1,N2p,MPI_COMM_WORLD,
					       &local_n0, &local_n0_start);
  
  double* f=fftw_alloc_real(2 * alloc_local);
  fftw_complex* F=fftw_alloc_complex(alloc_local);
  
  fftw_plan rcplan=fftw_mpi_plan_dft_r2c_3d(N0, N1, N2, f, F, MPI_COMM_WORLD,
					    FFTW_MEASURE);
  
  fftw_plan crplan=fftw_mpi_plan_dft_c2r_3d(N0, N1, N2, F, f, MPI_COMM_WORLD,
					    FFTW_MEASURE);

  init3r(f, local_n0, local_n0_start, N1, N2);
  show3r(f, local_n0, N1, N2);
  fftw_mpi_execute_dft_r2c(rcplan,f,F);
  show3c(F, local_n0, N1, N2p);
  fftw_mpi_execute_dft_c2r(crplan,F,f);
  show3r(f, local_n0, N1, N2);
  
  double *T=new double[N];
  for(int i=0; i < N; ++i) {
    init3r(f, local_n0, local_n0_start, N1, N2);
    seconds();
    fftw_mpi_execute_dft_r2c(rcplan,f,F);
    T[i]=seconds();
    //fftw_mpi_execute_dft_c2r(crplan,F,f);
  }  
  if(mpirank == 0)
    timings("FFT",m,T,N);
  delete[] T;

  fftw_destroy_plan(rcplan);
  fftw_destroy_plan(crplan);

  MPI_Finalize();
}
