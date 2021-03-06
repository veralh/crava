/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#include <iostream>
#include <string.h>

#include "lib/utils.h"
#include "src/definitions.h"

#include "fftw.h"
#include "rfftw.h"
#include "fftw-int.h"
#include "f77_func.h"

#include "nrlib/iotools/logkit.hpp"

//------------------------------------------------------------
void
Utils::writeTitler(const std::string & text)
{
  const int length = static_cast<int>(text.length());
  LogKit::LogFormatted(LogKit::Low,"\n%s\n",text.c_str());
  for (int i = 0 ; i < length ; i++)
    LogKit::LogFormatted(LogKit::Low,"-");
  LogKit::LogFormatted(LogKit::Low,"\n");
}

//------------------------------------------------------------
void
Utils::copyVector(const int * from,
                  int       * to,
                  int         ndim)
{
  for (int i = 0 ; i < ndim ; i++)
    to[i] = from[i];
}

//------------------------------------------------------------
void
Utils::copyVector(const float * from,
                  float       * to,
                  int           ndim)
{
  for (int i = 0 ; i < ndim ; i++)
    to[i] = from[i];
}

//------------------------------------------------------------
void
Utils::copyMatrix(const float ** from,
                  float       ** to,
                  int            ndim1,
                  int            ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++)
    for (int j = 0 ; j < ndim2 ; j++)
      to[i][j] = from[i][j];
}

//------------------------------------------------------------
void
Utils::writeVectorToFile(const std::string & fileName,
                         float             * vector,
                         int                  ndim)
{
  std::ofstream fout;
  NRLib::OpenWrite(fout,fileName);
  fout << std::setprecision(6);
  for (int i = 0 ; i < ndim ; i++) {
    fout << std::setw(6)  << i + 1
         << std::setw(12) << vector[i] << "\n";
  }
  fout << std::endl;
  fout.close();
}

//------------------------------------------------------------
void
Utils::writeVectorToFile(const std::string        & fileName,
                         const std::vector<float> & vector)
{
  std::ofstream fout;
  NRLib::OpenWrite(fout,fileName);
  fout << std::setprecision(6);
  for (size_t i = 0 ; i < vector.size() ; i++) {
    fout << std::setw(6)  << i + 1
         << std::setw(12) << vector[i] << "\n";
  }
  fout << std::endl;
  fout.close();
}

//------------------------------------------------------------
void
Utils::writeVector(float * vector,
                   int     ndim)
{
  for (int i = 0 ; i < ndim ; i++) {
    LogKit::LogFormatted(LogKit::Low,"%10.6f ",vector[i]);
  }
  LogKit::LogFormatted(LogKit::Low,"\n");
}

//------------------------------------------------------------
void
Utils::writeVector(double * vector,
                   int      ndim)
{
  for (int i = 0 ; i < ndim ; i++) {
    LogKit::LogFormatted(LogKit::Low,"%10.6f ",vector[i]);
  }
  LogKit::LogFormatted(LogKit::Low,"\n");
}

//------------------------------------------------------------
void
Utils::writeMatrix(float ** matrix,
                   int      ndim1,
                   int      ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++) {
    for (int j = 0 ; j < ndim2 ; j++) {
      LogKit::LogFormatted(LogKit::Low,"%10.6f ",matrix[i][j]);
    }
    LogKit::LogFormatted(LogKit::Low,"\n");
  }
}

//------------------------------------------------------------
void
Utils::writeMatrix(double ** matrix,
                   int       ndim1,
                   int       ndim2)
{
  for (int i = 0 ; i < ndim1 ; i++) {
    for (int j = 0 ; j < ndim2 ; j++) {
      LogKit::LogFormatted(LogKit::Low,"%10.6f ",matrix[i][j]);
    }
    LogKit::LogFormatted(LogKit::Low,"\n");
  }
}

//------------------------------------------------------------
void
Utils::fft(fftw_real* rAmp,fftw_complex* cAmp,int nt)
{
  rfftwnd_plan p1 = rfftwnd_create_plan(1, &nt, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE | FFTW_IN_PLACE);
  rfftwnd_one_real_to_complex(p1, rAmp, cAmp);
  fftwnd_destroy_plan(p1);
}

//------------------------------------------------------------
void
Utils::fftInv(fftw_complex* cAmp,fftw_real* rAmp,int nt)
{
  rfftwnd_plan p2 = rfftwnd_create_plan(1, &nt, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE | FFTW_IN_PLACE);
  rfftwnd_one_complex_to_real(p2, cAmp, rAmp);
  fftwnd_destroy_plan(p2);
  double sf = 1.0/double(nt);
  for(int i=0;i<nt;i++)
    rAmp[i]*=fftw_real(sf);
}
//-----------------------------------------------------------
int
Utils::findEnd(std::string & seek, int start, std::string & find)
{
  size_t i    = start;
  size_t end  = seek.length();
  size_t ok   = find.length();
  size_t flag = 0;

  while(flag < ok && i < end)
  {
    if(seek[i] == find[flag])
      flag++;
    else
      flag = 0;
    i++;
  }
  if(flag == ok)
    return(static_cast<int>(i)-1);
  else
    return(-1);
}
//-------------------------------------------------------------
void
Utils::readUntilStop(int pos, std::string & in, std::string & out, std::string read)
{
  /*
    reads a string from position pos+1
    until the terminating char given in stop
    returns the string excluding the stop
  */

  size_t stop = in.find(read,pos);
  if (stop != std::string::npos)
    stop = in.length();
  out = "";
  size_t i=0;
  while (i < stop-pos-1)
  {
    out.push_back(in[pos+1+i]);
    i++;
  }
}
