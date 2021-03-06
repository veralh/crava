#include "nrlib/trend/trendstorage.hpp"
#include "nrlib/trend/trend.hpp"
#include "nrlib/grid/grid2d.hpp"
#include "nrlib/iotools/stringtools.hpp"

#include "rplib/distributionsrock.h"
#include "rplib/distributionssolid.h"
#include "rplib/distributionsdryrock.h"
#include "rplib/distributionsfluid.h"

#include "rplib/rock.h"

#include "rplib/distributionsrockstorage.h"
#include "rplib/distributionssolidstorage.h"
#include "rplib/distributionsdryrockstorage.h"
#include "rplib/distributionsfluidstorage.h"

#include "rplib/distributionsrocktabulated.h"
#include "rplib/distributionsrockbounding.h"
#include "rplib/distributionsrockmix.h"
#include "rplib/distributionsrockdem.h"
#include "rplib/distributionsrockgassmann.h"

#include "rplib/distributionwithtrend.h"
#include "rplib/distributionwithtrendstorage.h"
#include "rplib/distributionsstoragekit.h"

#include "src/modelsettings.h"
#include "src/blockedlogsforrockphysics.h"

#include <typeinfo>

DistributionsRockStorage::DistributionsRockStorage()
{
}

DistributionsRockStorage::~DistributionsRockStorage()
{
}

std::vector<DistributionsRock *>
DistributionsRockStorage::CreateDistributionsRockMix(const int                                                       & n_vintages,
                                                     const std::string                                               & path,
                                                     const std::vector<std::string>                                  & trend_cube_parameters,
                                                     const std::vector<std::vector<double> >                         & trend_cube_sampling,
                                                     const std::vector<BlockedLogsForRockPhysics *>                  & blockedLogs,
                                                     const std::vector<std::string>                                  & constituent_label,
                                                     const std::vector<std::vector<DistributionWithTrendStorage *> > & constituent_volume_fraction,
                                                     const std::map<std::string, DistributionsRockStorage *>         & model_rock_storage,
                                                     const std::map<std::string, DistributionsSolidStorage *>        & model_solid_storage,
                                                     const std::map<std::string, DistributionsDryRockStorage *>      & model_dry_rock_storage,
                                                     const std::map<std::string, DistributionsFluidStorage *>        & model_fluid_storage,
                                                     DEMTools::MixMethod                                               mix_method,
                                                     std::string                                                     & errTxt) const
{
  int n_constituents = static_cast<int>(constituent_label.size());

  std::vector<int> n_vintages_constit(n_constituents);
  for(int i=0; i<n_constituents; i++)
    n_vintages_constit[i] = static_cast<int>(constituent_volume_fraction[i].size());

  std::vector<double> alpha(n_constituents);
  for(int i=0; i<n_constituents; i++) {
    if(constituent_volume_fraction[i][0] != NULL)
      alpha[i] = constituent_volume_fraction[i][0]->GetOneYearCorrelation();
    else
      alpha[i] = 1;
  }

  std::vector<double> s_min;
  std::vector<double> s_max;

  FindSMinMax(trend_cube_sampling, s_min, s_max);

  const std::vector<std::vector<float> > dummy_blocked_logs;

  for(int i=0; i<n_constituents; i++)
    CheckValuesInZeroOne(constituent_volume_fraction[i], "volume-fraction", path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, errTxt);

  std::vector<DistributionsRock *>                   final_dist_rock(n_vintages, NULL);
  std::vector<std::vector<DistributionWithTrend *> > all_volume_fractions(n_vintages);

  for(int i=0; i<n_vintages; i++)
    all_volume_fractions[i].resize(n_constituents, NULL);

  for(int i=0; i<n_vintages; i++) {

    for (int s=0; s<n_constituents; s++) {

      if(i < n_vintages_constit[s]) {
        if(constituent_volume_fraction[s][i] != NULL)
          all_volume_fractions[i][s] = constituent_volume_fraction[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, errTxt);
      }
      else {
        if(all_volume_fractions[i-1][s] != NULL)
          all_volume_fractions[i][s] = all_volume_fractions[i-1][s]->Clone();
      }
    }

    CheckVolumeConsistency(all_volume_fractions[i], errTxt);
  }

  bool mix_rock  = false;
  bool mix_fluid = false;
  bool mix_solid = false;

  std::vector<int> constituent_type(n_constituents);

  FindMixTypesForRock(constituent_label,
                      n_constituents,
                      model_rock_storage,
                      model_solid_storage,
                      model_dry_rock_storage,
                      model_fluid_storage,
                      mix_rock,
                      mix_solid,
                      mix_fluid,
                      constituent_type,
                      errTxt);


  if(mix_rock == true) {

    std::vector<std::vector<DistributionsRock *> > distr_rock(n_vintages);
    for(int i=0; i<n_vintages; i++)
      distr_rock[i].resize(n_constituents, NULL);

    for(int s=0; s<n_constituents; s++) {
      std::vector<DistributionsRock *> distr_rock_all_vintages = ReadRock(n_vintages,
                                                                          constituent_label[s],
                                                                          path,
                                                                          trend_cube_parameters,
                                                                          trend_cube_sampling,
                                                                          blockedLogs,
                                                                          model_rock_storage,
                                                                          model_solid_storage,
                                                                          model_dry_rock_storage,
                                                                          model_fluid_storage,
                                                                          errTxt);

      int n_vintages_rock = static_cast<int>(distr_rock_all_vintages.size());

      for(int i=0; i<n_vintages; i++) {
        if(i < n_vintages_rock)
          distr_rock[i][s] = distr_rock_all_vintages[i];
        else
          distr_rock[i][s] = distr_rock[i-1][s]->Clone();
      }
    }

    if(errTxt == "") {
      for(int i=0; i<n_vintages; i++)
        final_dist_rock[i] = new DistributionsRockMixOfRock(distr_rock[i],
                                                            all_volume_fractions[i],
                                                            mix_method,
                                                            alpha,
                                                            s_min,
                                                            s_max);


      for(int i=0; i<n_vintages; i++) {
        for(size_t s=0; s<distr_rock.size(); s++)
          delete distr_rock[i][s];

        for(size_t s=0; s<all_volume_fractions.size(); s++) {
          if(all_volume_fractions[i][s] != NULL) {
            if(all_volume_fractions[i][s]->GetIsShared() == false)
              delete all_volume_fractions[i][s];
          }
        }
      }
    }
  }

  else if(mix_fluid == true && mix_solid == true) {
    std::vector<std::vector<DistributionsFluid *> >    distr_fluid(n_vintages);
    std::vector<std::vector<DistributionsSolid *> >    distr_solid(n_vintages);
    std::vector<std::vector<DistributionWithTrend *> > fluid_volume_fractions(n_vintages);
    std::vector<std::vector<DistributionWithTrend *> > solid_volume_fractions(n_vintages);
    std::vector<double>                                solid_alpha;
    std::vector<double>                                fluid_alpha;

    int n_fluids = 0;
    int n_solids = 0;

    for(int s=0; s<n_constituents; s++) {

      if(constituent_type[s] == ModelSettings::FLUID) {

        std::vector<DistributionsFluid *> constit_fluid_all_vintages = ReadFluid(n_vintages,
                                                                                 constituent_label[s],
                                                                                 path,
                                                                                 trend_cube_parameters,
                                                                                 trend_cube_sampling,
                                                                                 model_fluid_storage,
                                                                                 errTxt);

        int n_vintages_fluid = static_cast<int>(constit_fluid_all_vintages.size());

        for(int i=0; i<n_vintages; i++)
          distr_fluid[i].resize(n_fluids+1);

        for(int i=0; i<n_vintages; i++) {
          if(i < n_vintages_fluid)
            distr_fluid[i][n_fluids] = constit_fluid_all_vintages[i];
          else
            distr_fluid[i][n_fluids] = distr_fluid[i-1][n_fluids]->Clone();
        }

        for(int i=0; i<n_vintages; i++)
          fluid_volume_fractions[i].push_back(all_volume_fractions[i][s]);

        fluid_alpha.push_back(alpha[s]);

        n_fluids++;

      }


      else {

        std::vector<DistributionsSolid *> constit_solid_all_vintages = ReadSolid(n_vintages,
                                                                                 constituent_label[s],
                                                                                 path,
                                                                                 trend_cube_parameters,
                                                                                 trend_cube_sampling,
                                                                                 model_solid_storage,
                                                                                 errTxt);

        int n_vintages_solid = static_cast<int>(constit_solid_all_vintages.size());

        for(int i=0; i<n_vintages_solid; i++)
          distr_solid[i].resize(n_solids+1);

        for(int i=0; i<n_vintages; i++) {
          if(i < n_vintages_solid)
            distr_solid[i][n_solids] = constit_solid_all_vintages[i];
          else
            distr_solid[i][n_solids] = distr_solid[i-1][n_solids]->Clone();
        }

        for(int i=0; i<n_vintages; i++)
          solid_volume_fractions[i].push_back(all_volume_fractions[i][s]);

        solid_alpha.push_back(alpha[s]);

        n_solids++;

      }

    }

    // Reuse alpha, but now it is sorted
    for(int i=0; i<n_solids; i++)
      alpha[i] = solid_alpha[i];
    for(int i=0; i<n_fluids; i++)
      alpha[i+n_solids] = fluid_alpha[i];

    if(errTxt == "") {
      for(int i=0; i<n_vintages; i++)
        final_dist_rock[i] = new DistributionsRockMixOfSolidAndFluid(distr_solid[i],
                                                                     distr_fluid[i],
                                                                     solid_volume_fractions[i],
                                                                     fluid_volume_fractions[i],
                                                                     mix_method,
                                                                     alpha,
                                                                     s_min,
                                                                     s_max);

      for(int i=0; i<n_vintages; i++) {
        for(size_t s=0; s<distr_solid[i].size(); s++)
          delete distr_solid[i][s];

        for(size_t s=0; s<distr_fluid[i].size(); s++)
          delete distr_fluid[i][s];

        for(size_t s=0; s<solid_volume_fractions[i].size(); s++) {
          if(solid_volume_fractions[i][s] != NULL) {
            if(solid_volume_fractions[i][s]->GetIsShared() == false)
              delete solid_volume_fractions[i][s];
          }
        }

        for(size_t s=0; s<fluid_volume_fractions[i].size(); s++) {
          if(fluid_volume_fractions[i][s] != NULL) {
            if(fluid_volume_fractions[i][s]->GetIsShared() == false)
              delete fluid_volume_fractions[i][s];
          }
        }
      }
    }
  }

  return(final_dist_rock);
}

