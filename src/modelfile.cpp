#include <fstream>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "lib/global_def.h"
#include "lib/lib_misc.h"

#include "nrlib/iotools/logkit.hpp"
#include "nrlib/segy/segy.hpp"

#include "src/inputfiles.h"
#include "src/modelsettings.h"
#include "src/modelfile.h"
#include "src/model.h"
#include "src/vario.h"
#include "src/definitions.h"
#include "src/fftgrid.h"

ModelFile::ModelFile(char * fileName)
{
  modelSettings_         = new ModelSettings();
  inputFiles_            = new InputFiles();
  failed_                = false;

  char ** params;
  int nParam = 0;
  parse(fileName,params,nParam);

  int nCommands = 38;
  bool * genNeed = new bool[nCommands]; //If run in generate mode, this list holds the necessity.

  //The following variables controls the alternative command system.
  //This is used if command A or B or C is necessary.
  //It allows the commands to be both exclusive and non-exclusive.
  //One of the commands is given as needed, the rest as optional.
  //If you intend to use this, and feel uncertain, talk to Ragnar.

  int *  alternativeUsed = new int[nCommands]; 

  //alternativeUsed[i] = k means that command k was used instead of i.
  //alternativeUsed[i] = -k means k used, i can also be used.

  int ** alternative     = new int * [nCommands]; 

  //alternative[i] == NULL means no alternative to command i, otherwise
  //alternative[i][0] is the number of alternatives, with negative sign if non-exclusive.
  //alternative[i][1] to alternative[i][abs(alternative[i][0])+1] is the number for the
  //  alternative commands.

  char * command;
  char ** commandList = new char*[nCommands];
  for(int i=0 ; i<nCommands ; i++)
  {
    commandList[i] = new char[30];
    genNeed[i] = false;
    alternative[i] = NULL;
    alternativeUsed[i] = 0;
  }

  int neededCommands =  2; // The neededCommands first in the list are normally necessary.
  //
  // Mandatory commands
  // 
  strcpy(commandList[0],"WELLS");
  strcpy(commandList[1],"DEPTH");                                       // ==> TIME_OUTPUT_INTERVAL
  genNeed[1] = true;

  //
  // Optional commands
  //
  strcpy(commandList[neededCommands+ 0],"SEISMIC");                     // ==> PP_SEISMIC
  genNeed[neededCommands+0] = true;
  strcpy(commandList[neededCommands+ 1],"ANGULARCORRELATION");          // ==> ANGULAR_CORRELATION
  strcpy(commandList[neededCommands+ 2],"SEED");
  strcpy(commandList[neededCommands+ 3],"LATERALCORRELATION");          // ==> LATERAL_CORRELATION
  strcpy(commandList[neededCommands+ 4],"NSIMULATIONS");
  strcpy(commandList[neededCommands+ 5],"PREDICTION");             
  strcpy(commandList[neededCommands+ 6],"PADDING");                     // ==> FFTGRID_PADDING
  strcpy(commandList[neededCommands+ 7],"PREFIX");
  strcpy(commandList[neededCommands+ 8],"AREA");                        // ==> INVERSION_AREA
  genNeed[neededCommands+8] = true;
  strcpy(commandList[neededCommands+ 9],"WHITENOISE");                  // ==> WHITE_NOISE_COMPONENT
  strcpy(commandList[neededCommands+10],"OUTPUT");
  strcpy(commandList[neededCommands+11],"SEGYOFFSET");                  // ==> SEISMIC_START_TIME
  strcpy(commandList[neededCommands+12],"FORCEFILE");
  strcpy(commandList[neededCommands+13],"DEBUG");
  strcpy(commandList[neededCommands+14],"KRIGING");
  strcpy(commandList[neededCommands+15],"LOCALWAVELET");
  strcpy(commandList[neededCommands+16],"ENERGYTRESHOLD");
  strcpy(commandList[neededCommands+17],"PARAMETERCORRELATION");        // ==> PARAMETER_CORRELATION
  strcpy(commandList[neededCommands+18],"REFLECTIONMATRIX");            // ==> REFLECTION_MATRIX
  strcpy(commandList[neededCommands+19],"FREQUENCYBAND");
  strcpy(commandList[neededCommands+20],"BACKGROUND");
  genNeed[neededCommands+20] = true;
  strcpy(commandList[neededCommands+21],"MAX_DEVIATION_ANGLE");
  strcpy(commandList[neededCommands+22],"empty_slot");
  strcpy(commandList[neededCommands+23],"SEISMICRESOLUTION");           // ==> SEISMIC_RESOLUTION
  strcpy(commandList[neededCommands+24],"WAVELETLENGTH");
  strcpy(commandList[neededCommands+25],"DEPTH_CONVERSION");
  strcpy(commandList[neededCommands+26],"PS_SEISMIC");
  strcpy(commandList[neededCommands+27],"PUNDEF");
  strcpy(commandList[neededCommands+28],"ALLOWED_PARAMETER_VALUES");
  strcpy(commandList[neededCommands+29],"ALLOWED_RESIDUAL_VARIANCES");
  strcpy(commandList[neededCommands+30],"CORRELATION_DIRECTION");
  strcpy(commandList[neededCommands+31],"WAVELET_ESTIMATION_INTERVAL");
  strcpy(commandList[neededCommands+32],"FACIES_ESTIMATION_INTERVAL");
  strcpy(commandList[neededCommands+33],"LOG_LEVEL");
  strcpy(commandList[neededCommands+34],"TRACE_HEADER_FORMAT");
  strcpy(commandList[neededCommands+35],"BACKGROUND_VELOCITY");

  char errText[MAX_STRING];
  char ** errorList = new char*[nCommands];
  int error;
  int nErrors = 0;
  if(params[nParam-1][0] != ';')
  {
    params[nParam] = new char[2];
    strcpy(params[nParam],";");
    nParam++;
    strcpy(errText,"Model file does not end with ';'.\n");
    errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }
  //
  // NBNB-PAL: Declared long so that we can handle 63(?) rather than 31 keywords 
  //
#if defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS)
  long long commandsUsed = 0;
  long long comFlag      = 0;
#else
  long commandsUsed = 0;
  long comFlag      = 0;
#endif
  int  curPos = 0;
  bool wrongCommand = false;

  while(curPos < nParam)
  {
    command = uppercase(params[curPos]);
    curPos++;
    int i;
    for(i=0;i<nCommands;i++)
      if(strcmp(commandList[i],command) == 0)
        break;

    if(i < nCommands) 
    {
#if defined(__WIN32__) || defined(WIN32) || defined(_WINDOWS)
      comFlag = static_cast<long long>(pow(2.0f,i));
#else
      comFlag = static_cast<long>(pow(2.0f,i));
#endif
      if((comFlag & commandsUsed) > 0) //Bitwise and
      {
        curPos += getParNum(params, curPos, error, errText, " ", 0, -1) + 1;
        error = 1;
        sprintf(errText, "Command %s specified more than once.\n",commandList[i]);
      }
      else if(alternativeUsed[i] > 0) 
      {
        curPos += getParNum(params, curPos, error, errText, " ", 0, -1) + 1;
        error = 1;
        sprintf(errText, "Can not have both command %s and %s.\n", 
                commandList[i], commandList[alternativeUsed[i]]);
      }
      else
      {
        commandsUsed = (commandsUsed | comFlag);
        if(alternative[i] != NULL) {
          int altSign = (alternative[i][0] > 0) ? 1 : -1;
          for(int j=0;j<altSign*alternative[i][0];j++)
            alternativeUsed[alternative[i][j+1]] = altSign*i;
        }
        switch(i)
        {
        case 0:
          error = readCommandWells(params, curPos, errText);
          break;
        case 1:
          error = readCommandTimeSurfaces(params, curPos, errText);
          break;
        case 2:
          error = readCommandSeismic(params, curPos, errText);
          break;
        case 3:
          error = readCommandAngularCorr(params, curPos, errText);
          break;
        case 4:
          error = readCommandSeed(params, curPos, errText);
          break;
        case 5:
          error = readCommandLateralCorr(params, curPos, errText);
          break;
        case 6:
          error = readCommandSimulate(params, curPos, errText);
          break;
        case 7:
          curPos += getParNum(params, curPos, error, errText, "PREDICTION", 0)+1;
          modelSettings_->setWritePrediction(true);
          break;
        case 8:
          error = readCommandPadding(params, curPos, errText);
          break;
        case 9:
          error = readCommandPrefix(params, curPos, errText);
          break;
        case 10:
          error = readCommandArea(params, curPos, errText);
          break;
        case 11:
          error = readCommandWhiteNoise(params, curPos, errText);
          break;
        case 12:
          error = readCommandOutput(params, curPos, errText);
          break;
        case 13:
          error = readCommandSegYOffset(params, curPos, errText);
          break;
        case 14:
          error = readCommandForceFile(params, curPos, errText);
          break;
        case 15:
          error = readCommandDebug(params, curPos, errText);
          break;
        case 16:
          error = readCommandKriging(params, curPos, errText);
          break;
        case 17:
          error = readCommandLocalWavelet(params, curPos, errText);
          break;
        case 18:
          error = readCommandEnergyTreshold(params, curPos, errText);
          break;
        case 19:
          error = readCommandParameterCorr(params, curPos, errText);
          break;
        case 20:
          error = readCommandReflectionMatrix(params, curPos, errText);
          break;
        case 21:
          error = readCommandFrequencyBand(params, curPos, errText);
          break;
        case 22:
          error = readCommandBackground(params, curPos, errText);
          break;
        case 23:
          error = readCommandMaxDeviationAngle(params,curPos,errText);
          break;
        case 24:
          // empty slot
          break;
        case 25:
          error = readCommandSeismicResolution(params, curPos, errText);
          break;
        case 26:
          error = readCommandWaveletTaperingL(params, curPos, errText);
          break;
        case 27:
          error = readCommandDepthConversion(params, curPos, errText);
          break;
        case 28:
          error = readCommandSeismic(params, curPos, errText, ModelSettings::PSSEIS);
          break;
        case 29:
          error = readCommandPUndef(params,curPos,errText);
          break;
        case 30:
          error = readCommandAllowedParameterValues(params,curPos,errText);
          break;
        case 31:
          error = readCommandAllowedResidualVariances(params,curPos,errText);
          break;
        case 32:
          error = readCommandCorrelationDirection(params,curPos,errText);
          break;
        case 33:
          error = readCommandWaveletEstimationInterval(params,curPos,errText);
          break;
        case 34:
          error = readCommandFaciesEstimationInterval(params,curPos,errText);
          break;
        case 35:
          error = readCommandLogLevel(params,curPos,errText);
          break;
        case 36:
          error = readCommandTraceHeaderFormat(params,curPos,errText);
          break;
        case 37:
          error = readCommandBackgroundVelocity(params,curPos,errText);
          break;
        default:
          sprintf(errText, "Unknown command: %s\n",command);
          wrongCommand = true;
          curPos += getParNum(params, curPos, error, errText, "-", 0, -1)+1;
          error = 1;
          break;
        }
      }
    }
    else 
    {
      sprintf(errText, "Unknown command: %s.\n",command);
      wrongCommand = true;
      curPos += getParNum(params, curPos, error, errText, "-", 0, -1)+1;
      error = 1;
    }

    if(error > 0)
    {
      errorList[nErrors] = new char[strlen(errText)+1];
      strcpy(errorList[nErrors], errText);
      nErrors++;
    }
    if(strlen(params[curPos-1]) > 1)
    {
      sprintf(errText,"Found %s instead of ';'\n", params[curPos-1]);
      errorList[nErrors] = new char[strlen(errText)+1];
      strcpy(errorList[nErrors], errText);
      nErrors++;
    }
  }
 
  //
  // Check model file validity 
  //
  if(!modelSettings_->getGenerateSeismic()) 
  {
    for(int i=0;i<neededCommands;i++)
    {
      if((commandsUsed % 2) == 0 && alternativeUsed[i] == 0)
      {
        if(alternative[i] == NULL)
          sprintf(errText,"Necessary command %s not specified\n", commandList[i]);
        else {
          sprintf(errText,"Command %s", commandList[i]);
          int j;
          for(j=1;j<abs(alternative[i][0]);j++)
            sprintf(errText,"%s, %s", errText, commandList[alternative[i][j]]);
          sprintf(errText,"%s or %s must be specified.\n", errText, commandList[alternative[i][j]]);
        }
        errorList[nErrors] = new char[strlen(errText)+1];
        strcpy(errorList[nErrors], errText);
        nErrors++;
      }
      commandsUsed /= 2;
    }
  }
  else for(int i=0;i<nCommands;i++)
  {
    if(genNeed[i] == true && ((commandsUsed % 2) == 0))
    {
      sprintf(errText,"Necessary command %s not specified\n", commandList[i]);
      errorList[nErrors] = new char[strlen(errText)+1];
      strcpy(errorList[nErrors], errText);
      nErrors++;
    }
    commandsUsed /= 2;
  }
  delete [] genNeed;

  if(modelSettings_->getDoInversion() && modelSettings_->getNumberOfAngles()==0) 
  {
    strcpy(errText,"An inversion run has been requested, but no seismic data are given.\nPlease specify seismic data using keywords PP_SEISMIC or PS_SEISMIC.\n");
    errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }

  if(modelSettings_->getAreaParameters()==NULL && modelSettings_->getNumberOfAngles()==0) 
  {
    strcpy(errText,"Command AREA is required when no seismic data are given.\n");
    errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }

  if(modelSettings_->getGenerateSeismic() && modelSettings_->getGenerateBackground())
  {
    strcpy(errText,"Background model and seismic cannot both be generated.\n");
    errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }
  
  if(modelSettings_->getGenerateBackground() && modelSettings_->getMaxHzBackground() < modelSettings_->getLowCut())
  {
    sprintf(errText,"The frequency high cut for the background (%.1f) must be larger than the frequency low cut for the inversion (%.1f).\n",
            modelSettings_->getMaxHzBackground(),modelSettings_->getLowCut());
    errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }
  
  //if(nWaveletTransfArgs_ > 0)
  //{
  //  if(nWaveletTransfArgs_ != modelSettings_->getNumberOfAngles())
  //  {
  //    sprintf(errText,"The number of arguments in the wavelet transformations differ from the number of seismic grids (%d vs %d).\n",
  //      nWaveletTransfArgs_, modelSettings_->getNumberOfAngles());
  //    errorList[nErrors] = new char[strlen(errText)+1];
  //   strcpy(errorList[nErrors], errText);
  //   nErrors++;
  //  }
  //}

  bool doFaciesProb         = (modelSettings_->getGridOutputFlag() & ModelSettings::FACIESPROB        ) > 0;
  bool doFaciesProbRelative = (modelSettings_->getGridOutputFlag() & ModelSettings::FACIESPROBRELATIVE) > 0;
  if((doFaciesProb || doFaciesProbRelative) && !modelSettings_->getFaciesLogGiven())
  {
    sprintf(errText,"You cannot calculate facies probabilities without specifying a facies log for subcommand HEADER of command WELLS.\n");
      errorList[nErrors] = new char[strlen(errText)+1];
    strcpy(errorList[nErrors], errText);
    nErrors++;
  }

  if(nErrors > 0)
  {
    LogKit::LogFormatted(LogKit::LOW,"\nThe following errors were found when parsing %s:\n", fileName);
    for(int i=0;i<nErrors;i++)
    {
      LogKit::LogFormatted(LogKit::LOW,"%s", errorList[i]);
      delete [] errorList[i];
    }
    failed_ = true;    
  }
 
  if (wrongCommand)
  {
    LogKit::LogFormatted(LogKit::LOW,"\nValid commands are:\n\n");
    LogKit::LogFormatted(LogKit::LOW,"\nGeneral settings:\n");
    LogKit::LogFormatted(LogKit::LOW,"  AREA\n");
    LogKit::LogFormatted(LogKit::LOW,"  DEBUG\n");
    LogKit::LogFormatted(LogKit::LOW,"  FORCEFILE\n");
    LogKit::LogFormatted(LogKit::LOW,"  FREQUENCYBAND\n");
    LogKit::LogFormatted(LogKit::LOW,"  OUTPUT\n");
    LogKit::LogFormatted(LogKit::LOW,"  PADDING\n");
    LogKit::LogFormatted(LogKit::LOW,"  PREFIX\n");
    LogKit::LogFormatted(LogKit::LOW,"  PUNDEF\n");
    LogKit::LogFormatted(LogKit::LOW,"  REFLECTIONMATRIX\n");
    LogKit::LogFormatted(LogKit::LOW,"  SEED\n");
    LogKit::LogFormatted(LogKit::LOW,"  LOG_LEVEL\n");
    LogKit::LogFormatted(LogKit::LOW,"\nModelling mode:\n");
    LogKit::LogFormatted(LogKit::LOW,"  KRIGING\n");
    LogKit::LogFormatted(LogKit::LOW,"  PREDICTION\n");
    LogKit::LogFormatted(LogKit::LOW,"  NSIMULATIONS\n");
    LogKit::LogFormatted(LogKit::LOW,"\nWell data:\n");
    LogKit::LogFormatted(LogKit::LOW,"  WELLS\n");
    LogKit::LogFormatted(LogKit::LOW,"  ALLOWED_PARAMETER_VALUES\n");
    LogKit::LogFormatted(LogKit::LOW,"  MAX_DEVIATION_ANGLE\n");
    LogKit::LogFormatted(LogKit::LOW,"  SEISMICRESOLUTION\n");
    LogKit::LogFormatted(LogKit::LOW,"\nSurface data:\n");
    LogKit::LogFormatted(LogKit::LOW,"  DEPTH\n");
    LogKit::LogFormatted(LogKit::LOW,"  DEPTH_CONVERSION\n");
    LogKit::LogFormatted(LogKit::LOW,"  CORRELATION_DIRECTION      \n");
    LogKit::LogFormatted(LogKit::LOW,"  FACIES_ESTIMATION_INTERVAL \n");
    LogKit::LogFormatted(LogKit::LOW,"  WAVELET_ESTIMATION_INTERVAL\n");
    LogKit::LogFormatted(LogKit::LOW,"\nSeismic data:\n");
    LogKit::LogFormatted(LogKit::LOW,"  SEISMIC\n");
    LogKit::LogFormatted(LogKit::LOW,"  PS_SEISMIC\n");
    LogKit::LogFormatted(LogKit::LOW,"  SEGYOFFSET\n");
    LogKit::LogFormatted(LogKit::LOW,"  TRACE_HEADER_FORMAT\n");
    LogKit::LogFormatted(LogKit::LOW,"  ENERGYTRESHOLD\n");
    LogKit::LogFormatted(LogKit::LOW,"  LOCALWAVELET\n");
    LogKit::LogFormatted(LogKit::LOW,"  WAVELETLENGTH\n");
    LogKit::LogFormatted(LogKit::LOW,"\nPrior model:\n");
    LogKit::LogFormatted(LogKit::LOW,"  BACKGROUND\n");
    LogKit::LogFormatted(LogKit::LOW,"  BACKGROUND_VELOCITY\n");
    LogKit::LogFormatted(LogKit::LOW,"  LATERALCORRELATION\n");
    LogKit::LogFormatted(LogKit::LOW,"  PARAMETERCORRELATION\n");
    LogKit::LogFormatted(LogKit::LOW,"  ALLOWED_RESIDUAL_VARIANCES\n");
    LogKit::LogFormatted(LogKit::LOW,"\nError model:\n");
    LogKit::LogFormatted(LogKit::LOW,"  ANGULARCORRELATION\n");
    LogKit::LogFormatted(LogKit::LOW,"  WHITENOISE\n");
  }

  for(int i=0;i<nParam;i++)
    delete [] params[i];
  delete [] params;

  for(int i=0;i<nCommands;i++)
    delete [] commandList[i];
  delete [] commandList;

  for(int i=0;i<nCommands;i++)
    delete [] alternative[i];
  delete [] alternative;
  
  delete [] alternativeUsed;
  delete [] errorList;
}

