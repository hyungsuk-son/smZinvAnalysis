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

#include "smZInvAnalysis/smZInvAnalysis.h"

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

  // Data15
  //SH::addGrid (sh, "data15_13TeV.00284213.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950");
  //SH::addGrid (sh, "data15_13TeV.00284285.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950");
  //SH::addGrid (sh, "data15_13TeV.00284420.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950");
  //SH::addGrid (sh, "data15_13TeV.00284427.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950");
  //SH::addGrid (sh, "data15_13TeV.00284484.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950");
  // Data16
  //SH::addGrid (sh, "data16_13TeV.00311473.physics_Main.merge.DAOD_EXOT5.f758_m1710_p2950");
  // mc15c (EXOT5)
  /*
  SH::addGrid (sh, "mc15_13TeV.364142.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364143.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364144.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364145.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364146.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364147.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364150.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  SH::addGrid (sh, "mc15_13TeV.364155.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV1000_E_CMS.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949");
  */

  // mc15c (STDM4)
  /*
  SH::addGrid (sh, "mc15_13TeV.364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  SH::addGrid (sh, "mc15_13TeV.364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS.merge.DAOD_STDM4.e5271_s2726_r7772_r7676_p2952");
  */
  // ttbar (410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad)
  // EXOT5 derivation
  //SH::addGrid (sh, "mc15_13TeV.410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad.merge.DAOD_EXOT5.e5458_s2726_r7772_r7676_p2949");
  // My skim_v3
  SH::addGrid (sh, "user.hson.mc15c.13TeV.DAOD_EXOT5_skim_v3.03122018.410501.e5458_s2726_r7772_r7676_p2949_mini-xAOD.root");

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
  job.options()->setDouble (EL::Job::optCacheSize, 10*1024*1024);
  job.options()->setDouble (EL::Job::optCacheLearnEntries, 20);

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
  smZInvAnalysis* alg = new smZInvAnalysis();
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

  driver.options()->setString("nc_outputSampleName", "user.hson.xAOD.03132018.mc15c.ttbar.skim_v3.%in:name[2]%.%in:name[6]%"); //For PrunDriver
//  driver.outputSampleName = "user.hson.gridtest1.11142015.%in:name[2]%.%in:name[6]%"; //For GridDriver
//  driver.options()->setDouble("nc_nFiles", 1); // FOR TESTING!
  //driver.options()->setDouble("nc_nFilesPerJob", 1);
  driver.options()->setString(EL::Job::optGridMergeOutput, "true");
  //driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB
  //driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 2);
//  driver.options()->setDouble(EL::Job::optGridMaxNFilesPerJob, 3);
//  driver.options()->setString("nc_excludedSite", "ANALY_SCINET,ANALY_VICTORIA,ANALY_CERN_CLOUD,ANALY_IN2P3-CC,ANALY_LAPP,ANALY_CONNECT_SHORT,ANALY_SFU,ANALY_CONNECT,ANALY_RAL_SL6,ANALY_GRIF-LPNHE,ANALY_HU_ATLAS_Tier2,ANALY_OU_OCHEP_SWT2,ANALY_IFIC,ANALY_ECDF_SL6");
//  driver.options()->setString("nc_excludedSite", "ANALY_INFN-NAPOLI-RECAS,ANALY_INFN-NAPOLI,ANALY_DESY-HH,ANALY_GRIF-IRFU,ANALY_AUSTRALIA,ANALY_SFU,ANALY_SCINET,ANALY_CPPM,ANALY_SiGNET,ANALY_LPC,ANALY_NSC,ANALY_CONNECT,ANALY_MWT2_SL6,ANALY_BU_ATLAS_Tier2_SL6,ANALY_wuppertalprod,ANALY_ARNES,ANALY_SLAC_SHORT_1HR,ANALY_SLAC,ANALY_RAL_SL6,ANALY_INFN-MILANO-ATLASC");
//  driver.options()->setString("nc_excludedSite", "ANALY_TRIUMF");
//  driver.options()->setString("nc_site", "ANALY_CERN_SHORT,ANALY_CERN_SLC6,ANALY_PIC_SL6,ANALY_SARA"); // The Reflex dictionary build only works on a few sites
//  driver.options()->setString("nc_site", "ANALY_CERN_SLC6"); // The Reflex dictionary build only works on a few sites

//  driver.submit( job, submitDir );  // with monitoring
  driver.submitOnly(job, submitDir);  //without monitoring


  // https://atlaswww.hep.anl.gov/asc/WebSVN/filedetails.php?repname=dijets&path=%2Fcode%2Ftrunk%2Futil%2FrunDijetResonance.cxx
  // For grid, save list of ouput containers to the submission directory
  /*
  std::ofstream fileList((submitDir+"/outputContainers.txt"), std::ios_base::out);
  for( unsigned int iCont=0; iCont < outputContainerNames.size(); ++iCont){
    fileList << outputContainerNames.at(iCont)+"\n";
  }
  fileList.close();
  */

  return 0;
}
