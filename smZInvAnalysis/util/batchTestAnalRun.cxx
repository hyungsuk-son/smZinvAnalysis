#include "xAODRootAccess/Init.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoop/SlurmDriver.h"
#include "EventLoop/LSFDriver.h"
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
  // If you want to use grid datasets the easiest option for discovery is scanRucio
//  SH::scanRucio (sh, "data15_13TeV.00284484.physics_Main.merge.DAOD_EXOT5.f644_m1518_p2524");
//  SH::scanRucio (sh, "data15_13TeV.00284427.physics_Main.merge.DAOD_EXOT5.f643_m1518_p2524");
  // If you know the name of all your grid datasets you can also skip the dq2-ls step and add the datasets directly
//  SH::addGrid (sh, "data15_13TeV.00281411.physics_Main.merge.AOD.f629_m1504");
//  SH::addGrid (sh, "data15_13TeV.00282784.physics_Main.merge.AOD.f640_m1511");



  // EXOT5
  // Data16
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/DATA");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10316590._000203.pool.root.1").scan(sh,inputFilePath); // Data16 run311481
  // MC15c
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531595._000002.pool.root.1").scan(sh,inputFilePath); // Znunu_MAXHTPTV280_500_CFilterBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531581._000001.pool.root.1").scan(sh,inputFilePath); // Zee_MAXHTPTV140_280_BFilter
  //SH::ScanDir().filePattern("DAOD_EXOT5.09043234._000006.pool.root.1").scan(sh,inputFilePath); // 410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad
  //SH::ScanDir().filePattern("DAOD_EXOT5.08602669._000018.pool.root.1").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.pool.root.1").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531487._000003.pool.root.1").scan(sh,inputFilePath); // 364137.Ztautau_MAXHTPTV280_500_CVetoBVeto

  // Full dataset
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  // EXOT5
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000*pool.root*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531563._000*pool.root*").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto (106 files)
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531595._000*pool.root*").scan(sh,inputFilePath); // 364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto (77 files)
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531416._000*pool.root*").scan(sh,inputFilePath); // 364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter (186 files)
  // STDM4
  //SH::ScanDir().filePattern("DAOD_STDM4.10349557._000*pool.root*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (48 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10350125._000*pool.root*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto (28 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349019._000*pool.root*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter (48 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10348687._000*pool.root*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (47 files)
  //SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto (files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349228._000*pool.root*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter (38 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349739._0000*pool.root*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto (75 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10350262._000*pool.root*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000 (82 files)
  //SH::ScanDir().filePattern("*pool.root*").scan(sh,inputFilePath); //  ( files)
  //SH::ScanDir().filePattern("*pool.root*").scan(sh,inputFilePath); //  ( files)
  //SH::ScanDir().filePattern("*pool.root*").scan(sh,inputFilePath); //  ( files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349856._000*pool.root*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000 (84 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10350063._000*pool.root*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter (106 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349286._000*pool.root*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto (107 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10350277._000*pool.root*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter (122 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349766._000*pool.root*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (132/222 files)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349762._000*pool.root*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (91/222 files)

  // my skimmed sample
  // DATA16
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/DATA/Run2");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10316590._000203.mini-xAOD.root.1").scan(sh,inputFilePath); // Run 311365
  // MC15c
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/atlas09Home/Dataset/MC/skim");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531595._000002.mini-xAOD.root.1").scan(sh,inputFilePath); // 364152.Znunu_MAXHTPTV280_500_CFilterBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.08602669._000018.mini-xAOD.root.1").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11407323*").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto old
  //SH::ScanDir().filePattern("user.hson.11414325*").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto new
  //SH::ScanDir().filePattern("user.hson.11415728*").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto new_new
  //SH::ScanDir().filePattern("user.hson.11415750*").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto new_new_new
  //SH::ScanDir().filePattern("user.hson.11461286*").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto FileMetaData retrieved in the Grid
  //SH::ScanDir().filePattern("user.hson.11473366*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11473382*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11473387*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
  //SH::ScanDir().filePattern("user.hson.11473386*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
  //SH::ScanDir().filePattern("user.hson.11473667._000*").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.12466751._0000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.12466753._0000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
  //SH::ScanDir().filePattern("user.hson.12466755._0000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
  //SH::ScanDir().filePattern("user.hson.12466756._0000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
  //SH::ScanDir().filePattern("user.hson.12466757._0000*").scan(sh,inputFilePath); // .364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS

  // STDM4
  // MC15c
  // my skimmed sample
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("user.hson.11473951*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto //noskim
  //SH::ScanDir().filePattern("user.hson.11483437*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto //skim
  //SH::ScanDir().filePattern("user.hson.11483559*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000 // skim
  //SH::ScanDir().filePattern("user.hson.11487814*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter


  // TRUTH1
  // MC15c
  // MADGRAPH
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("user.hson.12325022.EXT0._0002*").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto (full dataset) old
  //SH::ScanDir().filePattern("user.hson.12424890.EXT0._0002*").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto (full dataset, 204 files)) old
  //SH::ScanDir().filePattern("user.hson.12424903.EXT0._0000*").scan(sh,inputFilePath); // 363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto (full dataset, 84 files)) old
  //SH::ScanDir().filePattern("user.hson.12440341.EXT0._0002*").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto (full dataset, 204 files)) old2
  //SH::ScanDir().filePattern("user.hson.12440490.EXT0._0000*").scan(sh,inputFilePath); // 363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto (full dataset, 84 files)) old2
  //SH::ScanDir().filePattern("user.hson.12549947.EXT0._0000*").scan(sh,inputFilePath); // 363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto (full dataset, 84 files))
  //SH::ScanDir().filePattern("user.hson.12549944.EXT0._0002*").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto (full dataset, 204 files))
  // Sherpa
  //SH::ScanDir().filePattern("user.hson.12327093.EXT0._0005*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (full dataset) old
  //SH::ScanDir().filePattern("user.hson.12383809.EXT0._0005*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (full dataset, 523 files) old
  //SH::ScanDir().filePattern("user.hson.12440644.EXT0._0004*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (full dataset, 403 files)
  //SH::ScanDir().filePattern("user.hson.12440561.EXT0._0000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (full dataset, 523 files)




  // MC16a
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a");
  // Derivation
  // EXOT5
  //SH::ScanDir().filePattern("DAOD_EXOT5.118692*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.1189234*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  // Error test
  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/derivation/EXOT5/mc16_13TeV.364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter.deriv.DAOD_EXOT5.e5340_s3126_r9364_r9315_p3482");
  //SH::ScanDir().filePattern("DAOD_EXOT5.13472077._000*").scan(sh,inputFilePath); // 1_364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter (Error test)
  SH::ScanDir().filePattern("DAOD_EXOT5.13472080._000*").scan(sh,inputFilePath); // 2_364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter (Error test)
  // STDM4
  //SH::ScanDir().filePattern("DAOD_STDM4.127725*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.127729*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  // Skim
  // EXOT5
  //SH::ScanDir().filePattern("user.hson.14171858._0000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.14195993._0000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  // STDM4
  //SH::ScanDir().filePattern("user.hson.14171859._0000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.14195996._0000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  // Data15
  //SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 



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
//  job.options()->setDouble (EL::Job::optMaxEvents, 500);
  //job.options()->setDouble (EL::Job::optRetries, 30);
  //job.options()->setDouble (EL::Job::optCacheSize, 10*1024*1024);
  //job.options()->setDouble (EL::Job::optCacheLearnEntries, 50);

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
//  EL::PrunDriver driver;  //grid
//  EL::GridDriver driver; //grid in the background
//  EL::SlurmDriver driver; //batch
  //driver.options()->setString (EL::Job::optSubmitFlags, "-L /bin/bash"); // or whatever shell you are using for BatchDriver
//  driver.shellInit = "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase && source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh";
//  std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem=2000 -p short -t 4:00:00";
//  job.options()->setString(EL::Job::optSubmitFlags, slurmOptions);

  system("mkdir -p ~/bin/; ln -s /usr/bin/sbatch ~/bin/bsub; export PATH=$PATH:~/bin");
  std::string slurmJobName = "test";
  std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --begin=now+2000 --exclude=m4lmem01,alpha018 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --begin=now+1hour --exclude=m4lmem01,alpha018 -o stdout.%j -e stderr.%j --mail-type=END --mail-user=Hyungsuk.Son@tufts.edu --job-name="+slurmJobName;
  EL::LSFDriver driver; //batch
  job.options()->setBool(EL::Job::optResetShell, false);
  job.options()->setString(EL::Job::optSubmitFlags, slurmOptions);


  //driver.options()->setString("nc_outputSampleName", "user.hson.data15_13TeV.DAOD.02082017rerererere.%in:name[2]%.%in:name[6]%"); //For PrunDriver
  //  driver.outputSampleName = "user.hson.gridtest1.11142015.%in:name[2]%.%in:name[6]%"; //For GridDriver
//  driver.options()->setDouble("nc_nFiles", 2); // FOR TESTING!
//  driver.options()->setDouble("nc_nFilesPerJob", 1);
//  driver.options()->setDouble(EL::Job::optGridNFilesPerJob, 1);

//  driver.options()->setDouble(EL::Job::optGridMemory,10240); // 10 GB

//  driver.submit( job, submitDir );  // with monitoring
  driver.submitOnly(job, submitDir);  //without monitoring

  return 0;
}
