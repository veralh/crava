/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#include <iostream>
#include <iomanip>
#include <math.h>

#include "nrlib/flens/nrlib_flens.hpp"
#include "nrlib/iotools/logkit.hpp"

#include "fftw.h"
#include "rfftw.h"
#include "fftw-int.h"
#include "f77_func.h"

#include "src/definitions.h"
#include "src/blockedlogsforrockphysics.h"
#include "src/blockedlogs.h"
#include "src/welldata.h"
#include "src/wavelet.h"
#include "src/wavelet1D.h"
#include "src/fftgrid.h"
#include "src/simbox.h"
#include "src/modelsettings.h"
#include "src/io.h"

#include "rplib/demmodelling.h"

BlockedLogsForRockPhysics::BlockedLogsForRockPhysics(WellData  * well,
                                                     Simbox    * simbox)
{
  int nFacies = well->getNFacies();
  facies_names_.resize(nFacies);
  for(int i=0; i<nFacies; i++)
    facies_names_[i] = well->getFaciesName(i);

  int * bInd    = new int[well->getNd()]; // Gives which block each well log entry contributes to
  int   nLayers = simbox->getnz();
  int   nBlocks;
  int   firstM;
  int   lastM;
  int   firstB;
  int   lastB;
  int * ipos;                     ///<
  int * jpos;                     ///< Simbox IJK value for block
  int * kpos;                     ///<
  float dz;

  BlockedLogs::findSizeAndBlockPointers(well, simbox, bInd, nLayers, firstM, lastM, nBlocks);

  BlockedLogs::findBlockIJK(well, simbox, bInd, firstM, lastM, nLayers, nBlocks, ipos, jpos, kpos, dz, firstB, lastB);

  delete [] ipos;
  delete [] jpos;
  delete [] kpos;

  int dummy;

  float * blocked_alpha    = NULL;
  float * blocked_beta     = NULL;
  float * blocked_rho      = NULL;
  float * blocked_porosity = NULL;
  int   * blocked_facies   = NULL;

  BlockedLogs::blockContinuousLog(bInd, well->getAlpha(dummy), firstM, lastM, nBlocks, blocked_alpha);
  BlockedLogs::blockContinuousLog(bInd, well->getBeta(dummy), firstM, lastM, nBlocks, blocked_beta);
  BlockedLogs::blockContinuousLog(bInd, well->getRho(dummy), firstM, lastM, nBlocks, blocked_rho);
  //BlockedLogs::blockContinuousLog(bInd, well->getPorosity(dymmy), firstM, lastM, nBlocks, blocked_porosity); //NBNB Marit: Can not implement well->getPorosity(dymmy) before multizone inversion is ready
  BlockedLogs::blockDiscreteLog(bInd, well->getFacies(dummy), well->getFaciesNr(), well->getNFacies(), firstM, lastM, nBlocks, blocked_facies);

  delete [] bInd;

  alpha_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));
  beta_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));
  rho_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));
  porosity_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));

  assignToFacies(blocked_alpha, blocked_facies, well->getFaciesNr(), alpha_);
  assignToFacies(blocked_beta, blocked_facies, well->getFaciesNr(), beta_);
  assignToFacies(blocked_rho, blocked_facies, well->getFaciesNr(), rho_);
  assignToFacies(blocked_porosity, blocked_facies, well->getFaciesNr(), porosity_);

  delete [] blocked_alpha;
  delete [] blocked_beta;
  delete [] blocked_rho;
  delete [] blocked_porosity;
  delete [] blocked_facies;

  bulk_modulus_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));
  shear_modulus_.resize(nFacies, std::vector<float>(nBlocks, RMISSING));

  for(int i=0; i<nFacies; i++) {
    for(int j=0; j<nBlocks; j++) {
      if(alpha_[i][j] != RMISSING && beta_[i][j] != RMISSING && rho_[i][j] != RMISSING) {

        double bulk;
        double shear;

        DEMTools::CalcElasticParamsFromSeismicParams(alpha_[i][j], beta_[i][j], rho_[i][j], bulk, shear);

        bulk_modulus_[i][j]  = static_cast<float>(bulk);
        shear_modulus_[i][j] = static_cast<float>(shear);
      }
    }
  }

}

//------------------------------------------------------------------------------
BlockedLogsForRockPhysics::~BlockedLogsForRockPhysics(void)
{
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getAlphaForFacies(const std::string & facies_name)
{
  std::vector<float> alpha_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      alpha_given_facies = alpha_[i];
  }

  return alpha_given_facies;
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getBetaForFacies(const std::string & facies_name)
{
  std::vector<float> beta_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      beta_given_facies = beta_[i];
  }

  return beta_given_facies;
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getRhoForFacies(const std::string & facies_name)
{
  std::vector<float> rho_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      rho_given_facies = rho_[i];
  }

  return rho_given_facies;
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getBulkForFacies(const std::string & facies_name)
{
  std::vector<float> bulk_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      bulk_given_facies = bulk_modulus_[i];
  }

  return bulk_given_facies;
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getShearForFacies(const std::string & facies_name)
{
  std::vector<float> shear_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      shear_given_facies = shear_modulus_[i];
  }

  return shear_given_facies;
}

//------------------------------------------------------------------------------
std::vector<float>
BlockedLogsForRockPhysics::getPorosityForFacies(const std::string & facies_name)
{
  std::vector<float> porosity_given_facies;
  for(size_t i=0; i<facies_names_.size(); i++) {
    if(facies_name == facies_names_[i])
      porosity_given_facies = porosity_[i];
  }

  return porosity_given_facies;
}

//------------------------------------------------------------------------------
void
BlockedLogsForRockPhysics::assignToFacies(const float                      * wellLog,
                                          const int                        * faciesLog,
                                          const int                        * faciesNumbers,
                                          std::vector<std::vector<float> > & blockedLog)
{
  if (wellLog != NULL) {
    int nFacies = blockedLog.size();
    int nBlocks = blockedLog[0].size();

    for (int m = 0; m < nBlocks; m++) {
      if (wellLog[m] != RMISSING && faciesLog[m] != IMISSING) {
        for(int j=0; j<nFacies; j++) {
          if(faciesNumbers[j] == faciesLog[m])
            blockedLog[j][m] = wellLog[m];
        }
      }
    }
  }
}