ModelFile::~ModelFile()
{
  //Both modelSettings and inputFiles are taken out and deleted outside.
}

 void
 ModelFile::parse_old(char * fileName, char **& params, int & nParam)
 {
   FILE* file = fopen(fileName,"r");
   if(file == 0)
   {
     LogKit::LogFormatted(LogKit::LOW,"Error: Could not open file %s for reading.\n", fileName);
     exit(1);
   }

  char tmpStr[MAX_STRING];
  while(fscanf(file,"%s",tmpStr) != EOF)
  {
    if(tmpStr[0] != '!')
      nParam++;
    else
      readToEOL(file);
  }
  fclose(file);

  params = new char *[nParam+1];
  file = fopen(fileName,"r");
  for(int i=0;i<nParam;i++)
  {  
    fscanf(file,"%s",tmpStr);
    while(tmpStr[0] == '!')
    {
      readToEOL(file);
      fscanf(file,"%s",tmpStr);
    }
    params[i] = new char[strlen(tmpStr)+1];
    strcpy(params[i],tmpStr);
  }
  fclose(file);
 }

 void
 ModelFile::parse(char * fileName, char **& params, int & nParam)
 {
   std::ifstream file(fileName);

   if (!file) {
     LogKit::LogFormatted(LogKit::ERROR,"Error: Could not open file %s for reading.\n", fileName);
     exit(1);
   }

   std::string text;
   std::vector<std::string> entries;

   while (std::getline(file,text))
   { 
     unsigned int iStart = entries.size();
     trimString(text);              // Remove leading and trailing whitespaces
     if (text.empty())              // Skip empty lines
       continue;
     if (text.substr(0,1) == "!")   // Skip comments
       continue;
     findApostropheEnclosedParts(text);
     splitString(entries,text);
     reintroduceBlanks(entries,iStart);
   }

   //
   // Copy strings to old style char array params
   //
   nParam = entries.size();
   params = new char *[nParam+1];
   for(int i=0 ; i<nParam ; i++)
   {  
     params[i] = new char[entries[i].length() + 1];
     strcpy(params[i],entries[i].c_str());
   }
 }


