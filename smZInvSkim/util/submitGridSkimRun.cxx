#include "xAODRootAccess/Init.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoopGrid/PrunDriver.h"
#include "EventLoopGrid/GridDriver.h"
#include "EventLoopGrid/GridWorker.h"
#include <TSystem.h>
#include "SampleHandler/Sample.h"
#include "SampleHandler/SampleGrid.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ScanDir.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/MetaFields.h"
#include "SampleHandler/MetaObject.h"
#include <EventLoopAlgs/NTupleSvc.h>
#include <EventLoop/OutputStream.h>

#include "smZInvSkim/smZInvSkim.h"

int main( int argc, char* argv[] ) {

  // Take the submit directory from the input if provided:
  std::string submitDir = "submitDir";
  if( argc > 1 ) submitDir = argv[ 1 ];

  // Set up the job for xAOD access:
  xAOD::Init().ignore();

  // Construct the samples to run on:
  SH::SampleHandler sh;

  // use SampleHandler to scan all of the subdirectories of a directory for particular MC single file:
//  const char* inputFilePath = gSystem->ExpandPathName ("$ALRB_TutorialData/r6630/");
//  SH::ScanDir().sampleDepth(1).samplePattern("AOD.05352803._000031.pool.root.1").scan(sh, inputFilePath);
//  const char* inputFilePath = gSystem->ExpandPathName ("/afs/cern.ch/user/h/hson/Work/xAOD_data");
//  SH::DiskListLocal list (inputFilePath);
//  SH::scanDir (sh, list, "data15_13TeV.00271048.physics_Main.merge.AOD.f611_m1463._lb0408._0001.1"); // specifying one particular file for testing
  // If you want to use grid datasets the easiest option for discovery is scanDQ2
//  SH::scanDQ2 (sh, "data15_13TeV.00270816.physics_Main.merge.AOD.f611_m1463");
  //SH::scanRucio (sh, "mc15_13TeV.364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  // If you know the name of all your grid datasets you can also skip the dq2-ls step and add the datasets directly


  // Retrieve dataset from the list file
  // https://atlaswww.hep.anl.gov/asc/WebSVN/filedetails.php?repname=dijets&path=%2Fcode%2Ftrunk%2Futil%2FrunDijetResonance.cxx
 
  // Create my output mini-xAOD dataset name
  std::string myTagVer = "v2.1";
  // MC16a backgrounds
  // EXOT5
  //std::string myTagName = "mc16a_13TeV.Sherpa_Zmumu_skim_EXOT5_"+myTagVer; // Zmumu
  //std::string myTagName = "mc16a_13TeV.Sherpa_Zee_skim_EXOT5_"+myTagVer; // Zee
  //std::string myTagName = "mc16a_13TeV.Sherpa_Znunu_skim_EXOT5_"+myTagVer; // Znunu
  //std::string myTagName = "mc16a_13TeV.Sherpa_Ztautau_skim_EXOT5_"+myTagVer; // Ztautau
  //std::string myTagName = "mc16a_13TeV.Sherpa_Wmunu_skim_EXOT5_"+myTagVer; // Wmunu
  std::string myTagName = "mc16a_13TeV.Sherpa_Wenu_skim_EXOT5_"+myTagVer; // Wenu
  //std::string myTagName = "mc16a_13TeV.Sherpa_Wtaunu_skim_EXOT5_"+myTagVer; // Wtaunu
  //std::string myTagName = "mc16a_13TeV.Sherpa_Diboson_skim_EXOT5_"+myTagVer; // Diboson
  //std::string myTagName = "mc16a_13TeV.PowhegPythiaEvtGen_Top_skim_EXOT5_"+myTagVer; // Top
  //std::string myTagName = "mc16a_13TeV.Pythia8EvtGen_Multijet_skim_EXOT5_"+myTagVer; // Multijet
  // STDM4
  //std::string myTagName = "mc16a_13TeV.Sherpa_Zmumu_skim_STDM4_"+myTagVer; // Zmumu
  //std::string myTagName = "mc16a_13TeV.Sherpa_Zee_skim_STDM4_"+myTagVer; // Zee
  // Data
  //std::string myTagName = "data15_13TeV.skim_EXOT5_"+myTagVer; // Data 2015 (EXOT5 derivation)
  //std::string myTagName = "data16_13TeV.skim_EXOT5_"+myTagVer; // Data 2016 (EXOT5 derivation)
  std::string containerName;
  std::vector< std::string > outputContainerNames; //for grid only

  // List of input dataset
  // MC16a backgrounds
  std::string listFilePath = "/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/smZinvAnalysis/smZInvSkim/util/";
  // EXOT5
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Zmumu_dataset.txt" ); // Zmumu
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Zee_dataset.txt" ); // Zee
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Znunu_dataset.txt" ); // Znunu
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Ztautau_dataset.txt" ); // Ztautau
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Wmunu_dataset.txt" ); // Wmunu
  std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Wenu_dataset.txt" ); // Wenu
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Wtaunu_dataset.txt" ); // Wtaunu
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Diboson_dataset.txt" ); // Diboson
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Top_dataset.txt" ); // Top
  //std::ifstream inFile( listFilePath+"mc16a_deriv_EXOT5_Multijet_dataset.txt" ); // Multijet
  // STDM4
  //std::ifstream inFile( listFilePath+"mc16a_deriv_STDM4_Zmumu_dataset.txt" ); // Zmumu
  //std::ifstream inFile( listFilePath+"mc16a_deriv_STDM4_Zee_dataset.txt" ); // Zee
  // Data
  //std::ifstream inFile( listFilePath+"data15_deriv_EXOT5.txt" ); // Data 2015 (EXOT5 derivation)
  //std::ifstream inFile( listFilePath+"data16_deriv_EXOT5.txt" ); // Data 2016 (EXOT5 derivation)
  while(std::getline(inFile, containerName) ){
    if (containerName.size() > 1 && containerName.find("#") != 0 ){
      std::cout << "Adding container : " << containerName << std::endl;
      SH::addGrid( sh, containerName);
      //Add output container name to file of containers
      // follows grid format: "user.hson."+myTagName+".%in:name[2]%.%in:name[6]%"

      // Container name example (from dataset name)
      // For MC dataset ex) mc15_13TeV.364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949
      // For Data dataset ex) data15_13TeV.00276262.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950
      int namePosition1 = 0;
      int namePosition2 = 0;
      namePosition1 = containerName.find_first_of(".", namePosition1);
      namePosition2 = containerName.find_last_of(".")+1;
      std::string datasetNum = ""; // Copy wanted string from input dataset (containerName)
      if (containerName.find("mc")!=std::string::npos){ // For MC
        datasetNum = containerName.substr(namePosition1, 8); // ex) .364151.
      }
      if (containerName.find("data")!=std::string::npos){ // For Data
        datasetNum = containerName.substr(namePosition1, 10); // ex) .00276262.
      }
      std::string tagName = containerName.substr(namePosition2); // ex) e5308_s2726_r7772_r7676_p2949

      outputContainerNames.push_back( ("user.hson."+myTagName+datasetNum+tagName+"_mini-xAOD.root/") );
      std::cout << "Printing output container name : " << "user.hson."+myTagName+datasetNum+tagName+"_mini-xAOD.root/" << std::endl;
    }
  }



  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sh.setMetaString( "nc_tree", "CollectionTree" );
  sh.setMetaString("nc_grid_filter", "*AOD*");
//  sh.setMetaString("nc_grid_filter", "*");

  // Print what we found:
  sh.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler( sh );
  //job.options()->setDouble (EL::Job::optMaxEvents, 500);
  //job.options()->setDouble (EL::Job::optRetries, 30);
  // Use TTreeCache to precache data files to speed up analysis
  //job.options()->setDouble (EL::Job::optCacheSize, 10*1024*1024);
  //job.options()->setDouble (EL::Job::optCacheLearnEntries, 20);

/*  
  // For ntuple
  // define an output and an ntuple associated to that output
  EL::OutputStream output  ("myOutput");
  job.outputAdd (output);
  EL::NTupleSvc *ntuple = new EL::NTupleSvc ("myOutput");
  job.algsAdd (ntuple);
  //---------------------------------------------------------
*/

  // Add our analysis to the job:
  smZInvSkim* alg = new smZInvSkim();
  job.algsAdd( alg );

/*
  // For ntuple
  // Let your algorithm know the name of the output file
  alg->outputName = "myOutput"; // give the name of the output to our algorithm
  //-----------------------------------------------------------------------------
*/

  // Run the job using the local/direct driver:
//  EL::DirectDriver driver; //local
  EL::PrunDriver driver;  //grid
//  EL::GridDriver driver; //grid in the background

  driver.options()->setString("nc_outputSampleName", "user.hson."+myTagName+".%in:name[2]%.%in:name[6]%"); //For PrunDriver
//  driver.outputSampleName = "user.hson.gridtest1.11142015.%in:name[2]%.%in:name[6]%"; //For GridDriver
//  driver.options()->setDouble("nc_nFiles", 1); // FOR TESTING!
  //driver.options()->setDouble("nc_nFilesPerJob", 1);
  //driver.options()->setString(EL::Job::optGridMergeOutput, "true");
  //driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB
  //driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 2);
//  driver.options()->setDouble(EL::Job::optGridMaxNFilesPerJob, 3);
  driver.options()->setDouble(EL::Job::optGridNGBPerJob, 6);
  driver.options()->setDouble(EL::Job::optGridNGBPerMergeJob, 5);
//  driver.options()->setString("nc_excludedSite", "ANALY_SCINET,ANALY_VICTORIA,ANALY_CERN_CLOUD,ANALY_IN2P3-CC,ANALY_LAPP,ANALY_CONNECT_SHORT,ANALY_SFU,ANALY_CONNECT,ANALY_RAL_SL6,ANALY_GRIF-LPNHE,ANALY_HU_ATLAS_Tier2,ANALY_OU_OCHEP_SWT2,ANALY_IFIC,ANALY_ECDF_SL6");
//  driver.options()->setString("nc_excludedSite", "ANALY_OX_SL6,ANALY_TAIWAN_PNFS_SL6,ANALY_TAIWAN_SL6,ANALY_CYF,ANALY_CYF,ANALY_FZU");
//  driver.options()->setString("nc_excludedSite", "ANALY_TRIUMF");
//  driver.options()->setString("nc_site", "ANALY_CERN_SHORT,ANALY_CERN_SLC6,ANALY_PIC_SL6,ANALY_SARA"); // The Reflex dictionary build only works on a few sites
//  driver.options()->setString("nc_site", "ANALY_CERN_SLC6"); // The Reflex dictionary build only works on a few sites

//  driver.submit( job, submitDir );  // with monitoring
  driver.submitOnly(job, submitDir);  //without monitoring


  // For grid, save list of ouput containers to the submission directory
  // https://atlaswww.hep.anl.gov/asc/WebSVN/filedetails.php?repname=dijets&path=%2Fcode%2Ftrunk%2Futil%2FrunDijetResonance.cxx
  std::ofstream fileList((submitDir+"/outputContainers.txt"), std::ios_base::out);
  for( unsigned int iCont=0; iCont < outputContainerNames.size(); ++iCont){
    fileList << outputContainerNames.at(iCont)+"\n";
  }
  fileList.close();


  return 0;
}
