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



  // Full Skim Dataset (mc16a)
  // EXOT5 for Znunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408357._000*").scan(sh,inputFilePath); // 364142.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408359._000*").scan(sh,inputFilePath); // 364143.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408360._000*").scan(sh,inputFilePath); // 364144.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408361._000*").scan(sh,inputFilePath); // 364145.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408362._000*").scan(sh,inputFilePath); // 364146.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408364._000*").scan(sh,inputFilePath); // 364147.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408366._000*").scan(sh,inputFilePath); // 364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408369._000*").scan(sh,inputFilePath); // 364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408371._000*").scan(sh,inputFilePath); // 364150.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408372._000*").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408373._000*").scan(sh,inputFilePath); // 364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408375._000*").scan(sh,inputFilePath); // 364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408376._000*").scan(sh,inputFilePath); // 364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408378._000*").scan(sh,inputFilePath); // 364155.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Zee
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408311._000*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408314._000*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408315._000*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408316._000*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408320._000*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408322._000*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408323._000*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408324._000*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408325._000*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408326._000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408331._000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408334._000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408335._000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408336._000*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // EXOT5 for Zmumu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408253._000*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408254._000*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408257._000*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408261._000*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408264._000*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408268._000*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408271._000*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408274._000*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408276._000*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("user.hson.14408279._000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408284._000*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408288._000*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408292._000*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408293._000*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS
  // EXOT5 for Ztautau
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408385._000*").scan(sh,inputFilePath); // 364128.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408387._000*").scan(sh,inputFilePath); // 364129.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408389._000*").scan(sh,inputFilePath); // 364130.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408391._000*").scan(sh,inputFilePath); // 364131.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408395._000*").scan(sh,inputFilePath); // 364132.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408401._000*").scan(sh,inputFilePath); // 364133.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408405._000*").scan(sh,inputFilePath); // 364134.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408408._000*").scan(sh,inputFilePath); // 364135.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408409._000*").scan(sh,inputFilePath); // 364136.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408411._000*").scan(sh,inputFilePath); // 364137.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408414._000*").scan(sh,inputFilePath); // 364138.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408417._000*").scan(sh,inputFilePath); // 364139.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408418._000*").scan(sh,inputFilePath); // 364140.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408422._000*").scan(sh,inputFilePath); // 364141.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV1000_E_CMS
  // EXOT5 for Wenu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408545._000*").scan(sh,inputFilePath); // 364170.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408548._000*").scan(sh,inputFilePath); // 364171.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408550._000*").scan(sh,inputFilePath); // 364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408554._000*").scan(sh,inputFilePath); // 364173.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408557._000*").scan(sh,inputFilePath); // 364174.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408563._000*").scan(sh,inputFilePath); // 364175.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408564._000*").scan(sh,inputFilePath); // 364176.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408566._000*").scan(sh,inputFilePath); // 364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408569._000*").scan(sh,inputFilePath); // 364178.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408572._000*").scan(sh,inputFilePath); // 364179.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408575._000*").scan(sh,inputFilePath); // 364180.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408577._000*").scan(sh,inputFilePath); // 364181.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408580._000*").scan(sh,inputFilePath); // 364182.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408583._000*").scan(sh,inputFilePath); // 364183.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV1000_E_CMS
  // EXOT5 for Wmunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408461._000*").scan(sh,inputFilePath); // 364156.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408464._000*").scan(sh,inputFilePath); // 364157.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408468._000*").scan(sh,inputFilePath); // 364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408472._000*").scan(sh,inputFilePath); // 364159.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408477._000*").scan(sh,inputFilePath); // 364160.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408484._000*").scan(sh,inputFilePath); // 364161.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14408486._000*").scan(sh,inputFilePath); // 364162.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408491._000*").scan(sh,inputFilePath); // 364163.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14482395._000*").scan(sh,inputFilePath); // 364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408499._000*").scan(sh,inputFilePath); // 364165.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408503._000*").scan(sh,inputFilePath); // 364166.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408508._000*").scan(sh,inputFilePath); // 364167.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14408511._000*").scan(sh,inputFilePath); // 364168.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408513._000*").scan(sh,inputFilePath); // 364169.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Wtaunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408605._000*").scan(sh,inputFilePath); // 364184.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408608._000*").scan(sh,inputFilePath); // 364185.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408611._000*").scan(sh,inputFilePath); // 364186.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14408613._000*").scan(sh,inputFilePath); // 364187.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408615._000*").scan(sh,inputFilePath); // 364188.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408620._000*").scan(sh,inputFilePath); // 364189.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14418609._000*").scan(sh,inputFilePath); // 364190.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408625._000*").scan(sh,inputFilePath); // 364191.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408627._000*").scan(sh,inputFilePath); // 364192.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14408631._000*").scan(sh,inputFilePath); // 364193.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14408635._000*").scan(sh,inputFilePath); // 364194.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14408637._000*").scan(sh,inputFilePath); // 364195.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14418612._000*").scan(sh,inputFilePath); // 364196.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14408641._000*").scan(sh,inputFilePath); // 364197.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Top
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408680._0000*").scan(sh,inputFilePath); // 410501_0 (ttbar) 410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad (seperate into 5 batch runs)
//  SH::ScanDir().filePattern("user.hson.14408680._0001*").scan(sh,inputFilePath); // 410501_1 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14408680._0002*").scan(sh,inputFilePath); // 410501_2 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14408680._0003*").scan(sh,inputFilePath); // 410501_3 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14408680._0004*").scan(sh,inputFilePath); // 410501_4 (ttbar)
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_5 (ttbar) // No longer use
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_6 (ttbar) // No longer use 
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_7 (ttbar) // No longer use
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_8 (ttbar) // No longer use
//  SH::ScanDir().filePattern("user.hson.14408682._000*").scan(sh,inputFilePath); // 410011.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_top
//  SH::ScanDir().filePattern("user.hson.14408686._000*").scan(sh,inputFilePath); // 410012.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_antitop
//  SH::ScanDir().filePattern("user.hson.14408689._000*").scan(sh,inputFilePath); // 410013.PowhegPythiaEvtGen_P2012_Wt_inclusive_top
//  SH::ScanDir().filePattern("user.hson.14408692._000*").scan(sh,inputFilePath); // 410014.PowhegPythiaEvtGen_P2012_Wt_inclusive_antitop
//  SH::ScanDir().filePattern("user.hson.14408695._000*").scan(sh,inputFilePath); // 410025.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_top
//  SH::ScanDir().filePattern("user.hson.14408697._000*").scan(sh,inputFilePath); // 410026.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_antitop
  // EXOT5 for Diboson
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408664._000*").scan(sh,inputFilePath); // 363494.Sherpa_221_NNPDF30NNLO_vvvv
//  SH::ScanDir().filePattern("user.hson.14408654._000*").scan(sh,inputFilePath); // 364250.Sherpa_222_NNPDF30NNLO_llll
//  SH::ScanDir().filePattern("user.hson.14408658._000*").scan(sh,inputFilePath); // 364253.Sherpa_222_NNPDF30NNLO_lllv
//  SH::ScanDir().filePattern("user.hson.14408660._000*").scan(sh,inputFilePath); // 364254.Sherpa_222_NNPDF30NNLO_llvv
//  SH::ScanDir().filePattern("user.hson.14408661._000*").scan(sh,inputFilePath); // 364255.Sherpa_222_NNPDF30NNLO_lvvv
//  SH::ScanDir().filePattern("user.hson.14408666._000*").scan(sh,inputFilePath); // 363355.Sherpa_221_NNPDF30NNLO_ZqqZvv
//  SH::ScanDir().filePattern("user.hson.14408667._000*").scan(sh,inputFilePath); // 363356.Sherpa_221_NNPDF30NNLO_ZqqZll
//  SH::ScanDir().filePattern("user.hson.14418616._000*").scan(sh,inputFilePath); // 363357.Sherpa_221_NNPDF30NNLO_WqqZvv
//  SH::ScanDir().filePattern("user.hson.14418617._000*").scan(sh,inputFilePath); // 363358.Sherpa_221_NNPDF30NNLO_WqqZll
//  SH::ScanDir().filePattern("user.hson.14408670._000*").scan(sh,inputFilePath); // 363359.Sherpa_221_NNPDF30NNLO_WpqqWmlv
//  SH::ScanDir().filePattern("user.hson.14408673._000*").scan(sh,inputFilePath); // 363360.Sherpa_221_NNPDF30NNLO_WplvWmqq
//  SH::ScanDir().filePattern("user.hson.14408674._000*").scan(sh,inputFilePath); // 363489.Sherpa_221_NNPDF30NNLO_WlvZqq
  // EXOT5 for Multijet
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14408749._000*").scan(sh,inputFilePath); // 361020.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ0W
//  SH::ScanDir().filePattern("user.hson.14408752._000*").scan(sh,inputFilePath); // 361021.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ1W
//  SH::ScanDir().filePattern("user.hson.14408755._000*").scan(sh,inputFilePath); // 361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W
//  SH::ScanDir().filePattern("user.hson.14408758._000*").scan(sh,inputFilePath); // 361023.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ3W
//  SH::ScanDir().filePattern("user.hson.14408760._000*").scan(sh,inputFilePath); // 361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W
//  SH::ScanDir().filePattern("user.hson.14418620._000*").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
//  SH::ScanDir().filePattern("user.hson.14408768._000*").scan(sh,inputFilePath); // 361026.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ6W
//  SH::ScanDir().filePattern("user.hson.14408771._000*").scan(sh,inputFilePath); // 361027.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ7W
//  SH::ScanDir().filePattern("user.hson.14408773._000*").scan(sh,inputFilePath); // 361028.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ8W
//  SH::ScanDir().filePattern("user.hson.14408776._000*").scan(sh,inputFilePath); // 361029.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ9W
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 361030.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ10W
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 361031.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ11W
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 361032.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ12W

  // STDM4 for Zee (skim tag p3371)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim");