void ModelFile::trimString(std::string & str)
{
  std::string whitespaces (" \t\f\v\n\r");
  size_t startpos = str.find_first_not_of(whitespaces);
  size_t endpos   = str.find_last_not_of(whitespaces);
  if ( std::string::npos == startpos || std::string::npos == endpos)
    str.clear();
  else
    str = str.substr(startpos,endpos-startpos+1);
}

void ModelFile::findApostropheEnclosedParts(std::string & str)
{
  // Treat strings enclosed by apostrophes (""). 
  // 1. Replace the delimiting apostrophes by blanks. Then comes a trick:
  // 2. Replace any blanks by the only character that cannot be present 
  //    in the string ==> an apostrophe (").
  //
  std::string str_copy(str);
  std::string apostrophe("\"");
  std::string blank(" ");
  size_t startpos = str.find_first_of(apostrophe);
  size_t endpos;

  while (startpos != std::string::npos) // Apostrophes present
  {
    endpos = str.find_first_of(apostrophe,startpos + 1);
    if (endpos != std::string::npos)
    {
      std::string tmp = str.substr(startpos,endpos-startpos);
      NRLib2::Substitute(tmp,blank,apostrophe);
      str.replace(startpos,endpos-startpos,tmp);
      str.replace(startpos, 1, blank);
      str.replace(endpos, 1, blank);
    }
    else
    {
      std::cout << "ERROR: The following line in the model file contains an odd number of apostrophes (\")\n";
      std::cout << str_copy << std::endl;
      exit(1);
    }
    startpos = str.find_first_of(apostrophe,endpos + 1);
  }
}

void ModelFile::splitString(std::vector<std::string> & entries,
                            const std::string        & str)
{
  std::string blank(" ");
  std::string semicolon(";");
  size_t startpos = str.find_first_not_of(blank);
  size_t endpos;
  while (startpos != std::string::npos) // Entries present
  {
    endpos = str.find_first_of(blank,startpos + 1);
    if (endpos == std::string::npos) // At the end of the string
      endpos = str.size();
    if (str.substr(endpos-1,1) == semicolon && (endpos > startpos + 1))
    {
      entries.push_back(str.substr(startpos,endpos-startpos-1));
      entries.push_back(semicolon);
    }
    else
      entries.push_back(str.substr(startpos,endpos-startpos));
    startpos = str.find_first_not_of(blank,endpos + 1);
  }
}         

void ModelFile::reintroduceBlanks(std::vector<std::string> & entries,
                                  unsigned int iStart)
{
  // Apostrophes introduced in method findApostropheEnclosedParts() are 
  // here replaced by blanks again.
  std::string apostrophe("\"");
  std::string blank(" ");
  for (unsigned int i = iStart ; i < entries.size() ; i++)
  {
    NRLib2::Substitute(entries[i],apostrophe,blank);
  }
}