//----------------------------------------------------------------------------------//
TabulatedVelocityRockStorage::TabulatedVelocityRockStorage(std::vector<DistributionWithTrendStorage *> vp,
                                                           std::vector<DistributionWithTrendStorage *> vs,
                                                           std::vector<DistributionWithTrendStorage *> density,
                                                           std::vector<DistributionWithTrendStorage *> correlation_vp_vs,
                                                           std::vector<DistributionWithTrendStorage *> correlation_vp_density,
                                                           std::vector<DistributionWithTrendStorage *> correlation_vs_density,
                                                           std::string                                 rock_name)
: vp_(vp),
  vs_(vs),
  density_(density),
  correlation_vp_vs_(correlation_vp_vs),
  correlation_vp_density_(correlation_vp_density),
  correlation_vs_density_(correlation_vs_density),
  rock_name_(rock_name)
{
}

TabulatedVelocityRockStorage::~TabulatedVelocityRockStorage()
{
  if(vp_[0]->GetIsShared() == false)
    delete vp_[0];

  if(vs_[0]->GetIsShared() == false)
    delete vs_[0];

  if(density_[0]->GetIsShared() == false)
    delete density_[0];

  if(correlation_vp_vs_[0]->GetIsShared() == false)
    delete correlation_vp_vs_[0];

  if(correlation_vp_density_[0]->GetIsShared() == false)
    delete correlation_vp_density_[0];

  if(correlation_vs_density_[0]->GetIsShared() == false)
    delete correlation_vs_density_[0];
}

