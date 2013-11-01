#include "mpitranspose.h"
#include "cmult-sse2.h"

namespace fftwpp {

bool mpitranspose::overlap=true;
double mpitranspose::safetyfactor=2.0; // For conservative latency estimate.

inline void copy(Complex *from, Complex *to, unsigned int length,
                 unsigned int threads=1)
{
#ifndef FFTWPP_SINGLE_THREAD
#pragma omp parallel for num_threads(threads)
#endif  
for(unsigned int i=0; i < length; ++i)
  to[i]=from[i];
}

void mpitranspose::inphase0(Complex *data)
{
  if(size == 1) return;
  
  unsigned int blocksize=2*n*(a > 1 ? b : a)*m*L;
  Ialltoall(data,blocksize,MPI_DOUBLE,work,blocksize,MPI_DOUBLE,split2,
            request,sched2);
}
  
void mpitranspose::insync0(Complex *data)
{
  if(size == 1) return;
  Wait(split2size-1,request,sched2);
}

void mpitranspose::inphase1(Complex *data)
{
  if(a > 1) {
    Tin2->transpose(work,data); // a x n*b x m*L
    unsigned int blocksize=2*n*a*m*L;
    Ialltoall(data,blocksize,MPI_DOUBLE,work,blocksize,MPI_DOUBLE,split,
             request,sched);
  }
}  
    
void mpitranspose::insync1(Complex *data)
{
  if(a > 1)
    Wait(splitsize-1,request,sched);
}

void mpitranspose::inpost(Complex *data)
{
  if(size == 1) return;
  Tin1->transpose(work,data); // b x n*a x m*L
}

void mpitranspose::outphase0(Complex *data)
{
  if(size == 1) return;
  
  // Inner transpose each N/a x M/a matrix over b processes
  Tout1->transpose(data,work); // n*a x b x m*L
  unsigned int blocksize=2*n*a*m*L;
  Ialltoall(work,blocksize,MPI_DOUBLE,data,blocksize,MPI_DOUBLE,split,
            request,sched);
}

void mpitranspose::outsync0(Complex *data) 
{
  if(a > 1)
    Wait(splitsize-1,request,sched);
}
  
void mpitranspose::outphase1(Complex *data) 
{
  if(a > 1) {
  // Outer transpose a x a matrix of N/a x M/a blocks over a processes
    Tout2->transpose(data,work); // n*b x a x m*L
    unsigned int blocksize=2*n*b*m*L;
    Ialltoall(work,blocksize,MPI_DOUBLE,data,blocksize,MPI_DOUBLE,split2,
              request,sched2);
  }
}

void mpitranspose::outsync1(Complex *data) 
{
  if(size > 1)
    Wait(split2size-1,request,sched2);
}
  
void mpitranspose::nMTranspose(Complex *data)
{
  if(!Tin3) Tin3=new Transpose(n,M,L,data,work,threads);
  Tin3->transpose(data,work); // n X M x L
  copy(work,data,n*M*L,threads);
}
  
void mpitranspose::NmTranspose(Complex *data)
{
  if(!Tout3) Tout3=new Transpose(N,m,L,data,work,threads);
  Tout3->transpose(data,work); // N x m x L
  copy(work,data,N*m*L,threads);
}
  
/* Given a process which_pe and a number of processes npes, fills
   the array sched[npes] with a sequence of processes to communicate
   with for a deadlock-free, optimum-overlap all-to-all communication.
   (All processes must call this routine to get their own schedules.)
   The schedule can be re-ordered arbitrarily as long as all processes
   apply the same permutation to their schedules.

   The algorithm here is based upon the one described in:
   J. A. M. Schreuder, "Constructing timetables for sport
   competitions," Mathematical Programming Study 13, pp. 58-67 (1980). 
   In a sport competition, you have N teams and want every team to
   play every other team in as short a time as possible (maximum overlap
   between games).  This timetabling problem is therefore identical
   to that of an all-to-all communications problem.  In our case, there
   is one wrinkle: as part of the schedule, the process must do
   some data transfer with itself (local data movement), analogous
   to a requirement that each team "play itself" in addition to other
   teams.  With this wrinkle, it turns out that an optimal timetable
   (N parallel games) can be constructed for any N, not just for even
   N as in the original problem described by Schreuder.
*/
void fill1_comm_sched(int *sched, int which_pe, int npes)
{
  int pe, i, n, s = 0;
//  assert(which_pe >= 0 && which_pe < npes);
  if (npes % 2 == 0) {
    n = npes;
    sched[s++] = which_pe;
  }
  else
    n = npes + 1;
  for (pe = 0; pe < n - 1; ++pe) {
    if (npes % 2 == 0) {
      if (pe == which_pe) sched[s++] = npes - 1;
      else if (npes - 1 == which_pe) sched[s++] = pe;
    }
    else if (pe == which_pe) sched[s++] = pe;

    if (pe != which_pe && which_pe < n - 1) {
      i = (pe - which_pe + (n - 1)) % (n - 1);
      if (i < n/2)
        sched[s++] = (pe + i) % (n - 1);
	       
      i = (which_pe - pe + (n - 1)) % (n - 1);
      if (i < n/2)
        sched[s++] = (pe - i + (n - 1)) % (n - 1);
    }
  }
//  assert(s == npes);
}

} // end namespace fftwpp