int 
ModelFile::readCommandWells(char ** params, int & pos, char * errText)
{
  sprintf(errText,"%c",'\0');
  char *commandName = params[pos-1];
  char *command     = new char[MAX_STRING];

  int error    = 0;
  int tmperror = 0;

  char tmpErrText[MAX_STRING];
  int * iwhich    = NULL;
  int nIndicators = 0;

  //
  // Make order of keywords INDICATORS and HEADERS arbitrary
  //
  strcpy(command,params[pos]);
  uppercase(command);
  while (strcmp(command,"HEADERS")==0 || strcmp(command,"INDICATORS")==0) 
  {
    if(strcmp(command,"HEADERS")==0)
    {
      int nHeader = getParNum(params, pos+1, error, errText, "HEADERS in WELLS", 4,5);
      if(nHeader==5)// facies log present
        modelSettings_->setFaciesLogGiven(true);
      // Read headers from model file
      if(error==0)
      { 
        for(int i=0 ; i<nHeader ; i++)
          modelSettings_->setLogName(i,uppercase(params[pos+1+i]));
        if(nHeader<5)// no facies log, dummy name
          modelSettings_->setLogName(4,"FACIES");
      }
      pos+= nHeader+2;
    }

    if(strcmp(command,"INDICATORS")==0)
    {
      nIndicators = getParNum(params, pos+1, tmperror, tmpErrText, "INDICATORS in WELLS", 1, 3);
      if (tmperror == 0)
      {
        iwhich = new int[nIndicators];
        for(int i=0 ; i<nIndicators ; i++)
        {
          strcpy(command,uppercase(uppercase(params[pos+1+i])));
          if (strcmp(command,"BACKGROUNDTREND")==0) 
            iwhich[i] = 0;
          else if (strcmp(command,"WAVELET")==0)         
            iwhich[i] = 1;
          else if (strcmp(command,"FACIES")==0)          
            iwhich[i] = 2;
          else 
            iwhich[i] = IMISSING;
        } 
      }
      else
      {
        sprintf(errText,"%s%s",errText,tmpErrText);
        return(1);
      }
      pos += nIndicators+2;
    }
    strcpy(command,params[pos]);
    uppercase(command);
  }
  delete [] command;

  tmperror = 0;

  int nWells = 0;
  int nElements = getParNum(params, pos, tmperror, tmpErrText, commandName, 1, -1);
  if(tmperror > 0)
  {
    sprintf(errText,"%s%s",errText,tmpErrText);
    pos += 1;
    return(1);
  }
  if(nIndicators > 0 && (nElements % (nIndicators+1)) != 0)
  {
    sprintf(tmpErrText, "Each well must have exactly %d indicators.\n",nIndicators);
    sprintf(errText,"%s%s",errText,tmpErrText);
    pos += 1;
    return(1);
  }
  else
    nWells = nElements/(nIndicators+1);

  int ** ind = NULL;
  if (nIndicators > 0) 
  {
    ind = new int * [nIndicators];
    for (int i=0 ; i<nIndicators ; i++)
      ind[i] = new int[nWells];
  }
  
  bool invalidIndicator = false;

  for(int i=0 ; i<nWells ; i++)
  {
    int iw = pos+(nIndicators+1)*i;

    inputFiles_->addWellFile(params[iw]);
    error += checkFileOpen(&(params[iw]), 1, commandName, errText, 0);

    for (int j=0 ; j<nIndicators ; j++)
    {
      int indicator = atoi(params[iw+j+1]); 
      if (indicator == 0 || indicator == 1)
        ind[j][i] = indicator;
      else
      {
        ind[j][i] = IMISSING;
        invalidIndicator = true;
      }
    }
  }
  if (invalidIndicator)
  { 
    sprintf(tmpErrText, "Invalid indicator found in command %s. Use 0 or 1 only.\n",commandName);
    sprintf(errText,"%s%s",errText,tmpErrText);
    error = 1;
  }
  modelSettings_->setNumberOfWells(nWells);
  modelSettings_->setAllIndicatorsTrue(nWells);

  for (int j=0 ; j<nIndicators ; j++)
  {
    if (iwhich[j]==0)         // BACKGROUNDTREND
      modelSettings_->setIndicatorBGTrend(ind[j],nWells);
    else if (iwhich[j]==1)    // WAVELET         
      modelSettings_->setIndicatorWavelet(ind[j],nWells);
    else if (iwhich[j]==2)    // FACIES
      modelSettings_->setIndicatorFacies(ind[j],nWells);
    else
    {
      sprintf(tmpErrText, "Invalid INDICATOR type found for command %s.\n",commandName);
      sprintf(tmpErrText, "%sChoose from BACKGROUNDTREND, WAVELET, and FACIES.\n",tmpErrText);
      sprintf(errText,"%s%s",errText,tmpErrText);
      error = 1;
    }
  }

  pos += nElements + 1;

  if (nIndicators > 0) 
  {
    for (int i=0 ; i<nIndicators ; i++)
      if (ind[i] != NULL)
        delete [] ind[i];
    if (ind != NULL)
      delete [] ind;
    if (iwhich != NULL)
      delete [] iwhich;
  }
  return(error);
}

int 
ModelFile::readCommandBackground(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2, -1);
  if(error == 1)
  {
    pos += nPar+1;
    return(1);
  }
  //
  // If the number of parameters are exactly 3 we assume the background model
  // to be given either on file or as constants.
  //
  if (nPar == 3) 
  {
    modelSettings_->setGenerateBackground(false);

    for(int i=0 ; i<3 ; i++)
    {
      if(isNumber(params[pos+i])) // constant background
      { 
        modelSettings_->setConstBackValue(i, static_cast<float>(atof(params[pos+i])));
      }
      else // not constant, read from file.
      {
        inputFiles_->setBackFile(i,params[pos+i]);
        
        if(checkFileOpen(&(params[pos+i]), 1, params[pos-1], errText) > 0) 
        {
          error = 1;
          modelSettings_->setConstBackValue(i, RMISSING);

        }
        else
          modelSettings_->setConstBackValue(i, -1); // Indicates that background is read from file
      }
    }
  }
  else
  {
    //
    // Currently we have to generate all or none parameters. The reason
    // is that the kriging algorithm handles all parameters simulataneously.
    //
    modelSettings_->setGenerateBackground(true);

    int nSubCommands = 2;
    char ** subCommand = new char * [nSubCommands];
    for(int i=0 ; i<nSubCommands ; i++)
      subCommand[i] = new char[30];
    strcpy(subCommand[0],"VARIOGRAM");
    strcpy(subCommand[1],"FREQUENCY");
    int subCom, comPar, curPar = 0;
    while(curPar < nPar && error == 0) {
      comPar = 1;
      while(isNumber(params[pos+curPar+comPar]))
        comPar++;
      subCom = 0;
      while(subCom < nSubCommands && strcmp(params[pos+curPar], subCommand[subCom]) != 0)
        subCom++;
      switch(subCom) {
      case 0:
        if(comPar > 1) {
          error = 1;
          sprintf(errText,"%sSubcommand VARIOGRAM in command %s needs variogram type.\n",
                  errText,params[pos-1]);
        }
        else {
          comPar = 2;
          while(isNumber(params[pos+curPar+comPar]))
            comPar++;
          if(strcmp(params[pos+curPar],"VARIOGRAM") == 0) {

            Vario * vario = createVario(&(params[pos+curPar+1]), comPar-1, params[pos-1], errText);
            if(vario == NULL)
            {
              error = 1;
            }
            else
            {
              modelSettings_->setBackgroundVario(vario);
              //
              // NBNB-PAL: Temporary setting until 2D-kriging of BG allows other variograms to be used 
              //
              if(strcmp(params[pos+curPar+1],"GENEXP") != 0)
              {
                sprintf(errText,"%sSubcommand VARIOGRAM in command %s only allows variogram type \'genexp\'.",
                        errText,params[pos-1]);
                error = 1;
              }
            }
          }
        }
        break;
      case 1:
        if(comPar != 2) {
          error = 1;
          sprintf(errText,"%sSubcommand FREQUENCY in command %s takes 1 parameter, not %d as was given.\n",
                  errText,params[pos-1], comPar-1);
        }
        else 
          modelSettings_->setMaxHzBackground(float(atof(params[pos+curPar+1])));
        break;
      default: 
        error = 1;
        sprintf(errText,"%sUnknown subcommand %s found in command %s.\n", 
                errText,params[pos+curPar],params[pos-1]);
        break;
      }
      curPar += comPar;
    }
    for(int i=0;i<nSubCommands;i++)
      delete [] subCommand[i];
    delete [] subCommand;
  }
  pos += nPar+1;
  return(error);
}

int 
ModelFile::readCommandArea(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 7);

  if(error==1)
  {
    pos += nPar+1;
    return(1);
  }
  double x0      = static_cast<double>(atof(params[pos]));            
  double y0      = static_cast<double>(atof(params[pos+1]));          
  double lx      = static_cast<double>(atof(params[pos+2]));          
  double ly      = static_cast<double>(atof(params[pos+3]));          
  double azimuth = static_cast<double>(atof(params[pos+4])); 
  double dx      = static_cast<double>(atof(params[pos+5]));          
  double dy      = static_cast<double>(atof(params[pos+6]));          

  // Convert from azimuth (in degrees) to internal angle (in radians).
  double rot = (-1)*azimuth*(M_PI/180.0);

  int nx = static_cast<int>(lx/dx);
  int ny = static_cast<int>(ly/dy);

  SegyGeometry * geometry = new SegyGeometry(x0, y0, dx, dy, nx, ny, 0, 0, 1, 1, true, rot);

  modelSettings_->setAreaParameters(geometry);
  delete geometry;

  pos += nPar+1;
  return(error);
}