std::vector<DistributionsRock *>
TabulatedVelocityRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                                        const std::string                                           & path,
                                                        const std::vector<std::string>                              & trend_cube_parameters,
                                                        const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                                        const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                                        const std::map<std::string, DistributionsRockStorage *>     & /*model_rock_storage*/,
                                                        const std::map<std::string, DistributionsSolidStorage *>    & /*model_solid_storage*/,
                                                        const std::map<std::string, DistributionsDryRockStorage *>  & /*model_dry_rock_storage*/,
                                                        const std::map<std::string, DistributionsFluidStorage *>    & /*model_fluid_storage*/,
                                                        std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Tabulated rock physics model\n");

  std::vector<double> alpha(3);
  alpha[0] = vp_[0]     ->GetOneYearCorrelation();
  alpha[1] = vs_[0]     ->GetOneYearCorrelation();
  alpha[2] = density_[0]->GetOneYearCorrelation();

  std::vector<double> s_min;
  std::vector<double> s_max;

  FindSMinMax(trend_cube_sampling, s_min, s_max);

  int n_vintages_vp         = static_cast<int>(vp_.size());
  int n_vintages_vs         = static_cast<int>(vs_.size());
  int n_vintages_density    = static_cast<int>(density_.size());
  int n_vintages_vp_vs      = static_cast<int>(correlation_vp_vs_.size());
  int n_vintages_vp_density = static_cast<int>(correlation_vs_density_.size());
  int n_vintages_vs_density = static_cast<int>(correlation_vs_density_.size());

  std::string tmpErrTxt = "";

  // Use blockedLogs given facies
  int nWells = static_cast<int>(blockedLogs.size());

  std::vector<std::vector<float> > vp_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    vp_given_facies[i] = blockedLogs[i]->getAlphaForFacies(rock_name_);
  }

  std::vector<std::vector<float> > vs_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    vs_given_facies[i] = blockedLogs[i]->getBetaForFacies(rock_name_);
  }

  std::vector<std::vector<float> > density_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    density_given_facies[i] = blockedLogs[i]->getRhoForFacies(rock_name_);
  }

  for(int i=0; i<n_vintages_vp; i++) {
    if(i == 0) {
      if(vp_[i]->GetEstimate() == true && vp_given_facies.size() == 0)
        tmpErrTxt += "Vp can not be estimated as no Vp log is given in the wells\n";
    }
    else {
      if(vp_[i]->GetEstimate() == true)
        tmpErrTxt += "Vp can not be estimated for time lapse data\n";
    }
  }
  for(int i=0; i<n_vintages_vs; i++) {
    if(i == 0) {
      if(vs_[i]->GetEstimate() == true && vs_given_facies.size() == 0)
        tmpErrTxt += "Vs can not be estimated as no Vs log is given in the wells\n";
    }
    else {
      if(vs_[i]->GetEstimate() == true)
        tmpErrTxt += "Vs can not be estimated for time lapse data\n";
    }
  }
  for(int i=0; i<n_vintages_density; i++) {
    if(i == 0) {
      if(density_[i]->GetEstimate() == true && density_given_facies.size() == 0)
        tmpErrTxt += "Density can not be estimated as no density log is given in the wells\n";
    }
    else {
      if(density_[i]->GetEstimate() == true)
        tmpErrTxt += "Density can not be estimated for time lapse data\n";
    }
  }

  std::vector<double> corr_vp_vs(n_vintages, 0);
  std::vector<double> corr_vp_density(n_vintages, 0);
  std::vector<double> corr_vs_density(n_vintages, 0);

  for(int i=0; i<n_vintages_vp_vs; i++) {
    if(correlation_vp_vs_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(vp_given_facies.size() == 0 || vs_given_facies.size() == 0)
          tmpErrTxt += "<correlation-vp-vs> can not be estimated as both Vp and Vs logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-vp-vs> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_vp_vs_[i], "correlation", corr_vp_vs[i], errTxt);

    if(corr_vp_vs[i] > 1 || corr_vp_vs[i] < -1)
        errTxt += "<correlation-vp-vs> should be in the interval [-1,1] in the tabulated model\n";
  }

  for(int i=0; i<n_vintages_vp_density; i++) {
    if(correlation_vp_density_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(vp_given_facies.size() == 0 || density_given_facies.size() == 0)
          tmpErrTxt += "<correlation-vp-density> can not be estimated as both Vp and density logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-vp-density> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_vp_density_[i], "correlation", corr_vp_density[i], errTxt);

    if(corr_vp_density[i] > 1 || corr_vp_density[i] < -1)
        errTxt += "<correlation-vp-density> should be in the interval [-1,1] in the tabulated model\n";
  }

  for(int i=0; i<n_vintages_vs_density; i++) {
    if(correlation_vs_density_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(vs_given_facies.size() == 0 || density_given_facies.size() == 0)
          tmpErrTxt += "<correlation-vs-density> can not be estimated as both Vs and density logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-vs-density> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_vs_density_[i], "correlation", corr_vs_density[i], errTxt);

    if(corr_vs_density[i] > 1 || corr_vs_density[i] < -1)
        errTxt += "<correlation-vs-density> should be in the interval [-1,1] in the tabulated model\n";
  }

  std::vector<DistributionsRock *>     dist_rock(n_vintages, NULL);
  std::vector<DistributionWithTrend *> vp_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> vs_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> density_dist_with_trend(n_vintages, NULL);

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {

      if(i < n_vintages_vp)
        vp_dist_with_trend[i] = vp_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, vp_given_facies, tmpErrTxt);
      else
        vp_dist_with_trend[i] = vp_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_vs)
        vs_dist_with_trend[i] = vs_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, vs_given_facies, tmpErrTxt);
      else
        vs_dist_with_trend[i] = vs_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_density)
        density_dist_with_trend[i] = density_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, density_given_facies, tmpErrTxt);
      else
        density_dist_with_trend[i] = density_dist_with_trend[i-1]->Clone();

      if(i >= n_vintages_vp_vs)
        corr_vp_vs[i] = corr_vp_vs[i-1];

      if(i >= n_vintages_vp_density)
        corr_vp_density[i] = corr_vp_density[i-1];

      if(i >= n_vintages_vs_density)
        corr_vs_density[i] = corr_vs_density[i-1];
    }

    for(int i=0; i<n_vintages; i++) {
      std::string corrErrTxt = "";
      CheckPositiveDefiniteCorrMatrix(corr_vp_vs[i], corr_vp_density[i], corr_vs_density[i], corrErrTxt);
      if(corrErrTxt != "") {
        if(n_vintages > 1)
          tmpErrTxt += "Vintage "+NRLib::ToString(i+1)+":";
        tmpErrTxt += corrErrTxt;
      }
    }
  }

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {
      DistributionsRock * rock = new DistributionsRockTabulated(vp_dist_with_trend[i],
                                                                vs_dist_with_trend[i],
                                                                density_dist_with_trend[i],
                                                                corr_vp_vs[i],
                                                                corr_vp_density[i],
                                                                corr_vs_density[i],
                                                                DEMTools::Velocity,
                                                                alpha,
                                                                s_min,
                                                                s_max);

      dist_rock[i] = rock;
    }

    for(int i=0; i<n_vintages; i++) {
      if(vp_dist_with_trend[i]->GetIsShared() == false)
        delete vp_dist_with_trend[i];
      if(vs_dist_with_trend[i]->GetIsShared() == false)
        delete vs_dist_with_trend[i];
      if(density_dist_with_trend[i]->GetIsShared() == false)
        delete density_dist_with_trend[i];
    }
  }

  else{
    errTxt += "Problems with the Tabulated rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(dist_rock);

}

//----------------------------------------------------------------------------------//
TabulatedModulusRockStorage::TabulatedModulusRockStorage(std::vector<DistributionWithTrendStorage *> bulk_modulus,
                                                         std::vector<DistributionWithTrendStorage *> shear_modulus,
                                                         std::vector<DistributionWithTrendStorage *> density,
                                                         std::vector<DistributionWithTrendStorage *> correlation_bulk_shear,
                                                         std::vector<DistributionWithTrendStorage *> correlation_bulk_density,
                                                         std::vector<DistributionWithTrendStorage *> correlation_shear_density,
                                                         std::string                                 rock_name)
: bulk_modulus_(bulk_modulus),
  shear_modulus_(shear_modulus),
  density_(density),
  correlation_bulk_shear_(correlation_bulk_shear),
  correlation_bulk_density_(correlation_bulk_density),
  correlation_shear_density_(correlation_shear_density),
  rock_name_(rock_name)
{
}

TabulatedModulusRockStorage::~TabulatedModulusRockStorage()
{
 if(bulk_modulus_[0]->GetIsShared() == false)
    delete bulk_modulus_[0];

  if(shear_modulus_[0]->GetIsShared() == false)
    delete shear_modulus_[0];

  if(density_[0]->GetIsShared() == false)
    delete density_[0];

  if(correlation_bulk_shear_[0]->GetIsShared() == false)
    delete correlation_bulk_shear_[0];

  if(correlation_bulk_density_[0]->GetIsShared() == false)
    delete correlation_bulk_density_[0];

  if(correlation_shear_density_[0]->GetIsShared() == false)
    delete correlation_shear_density_[0];
}

