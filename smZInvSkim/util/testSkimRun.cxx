#include "xAODRootAccess/Init.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ScanDir.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "EventLoop/Job.h"
#include "EventLoop/DirectDriver.h"
#include "SampleHandler/DiskListLocal.h"
#include <TSystem.h>
#include "SampleHandler/ScanDir.h"
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
  //const char* inputFilePath = gSystem->ExpandPathName ("$ALRB_TutorialData/r6630/");
  //SH::ScanDir().sampleDepth(1).samplePattern("AOD.05352803._000031.pool.root.1").scan(sh, inputFilePath);

  // Data
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/DATA");
  //SH::ScanDir().filePattern("DAOD_EXOT5.07502101._000026.pool.root.1").scan(sh,inputFilePath); // Data
  //SH::ScanDir().filePattern("DAOD_EXOT5.07502101*").scan(sh,inputFilePath); // Run 284154

  // MC
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992543._000007.pool.root.1").scan(sh,inputFilePath); // MC Znunu
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992459._000012.pool.root.1").scan(sh,inputFilePath); // MC Zmumu
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992561._000006.pool.root.1").scan(sh,inputFilePath); // MC Zee
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992407._000001.pool.root.1").scan(sh,inputFilePath); // MC Pile-up reweighting tool problem sample
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992770._000001.pool.root.1").scan(sh,inputFilePath); // MC Zmumu
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992785._000001.pool.root.1").scan(sh,inputFilePath); // MC Zee
  //SH::ScanDir().filePattern("DAOD_EXOT5.07311123._000033.pool.root.1").scan(sh,inputFilePath); // MC Multijet sample
  //SH::ScanDir().filePattern("DAOD_EXOT5.07311213._000003.pool.root.1").scan(sh,inputFilePath); // MC Diboson sample
  //SH::ScanDir().filePattern("DAOD_EXOT5.07310484._000006.pool.root.1").scan(sh,inputFilePath); // MC ttbar (nonallhad) sample
  //SH::ScanDir().filePattern("DAOD_EXOT5.08040643._000110.pool.root.1").scan(sh,inputFilePath); // MC ttbar (allhad) sample
  //SH::ScanDir().filePattern("DAOD_EXOT5.09453013._000008.pool.root.1").scan(sh,inputFilePath); // MC MGPy8EG_N30NLO_Zmumu
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000019.pool.root.1").scan(sh,inputFilePath); // MC15c Zmumu (after the EventShape bugfix)
  //SH::ScanDir().filePattern("DAOD_EXOT5.08599500._000002.pool.root.1").scan(sh,inputFilePath); // MC15c Zmumu
  //SH::ScanDir().filePattern("DAOD_EXOT5.07992821._00000*").scan(sh,inputFilePath);


  // Data15
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/derivation");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10298531._000*").scan(sh,inputFilePath); // 284484 (random 10 files)
  //SH::ScanDir().filePattern("DAOD_EXOT5.13612879._0000*").scan(sh,inputFilePath); // 276073 (Test:including 0 event file)

  // MC15c (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531595._000002.pool.root.1").scan(sh,inputFilePath); // Znunu_MAXHTPTV280_500_CFilterBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531581._000001.pool.root.1").scan(sh,inputFilePath); // Zee_MAXHTPTV140_280_BFilter
  //SH::ScanDir().filePattern("DAOD_EXOT5.09043234._000006.pool.root.1").scan(sh,inputFilePath); // 410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad
  //SH::ScanDir().filePattern("DAOD_EXOT5.08602669._000018.pool.root.1").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.pool.root.1").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto

  // MC15c (STDM4)
  //SH::ScanDir().filePattern("DAOD_STDM4.10349557._000034.pool.root.1").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.10348687._000027.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto

  // Rel. 21
  // MC16c (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/MC");
  //SH::ScanDir().filePattern("DAOD_EXOT5.11892347._000020.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.11869241._000002.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  // MC16c (STDM4)
  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/derivation");
  //SH::ScanDir().filePattern("DAOD_STDM4.12772535._000010.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3371, skim)
  SH::ScanDir().filePattern("DAOD_STDM4.13860139._000007.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3523, no-skim)
  //SH::ScanDir().filePattern("DAOD_EXOT5.13463081._000027.pool.root.1").scan(sh,inputFilePath); // Multijet
  //SH::ScanDir().filePattern("DAOD_EXOT5.13463907._000007.pool.root.1").scan(sh,inputFilePath); // Zmumu
  //SH::ScanDir().filePattern("DAOD_EXOT5.13461333._000007.pool.root.1").scan(sh,inputFilePath); // Zee
  //SH::ScanDir().filePattern("DAOD_EXOT5.13472375._000019.pool.root.1").scan(sh,inputFilePath); // Znunu


  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sh.setMetaString( "nc_tree", "CollectionTree" );

  // Print what we found:
  sh.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler( sh );
  // make sure we can read trigger decision
  job.options()->setString(EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_class);
  //job.options()->setDouble (EL::Job::optMaxEvents, 1000); // for testing
/*
  // For ntuple
  // define an output and an ntuple associated to that output (For ntuple)
  EL::OutputStream output  ("myOutput");
  job.outputAdd (output);
  EL::NTupleSvc *ntuple = new EL::NTupleSvc ("myOutput");
  job.algsAdd (ntuple);
  //---------------------------------------------------------------------
*/
  // Add our analysis to the job:
  smZInvSkim* alg = new smZInvSkim();
  job.algsAdd( alg );
/*
  // For ntuple
  // Let your algorithm know the name of the output file (For ntuple)
  alg->outputName = "myOutput"; // give the name of the output to our algorithm
  //----------------------------------------------------------------------------
*/
  // Run the job using the local/direct driver:
  EL::DirectDriver driver;
  driver.submit( job, submitDir );

  return 0;
}