int 
ModelFile::readCommandTimeSurfaces(char ** params, int & pos, char * errText)
{
  int error = 0;
  int nPar  = getParNum(params, pos, error, errText, params[pos-1], 3, 4);

  inputFiles_->addTimeSurfFile(params[pos]);

  if(nPar == 3)
  {
    inputFiles_->addTimeSurfFile(params[pos+1]);
    
    if (!isNumber(params[pos]))
      error += checkFileOpen((&(params[pos])), 1, params[pos-1], errText);
    if (!isNumber(params[pos+1]))
      error += checkFileOpen((&(params[pos+1])), 1, params[pos-1], errText);
    modelSettings_->setTimeNz(atoi(params[pos+2]));
  }
  else // nPar = 4
  {
    modelSettings_->setParallelTimeSurfaces(true);
    if(isNumber(params[pos+1])) //Only one reference surface
    {
      error = checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
      modelSettings_->setTimeDTop(static_cast<double>(atof(params[pos+1])));
      modelSettings_->setTimeLz  (static_cast<double>(atof(params[pos+2])));
      modelSettings_->setTimeDz  (static_cast<double>(atof(params[pos+3])));
    }
    else {
      sprintf(errText,"Command %s with 4 arguments must have a number as argument number 2.\n", 
        params[pos-1]);
      error = 1;
    }
  }
  pos += nPar+1;
  return(error);
}

int 
ModelFile::readCommandDepthConversion(char ** params, int & pos, char * errText)
{
  modelSettings_->setDepthDataOk(true);

  inputFiles_->setVelocityField("CONSTANT");

  int error = 0;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1, -1);
  if(error == 1)
  {
    pos += nPar+1;
    sprintf(errText,"Command %s takes 2 to 6 arguments; %d given in file.\n",
            params[pos-1], nPar);
    return(error);
  }

  int domain = modelSettings_->getOutputDomainFlag();
  domain = (domain | ModelSettings::DEPTHDOMAIN);
  modelSettings_->setOutputDomainFlag(domain);

  int nSubCommands = 3;
  char ** subCommand = new char * [nSubCommands];
  for(int i=0 ; i<nSubCommands ; i++)
    subCommand[i] = new char[30];
  strcpy(subCommand[0], "VELOCITY_FIELD");
  strcpy(subCommand[1], "TOP_SURFACE");
  strcpy(subCommand[2], "BASE_SURFACE");

  int  curPar = 0;
  while(curPar < nPar && error == 0) 
  {
    //
    // All subcommands take one elements
    //
    int comPar = 2;
    int subCom = 0;
    while(subCom < nSubCommands && strcmp(params[pos+curPar], subCommand[subCom]) != 0)
      subCom++;

    switch(subCom) 
    {
    case 0:
      inputFiles_->setVelocityField(params[pos+curPar+1]); // Can be file name or command
      if (NRLib2::Uppercase(inputFiles_->getVelocityField()) != "CONSTANT" 
          && NRLib2::Uppercase(inputFiles_->getVelocityField()) != "FROM_INVERSION")
      { 
        error += checkFileOpen(&(params[pos+curPar+1]), 1, params[pos-1], errText);
      }
      break;

    case 1:
      inputFiles_->setDepthSurfFile(0, params[pos+curPar+1]);
      error += checkFileOpen(&(params[pos+curPar+1]), 1, params[pos-1], errText);
      break;

    case 2:
      inputFiles_->setDepthSurfFile(1, params[pos+curPar+1]);
      error += checkFileOpen(&(params[pos+curPar+1]), 1, params[pos-1], errText);
      break;

    default: 
      error = 1;
      sprintf(errText,"Unknown subcommand %s found in command %s.\n", params[pos+curPar],params[pos-1]);
      sprintf(errText,"%s\nValid subcommands are:\n",errText);
      for (int i = 0 ; i < nSubCommands ; i++)
        sprintf(errText,"%s  %s\n",errText,subCommand[i]);
      break;
    }
    curPar += comPar;
  }

  for(int i=0;i<nSubCommands;i++)
    delete [] subCommand[i];
  delete [] subCommand;

  if (inputFiles_->getVelocityField()=="CONSTANT"
      && (inputFiles_->getDepthSurfFile(0)=="" 
          || inputFiles_->getDepthSurfFile(1)=="")) 
  {
    error++;
    sprintf(errText,"%sFor CONSTANT velocity fields both top and base depth surfaces must be given (Command %s).\n",
            errText,params[pos-1]);
  }
  if (inputFiles_->getVelocityField()=="FROM_INVERSION"
      && (inputFiles_->getDepthSurfFile(0)=="" 
          && inputFiles_->getDepthSurfFile(1)=="")) 
  {
    error++;
    sprintf(errText,"%sWhen the velocity field is taken from inversion either a top depth or a base depth surface must be given (Command %s).\n",
            errText,params[pos-1]);
  }

  pos += nPar+1;
  return(error);
}

int 
ModelFile::readCommandSeismic(char ** params, int & pos, char * errText, int seisType)
{
  int nCol = 5;
  int error = 0;
  int nSeisData = getParNum(params, pos, error, errText, params[pos-1], 2, -1);

  if(error == 0)
  {
    if((nSeisData % nCol) != 0)
    {
      sprintf(errText, "Number of parameters in command %s must be multiple of %d (%d were found).\n", 
        params[pos-1], nCol, nSeisData);
      error = 1;
    }
    else
      nSeisData = nSeisData/nCol;
  }
  if(error == 1)
  {
    pos += nSeisData+1;
    return(1);
  }

  for(int i=0;i<nSeisData;i++)
  {
    // 1. file containing seismic
    inputFiles_->addSeismicFile(params[pos+nCol*i]);
    if(params[pos+nCol*i][0] == '?')
      modelSettings_->setGenerateSeismic(true);
    else
      error += checkFileOpen(&(params[pos+nCol*i]), 1, "SEISMIC", errText);

    // 2. angle
    modelSettings_->addAngle(float(atof(params[pos+nCol*i+1])*M_PI/180.0));

    // 3. signal-to-noise ratio
    if (params[pos+nCol*i+2][0] == '*') 
    {
      modelSettings_->addEstimateSNRatio(true);
      modelSettings_->addSNRatio(RMISSING);
    }
    else 
    {
      modelSettings_->addEstimateSNRatio(false);
      modelSettings_->addSNRatio(float(atof(params[pos+nCol*i+2])));
    }

    // 4. file containing wavelet
    inputFiles_->addWaveletFile(params[pos+nCol*i+3]);
    if(params[pos+nCol*i+3][0] == '*')
      modelSettings_->addEstimateWavelet(true);
    else
    {
      modelSettings_->addEstimateWavelet(false);
        error += checkFileOpen(&(params[pos+nCol*i+3]), 1, "WAVELET", errText);
    }

    // 5. wavelet scale
    if (params[pos+nCol*i+4][0] == '*') 
    {
      modelSettings_->addMatchEnergies(true);
      modelSettings_->addWaveletScale(RMISSING);
    }
    else
    {
      modelSettings_->addMatchEnergies(false);
      modelSettings_->addWaveletScale(float(atof(params[pos+nCol*i+4])));
    }

    // 6. Seismic type
    modelSettings_->addSeismicType(seisType);

    //Temporary
    modelSettings_->addLocalSegyOffset(-1);
    modelSettings_->addTraceHeaderFormat(NULL);

    // Consistency check
    if(modelSettings_->getGenerateSeismic() || error == 0)
      if (modelSettings_->getEstimateWavelet(i) || modelSettings_->getMatchEnergies(i))
        sprintf(errText, "For seismic data generation both wavelet and wavelet scale must be given\n");
  }

  pos += nCol*nSeisData+1;
  return(error);
}

int 
ModelFile::readCommandAngularCorr(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2, 3);
  if(error == 0)
  {
    Vario * vario = createVario(&(params[pos]), nPar, params[pos-1], errText);
    if(vario != NULL)
    {
      vario->convertRangesFromDegToRad();
      modelSettings_->setAngularCorr(vario);
    }
    else
      error = 1;
  }
  pos += nPar+1;

  return(error);
}

int 
ModelFile::readCommandLateralCorr(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 4,5);
  if(error == 0)
  {
    Vario * vario = createVario(&(params[pos]), nPar, params[pos-1], errText); 
    if(vario != NULL)
      modelSettings_->setLateralCorr(vario);
    else
      error = 1;
  }
  pos += nPar+1;
  return(error);
}


int 
ModelFile::readCommandSimulate(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  int nSimulations = atoi(params[pos]);
  modelSettings_->setNumberOfSimulations(nSimulations);
  pos += nPar+1;
  return(error);
}