std::vector<DistributionsRock *>
TabulatedModulusRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                                       const std::string                                           & path,
                                                       const std::vector<std::string>                              & trend_cube_parameters,
                                                       const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                                       const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                                       const std::map<std::string, DistributionsRockStorage *>     & /*model_rock_storage*/,
                                                       const std::map<std::string, DistributionsSolidStorage *>    & /*model_solid_storage*/,
                                                       const std::map<std::string, DistributionsDryRockStorage *>  & /*model_dry_rock_storage*/,
                                                       const std::map<std::string, DistributionsFluidStorage *>    & /*model_fluid_storage*/,
                                                       std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Tabulated rock physics model\n");

  std::vector<double> alpha(3);
  alpha[0] = bulk_modulus_[0] ->GetOneYearCorrelation();
  alpha[1] = shear_modulus_[0]->GetOneYearCorrelation();
  alpha[2] = density_[0]      ->GetOneYearCorrelation();

  std::vector<double> s_min;
  std::vector<double> s_max;

  FindSMinMax(trend_cube_sampling, s_min, s_max);

  int n_vintages_bulk          = static_cast<int>(bulk_modulus_.size());
  int n_vintages_shear         = static_cast<int>(shear_modulus_.size());
  int n_vintages_density       = static_cast<int>(density_.size());
  int n_vintages_bulk_shear    = static_cast<int>(correlation_bulk_shear_.size());
  int n_vintages_bulk_density  = static_cast<int>(correlation_bulk_density_.size());
  int n_vintages_shear_density = static_cast<int>(correlation_shear_density_.size());

  std::string tmpErrTxt = "";

  // Use blockedLogs given facies
  int nWells = static_cast<int>(blockedLogs.size());

  std::vector<std::vector<float> > bulk_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    bulk_given_facies[i] = blockedLogs[i]->getBulkForFacies(rock_name_);
  }

  std::vector<std::vector<float> > shear_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    shear_given_facies[i] = blockedLogs[i]->getShearForFacies(rock_name_);
  }

  std::vector<std::vector<float> > density_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    density_given_facies[i] = blockedLogs[i]->getRhoForFacies(rock_name_);
  }

  for(int i=0; i<n_vintages_bulk; i++) {
    if(i == 0) {
      if(bulk_modulus_[i]->GetEstimate() == true && bulk_given_facies.size() == 0)
        tmpErrTxt += "Bulk modulus can not be estimated as all of Vp, Vs and density logs are not given in the wells\n";
    }
    else {
      if(bulk_modulus_[i]->GetEstimate() == true)
        tmpErrTxt += "Bulk modulus can not be estimated for time lapse data\n";
    }
  }
  for(int i=0; i<n_vintages_shear; i++) {
    if(i == 0) {
      if(shear_modulus_[i]->GetEstimate() == true && shear_given_facies.size() == 0)
        tmpErrTxt += "Shear modulus can not be estimated as all of Vp, Vs and density logs are not given in the wells\n";
    }
    else {
      if(shear_modulus_[i]->GetEstimate() == true)
        tmpErrTxt += "Shear modulus can not be estimated for time lapse data\n";
    }
  }
  for(int i=0; i<n_vintages_density; i++) {
    if(i == 0) {
      if(density_[i]->GetEstimate() == true && density_given_facies.size() == 0)
        tmpErrTxt += "Density can not be estimated as no density log is given in the wells\n";
    }
    else {
      if(density_[i]->GetEstimate() == true)
        tmpErrTxt += "Density can not be estimated for time lapse data\n";
    }
  }

  std::vector<double> corr_bulk_shear(n_vintages, 0);
  std::vector<double> corr_bulk_density(n_vintages, 0);
  std::vector<double> corr_shear_density(n_vintages, 0);

  for(int i=0; i<n_vintages_bulk_shear; i++) {
    if(correlation_bulk_shear_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(bulk_given_facies.size() == 0 && shear_given_facies.size() == 0)
          tmpErrTxt += "<correlation-bulk-shear> can not be estimated as all of Vp, Vs and density logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-bulk-shear> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_bulk_shear_[i], "correlation", corr_bulk_shear[i], errTxt);

    if(corr_bulk_shear[i] > 1 || corr_bulk_shear[i] < -1)
        errTxt += "<correlation-bulk-shear> should be in the interval [-1,1] in the tabulated model\n";
  }

  for(int i=0; i<n_vintages_bulk_density; i++) {
    if(correlation_bulk_density_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(bulk_given_facies.size() == 0 && density_given_facies.size() == 0)
          tmpErrTxt += "<correlation-bulk-density> can not be estimated as all of Vp, Vs and density logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-bulk-density> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_bulk_density_[i], "correlation", corr_bulk_density[i], errTxt);

    if(corr_bulk_density[i] > 1 || corr_bulk_density[i] < -1)
        errTxt += "<correlation-bulk-density> should be in the interval [-1,1] in the tabulated model\n";
  }

  for(int i=0; i<n_vintages_shear_density; i++) {
    if(correlation_shear_density_[i]->GetEstimate() == true) {
      if(i == 0) {
        if(shear_given_facies.size() == 0 && density_given_facies.size() == 0)
          tmpErrTxt += "<correlation-shear-density> can not be estimated as all of Vp, Vs and density logs are not given in the wells\n";
        else
          tmpErrTxt += "Estimation of correlations has not been implemented yet\n";
      }
      else
        tmpErrTxt += "<correlation-shear-density> can not be estimatedfor time lapse data\n";
    }
    else
      FindDoubleValueFromDistributionWithTrend(correlation_shear_density_[i], "correlation", corr_shear_density[i], errTxt);

    if(corr_shear_density[i] > 1 || corr_shear_density[i] < -1)
        errTxt += "<correlation-shear-density> should be in the interval [-1,1] in the tabulated model\n";
  }

  const std::vector<std::vector<float> > dummy_blocked_logs;

  std::vector<DistributionsRock *>     dist_rock(n_vintages, NULL);
  std::vector<DistributionWithTrend *> bulk_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> shear_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> density_dist_with_trend(n_vintages, NULL);

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {
      if(i < n_vintages_bulk)
        bulk_dist_with_trend[i] = bulk_modulus_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, tmpErrTxt);
      else
        bulk_dist_with_trend[i] = bulk_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_shear)
        shear_dist_with_trend[i] = shear_modulus_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, tmpErrTxt);
      else
        shear_dist_with_trend[i] = shear_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_density)
        density_dist_with_trend[i] = density_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, density_given_facies, tmpErrTxt);
      else
        density_dist_with_trend[i] = density_dist_with_trend[i-1]->Clone();

      double lower_mega = 1.0e+6;
      double upper_mega = 1.6e+8;
      double test_bulk  = bulk_dist_with_trend[i]->ReSample(0,0);
      double test_shear = shear_dist_with_trend[i]->ReSample(0,0);

      if(test_bulk < lower_mega || test_bulk > upper_mega)
        tmpErrTxt += "Bulk modulus need to be given in kPa\n";
      if(test_shear < lower_mega || test_shear > upper_mega)
        tmpErrTxt += "Shear modulus need to be given in kPa\n";

      if(i >= n_vintages_bulk_shear)
        corr_bulk_shear.push_back(corr_bulk_shear[i-1]);

      if(i >= n_vintages_bulk_density)
        corr_bulk_density.push_back(corr_bulk_density[i-1]);

      if(i >= n_vintages_shear_density)
        corr_shear_density.push_back(corr_shear_density[i-1]);
    }

    for(int i=0; i<n_vintages; i++) {
      std::string corrErrTxt = "";
      CheckPositiveDefiniteCorrMatrix(corr_bulk_shear[i], corr_bulk_density[i], corr_shear_density[i], corrErrTxt);
      if(corrErrTxt != "") {
        if(n_vintages > 1)
          tmpErrTxt += "Vintage "+NRLib::ToString(i+1)+":";
        tmpErrTxt += corrErrTxt;
      }
    }
  }

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {
      DistributionsRock * rock = new DistributionsRockTabulated(bulk_dist_with_trend[i],
                                                                shear_dist_with_trend[i],
                                                                density_dist_with_trend[i],
                                                                corr_bulk_shear[i],
                                                                corr_bulk_density[i],
                                                                corr_shear_density[i],
                                                                DEMTools::Modulus,
                                                                alpha,
                                                                s_min,
                                                                s_max);

      dist_rock[i] = rock;
    }

    for(int i=0; i<n_vintages; i++) {
      if(bulk_dist_with_trend[i]->GetIsShared() == false)
        delete bulk_dist_with_trend[i];
      if(shear_dist_with_trend[i]->GetIsShared() == false)
        delete shear_dist_with_trend[i];
      if(density_dist_with_trend[i]->GetIsShared() == false)
        delete density_dist_with_trend[i];
    }
  }


 else {
    errTxt += "\nProblems with the Tabulated rock physics model:\n";
    errTxt += tmpErrTxt;
  }
  return(dist_rock);
}
//----------------------------------------------------------------------------------//
ReussRockStorage::ReussRockStorage(std::vector<std::string>                                  constituent_label,
                                   std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction,
                                   std::string                                               rock_name)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction),
  rock_name_(rock_name)
{
}

ReussRockStorage::~ReussRockStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] != NULL) {
      if(constituent_volume_fraction_[0][i]->GetIsShared() == false)
        delete constituent_volume_fraction_[0][i];
    }
  }
}