//  SH::ScanDir().filePattern("user.hson.14247353._000*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247354._000*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247355._000*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14247356._000*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247357._000*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247358._000*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14247359._000*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247360._000*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247361._000*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14247362._000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247363._000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247364._000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14247365._000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14247366._000*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // STDM4 for Zmumu (skim tag p3371)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim");
//  SH::ScanDir().filePattern("user.hson.14247320._000*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247321._000*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247323._000*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14247324._000*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247325._000*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247329._000*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14247330._000*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247332._000*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247335._000*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("user.hson.14247336._000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14247337._000*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14247338._000*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14247339._000*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14247341._000*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS

// STDM4 for Zee (no-skim tag p3523)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14709775._000*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709756._000*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709770._000*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14709761._000*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709786._000*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709772._000*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14709767._000*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709783._000*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709771._000*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14709787._000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709776._000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709782._000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14709764._000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14709779._000*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // STDM4 for Zmumu (no-skim tag p3523)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.14709683._000*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709673._000*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709668._000*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14709677._000*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709708._000*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709689._000*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14709678._000*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709698._000*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709684._000*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("user.hson.14709707._000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14709702._000*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14709695._000*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14709671._000*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14709691._000*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS


  // Full Skim Dataset (Data15)
  // Period A ~ G (run 276262 ~ 281075) : 313 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data15/Data15_Period_AtoG");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
  // Period H ~ J (run 281317 ~ 284484) : 328 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data15/Data15_Period_HtoJ");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 


  // Full Skim Dataset (Data16)
  // Period A ~ C (run 297730 ~ 302393) : 1030 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_AtoC/1");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_AtoC/2");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_AtoC/3");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_AtoC/4");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
  // Period D ~ G (run 302737 ~ 306451) : 1731 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/1");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/2");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/3");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/4");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/5");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/6");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_DtoG/7");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
  // Period I ~ L (run 307126 ~ 311481) : 1569 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/1");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/2");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/3");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/4");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/5");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data16/Data16_Period_ItoL/6");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 



  // MadGraph + Pythia8
  // EXOT5 for Znunu (My EXOT5 derivation tag, no my skim)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.15343820.EXT0._000*").scan(sh,inputFilePath); // 361515.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np0
