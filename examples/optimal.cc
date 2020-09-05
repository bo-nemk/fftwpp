// Determine optimal sizes for 1D complex to complex in-place FFTs.

#include "fftw++.h"

// Compile with:
// g++ -I .. -fopenmp example0.cc ../fftw++.cc -lfftw3 -lfftw3_omp

using namespace std;
using namespace utils;
using namespace fftwpp;

int main()
{
  fftw::maxthreads=get_max_threads();

  double eps=0.5;
  
  unsigned int N=1024;
  
  Complex *f=ComplexAlign(N);
  for(unsigned int i=0; i < N; i++) f[i]=i;
    
  cout << "# length\tmean\tstdev" << endl;

  for(unsigned int n=2; n < N; ++n) {
    utils::statistics S;
    unsigned int K=1;
    
    fft1d Forward(n,-1);
    
    for(;;) {
      double t0=utils::totalseconds();
      for(unsigned int i=0; i < K; ++i)
        Forward.fft(f);
      double t=utils::totalseconds();
      S.add((t-t0)/K);
      double mean=S.mean();
      if(K*mean < 100.0/CLOCKS_PER_SEC) {
        K *= 2;
        S.clear();
      }
      if(S.count() >= 2 && S.stdev() < eps*mean) {
        cout << n << "\t" << S.mean() << "\t" << S.stdev() << endl ;
        break;
      }
    }
  }
  
  deleteAlign(f);
}
