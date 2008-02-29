#ifndef FFTGRID_H
#define FFTGRID_H

#include <assert.h>

#include "fft/include/fftw.h"

class SegY;
class Corr;
class Wavelet;
class Simbox;
class RandomGen;

class FFTGrid{
public:

  FFTGrid(int nx, int ny, int nz, int nxp, int nyp, int nzp);
  FFTGrid(FFTGrid * fftGrid);
  FFTGrid() {} //Dummy constructor needed for FFTFileGrid
  virtual ~FFTGrid();

  void setType(int cubeType) {cubetype_ = cubeType;}
  void setAngle(float angle) {theta_ = angle;}

  void                 fillInFromSegY(SegY * segy );            // No mode
  void                 fillInFromStorm(Simbox     * tmpSimbox, 
                                       Simbox     * actSimBox,
                                       float      * grid,
                                       const char * parName);   // No mode
  void                 fillInFromRealFFTGrid(FFTGrid& fftGrid); // No mode
  void                 fillInConstant(float value);             // No mode
  fftw_real*           fillInParamCorr(Corr* corr,int minIntFq);// No mode
  void                 fillInErrCorr(Corr* parCorr);            // No mode
  virtual void         fillInComplexNoise(RandomGen * ranGen);  // No mode/randomaccess

  void                 fillInTest(float value1, float value2);  // No mode /DEBUG
  void                 fillInFromArray(float *value);

  virtual fftw_complex getNextComplex() ;                       // Accessmode read/readandwrite
  virtual float        getNextReal() ;                          // Accessmode read/readandwrite
  virtual int          setNextComplex(fftw_complex);            // Accessmode write/readandwrite
  virtual int          setNextReal(float);                      // Accessmode write/readandwrite
  float                getRealValue(int i, int j, int k, bool extSimbox = false);  // Accessmode randomaccess
  int                  setRealValue(int i, int j, int k, float value, bool extSimbox = false);  // Accessmode randomaccess
  fftw_complex         getFirstComplexValue();    
  virtual int          square();                                // No mode/randomaccess
  virtual int          expTransf();                             // No mode/randomaccess
  virtual int          logTransf();                             // No mode/randomaccess
  virtual void         realAbs();
  virtual int          collapseAndAdd(float* grid);             // No mode/randomaccess
  virtual void         fftInPlace();	                        // No mode/randomaccess
  virtual void         invFFTInPlace();                         // No mode/randomaccess


  virtual void         add(FFTGrid* fftGrid);                   // No mode/randomaccess
  virtual void         multiply(FFTGrid* fftGrid);              // pointwise multiplication!    
  bool                 consistentSize(int nx,int ny, int nz, int nxp, int nyp, int nzp);
  int                  getCounterForGet() const {return(counterForGet_);}
  int                  getCounterForSet() const {return(counterForSet_);}
  int                  getNx()    const {return(nx_);}
  int                  getNy()    const {return(ny_);}
  int                  getNz()    const {return(nz_);}
  int                  getNxp()   const {return(nxp_);}
  int                  getNyp()   const {return(nyp_);}
  int                  getNzp()   const {return(nzp_);}
  int                  getRNxp()  const {return(rnxp_);}
  int                  getcsize() const {return(csize_);}
  int                  getrsize() const {return(rsize_);}
  float                getTheta() const {return(theta_);}  
  float                getScale() const {return(scale_);}
  bool                 getIsTransformed() const {return(istransformed_);}
  enum                 gridTypes{CTMISSING,DATA,PARAMETER,COVARIANCE};
  enum                 accessMode{NONE,READ,WRITE,READANDWRITE,RANDOMACCESS};
  virtual void         multiplyByScalar(float scalar);      //No mode/randomaccess
  int                  getType() const {return(cubetype_);}
  virtual void         setAccessMode(int mode){assert(mode>=0);}
  virtual void         endAccess(){}
  virtual void         writeFile(const char * fileName, const Simbox * simbox, bool writeSegy = true); //Use this instead of the ones below.
  virtual void         writeStormFile(const char * fileName, const Simbox * simbox, bool ascii = false, 
                                      bool padding = false, bool flat = false);//No mode/randomaccess
  virtual int          writeSegyFile(const char * fileName, const Simbox * simbox);   //No mode/randomaccess
  virtual void         writeAsciiFile(char * fileName);
  virtual void         writeAsciiRaw(char * fileName);

  virtual bool         isFile() {return(0);}    // indicates wether the grid is in memory or on disk  

  enum                 outputFormat{NOOUTPUT = 0, STORMFORMAT = 1, SEGYFORMAT = 2, STORMASCIIFORMAT = 4};
  void                 setOutputFormat(int format) {formatFlag_ = format;} 

  virtual void         createRealGrid();
  virtual void         createComplexGrid();

  //This function interpolates seismic in all missing traces inside area, and mirrors to padding.
  //Also interpolates in traces where energy is lower than treshold.
  virtual void         interpolateSeismic(float energyTreshold = 0);

  void                 checkNaN(); //NBNB Ragnar: For debug purpose. Negative number = OK.
  float                getDistToBoundary(int i, int n , int np); 

protected:
  int    cubetype_;        // see enum gridtypes above
  float  theta_;           // angle in angle gather (case of data)
  float  scale_;           // To keep track of the scalings after fourier transforms
  int    nx_;              // size of original grid in depth (time)
  int    ny_;              // size of original grid in lateral x direction 
  int    nz_;              // size of original grid in lateral y direction
  int    nxp_;             // size of padded FFT grid in depth (time) 
  int    nyp_;             // size of padded FFT grid in lateral x direction 
  int    nzp_;             // size of padded FFT grid in lateral y direction

  int    cnxp_;	           // size in x direction for storage inplace algorithm (complex grid) nxp_/2+1
  int    rnxp_;            // expansion in x direction for storage inplace algorithm (real grid) 2*(nxp_/2+1)

  int    csize_;           // size of complex grid, cnxp_*nyp_*nzp_
  int    rsize_;           // size of real grid rnxp_*nyp_*nzp_
  int    counterForGet_;   // active cell in grid
  int    counterForSet_;   // active cell in grid

  bool   istransformed_;   // true if the grid contain Fourier values (i.e complex variables)

  fftw_complex* cvalue_;   // values of complex parameter in grid points
  fftw_real*    rvalue_;   // values of real parameter in grid points

  static int  formatFlag_; // Decides format of output (see format types above).

  //int                 setPaddingSize(int n, float p); 
  int                   getFillNumber(int i, int n, int np );

  int                   getXSimboxIndex(int i){return(getFillNumber(i, nx_, nxp_ ));}
  int                   getYSimboxIndex(int j){return(getFillNumber(j, ny_, nyp_ ));}
  int                   getZSimboxIndex(int k);
  void                  computeCircCorrT(Corr* corr,fftw_real* CircCorrT);
  void                  makeCircCorrTPosDef(fftw_real* CircularCorrT,int minIntFq);
  fftw_complex*         fft1DzInPlace(fftw_real*  in);
  fftw_real*            invFFT1DzInPlace(fftw_complex* in);

  //Supporting functions for interpolateSeismic
  int                   interpolateTrace(int index, short int * flags, int i, int j);
  void                  extrapolateSeismic(int imin, int imax, int jmin, int jmax);


};
#endif