int 
ModelFile::readCommandPadding(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 3);
  if(error == 0)
  {
    sprintf(errText,"%c",'\0');
    double xPadFac = atof(params[pos]);
    if(xPadFac > 1 || xPadFac < 0 || isNumber(params[pos]) == 0)
    {
      error = 1;
      sprintf(errText,"Padding in x-direction must be a number between 0 and 1 (found %s)\n",params[pos]);
    }
    else
    {
      modelSettings_->setXPadFac(xPadFac);
    }
    double yPadFac = atof(params[pos+1]);
    if(yPadFac > 1 || yPadFac <= 0 || isNumber(params[pos+1]) == 0)
    {
      error = 1;
      sprintf(errText,"%sPadding in y-direction must be a number between 0 and 1 (found %s)\n",
        errText,params[pos+1]);
    }
    else
    {
      modelSettings_->setYPadFac(yPadFac);
    }
    double zPadFac = atof(params[pos+2]);
    if(zPadFac > 1 || zPadFac <= 0 || isNumber(params[pos+2]) == 0)
    {
      error = 1;
      sprintf(errText,"%sPadding in z-direction must be a number between 0 and 1 (found %s)\n",
        errText,params[pos+2]);
    }
    else
    {
      modelSettings_->setZPadFac(zPadFac);
    }
  } 
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandSeed(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    if(isNumber(params[pos]))
      modelSettings_->setSeed(atoi(params[pos]));
    else
    {
      error = checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
      if(error == 0)
      {
        inputFiles_->setSeedFile(params[pos]);
      }
    }
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandPrefix(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
    ModelSettings::setFilePrefix(std::string(params[pos]));
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandOutput(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1, -1);
  if(error == 0)
  {
    //
    // WARNING: If nKeys becomes larger than 31, we get problems with 
    //          the 'int' data type and have to switch to 'long'
    //
    int i, key, nKeys = 29;
    char ** keywords;
    keywords = new char*[nKeys];
    for(i=0;i<nKeys;i++)
      keywords[i] = new char[40];
    i = 0;
    strcpy(keywords[i++],"STORM"); //0
    strcpy(keywords[i++],"SEGY");
    strcpy(keywords[i++],"ASCII");
    strcpy(keywords[i++],"SGRI");
    strcpy(keywords[i++],"CORRELATION"); //4
    strcpy(keywords[i++],"RESIDUALS");
    strcpy(keywords[i++],"VP");
    strcpy(keywords[i++],"VS");
    strcpy(keywords[i++],"RHO");
    strcpy(keywords[i++],"LAMELAMBDA");
    strcpy(keywords[i++],"LAMEMU");
    strcpy(keywords[i++],"POISSONRATIO");
    strcpy(keywords[i++],"AI");
    strcpy(keywords[i++],"SI");
    strcpy(keywords[i++],"VPVSRATIO");
    strcpy(keywords[i++],"MURHO");
    strcpy(keywords[i++],"LAMBDARHO");
    strcpy(keywords[i++],"BACKGROUND");
    strcpy(keywords[i++],"BACKGROUND_TREND");
    strcpy(keywords[i++],"FACIESPROB");
    strcpy(keywords[i++],"FACIESPROBRELATIVE");
    strcpy(keywords[i++],"EXTRA_GRIDS");
    strcpy(keywords[i++],"WELLS");        //22
    strcpy(keywords[i++],"BLOCKED_WELLS");
    strcpy(keywords[i++],"BLOCKED_LOGS");
    strcpy(keywords[i++],"WAVELETS");     //25
    strcpy(keywords[i++],"EXTRA_SURFACES");
    strcpy(keywords[i++],"PRIORCORRELATIONS");
    strcpy(keywords[i++],"NOTIME");       //28

    if (i != nKeys)
    { 
      sprintf(errText,"In readCommandOutput: i != nKeys  (%d != %d)\n", i,nKeys);
      pos += nPar+1;
      return(1);
    }

    int gridFlag = 0;
    int wellFlag = 0;
    int domainFlag = modelSettings_->getOutputDomainFlag();
    int formatFlag = 0;
    int otherFlag = 0;

    char * flag;
    //sprintf(errText,"%c",'\0');

    for(i=0; i<nPar; i++)
    {
      flag = uppercase(params[pos+i]);
      for(key = 0;key < nKeys;key++)
        if(strcmp(flag,keywords[key]) == 0)
          break;
      if(key < 4)   
        formatFlag = (formatFlag | static_cast<int>(pow(2.0f,key)));
      else if(key < 22)
        gridFlag = (gridFlag | static_cast<int>(pow(2.0f,(key-4))));
      else if(key < 25)
        wellFlag = (wellFlag | static_cast<int>(pow(2.0f,(key-22))));
      else if(key < 28)
        otherFlag = (otherFlag | static_cast<int>(pow(2.0f,(key-25))));
      else if(key < 29)
        domainFlag = (domainFlag & static_cast<int>(pow(2.0f,(key-28))));
      else
      {
        error = 1;
        sprintf(errText,"%sUnknown option '%s' in command %s. Valid options are\n", errText, flag,params[pos-1]);
        for (i = 0 ; i < nKeys ; i++)
          sprintf(errText,"%s  %s\n", errText, keywords[i]);
      }
    }
    for(i=0;i<nKeys;i++)
      delete [] keywords[i];
    delete [] keywords;

    if((gridFlag & ModelSettings::FACIESPROB) >0 && (gridFlag & ModelSettings::FACIESPROBRELATIVE)>0)
    {
      gridFlag -= ModelSettings::FACIESPROBRELATIVE;
      LogKit::LogFormatted(LogKit::LOW,"Warning: Both FACIESPROB and FACIESPROBRELATIVE are wanted as output. Only FACIESPROB is given.\n");
    }

    //Some backward compatibility lines.
    if((gridFlag & ModelSettings::BACKGROUND) > 0)
      otherFlag += ModelSettings::BACKGROUND_TREND_1D;
    if((gridFlag & ModelSettings::CORRELATION) > 0)
      otherFlag = (otherFlag | ModelSettings::PRIORCORRELATIONS);

    if (formatFlag != 0)
      modelSettings_->setOutputFormatFlag(formatFlag);
    if (gridFlag != 0)
      modelSettings_->setGridOutputFlag(gridFlag);
    if (wellFlag != 0)
      modelSettings_->setWellOutputFlag(wellFlag);
    if (otherFlag != 0)
      modelSettings_->setOtherOutputFlag(otherFlag);
    modelSettings_->setOutputDomainFlag(domainFlag);
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandWhiteNoise(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    float wnc = float(atof(params[pos]));
    if(wnc < 0.00005 || wnc > 0.1000001)
    {
      error = 1;
      sprintf(errText,"Error: Value given in command %s must be between 0.005 and 0.1\n",
        params[pos-1]);
    }
    else
      modelSettings_->setWNC(wnc);
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandSegYOffset(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
    modelSettings_->setSegyOffset(float(atof(params[pos])));
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandForceFile(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    int fileGrid = atoi(params[pos]);
    if(fileGrid < -1 || fileGrid > 1)
    {
      error = 1;
      sprintf(errText,"Error: Value given in command %s must be between -1 and 1\n",
        params[pos-1]);
    }
    modelSettings_->setFileGrid(fileGrid);
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandKriging(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1, -1);
  if(error == 0)
  {
    int i;
    float * krigingParams = new float[nPar];
    for(i=0;i<nPar;i++)
      krigingParams[i] = float(atof(params[pos+i]));
    modelSettings_->setKrigingParameter(int(krigingParams[0]));
    delete [] krigingParams;
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandLocalWavelet(char ** params, int & pos, char * errText)
{
  modelSettings_->setUseLocalWavelet(true);
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2, -1);
  if(error == 0)
  {
    int i, key, nKeys = 2;
    char ** keywords = new char*[nKeys];
    int * keyIndex = new int[nKeys];
    int * nArgs = new int[nKeys];
    for(i=0;i<nKeys;i++)
    {
      keywords[i] = new char[40];
      keyIndex[i] = -1;
    }
    i = 0;
    strcpy(keywords[i++],"SHIFT");
    strcpy(keywords[i++],"GAIN");

    char * param;
    sprintf(errText,"%c",'\0');
    int prevKey = -1;
    for(i=0; i<nPar; i++)
    {
      param = uppercase(params[pos+i]);
      for(key = 0;key < nKeys;key++)
        if(strcmp(param,keywords[key]) == 0)
          break;
      if(key < nKeys)
      {
        if(keyIndex[key] != -1)
        {
          error = 1;
          sprintf(errText,"%sError: Keyword %s given more than once in command %s.\n",
            errText, keywords[key], params[pos-1]);
        }
        keyIndex[key] = i;
        nArgs[key] = nPar-i-1;
        if(prevKey != -1)
          nArgs[prevKey] = keyIndex[key] - keyIndex[prevKey] - 1;
        prevKey = key;
      }
      else if(i == 0)
      {
        error = 1;
        sprintf(errText,"%sError: Unrecognized keyword %s given in command %s.\n",
          errText, keywords[key], params[pos-1]);
        break;
      }
    }
    if(error == 0) //Read keywords. Check number of arguments.
    {
      int j;
      for(i=0;i<nKeys;i++)
      {
        if(nArgs[i] == 0)
        {
          error = 1;
          sprintf(errText,"%sError: Keyword %s has 0 arguments in command %s.\n",
            errText, keywords[i], params[pos-1]);
        }
        for(j=i+1;j<nKeys;j++)
          if(nArgs[i] > 0 && nArgs[j] > 0 && nArgs[i] != nArgs[j])
          {
            error = 1;
            sprintf(errText,"%sError: Keywords %s and %s has different number of arguments (%d vs %d) in command %s.\n",
              errText, keywords[i], keywords[j], nArgs[i], nArgs[j], params[pos-1]);
          }
      }
    }
    if(error == 0) //Check file reading
    {
      for(i=0;i<nKeys;i++)
        if(keyIndex[i] > -1)
        {
          //
          // This must be redesigned when local wavelet is reactivated in new XML-reader
          //
          // There must be one counter for gain maps and one for shift maps. Both
          // counters must later be compared to number-of-angles which they must equal
          //
          //modelSettings_->setNumberOfLocalWaveletArgs(nArgs[i]);
          //nWaveletTransfArgs_ = nArgs[i];
          int openError = checkFileOpen(&(params[pos+keyIndex[i]+1]), nArgs[i], params[pos-1], errText);
          if(openError != 0)
            error = 1;
          
          LogKit::LogFormatted(LogKit::LOW,"ERROR: Keyword LOCALWAVELET has temporarily been deactivated..\n");
          exit(1);
        }
    }
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandEnergyTreshold(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    float energyTreshold = float(atof(params[pos]));
    if(energyTreshold < 0.0f || energyTreshold > 0.1000001)
    {
      error = 1;
      sprintf(errText,"Error: Value given in command %s must be between 0 and 0.1\n",
        params[pos-1]);
    }
    else
    {
      modelSettings_->setEnergyThreshold(energyTreshold);
    }
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandParameterCorr(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);

  if(error == 0)
  {
    inputFiles_->setParamCorrFile(params[pos]);
    error = checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandReflectionMatrix(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    inputFiles_->setReflMatrFile(params[pos]);
    error = checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandFrequencyBand(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2, 2);
  if(error == 0)
  {
    float lowCut  = float(atof(params[pos]));
    float highCut = float(atof(params[pos+1]));
    if(lowCut >= highCut)
    {
      error = 1;
      sprintf(errText,"Error: High cut must be lager than low cut in command %s\n",params[pos-1]);
    }
    else
    {
      modelSettings_->setLowCut(lowCut);
      modelSettings_->setHighCut(highCut);
    }
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandDebug(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 0, 1);
  if(error == 0)
  {
    int debugFlag = 1;
    if(nPar > 0)
      debugFlag = int(atoi(params[pos]));
    modelSettings_->setDebugFlag(debugFlag);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandSeismicResolution(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    float frequency = float(atof(params[pos]));
    if (frequency < 30.0f || frequency > 60.0f)
    {
      error = 1;
      sprintf(errText,"Error: The seismic resolution must be in the range 30Hz - 60Hz\n");
    }
    else
      modelSettings_->setMaxHzSeismic(frequency);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandWaveletTaperingL(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    modelSettings_->setWaveletTaperingL(float(atof(params[pos])));
  }
  pos += nPar+1;
  return(error);
}

int 
ModelFile::readCommandPUndef(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    modelSettings_->setPundef(float(atof(params[pos])));
  }
  pos += nPar+1;
  return(error);
}


int
ModelFile::readCommandMaxDeviationAngle(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    float angle = float(atof(params[pos]));
    if (angle <= 0.0f || angle > 90.0f)
    {
      error = 1;
      sprintf(errText,"Error: The deviation angle must be in the rang 0-90 degrees.\n");
    }
    else
      modelSettings_->setMaxDevAngle(angle);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandAllowedParameterValues(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 6);
  if(error == 0)
  {
    modelSettings_->setAlphaMin(float(atof(params[pos+0])));
    modelSettings_->setAlphaMax(float(atof(params[pos+1])));
    modelSettings_->setBetaMin (float(atof(params[pos+2])));
    modelSettings_->setBetaMax (float(atof(params[pos+3])));
    modelSettings_->setRhoMin  (float(atof(params[pos+4])));
    modelSettings_->setRhoMax  (float(atof(params[pos+5])));
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandAllowedResidualVariances(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 6);
  if(error == 0)
  {
    modelSettings_->setVarAlphaMin(float(atof(params[pos+0])));
    modelSettings_->setVarAlphaMax(float(atof(params[pos+1])));
    modelSettings_->setVarBetaMin (float(atof(params[pos+2])));
    modelSettings_->setVarBetaMax (float(atof(params[pos+3])));
    modelSettings_->setVarRhoMin  (float(atof(params[pos+4])));
    modelSettings_->setVarRhoMax  (float(atof(params[pos+5])));
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandCorrelationDirection(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    inputFiles_->setCorrDirFile(params[pos]);
    error += checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandWaveletEstimationInterval(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2);
  if(error == 0)
  {
    inputFiles_->setWaveletEstIntFile(0,params[pos  ]);
    inputFiles_->setWaveletEstIntFile(1,params[pos+1]);

    if (!isNumber(params[pos])) 
      error += checkFileOpen(&(params[pos  ]), 1, params[pos-1], errText);
    if (!isNumber(params[pos+1]))
      error += checkFileOpen(&(params[pos+1]), 1, params[pos-1], errText);
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandFaciesEstimationInterval(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 2);
  if(error == 0)
  {
    inputFiles_->setFaciesEstIntFile(0,params[pos  ]);
    inputFiles_->setFaciesEstIntFile(1,params[pos+1]);

    if (!isNumber(params[pos])) 
      error += checkFileOpen(&(params[pos  ]), 1, params[pos-1], errText);
    if (!isNumber(params[pos+1]))
      error += checkFileOpen(&(params[pos+1]), 1, params[pos-1], errText);

    sprintf(errText,"Command %s has not been implemented yet.\n",params[pos-1]);
    error = 1;
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandLogLevel(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    char * level = new char[MAX_STRING];
    strcpy(level,params[pos]);
    uppercase(level);
    int logLevel = LogKit::ERROR;
    if(strcmp(level,"ERROR") == 0)
      logLevel = LogKit::L_ERROR;
    else if(strcmp(level,"WARNING") == 0)
      logLevel = LogKit::L_WARNING;
    else if(strcmp(level,"LOW") == 0)
      logLevel = LogKit::L_LOW;
    else if(strcmp(level,"MEDIUM") == 0)
      logLevel = LogKit::L_MEDIUM;
    else if(strcmp(level,"HIGH") == 0)
      logLevel = LogKit::L_HIGH;
    else if(strcmp(level,"DEBUGLOW") == 0)
      logLevel = LogKit::L_DEBUGLOW;
    else if(strcmp(level,"DEBUGHIGH") == 0)
      logLevel = LogKit::L_DEBUGHIGH;
    else {
      sprintf(errText,"Unknown log level %s in command %s\n",params[pos],params[pos-1]);
      sprintf(errText,"%sChoose from ERROR, WARNING, LOW, MEDIUM, and HIGH\n",errText);
      error = 1;
    }
    modelSettings_->setLogLevel(logLevel);
    delete [] level;
  }
  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandTraceHeaderFormat(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1, -1);
  if(error == 1)
  {
    pos += nPar+1;
    return(1);
  }

  int type                   = 0;        // Format type (0=SEISWORKS, 1=IESX)
  int coordSysType           = IMISSING; // Choice of coordinate system
  int bypassCoordScaling     = IMISSING; // Bypass coordinate scaling
  int traceCoordGivenAsFloat = IMISSING; // trace xy-coord given as float
  int startPosCoordScaling   = IMISSING; // Start pos of coordinate scaling
  int startPosTraceXCoord    = IMISSING; // Start pos trace x coordinate
  int startPosTraceYCoord    = IMISSING; // Start pos trace y coordinate
  int startPosILIndex        = IMISSING; // Start pos inline index
  int startPosXLIndex        = IMISSING; // Start pos crossline index

  int nSubCommands = 11;
  char ** subCommand = new char * [nSubCommands];
  for(int i=0 ; i<nSubCommands ; i++)
    subCommand[i] = new char[30];
  strcpy(subCommand[0], "SEISWORKS");
  strcpy(subCommand[1], "IESX");
  strcpy(subCommand[2], "UTM");
  strcpy(subCommand[3], "ILXL");
  strcpy(subCommand[4], "BYPASS_COORDINATE_SCALING");
  strcpy(subCommand[5], "TRACE_XY_COORD_AS_FLOAT"); // Currently not used in SegY lib
  strcpy(subCommand[6], "START_POS_COORD_SCALING");
  strcpy(subCommand[7], "START_POS_TRACE_X_COORD");
  strcpy(subCommand[8], "START_POS_TRACE_Y_COORD");
  strcpy(subCommand[9], "START_POS_INLINE_INDEX");
  strcpy(subCommand[10],"START_POS_CROSSLINE_INDEX");

  int  curPar = 0;
  while(curPar < nPar && error == 0) 
  {
    //
    // Find number of elements in current subcommand
    //
    int comPar = 1;
    while(isNumber(params[pos+curPar+comPar]))
      comPar++;

    int subCom = 0;
    while(subCom < nSubCommands && strcmp(params[pos+curPar], subCommand[subCom]) != 0)
      subCom++;

    switch(subCom) 
    {
    case 0:
      type = 0; // SEISWORKS
      break;

    case 1:
      type = 1; // IESX
      break;

    case 2:
      coordSysType = 0; // UTM
      break;

    case 3:
      coordSysType = 1; // ILXL
      break;

    case 4:
      bypassCoordScaling = 1;
      break;

    case 5:
      traceCoordGivenAsFloat = 1; 
      break;

    case 6:
      if(comPar != 2) {
        error = 1;
        sprintf(errText,"Subcommand %s in command %s takes 1 parameter, not %d as was given.\n",
                subCommand[subCom],params[pos-1], comPar-1);
      }
      else 
        startPosCoordScaling = static_cast<int>(atof(params[pos+curPar+1]));
      break;

    case 7:
      if(comPar != 2) {
        error = 1;
        sprintf(errText,"Subcommand %s in command %s takes 1 parameter, not %d as was given.\n",
                subCommand[subCom],params[pos-1], comPar-1);
      }
      else 
        startPosTraceXCoord = static_cast<int>(atof(params[pos+curPar+1]));
      break;

    case 8:
      if(comPar != 2) {
        error = 1;
        sprintf(errText,"Subcommand %s in command %s takes 1 parameter, not %d as was given.\n",
                subCommand[subCom],params[pos-1], comPar-1);
      }
      else 
        startPosTraceYCoord = static_cast<int>(atof(params[pos+curPar+1]));
      break;

    case 9:
      if(comPar != 2) {
        error = 1;
        sprintf(errText,"Subcommand %s in command %s takes 1 parameter, not %d as was given.\n",
                subCommand[subCom],params[pos-1], comPar-1);
      }
      else 
        startPosILIndex = static_cast<int>(atof(params[pos+curPar+1]));
      break;

    case 10:
      if(comPar != 2) {
        error = 1;
        sprintf(errText,"Subcommand %s in command %s takes 1 parameter, not %d as was given.\n",
                subCommand[subCom],params[pos-1], comPar-1);
      }
      else 
        startPosXLIndex = static_cast<int>(atof(params[pos+curPar+1]));
      break;

    default: 
      error = 1;
      sprintf(errText,"Unknown subcommand %s found in command %s.\n", params[pos+curPar],params[pos-1]);
      sprintf(errText,"%s\nValid subcommands are:\n",errText);
      for (int i = 0 ; i < nSubCommands ; i++)
        sprintf(errText,"%s  %s\n",errText,subCommand[i]);
      break;
    }
    curPar += comPar;
  }
  for(int i=0;i<nSubCommands;i++)
    delete [] subCommand[i];
  delete [] subCommand;

  TraceHeaderFormat thf(type,
                        bypassCoordScaling,
                        startPosCoordScaling,
                        startPosTraceXCoord,
                        startPosTraceYCoord,
                        startPosILIndex,    
                        startPosXLIndex,    
                        coordSysType);
  modelSettings_->setTraceHeaderFormat(thf);

  pos += nPar+1;
  return(error);
}

int
ModelFile::readCommandBackgroundVelocity(char ** params, int & pos, char * errText)
{
  int error;
  int nPar = getParNum(params, pos, error, errText, params[pos-1], 1);
  if(error == 0)
  {
    inputFiles_->setBackVelFile(params[pos]);
    error += checkFileOpen(&(params[pos]), 1, params[pos-1], errText);
  }
  pos += nPar+1;
  return(error);
}

int 
ModelFile::getParNum(char ** params, int pos, int & error, char * errText,
                     const char * command, int min, int max)
{
  int i=0;
  error = 0;
  while(params[pos+i][0] != ';')
    i++;
  if(max == 0) //min is exact limit)
  {
    if(i != min)
    {
      error = 1;
      sprintf(errText,"Command %s takes %d arguments; %d given in file.\n",
        command, min, i);
    }
  }
  else if(i < min)//May have no upper limit; does not matter.
  {
    error = 1;
    sprintf(errText,
      "Command %s takes at least %d arguments; %d given in file.\n",
      command, min, i);
  }
  else if(max > 0 && i > max)
  {
    error = 1;
    sprintf(errText,
      "Command %s takes at most %d arguments; %d given in file.\n",
      command, max, i);
  }
  return i;
}


// checkFileOpen: checks if the files in fNames table can be opened. Error message is
//                given in errText if not, and error is binary coded numbering of failures.  
int 
ModelFile::checkFileOpen(char ** fNames, int nFiles, const char * command, char * errText, int start,
                     bool details)
{
  if(details == true && nFiles >= 32)
  {
    details = false;
    LogKit::LogFormatted(LogKit::LOW,"\nINFO: Log details for checkFileOpen() has been turned off since nFiles>=32\n");
  }
  int i, error = 0;
  int flag = 1;
  int nErr = 0;
  for(i=start;i<nFiles+start;i++)
  {
    if(fNames[i][0] != '*' && fNames[i][0] != '?')
    {
      FILE * file = fopen(fNames[i],"r");
      if(file == 0)
      {
        error+= flag;
        nErr++;
        sprintf(errText,"%sCould not open file %s (command %s).\n",errText,fNames[i], command);
      }
      else
        fclose(file);
      if(details == true)
        flag *= 2;
    }
  }
  return(error);
}

// checkFileOpen: checks if the files in fNames table can be opened. Error message is
//                given in errText if not, and error is binary coded numbering of failures.  
int 
ModelFile::checkFileOpen(const std::string & fName, 
                         const char        * command, 
                         char              * errText)
{
  int error = 0;
  if(fName != "*" && fName != "?")
  {
    FILE * file = fopen(fName.c_str(),"r");
    if(file == 0)
    {
      error = 1;
      sprintf(errText,"%sCould not open file %s (command %s).\n",errText,fName.c_str(), command);
    }
    else
      fclose(file);
  }
  return(error);
}

Vario *
ModelFile::createVario(char ** param, int nPar, const char * command, char * errText)
{
  int i, nVario = 2; //New vario: Increase this.
  char * vTypeIn = uppercase(param[0]);
  char ** vTypes = new char *[nVario];
  int * varPar = new int[nVario];
  for(i=0;i<nVario;i++)
    vTypes[i] = new char[20];
  strcpy(vTypes[0],"SPHERICAL"); //Name of vario
  varPar[0] = 3;                 //Number of parameters for vario
  strcpy(vTypes[1],"GENEXP"); 
  varPar[1] = 4; //New vario: Add below.

  for(i=0;i<nVario;i++)
    if(strcmp(vTypeIn,vTypes[i])==0)
      break;

  Vario * result = NULL;
  if(i == nVario || nPar-1 == varPar[i] || nPar-1 == varPar[i] - 2)
  {
    switch(i)
    {
    case 0:
      if(nPar-1==varPar[i]) {
        // Convert from azimuth to internal angle. Conversion to radians are later.
        float rot = static_cast<float>(90.0 - atof(param[3]));
        result = new SphericalVario(static_cast<float>(atof(param[1])), 
                                    static_cast<float>(atof(param[2])), 
                                    rot);
      }
      else
        // Do not convert the angle here
        result = new SphericalVario(static_cast<float>(atof(param[1])));
      break;
    case 1:
      if(nPar-1==varPar[i]) {
        // Convert from azimuth to internal angle. Conversion to radians are later.
        float rot = static_cast<float>(90.0 - atof(param[4]));
        result = new GenExpVario(static_cast<float>(atof(param[1])), 
                                 static_cast<float>(atof(param[2])), 
                                 static_cast<float>(atof(param[3])), 
                                 rot);
      }
      else
        // Do not convert the angle here.
        result = new GenExpVario(static_cast<float>(atof(param[1])), 
                                 static_cast<float>(atof(param[2])));
      break;
    default: //New vario: Add here.
      sprintf(errText, "Unknown variogram type %s in command %s.\n",
              param[0], command);
      break;
    }
  }
  else
    sprintf(errText, 
            "Variogram %s in command %s needs %d parameters (%d given).\n",
            param[0], command, varPar[i], nPar-1);
  
  for(i=0;i<nVario;i++)
    delete [] vTypes[i];
  delete [] vTypes;
  delete [] varPar;
  return(result);
}