std::vector<DistributionsRock *>
ReussRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                            const std::string                                           & path,
                                            const std::vector<std::string>                              & trend_cube_parameters,
                                            const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                            const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                            const std::map<std::string, DistributionsRockStorage *>     & model_rock_storage,
                                            const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                            const std::map<std::string, DistributionsDryRockStorage *>  & model_dry_rock_storage,
                                            const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                            std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Reuss rock physics model\n");

  std::vector<DistributionsRock *> rock;

  std::string tmpErrTxt = "";

  for(size_t i=0; i<constituent_volume_fraction_.size(); i++) {
    for(size_t j=0; j<constituent_volume_fraction_[i].size(); j++) {
      if(constituent_volume_fraction_[i][j] != NULL && constituent_volume_fraction_[i][j]->GetEstimate() == true) {
        tmpErrTxt += "The volume fractions can not be estimated from wells\n";
        break;
      }
    }
    if(tmpErrTxt != "")
      break;
  }

  if(tmpErrTxt == "")
    rock =   CreateDistributionsRockMix(n_vintages,
                                        path,
                                        trend_cube_parameters,
                                        trend_cube_sampling,
                                        blockedLogs,
                                        constituent_label_,
                                        constituent_volume_fraction_,
                                        model_rock_storage,
                                        model_solid_storage,
                                        model_dry_rock_storage,
                                        model_fluid_storage,
                                        DEMTools::Reuss,
                                        tmpErrTxt);

  else {
    errTxt += "\nProblems with the Reuss rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(rock);
}

//----------------------------------------------------------------------------------//
VoigtRockStorage::VoigtRockStorage(std::vector<std::string>                                  constituent_label,
                                   std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction,
                                   std::string                                               rock_name)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction),
  rock_name_(rock_name)
{
}

VoigtRockStorage::~VoigtRockStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] != NULL) {
      if(constituent_volume_fraction_[0][i]->GetIsShared() == false)
        delete constituent_volume_fraction_[0][i];
    }
  }
}

std::vector<DistributionsRock *>
VoigtRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                            const std::string                                           & path,
                                            const std::vector<std::string>                              & trend_cube_parameters,
                                            const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                            const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                            const std::map<std::string, DistributionsRockStorage *>     & model_rock_storage,
                                            const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                            const std::map<std::string, DistributionsDryRockStorage *>  & model_dry_rock_storage,
                                            const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                            std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Voigt rock physics model\n");

  std::vector<DistributionsRock *> rock;

  std::string tmpErrTxt = "";

  for(size_t i=0; i<constituent_volume_fraction_.size(); i++) {
    for(size_t j=0; j<constituent_volume_fraction_[i].size(); j++) {
      if(constituent_volume_fraction_[i][j] != NULL && constituent_volume_fraction_[i][j]->GetEstimate() == true) {
        tmpErrTxt += "The volume fractions can not be estimated from wells\n";
        break;
      }
    }
    if(tmpErrTxt != "")
      break;
  }

  if(tmpErrTxt == "")
    rock =   CreateDistributionsRockMix(n_vintages,
                                        path,
                                        trend_cube_parameters,
                                        trend_cube_sampling,
                                        blockedLogs,
                                        constituent_label_,
                                        constituent_volume_fraction_,
                                        model_rock_storage,
                                        model_solid_storage,
                                        model_dry_rock_storage,
                                        model_fluid_storage,
                                        DEMTools::Voigt,
                                        tmpErrTxt);

  else {
    errTxt += "\nProblems with the Voigt rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(rock);
}

//----------------------------------------------------------------------------------//
HillRockStorage::HillRockStorage(std::vector<std::string>                                  constituent_label,
                                 std::vector<std::vector<DistributionWithTrendStorage *> > constituent_volume_fraction,
                                 std::string                                               rock_name)
: constituent_label_(constituent_label),
  constituent_volume_fraction_(constituent_volume_fraction),
  rock_name_(rock_name)
{
}

HillRockStorage::~HillRockStorage()
{
  for(int i=0; i<static_cast<int>(constituent_volume_fraction_[0].size()); i++) {
    if(constituent_volume_fraction_[0][i] != NULL) {
      if(constituent_volume_fraction_[0][i]->GetIsShared() == false)
        delete constituent_volume_fraction_[0][i];
    }
  }
}

std::vector<DistributionsRock *>
HillRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                           const std::string                                           & path,
                                           const std::vector<std::string>                              & trend_cube_parameters,
                                           const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                           const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                           const std::map<std::string, DistributionsRockStorage *>     & model_rock_storage,
                                           const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                           const std::map<std::string, DistributionsDryRockStorage *>  & model_dry_rock_storage,
                                           const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                           std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Hill rock physics model\n");

  std::vector<DistributionsRock *> rock;

  std::string tmpErrTxt = "";

  for(size_t i=0; i<constituent_volume_fraction_.size(); i++) {
    for(size_t j=0; j<constituent_volume_fraction_[i].size(); j++) {
      if(constituent_volume_fraction_[i][j] != NULL && constituent_volume_fraction_[i][j]->GetEstimate() == true) {
        tmpErrTxt += "The volume fractions can not be estimated from wells\n";
        break;
      }
    }
    if(tmpErrTxt != "")
      break;
  }

  if(tmpErrTxt == "")
    rock =   CreateDistributionsRockMix(n_vintages,
                                        path,
                                        trend_cube_parameters,
                                        trend_cube_sampling,
                                        blockedLogs,
                                        constituent_label_,
                                        constituent_volume_fraction_,
                                        model_rock_storage,
                                        model_solid_storage,
                                        model_dry_rock_storage,
                                        model_fluid_storage,
                                        DEMTools::Hill,
                                        tmpErrTxt);

  else {
    errTxt += "\nProblems with the Hill rock physics model for <rock>:\n";
    errTxt += tmpErrTxt;
  }

  return(rock);
}

//----------------------------------------------------------------------------------//
DEMRockStorage::DEMRockStorage(std::string                                               host_label,
                               std::vector<DistributionWithTrendStorage *>               host_volume_fraction,
                               std::vector<std::string>                                  inclusion_label,
                               std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_volume_fraction,
                               std::vector<std::vector<DistributionWithTrendStorage *> > inclusion_aspect_ratio,
                               std::string                                               rock_name)
: host_label_(host_label),
  host_volume_fraction_(host_volume_fraction),
  inclusion_label_(inclusion_label),
  inclusion_volume_fraction_(inclusion_volume_fraction),
  inclusion_aspect_ratio_(inclusion_aspect_ratio),
  rock_name_(rock_name)
{
}

DEMRockStorage::~DEMRockStorage()
{
  if(host_volume_fraction_[0] != NULL) {
    if(host_volume_fraction_[0]->GetIsShared() == false)
      delete host_volume_fraction_[0];
  }

  for(int i=0; i<static_cast<int>(inclusion_volume_fraction_[0].size()); i++) {
    if(inclusion_volume_fraction_[0][i] != NULL) {
      if(inclusion_volume_fraction_[0][i]->GetIsShared() == false)
        delete inclusion_volume_fraction_[0][i];
    }
  }

  for(int i=0; i<static_cast<int>(inclusion_aspect_ratio_[0].size()); i++) {
    if(inclusion_aspect_ratio_[0][i]->GetIsShared() == false)
      delete inclusion_aspect_ratio_[0][i];
  }
}

