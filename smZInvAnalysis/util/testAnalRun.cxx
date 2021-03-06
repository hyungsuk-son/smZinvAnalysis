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
  //const char* inputFilePath = gSystem->ExpandPathName ("$ALRB_TutorialData/r6630/");
  //SH::ScanDir().sampleDepth(1).samplePattern("AOD.05352803._000031.pool.root.1").scan(sh, inputFilePath);

  // Data15
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/DATA");
  //SH::ScanDir().filePattern("DAOD_EXOT5.07502101*").scan(sh,inputFilePath); // Run 284154

  // MC15b
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
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531487._000003.pool.root.1").scan(sh,inputFilePath); // 364137.Ztautau_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531642._000037.pool.root.1").scan(sh,inputFilePath); // 364112.Zmumu_MAXHTPTV500_1000
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.pool.root.1").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto (Cutflow)
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531491._000027.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531563._000050.pool.root.1").scan(sh,inputFilePath); // 364151.Znunu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531491._000034.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto (Cutflow)

  // STDM4
  // MC15c
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  // Sherpa
  //SH::ScanDir().filePattern("DAOD_STDM4.10348491._000001.pool.root.1").scan(sh,inputFilePath);
  //SH::ScanDir().filePattern("DAOD_STDM4.10349557._000034.pool.root.1").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto 
  //SH::ScanDir().filePattern("DAOD_STDM4.10348687._000027.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  // MadGraph
  //SH::ScanDir().filePattern("DAOD_STDM4.12302499._000004.pool.root.1").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto



  // my skimmed sample
  // DATA16
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/DATA/Run2");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10316590._000203.mini-xAOD.root.1").scan(sh,inputFilePath); // Run 311365
  // MC15c (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531595._000002.mini-xAOD.root.1").scan(sh,inputFilePath); // 364152.Znunu_MAXHTPTV280_500_CFilterBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.08602669._000018.mini-xAOD.root.1").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11301992._000021.mini-xAOD.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD_localSkim.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD_new.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531536._000034.mini-xAOD_new_new.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11473386._000027.mini-xAOD.root").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
  // MC15c (STDM4)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/atlas09Home/Work/vbfZinvSkim/testRun_skim_364123.Zee_MAXHTPTV280_500_CVetoBVeto/data-mini-xAOD");
  //SH::ScanDir().filePattern("DAOD_STDM4.10531536._000034.mini-xAOD.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.11487814._000027.mini-xAOD.root").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
  //SH::ScanDir().filePattern("mc15_13TeV.364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952.root").scan(sh,inputFilePath);
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Work/vbfZinvSkim");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/kappa/90-days-archive/atlas/atlas09/hson02/Work/vbfZinvSkim");
  //SH::ScanDir().filePattern("mc15_13TeV.364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952.root").scan(sh,inputFilePath);
  //SH::ScanDir().filePattern("user.hson.12608084._000001.mini-xAOD.root").scan(sh,inputFilePath);

  // Final version
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC/skim");
  //SH::ScanDir().filePattern("user.hson.12609295._000019.mini-xAOD.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.12609334._000018.mini-xAOD.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto

  // Reco skim test
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/atlas09Home/Work/vbfZinvSkim/testRun_02112018_skim_364123.Zee_MAXHTPTV280_500_CVetoBVeto/data-mini-xAOD");
  //SH::ScanDir().filePattern("mc15_13TeV.364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_STDM4.e5299_s2726_r7772_r7676_p2952.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto



  // MC Derivation STDM4 (ATLAS Derivation 20.7.8.24)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC/derivation");
  //SH::ScanDir().filePattern("DAOD_STDM4.12301409._000137.pool.root.1").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12301956._000040.pool.root.1").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12301606._000007.pool.root.1").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12302296._000007.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531563._000104.pool.root.1").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12305071._000032.pool.root.1").scan(sh,inputFilePath); // 364137.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12302951._000172.pool.root.1").scan(sh,inputFilePath); // 364159.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12300562._000113.pool.root.1").scan(sh,inputFilePath); // 364165.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_STDM4.12664549._000011.pool.root.1").scan(sh,inputFilePath); // 427000.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ0W_mufilter
  //SH::ScanDir().filePattern("DAOD_STDM4.12303989._000003.pool.root.1").scan(sh,inputFilePath); // 410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad
  //SH::ScanDir().filePattern("").scan(sh,inputFilePath); // 
  //SH::ScanDir().filePattern("").scan(sh,inputFilePath); // 

  // Data Derivation EXOT5
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/Data/derivation");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10298531._000*").scan(sh,inputFilePath); // 284484 (10 random files)
  // Data skim EXOT5 (test)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Work/vbfZinvSkim/testRun_02242018_skim_data15_run284484_10randomFiles/data-mini-xAOD");
  //SH::ScanDir().filePattern("data15_13TeV.00284484.physics_Main.merge.DAOD_EXOT5.r7562_p2521_p2950.root").scan(sh,inputFilePath); // 284484 (10 random files)

  // MC Derivation EXOT5
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10531491._000026.pool.root.1").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto



  // MC skim_v3
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas10/hson02/Dataset/MC/skim_v3");
  //SH::ScanDir().filePattern("user.hson.13284801._000020.mini-xAOD.root").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC/skim_v3");
  //SH::ScanDir().filePattern("user.hson.13285029._000012.mini-xAOD.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.13284999._000012.mini-xAOD.root").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas12/hson02/Dataset/MC/derivation");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10140246._000042.pool.root.1").scan(sh,inputFilePath); // 410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/MC/derivation");
  //SH::ScanDir().filePattern("DAOD_EXOT5.11163397._000018.pool.root.1").scan(sh,inputFilePath); // 410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad

  // Data skim_v3
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas11/hson02/Dataset/Data/skim_v3");
  //SH::ScanDir().filePattern("user.hson.13303075._000010.mini-xAOD.root").scan(sh,inputFilePath); // run 284484




  // TRUTH1
  // from grid
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas08/MCSamples");
  //SH::ScanDir().filePattern("user.hson.12002993.EXT0._000011.DAOD_TRUTH1.test.pool.root").scan(sh,inputFilePath); // 363123.MGPy8EG_N30NLO_Zmumu_Ht0_70_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.12325022.EXT0._000025.DAOD_TRUTH1.test.pool.root").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto
  //SH::ScanDir().filePattern("user.hson.12327093.EXT0._000011.DAOD_TRUTH1.test.pool.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (1.9GB)
  //SH::ScanDir().filePattern("user.hson.12327093.EXT0._000034.DAOD_TRUTH1.test.pool.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto (0.9GB)
  // from local
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/Work/TRUTH1_truthJet/WorkArea/run");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/Work/test_TRUTH1_truthJet/WorkArea/run");
  //SH::ScanDir().filePattern("DAOD_TRUTH1.test.pool.root").scan(sh,inputFilePath); // 363123.MGPy8EG_N30NLO_Zmumu_Ht0_70_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_TRUTH1.zee.test.pool.root").scan(sh,inputFilePath); // 363147.MGPy8EG_N30NLO_Zee_Ht0_70_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_TRUTH1.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto.inputXAOD.test.pool.root").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_TRUTH1.Sherpa_221_Zee_MAXHTPTV140_280_BFilter.test.pool.root").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
  //SH::ScanDir().filePattern("DAOD_TRUTH1.Sherpa_221_Zee_MAXHTPTV0_70_CVetoBVeto.test.pool.root").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_TRUTH1.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto.test.pool.root").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto


  // Rel. 21
  // MC16a (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/MC");
  //SH::ScanDir().filePattern("DAOD_EXOT5.11892347._000020.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  //SH::ScanDir().filePattern("DAOD_EXOT5.11869241._000002.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto

  // MC16a (STDM4)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/MC");
  //SH::ScanDir().filePattern("DAOD_STDM4.13860139._000006.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3523, no-skim, maybe not available)
  //SH::ScanDir().filePattern("DAOD_STDM4.12772535._000010.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3371, skim)


  // data15
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/Data15");
  //SH::ScanDir().filePattern("DAOD_EXOT5.12599066._000076.pool.root.1").scan(sh,inputFilePath); // data15_13TeV.00284484

  // data16
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/Data16");
  //SH::ScanDir().filePattern("DAOD_EXOT5.12608605._00*").scan(sh,inputFilePath); // data16_13TeV.00297730

  // Skim
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/atlas09Home/Work/smZinvSkim/run/testRun_05032018_test/data-mini-xAOD");
  //SH::ScanDir().filePattern("mc16_13TeV.root").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/atlas09Home/Work/smZinvSkim/run/testRun_05032018_test_zmumu/data-mini-xAOD");
  //SH::ScanDir().filePattern("mc16_13TeV.364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto.deriv.DAOD_EXOT5.e5271_s3126_r9364_r9315_p3238.root").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim");
  //SH::ScanDir().filePattern("user.hson.14171858._000011.mini-xAOD.root").scan(sh,inputFilePath); //  364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3238, skim, EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim");
  //SH::ScanDir().filePattern("user.hson.14247336._000005.mini-xAOD.root").scan(sh,inputFilePath); //  364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3371, skim, STMD4)

  // Skim_v1 (STDM4, skim tag p3371)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim");
  //SH::ScanDir().filePattern("user.hson.14247336._000010.mini-xAOD.root").scan(sh,inputFilePath); //  364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3371, skim, STDM4)

  // Skim_v3 (STDM4, no-skim tag p3523)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim_v3");
  //SH::ScanDir().filePattern("user.hson.14709707._000012.mini-xAOD.root").scan(sh,inputFilePath); //  364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3523, no-skim, STDM4)

  // Skim_v2 (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim_v2");
  //SH::ScanDir().filePattern("user.hson.14316785._000010.mini-xAOD.root").scan(sh,inputFilePath); //  364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto (p3480, skim, EXOT5)
  //SH::ScanDir().filePattern("user.hson.14316903._000016.mini-xAOD.root").scan(sh,inputFilePath); // Wmunu
  //SH::ScanDir().filePattern("user.hson.14316951._000016.mini-xAOD.root").scan(sh,inputFilePath); // Wenu

  // Skim_v3 (EXOT5)
  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim_v3");
  //SH::ScanDir().filePattern("user.hson.14408372._000029.mini-xAOD.root").scan(sh,inputFilePath); // Znunu
  //SH::ScanDir().filePattern("user.hson.14408371._000061.mini-xAOD.root").scan(sh,inputFilePath); // Znunu (364150)
  SH::ScanDir().filePattern("user.hson.14408326._000018.mini-xAOD.root").scan(sh,inputFilePath); // Zee
  //SH::ScanDir().filePattern("user.hson.14408279._000007.mini-xAOD.root").scan(sh,inputFilePath); // Zmumu
  //SH::ScanDir().filePattern("user.hson.14408572._000047.mini-xAOD.root").scan(sh,inputFilePath); // Wenu
  //SH::ScanDir().filePattern("user.hson.14408499._000048.mini-xAOD.root").scan(sh,inputFilePath); // Wmunu
  //SH::ScanDir().filePattern("user.hson.14408680._000444.mini-xAOD.root").scan(sh,inputFilePath); // ttbar
  //SH::ScanDir().filePattern("user.hson.14408682._000006.mini-xAOD.root").scan(sh,inputFilePath); // Single Top
  //SH::ScanDir().filePattern("user.hson.14408666._000006.mini-xAOD.root").scan(sh,inputFilePath); // Diboson (DSID: 363355, ScaledMCWeight Error)
  //SH::ScanDir().filePattern("user.hson.14408667._000006.mini-xAOD.root").scan(sh,inputFilePath); // Diboson
  // For MET trigger SF test
  //SH::ScanDir().filePattern("user.hson.14408271._000018.mini-xAOD.root").scan(sh,inputFilePath); // Zmumu (364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto)
  //SH::ScanDir().filePattern("user.hson.14408486._000054.mini-xAOD.root").scan(sh,inputFilePath); // Wmunu (364162.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CVetoBVeto)
  //SH::ScanDir().filePattern("user.hson.14408366._000091.mini-xAOD.root").scan(sh,inputFilePath); // Znunu (364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto)

  // Skim (STDM4)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/hson02/Dataset/MC/MC16a/skim");
  //SH::ScanDir().filePattern("user.hson.14247362._000009.mini-xAOD.root").scan(sh,inputFilePath); // Zee
  //SH::ScanDir().filePattern("user.hson.14247330._000013.mini-xAOD.root").scan(sh,inputFilePath); // Zmumu


  // data15
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data15/Data15_Period_HtoJ");
  //SH::ScanDir().filePattern("user.hson.14248220._000011.mini-xAOD.root").scan(sh,inputFilePath); // data15_13TeV.00284484
  // data16
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL");
  //SH::ScanDir().filePattern("user.hson.14307750._000032.mini-xAOD.root").scan(sh,inputFilePath); // data16_13TeV.00311481


  // MadGraph + Pythia8 Derivation (STDM4)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/derivation");
  //SH::ScanDir().filePattern("DAOD_STDM4.14405032._000002.pool.root.1").scan(sh,inputFilePath); // Zmumu (363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto) tag: p3552
  //SH::ScanDir().filePattern("DAOD_STDM4.13908568._000005.pool.root.1").scan(sh,inputFilePath); // Zmumu (363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto) tag: p3517
  // Local Skim
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_09172018_MadGraph_Zee_add_TruthVertices/data-mini-xAOD");
  //SH::ScanDir().filePattern("*.root").scan(sh,inputFilePath); // Zmumu (363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto) tag: p3552

  // MadGraph + Pythia8 Derivation
  // my local custom EXOT5
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/custom/Deriv_test");
  //SH::ScanDir().filePattern("DAOD_EXOT5.test.Np1.pool.root").scan(sh,inputFilePath); // Znunu (361516.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np1)
  // my grid test EXOT5
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/custom/user.hson.361515.MadGraphPythia8EvtGen.EXOT5.test_EXT0");
  //SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // Znunu (361516.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np1)

  // MadGraph + Pythia8 Derivation (STDM4, my skim)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3/STDM4/MadGraph/Zee");
  //SH::ScanDir().filePattern("user.hson.15419073._000006.mini-xAOD.root").scan(sh,inputFilePath); // Zee (363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto) tag: p3517

  // MadGraph + Pythia8 Derivation (STDM4, derivation)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3/STDM4/MadGraph");
  //SH::ScanDir().filePattern("DAOD_STDM4.14405034._000013.pool.root.1").scan(sh,inputFilePath); // Zee (363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto) tag: e4649_s3126_r10201_p3552




  // Custom Derivation for Multijet background study (EXOT5)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/derivation/custom_EXOT5");
  //SH::ScanDir().filePattern("user.fkaya.15439235.EXT0._000*").scan(sh,inputFilePath); // run 284484


  // Skim test
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_06142018_newTag/data-mini-xAOD");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_06142018_newTag_Zmumu/data-mini-xAOD");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_06142018_newTag_Zee/data-mini-xAOD");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_06142018_newTag_Znunu/data-mini-xAOD");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Work/gitLab/run/testSkimRun_07232018_no-skim-Tag_Zmumu/data-mini-xAOD");
  //SH::ScanDir().filePattern("*.root").scan(sh,inputFilePath);

  // Cutflow test
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/derivation");
  // Znunu (EXOT5)
  //SH::ScanDir().filePattern("DAOD_EXOT5.13472375._000010.pool.root.1").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
  // Zmumu (STDM4, noskim)
  //SH::ScanDir().filePattern("DAOD_STDM4.13860139._000007.pool.root.1").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto

  // Error test
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3/EXOT5/Wmunu");
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_test");
  //SH::ScanDir().filePattern("user.hson.14408495._000019.mini-xAOD.root").scan(sh,inputFilePath); // Wmunu


  // Colette Derivation
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/beaucheminlab/fkaya01/Data15/DAOD_EXOT5_bQCD/c1_user.fkaya.data15_13TeV.00284484.physics_Main.deriv.DAOD_EXOT5.v_egamma_EXT0");
  //SH::ScanDir().filePattern("user.fkaya.15580410.EXT0._000028.DAOD_EXOT5.pool.root").scan(sh,inputFilePath); // Zmumu
  

  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sh.setMetaString( "nc_tree", "CollectionTree" );

  // Print what we found:
  sh.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler( sh );
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
  smZInvAnalysis* alg = new smZInvAnalysis();
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