//  SH::ScanDir().filePattern("user.hson.15343822.EXT0._000*").scan(sh,inputFilePath); // 361516.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np1
//  SH::ScanDir().filePattern("user.hson.15343838.EXT0._000*").scan(sh,inputFilePath); // 361517.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np2
//  SH::ScanDir().filePattern("user.hson.15343860.EXT0._000*").scan(sh,inputFilePath); // 361518.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np3
//  SH::ScanDir().filePattern("user.hson.15343888.EXT0._000*").scan(sh,inputFilePath); // 361519.MadGraphPythia8EvtGen_A14NNPDF23LO_Znunu_Np4
  // STDM4 for Zee (tag p3517, my skim)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.15419045._000*").scan(sh,inputFilePath); // 363147.MGPy8EG_N30NLO_Zee_Ht0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419047._000*").scan(sh,inputFilePath); // 363148.MGPy8EG_N30NLO_Zee_Ht0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419051._000*").scan(sh,inputFilePath); // 363149.MGPy8EG_N30NLO_Zee_Ht0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.15419054._000*").scan(sh,inputFilePath); // 363150.MGPy8EG_N30NLO_Zee_Ht70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419058._000*").scan(sh,inputFilePath); // 363151.MGPy8EG_N30NLO_Zee_Ht70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419060._000*").scan(sh,inputFilePath); // 363152.MGPy8EG_N30NLO_Zee_Ht70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.15419064._000*").scan(sh,inputFilePath); // 363153.MGPy8EG_N30NLO_Zee_Ht140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419067._000*").scan(sh,inputFilePath); // 363154.MGPy8EG_N30NLO_Zee_Ht140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419070._000*").scan(sh,inputFilePath); // 363155.MGPy8EG_N30NLO_Zee_Ht140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.15419073._000*").scan(sh,inputFilePath); // 363156.MGPy8EG_N30NLO_Zee_Ht280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419077._000*").scan(sh,inputFilePath); // 363157.MGPy8EG_N30NLO_Zee_Ht280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419080._000*").scan(sh,inputFilePath); // 363158.MGPy8EG_N30NLO_Zee_Ht280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.15419082._000*").scan(sh,inputFilePath); // 363159.MGPy8EG_N30NLO_Zee_Ht500_700_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419086._000*").scan(sh,inputFilePath); // 363160.MGPy8EG_N30NLO_Zee_Ht500_700_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419089._000*").scan(sh,inputFilePath); // 363161.MGPy8EG_N30NLO_Zee_Ht500_700_BFilter