std::vector<DistributionsRock *>
DEMRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                          const std::string                                           & path,
                                          const std::vector<std::string>                              & trend_cube_parameters,
                                          const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                          const std::vector<BlockedLogsForRockPhysics *>              & /*blockedLogs*/,
                                          const std::map<std::string, DistributionsRockStorage *>     & /*model_rock_storage*/,
                                          const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                          const std::map<std::string, DistributionsDryRockStorage *>  & /*model_dry_rock_storage*/,
                                          const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                          std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating DEM rock physics model\n");

  std::string tmpErrTxt = "";

    // Remember: Host info is included first in constituent vectors
  int n_inclusions = static_cast<int>(inclusion_volume_fraction_.size());
  int n_constituents = n_inclusions + 1;

  const std::vector<std::vector<float> > dummy_blocked_logs;

  std::vector<std::vector<DistributionWithTrendStorage *> > volume_fractions(n_constituents);
  volume_fractions[0] = host_volume_fraction_;

  for(int i=0; i<n_inclusions; i++)
    volume_fractions[i+1] = inclusion_volume_fraction_[i];

  for(int i=0; i<n_constituents; i++)
    CheckValuesInZeroOne(volume_fractions[i], "volume-fraction", path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, tmpErrTxt);

  // Order in alpha: aspect_ratios, host_volume_fraction, inclusion_volume_fractions
  std::vector<double> alpha(n_inclusions + n_constituents);

  for(int i=0; i<n_inclusions; i++)
    alpha[i] = inclusion_aspect_ratio_[i][0]->GetOneYearCorrelation();

  for(int i=0; i<n_constituents; i++) {
    if(volume_fractions[i][0] != NULL)
      alpha[i+n_inclusions] = volume_fractions[i][0]->GetOneYearCorrelation();
    else
      alpha[i+n_inclusions] = 1;
  }

  std::vector<double> s_min;
  std::vector<double> s_max;

  FindSMinMax(trend_cube_sampling, s_min, s_max);

  std::vector<int> n_vintages_aspect(n_constituents);
  for(int i=0; i<n_inclusions; i++)
    n_vintages_aspect[i] = static_cast<int>(inclusion_aspect_ratio_[i].size());

  std::vector<int> n_vintages_volume(n_constituents);
  for(int i=0; i<n_constituents; i++)
    n_vintages_volume[i] = static_cast<int>(volume_fractions[i].size());

 //Read host label
  std::vector<DistributionsSolid *> final_distr_solid(n_vintages);

  std::vector<DistributionsSolid *> distr_solid = ReadSolid(n_vintages,
                                                            host_label_,
                                                            path,
                                                            trend_cube_parameters,
                                                            trend_cube_sampling,
                                                            model_solid_storage,
                                                            tmpErrTxt);

  int n_vintages_solid = static_cast<int>(distr_solid.size());

  for(int i=0; i<n_vintages; i++) {
    if(i < n_vintages_solid)
      final_distr_solid[i] = distr_solid[i];
    else
      final_distr_solid[i] = final_distr_solid[i-1]->Clone();
  }

  //Read inclusion label
  std::vector<std::vector<DistributionsFluid *> > final_distr_fluid_inc(n_vintages);
  for(int i=0; i<n_vintages; i++)
    final_distr_fluid_inc[i].resize(n_inclusions, NULL);

  for (int s = 0; s < n_inclusions; s++) {
    std::vector<DistributionsFluid *> distr_fluid_all_vintages = ReadFluid(n_vintages,
                                                                           inclusion_label_[s],
                                                                           path,
                                                                           trend_cube_parameters,
                                                                           trend_cube_sampling,
                                                                           model_fluid_storage,
                                                                           tmpErrTxt);

    int n_vintages_inc = static_cast<int>(distr_fluid_all_vintages.size());

    for(int i=0; i<n_vintages; i++) {
      if(i < n_vintages_inc)
        final_distr_fluid_inc[i][s] = distr_fluid_all_vintages[i];
      else
        final_distr_fluid_inc[i][s] = final_distr_fluid_inc[i-1][s]->Clone();
    }
  }

  std::vector<DistributionsRock *>                   final_dist_rock(n_vintages, NULL);
  std::vector<std::vector<DistributionWithTrend *> > all_volume_fractions(n_vintages);
  std::vector<std::vector<DistributionWithTrend *> > all_aspect_ratios(n_vintages);

  for(size_t i=0; i<volume_fractions.size(); i++) {
    for(size_t j=0; j<volume_fractions[i].size(); j++) {
      if(volume_fractions[i][j] != NULL && volume_fractions[i][j]->GetEstimate() == true)
        tmpErrTxt += "Volume fractions can not be estimated from wells\n";
    }
  }
  for(size_t i=0; i<inclusion_aspect_ratio_.size(); i++) {
    for(size_t j=0; j<inclusion_aspect_ratio_[i].size(); j++) {
      if(inclusion_aspect_ratio_[i][j]->GetEstimate() == true)
        tmpErrTxt += "Aspect ratios can not be estimated from wells\n";
    }
  }

  for(int i=0; i<n_vintages; i++) {
    all_volume_fractions[i].resize(n_constituents, NULL);
    all_aspect_ratios[i].resize(n_inclusions, NULL);
  }

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {

      for (int s = 0; s < n_inclusions; s++) {

        if(i < n_vintages_aspect[s]) {
          if(inclusion_aspect_ratio_[s][i] != NULL)
            all_aspect_ratios[i][s] = inclusion_aspect_ratio_[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, tmpErrTxt);
         }
        else
          all_aspect_ratios[i][s] = all_aspect_ratios[i-1][s]->Clone();
      }

      for (int s = 0; s < n_constituents; s++) {

        if(i < n_vintages_volume[s]) {
          if(volume_fractions[s][i] != NULL)
            all_volume_fractions[i][s] = volume_fractions[s][i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, dummy_blocked_logs, tmpErrTxt);
        }
        else
          if(all_volume_fractions[i-1][s] != NULL)
            all_volume_fractions[i][s] = all_volume_fractions[i-1][s]->Clone();
      }

      CheckVolumeConsistency(all_volume_fractions[i], tmpErrTxt);

    }
  }

  if (tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++)
      final_dist_rock[i] = new DistributionsRockDEM(final_distr_solid[i],
                                                    final_distr_fluid_inc[i],
                                                    all_aspect_ratios[i],
                                                    all_volume_fractions[i],
                                                    alpha,
                                                    s_min,
                                                    s_max);



    for(int i=0; i<n_vintages; i++) {
      delete final_distr_solid[i];

      for(size_t s=0; s<final_distr_fluid_inc[i].size(); s++)
        delete final_distr_fluid_inc[i][s];

      for(size_t s=0; s<all_aspect_ratios[i].size(); s++) {
        if(all_aspect_ratios[i][s]->GetIsShared() == false)
          delete all_aspect_ratios[i][s];
      }

      for(size_t s=0; s<all_volume_fractions[i].size(); s++) {
        if (all_volume_fractions[i][s] != NULL) {
          if(all_volume_fractions[i][s]->GetIsShared() == false)
            delete all_volume_fractions[i][s];
        }
      }
    }
  }

  else {
    errTxt += "\nProblems with the DEM rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(final_dist_rock);
}

//----------------------------------------------------------------------------------//
GassmannRockStorage::GassmannRockStorage(std::string dry_rock,
                                         std::string fluid,
                                         std::string rock_name)
: dry_rock_(dry_rock),
  fluid_(fluid),
  rock_name_(rock_name)
{
}

GassmannRockStorage::~GassmannRockStorage()
{
}

std::vector<DistributionsRock *>
GassmannRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                               const std::string                                           & path,
                                               const std::vector<std::string>                              & trend_cube_parameters,
                                               const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                               const std::vector<BlockedLogsForRockPhysics *>              & /*blockedLogs*/,
                                               const std::map<std::string, DistributionsRockStorage *>     & /*model_rock_storage*/,
                                               const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                               const std::map<std::string, DistributionsDryRockStorage *>  & model_dry_rock_storage,
                                               const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                               std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Gassmann rock physics model\n");

  std::vector<double> s_min;
  std::vector<double> s_max;

  std::string tmpErrTxt = "";

  FindSMinMax(trend_cube_sampling, s_min, s_max);

  std::vector<DistributionsRock *>     dist_rock(n_vintages, NULL);

  std::vector<DistributionsDryRock *> final_distr_dryrock (n_vintages);
  std::vector<DistributionsDryRock *> distr_dryrock = ReadDryRock(n_vintages,
                                                                  dry_rock_,
                                                                  path,
                                                                  trend_cube_parameters,
                                                                  trend_cube_sampling,
                                                                  model_dry_rock_storage,
                                                                  model_solid_storage,
                                                                  tmpErrTxt);

  for(int i=0; i<n_vintages; i++) {
    if(i < static_cast<int>(distr_dryrock.size()))
      final_distr_dryrock[i] = distr_dryrock[i];
    else
      final_distr_dryrock[i] = final_distr_dryrock[i-1]->Clone();
  }


  std::vector<DistributionsFluid *> final_distr_fluid (n_vintages);
  std::vector<DistributionsFluid *> distr_fluid = ReadFluid(n_vintages,
                                                            fluid_,
                                                            path,
                                                            trend_cube_parameters,
                                                            trend_cube_sampling,
                                                            model_fluid_storage,
                                                            tmpErrTxt);

  for(int i=0; i<n_vintages; i++) {
    if(i < static_cast<int>(distr_fluid.size()))
      final_distr_fluid[i] = distr_fluid[i];
    else
      final_distr_fluid[i] = final_distr_fluid[i-1]->Clone();
  }


  if (tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {
      DistributionsRock * rock = new DistributionsRockGassmann(final_distr_dryrock[i],
                                                               final_distr_fluid[i],
                                                               s_min,
                                                               s_max);

      dist_rock[i] = rock;
    }
  }
  else {
    errTxt += "\nProblems with the Gassmann rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(dist_rock);
}

//----------------------------------------------------------------------------------//
BoundingRockStorage::BoundingRockStorage(std::string                                 upper_rock,
                                         std::string                                 lower_rock,
                                         std::vector<DistributionWithTrendStorage *> porosity,
                                         std::vector<DistributionWithTrendStorage *> bulk_weight,
                                         std::vector<DistributionWithTrendStorage *> shear_weight,
                                         double                                      correlation_weights,
                                         std::string                                 rock_name)
: upper_rock_(upper_rock),
  lower_rock_(lower_rock),
  porosity_(porosity),
  bulk_weight_(bulk_weight),
  shear_weight_(shear_weight),
  correlation_weights_(correlation_weights),
  rock_name_(rock_name)
{
}

BoundingRockStorage::~BoundingRockStorage()
{
  if(porosity_[0]->GetIsShared() == false)
    delete porosity_[0];
  if(bulk_weight_[0]->GetIsShared() == false)
    delete bulk_weight_[0];
  if(shear_weight_[0]->GetIsShared() == false)
    delete shear_weight_[0];
}