//  SH::ScanDir().filePattern("user.hson.15419092._000*").scan(sh,inputFilePath); // 363162.MGPy8EG_N30NLO_Zee_Ht700_1000_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419094._000*").scan(sh,inputFilePath); // 363163.MGPy8EG_N30NLO_Zee_Ht700_1000_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419098._000*").scan(sh,inputFilePath); // 363164.MGPy8EG_N30NLO_Zee_Ht700_1000_BFilter
//  SH::ScanDir().filePattern("user.hson.15419101._000*").scan(sh,inputFilePath); // 363165.MGPy8EG_N30NLO_Zee_Ht1000_2000_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419104._000*").scan(sh,inputFilePath); // 363166.MGPy8EG_N30NLO_Zee_Ht1000_2000_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419107._000*").scan(sh,inputFilePath); // 363167.MGPy8EG_N30NLO_Zee_Ht1000_2000_BFilter
//  SH::ScanDir().filePattern("user.hson.15419110._000*").scan(sh,inputFilePath); // 363168.MGPy8EG_N30NLO_Zee_Ht2000_E_CMS_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419114._000*").scan(sh,inputFilePath); // 363169.MGPy8EG_N30NLO_Zee_Ht2000_E_CMS_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419117._000*").scan(sh,inputFilePath); // 363170.MGPy8EG_N30NLO_Zee_Ht2000_E_CMS_BFilter
  // STDM4 for Zmumu (tag p3517, my skim)
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v3");
//  SH::ScanDir().filePattern("user.hson.15419167._000*").scan(sh,inputFilePath); // 363123.MGPy8EG_N30NLO_Zmumu_Ht0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419170._000*").scan(sh,inputFilePath); // 363124.MGPy8EG_N30NLO_Zmumu_Ht0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419175._000*").scan(sh,inputFilePath); // 363125.MGPy8EG_N30NLO_Zmumu_Ht0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.15419177._000*").scan(sh,inputFilePath); // 363126.MGPy8EG_N30NLO_Zmumu_Ht70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419179._000*").scan(sh,inputFilePath); // 363127.MGPy8EG_N30NLO_Zmumu_Ht70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419181._000*").scan(sh,inputFilePath); // 363128.MGPy8EG_N30NLO_Zmumu_Ht70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.15419183._000*").scan(sh,inputFilePath); // 363129.MGPy8EG_N30NLO_Zmumu_Ht140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419187._000*").scan(sh,inputFilePath); // 363130.MGPy8EG_N30NLO_Zmumu_Ht140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419191._000*").scan(sh,inputFilePath); // 363131.MGPy8EG_N30NLO_Zmumu_Ht140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.15419195._000*").scan(sh,inputFilePath); // 363132.MGPy8EG_N30NLO_Zmumu_Ht280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419204._000*").scan(sh,inputFilePath); // 363133.MGPy8EG_N30NLO_Zmumu_Ht280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419206._000*").scan(sh,inputFilePath); // 363134.MGPy8EG_N30NLO_Zmumu_Ht280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.15419209._000*").scan(sh,inputFilePath); // 363135.MGPy8EG_N30NLO_Zmumu_Ht500_700_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419212._000*").scan(sh,inputFilePath); // 363136.MGPy8EG_N30NLO_Zmumu_Ht500_700_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419215._000*").scan(sh,inputFilePath); // 363137.MGPy8EG_N30NLO_Zmumu_Ht500_700_BFilter
//  SH::ScanDir().filePattern("user.hson.15419219._000*").scan(sh,inputFilePath); // 363138.MGPy8EG_N30NLO_Zmumu_Ht700_1000_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419221._000*").scan(sh,inputFilePath); // 363139.MGPy8EG_N30NLO_Zmumu_Ht700_1000_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419223._000*").scan(sh,inputFilePath); // 363140.MGPy8EG_N30NLO_Zmumu_Ht700_1000_BFilter
//  SH::ScanDir().filePattern("user.hson.15419225._000*").scan(sh,inputFilePath); // 363141.MGPy8EG_N30NLO_Zmumu_Ht1000_2000_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419227._000*").scan(sh,inputFilePath); // 363142.MGPy8EG_N30NLO_Zmumu_Ht1000_2000_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419229._000*").scan(sh,inputFilePath); // 363143.MGPy8EG_N30NLO_Zmumu_Ht1000_2000_BFilter
//  SH::ScanDir().filePattern("user.hson.15419232._000*").scan(sh,inputFilePath); // 363144.MGPy8EG_N30NLO_Zmumu_Ht2000_E_CMS_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.15419235._000*").scan(sh,inputFilePath); // 363145.MGPy8EG_N30NLO_Zmumu_Ht2000_E_CMS_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.15419237._000*").scan(sh,inputFilePath); // 363146.MGPy8EG_N30NLO_Zmumu_Ht2000_E_CMS_BFilter


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
  std::string slurmJobName = "reco";
  // No delay
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021 -o stdout.%j -e stderr.%j --mail-type=END --mail-user=Hyungsuk.Son@tufts.edu --job-name="+slurmJobName;
  // Submit batch runs with 50min delay
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --begin=now+3000 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021,alpha029,alpha028,pcomp01,pmem02 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;

  // For Systematic
  std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem-per-cpu=64000 -p batch --time=2-2:00:00 --begin=now+2400 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021,alpha029,alpha028,alpha027,pcomp01,pmem02,pcomp07 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;
  // For Non-systematic
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem-per-cpu=32000 -p batch --time=2-2:00:00 --begin=now+2400 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021,alpha029,alpha028,pcomp01,pmem02,pcomp07 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem-per-cpu=32000 -p batch --time=2-2:00:00 --begin=now+2400 --exclude=m4lmem01,alpha027 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;

  // For no-systematics using ALTAS QOS (--account=atlas --qos=atlas)
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem-per-cpu=32000 -p batch --time=2-2:00:00 --account=atlas --qos=atlas --begin=now+2400 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021,alpha029,alpha028,pcomp01,pmem02 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;

  // Submit batch runs with 1hour delay
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --begin=now+1hour --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;
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