std::vector<DistributionsRock *>
BoundingRockStorage::GenerateDistributionsRock(const int                                                   & n_vintages,
                                               const std::string                                           & path,
                                               const std::vector<std::string>                              & trend_cube_parameters,
                                               const std::vector<std::vector<double> >                     & trend_cube_sampling,
                                               const std::vector<BlockedLogsForRockPhysics *>              & blockedLogs,
                                               const std::map<std::string, DistributionsRockStorage *>     & model_rock_storage,
                                               const std::map<std::string, DistributionsSolidStorage *>    & model_solid_storage,
                                               const std::map<std::string, DistributionsDryRockStorage *>  & model_dry_rock_storage,
                                               const std::map<std::string, DistributionsFluidStorage *>    & model_fluid_storage,
                                               std::string                                                 & errTxt) const
{
  LogKit::LogFormatted(LogKit::Low," Generating Bounding rock physics model\n");

  std::string tmpErrTxt = "";

  int n_vintages_porosity     = static_cast<int>(porosity_.size());
  int n_vintages_bulk_weight  = static_cast<int>(bulk_weight_.size());
  int n_vintages_shear_weight = static_cast<int>(shear_weight_.size());

  std::vector<double> alpha(3);
  alpha[0] = porosity_[0]    ->GetOneYearCorrelation();
  alpha[1] = bulk_weight_[0] ->GetOneYearCorrelation();
  alpha[2] = shear_weight_[0]->GetOneYearCorrelation();

  std::vector<double> s_min;
  std::vector<double> s_max;

  std::vector<DistributionsRock *> final_distr_upper_rock(n_vintages);
  std::vector<DistributionsRock *> distr_upper_rock;

  LogKit::LogFormatted(LogKit::Low,"Upper bound:\n");
  distr_upper_rock = ReadRock(n_vintages,
                              upper_rock_,
                              path,
                              trend_cube_parameters,
                              trend_cube_sampling,
                              blockedLogs,
                              model_rock_storage,
                              model_solid_storage,
                              model_dry_rock_storage,
                              model_fluid_storage,
                              tmpErrTxt);

  for(int i=0; i<n_vintages; i++) {
    if(i < static_cast<int>(distr_upper_rock.size()))
      final_distr_upper_rock[i] = distr_upper_rock[i];
    else
      final_distr_upper_rock[i] = final_distr_upper_rock[i-1]->Clone();
  }

  if(distr_upper_rock[0] != NULL) {
    if(distr_upper_rock[0]->GetIsOkForBounding() == false) {
      tmpErrTxt += "The upper bound does not follow the requirements for the bounding model:\n";
      tmpErrTxt += "  The solid and fluid being mix need to be tabulated where the variables don't have distributions nor trends\n";
    }
  }

  std::map<std::string, DistributionsRockStorage *>::const_iterator m_all = model_rock_storage.find(upper_rock_);
  if (m_all != model_rock_storage.end()) {
    if(typeid(*(m_all->second)) != typeid(VoigtRockStorage))
      tmpErrTxt += "The upper bound in the Bounding rock physics model needs to follow a Voigt model\n";
  }

  std::vector<DistributionsRock *> final_distr_lower_rock(n_vintages);
  std::vector<DistributionsRock *> distr_lower_rock;

  LogKit::LogFormatted(LogKit::Low,"Lower bound:\n");
  distr_lower_rock = ReadRock(n_vintages,
                              lower_rock_,
                              path,
                              trend_cube_parameters,
                              trend_cube_sampling,
                              blockedLogs,
                              model_rock_storage,
                              model_solid_storage,
                              model_dry_rock_storage,
                              model_fluid_storage,
                              tmpErrTxt);

  for(int i=0; i<n_vintages; i++) {
    if(i < static_cast<int>(distr_lower_rock.size()))
      final_distr_lower_rock[i] = distr_lower_rock[i];
    else
      final_distr_lower_rock[i] = final_distr_lower_rock[i-1]->Clone();
  }

  if(distr_lower_rock[0] != NULL) {
    if(distr_lower_rock[0]->GetIsOkForBounding() == false) {
      tmpErrTxt += "The lower bound does not follow the requirements for the bounding model:\n";
      tmpErrTxt += "  The solid and fluid being mix need to be tabulated where the variables don't have distributions nor trends\n";
    }
  }

  std::map<std::string, DistributionsRockStorage *>::const_iterator n_all = model_rock_storage.find(lower_rock_);
  if (n_all != model_rock_storage.end()) {
    if(typeid(*(n_all->second)) != typeid(ReussRockStorage))
      tmpErrTxt += "The lower bound in the Bounding rock physics model needs to follow a Reuss model\n";
  }

  int nWells = static_cast<int>(blockedLogs.size());

  std::vector<std::vector<float> > bulk_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    bulk_given_facies[i] = blockedLogs[i]->getBulkForFacies(rock_name_);
  }

  std::vector<std::vector<float> > shear_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    shear_given_facies[i] = blockedLogs[i]->getShearForFacies(rock_name_);
  }

  std::vector<std::vector<float> > density_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    density_given_facies[i] = blockedLogs[i]->getRhoForFacies(rock_name_);
  }

  std::vector<std::vector<float> > porosity_given_facies(nWells);
  for(int i=0; i<nWells; i++) {
    porosity_given_facies[i] = blockedLogs[i]->getPorosityForFacies(rock_name_);
  }

  bool logs_given = true;
  if(bulk_given_facies.size() == 0 || shear_given_facies.size() == 0 || density_given_facies.size() == 0 || porosity_given_facies.size() == 0)
    logs_given = false;

  for(int i=0; i<n_vintages_porosity; i++) {
    if(i == 0) {
      if(porosity_[i]->GetEstimate() == true && porosity_given_facies.size() == 0)
        tmpErrTxt += "Porosity can not be estimated as porosity not is given in the wells.\n";
    }
    else {
      if(porosity_[i]->GetEstimate() == true)
        tmpErrTxt += "Porosity can not be estimated for time lapse data.\n";
    }
  }

  for(int i=0; i<n_vintages_bulk_weight; i++) {
    if(i == 0) {
      if(bulk_weight_[i]->GetEstimate() == true && logs_given == false)
        tmpErrTxt += "<bulk-modulus-weight> can not be estimated as all of Vp, Vs and density not are given in the wells.\n";
    }
    else {
      if(bulk_weight_[i]->GetEstimate() == true)
        tmpErrTxt += "<bulk-modulus-weight> can not be estimated for time lapse data.\n";
    }
  }

  for(int i=0; i<n_vintages_shear_weight; i++) {
    if(i == 0) {
      if(shear_weight_[i]->GetEstimate() == true && logs_given == false)
        tmpErrTxt += "<shear-modulus-weight> can not be estimated as all of Vp, Vs and density not are given in the wells.\n";

    }
    else {
      if(shear_weight_[i]->GetEstimate() == true)
        tmpErrTxt += "<shear-modulus-weight> can not be estimatedfor time lapse data.\n";
    }
  }

  std::vector<std::vector<float> > bulk_weight_given_facies(nWells);
  std::vector<std::vector<float> > shear_weight_given_facies(nWells);

  if(bulk_weight_[0]->GetEstimate() == true || shear_weight_[0]->GetEstimate() == true) {

    // Calculate bulk- and shear-weights

    Rock * sample_lower_rock; // Sample from the Reuss rock model
    Rock * sample_upper_rock; // Sample from the Voigt rock model

    const std::vector<double> dummy_trend_params(2,0); //Use dummy as the Reuss and Voigt rocks in the Bounding model not can contain trends

    sample_lower_rock = final_distr_lower_rock[0]->GenerateSample(dummy_trend_params); // Only estimate variables in 3D model, not 4D
    sample_upper_rock = final_distr_upper_rock[0]->GenerateSample(dummy_trend_params);

    double vp_lower;
    double vs_lower;
    double rho_lower;
    double bulk_lower;
    double shear_lower;

    double vp_upper;
    double vs_upper;
    double rho_upper;
    double bulk_upper;
    double shear_upper;

    for(int i=0; i<nWells; i++) {

      int n_well_samples = static_cast<int>(porosity_given_facies[i].size());

      bulk_weight_given_facies[i].resize(n_well_samples, RMISSING);
      shear_weight_given_facies[i].resize(n_well_samples, RMISSING);

      for(int j=0; j<n_well_samples; j++) {
        if(porosity_given_facies[i][j] != RMISSING && bulk_given_facies[i][j] != RMISSING && shear_given_facies[i][j] != RMISSING) {

          sample_lower_rock->SetPorosity(porosity_given_facies[i][j]);
          sample_upper_rock->SetPorosity(porosity_given_facies[i][j]);

          sample_lower_rock->GetSeismicParams(vp_lower, vs_lower, rho_lower);
          sample_upper_rock->GetSeismicParams(vp_upper, vs_upper, rho_upper);

          DEMTools::CalcElasticParamsFromSeismicParams(vp_lower, vs_lower, rho_lower, bulk_lower, shear_lower);
          DEMTools::CalcElasticParamsFromSeismicParams(vp_upper, vs_upper, rho_upper, bulk_upper, shear_upper);

          bulk_weight_given_facies[i][j]  = static_cast<float>((bulk_given_facies[i][j]  - bulk_lower)  / (bulk_upper  - bulk_lower));
          shear_weight_given_facies[i][j] = static_cast<float>((shear_given_facies[i][j] - shear_lower) / (shear_upper - shear_lower));
        }
      }
    }
  }

  CheckValuesInZeroOne(porosity_,     "porosity",             path, trend_cube_parameters, trend_cube_sampling, porosity_given_facies,     tmpErrTxt);
  CheckValuesInZeroOne(bulk_weight_,  "bulk-modulus-weight",  path, trend_cube_parameters, trend_cube_sampling, bulk_weight_given_facies,  tmpErrTxt);
  CheckValuesInZeroOne(shear_weight_, "shear-modulus-weight", path, trend_cube_parameters, trend_cube_sampling, shear_weight_given_facies, tmpErrTxt);

  std::vector<DistributionsRock *>     dist_rock(n_vintages, NULL);
  std::vector<DistributionWithTrend *> porosity_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> bulk_weight_dist_with_trend(n_vintages, NULL);
  std::vector<DistributionWithTrend *> shear_weight_dist_with_trend(n_vintages, NULL);

  if(tmpErrTxt == "") {
    for(int i=0; i<n_vintages; i++) {
      if(i < n_vintages_porosity)
        porosity_dist_with_trend[i] = porosity_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, porosity_given_facies, tmpErrTxt);
      else
        porosity_dist_with_trend[i] = porosity_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_bulk_weight)
        bulk_weight_dist_with_trend[i] = bulk_weight_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, bulk_weight_given_facies, tmpErrTxt);
      else
        bulk_weight_dist_with_trend[i] = bulk_weight_dist_with_trend[i-1]->Clone();

      if(i < n_vintages_shear_weight)
        shear_weight_dist_with_trend[i] = shear_weight_[i]->GenerateDistributionWithTrend(path, trend_cube_parameters, trend_cube_sampling, shear_weight_given_facies, tmpErrTxt);
      else
        shear_weight_dist_with_trend[i] = shear_weight_dist_with_trend[i-1]->Clone();
    }
  }

  if(tmpErrTxt == "") {
    FindSMinMax(trend_cube_sampling, s_min, s_max);

    for(int i=0; i<n_vintages; i++) {
      DistributionsRock * rock = new DistributionsRockBounding(final_distr_upper_rock[i],
                                                               final_distr_lower_rock[i],
                                                               porosity_dist_with_trend[i],
                                                               bulk_weight_dist_with_trend[i],
                                                               shear_weight_dist_with_trend[i],
                                                               correlation_weights_,
                                                               alpha,
                                                               s_min,
                                                               s_max);

      dist_rock[i] = rock;
    }

    for(int i=0; i<n_vintages; i++) {
      delete final_distr_upper_rock[i];

      delete final_distr_lower_rock[i];

      if(porosity_dist_with_trend[i]->GetIsShared() == false)
        delete porosity_dist_with_trend[i];

      if(bulk_weight_dist_with_trend[i]->GetIsShared() == false)
        delete bulk_weight_dist_with_trend[i];

      if(shear_weight_dist_with_trend[i]->GetIsShared() == false)
        delete shear_weight_dist_with_trend[i];
    }

  }

  else {
    errTxt += "\nProblems with the Bounding rock physics model:\n";
    errTxt += tmpErrTxt;
  }

  return(dist_rock);

}

