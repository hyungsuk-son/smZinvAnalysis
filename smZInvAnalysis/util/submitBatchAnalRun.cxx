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



  // Full Skim Dataset (MC15c)
  // EXOT5 for Znunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas10/hson02/Dataset/MC/skim_v3");
//  SH::ScanDir().filePattern("user.hson.13284772._000*").scan(sh,inputFilePath); // 364142.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284776._000*").scan(sh,inputFilePath); // 364143.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284779._000*").scan(sh,inputFilePath); // 364144.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13284781._000*").scan(sh,inputFilePath); // 364145.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284783._000*").scan(sh,inputFilePath); // 364146.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284787._000*").scan(sh,inputFilePath); // 364147.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13284792._000*").scan(sh,inputFilePath); // 364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284795._000*").scan(sh,inputFilePath); // 364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284799._000*").scan(sh,inputFilePath); // 364150.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13284801._000*").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284804._000*").scan(sh,inputFilePath); // 364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284808._000*").scan(sh,inputFilePath); // 364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13284809._000*").scan(sh,inputFilePath); // 364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13284812._000*").scan(sh,inputFilePath); // 364155.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV1000_E_CMS
  // STDM4 for Zee
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC/skim_v3");
//  SH::ScanDir().filePattern("user.hson.13285011._000*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285013._000*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285015._000*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13285016._000*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285018._000*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285021._000*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13285022._000*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285024._000*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285025._000*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13285029._000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285030._000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285032._000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285035._000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13285037._000*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // STDM4 for Zmumu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13284975._000*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284977._000*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284979._000*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13284981._000*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284982._000*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13284984._000*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13284987._000*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13284991._000*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("DAOD_STDM4.123008*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("user.hson.13284999._000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285001._000*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285004._000*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285008._000*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13285009._000*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS
  // STDM4 for Ztautau
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas10/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13285039._000*").scan(sh,inputFilePath); // 364128.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285040._000*").scan(sh,inputFilePath); // 364129.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285042._000*").scan(sh,inputFilePath); // 364130.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13285044._000*").scan(sh,inputFilePath); // 364131.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285047._000*").scan(sh,inputFilePath); // 364132.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285048._000*").scan(sh,inputFilePath); // 364133.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13285051._000*").scan(sh,inputFilePath); // 364134.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285052._000*").scan(sh,inputFilePath); // 364135.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285054._000*").scan(sh,inputFilePath); // 364136.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13285056._000*").scan(sh,inputFilePath); // 364137.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285058._000*").scan(sh,inputFilePath); // 364138.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285061._000*").scan(sh,inputFilePath); // 364139.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285063._000*").scan(sh,inputFilePath); // 364140.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("DAOD_STDM4.12304500._000*").scan(sh,inputFilePath); // 364141.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV1000_E_CMS
// STDM4 for Wenu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13285091._000*").scan(sh,inputFilePath); // 364170.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285093._000*").scan(sh,inputFilePath); // 364171.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285095._000*").scan(sh,inputFilePath); // 364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13285096._000*").scan(sh,inputFilePath); // 364173.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285098._000*").scan(sh,inputFilePath); // 364174.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285099._000*").scan(sh,inputFilePath); // 364175.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13285101._000*").scan(sh,inputFilePath); // 364176.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285103._000*").scan(sh,inputFilePath); // 364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285105._000*").scan(sh,inputFilePath); // 364178.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13285107._000*").scan(sh,inputFilePath); // 364179.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285109._000*").scan(sh,inputFilePath); // 364180.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285112._000*").scan(sh,inputFilePath); // 364181.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285113._000*").scan(sh,inputFilePath); // 364182.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13285116._000*").scan(sh,inputFilePath); // 364183.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV1000_E_CMS
  // STDM4 for Wmunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas09/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13285066._000*").scan(sh,inputFilePath); // 364156.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285067._000*").scan(sh,inputFilePath); // 364157.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285069._000*").scan(sh,inputFilePath); // 364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13285071._000*").scan(sh,inputFilePath); // 364159.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285073._000*").scan(sh,inputFilePath); // 364160.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285076._000*").scan(sh,inputFilePath); // 364161.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13285077._000*").scan(sh,inputFilePath); // 364162.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285079._000*").scan(sh,inputFilePath); // 364163.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285080._000*").scan(sh,inputFilePath); // 364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13285082._000*").scan(sh,inputFilePath); // 364165.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285083._000*").scan(sh,inputFilePath); // 364166.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285085._000*").scan(sh,inputFilePath); // 364167.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285086._000*").scan(sh,inputFilePath); // 364168.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13285089._000*").scan(sh,inputFilePath); // 364169.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV1000_E_CMS
  // STDM4 for Wtaunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas10/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13285119._000*").scan(sh,inputFilePath); // 364184.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285122._000*").scan(sh,inputFilePath); // 364185.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285125._000*").scan(sh,inputFilePath); // 364186.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.13285128._000*").scan(sh,inputFilePath); // 364187.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285130._000*").scan(sh,inputFilePath); // 364188.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285134._000*").scan(sh,inputFilePath); // 364189.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.13285137._000*").scan(sh,inputFilePath); // 364190.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285140._000*").scan(sh,inputFilePath); // 364191.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285145._000*").scan(sh,inputFilePath); // 364192.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.13285148._000*").scan(sh,inputFilePath); // 364193.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.13285151._000*").scan(sh,inputFilePath); // 364194.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.13285155._000*").scan(sh,inputFilePath); // 364195.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.13285157._000*").scan(sh,inputFilePath); // 364196.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.13285160._000*").scan(sh,inputFilePath); // 364197.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Top
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas12/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13451819._0000*").scan(sh,inputFilePath); // 410501_0 (ttbar) 410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad (seperate into 6 batch runs)
//  SH::ScanDir().filePattern("user.hson.13451819._0001*").scan(sh,inputFilePath); // 410501_1 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13451819._0002*").scan(sh,inputFilePath); // 410501_2 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13451819._0003*").scan(sh,inputFilePath); // 410501_3 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13451819._0004*").scan(sh,inputFilePath); // 410501_4 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13451819._0005*").scan(sh,inputFilePath); // 410501_5 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13451819._0006*").scan(sh,inputFilePath); // 410501_6 (ttbar)
//  SH::ScanDir().filePattern("user.hson.13284846._000*").scan(sh,inputFilePath); // 410011.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_top
//  SH::ScanDir().filePattern("user.hson.13284848._000*").scan(sh,inputFilePath); // 410012.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_antitop
//  SH::ScanDir().filePattern("user.hson.13284849._000*").scan(sh,inputFilePath); // 410013.PowhegPythiaEvtGen_P2012_Wt_inclusive_top
//  SH::ScanDir().filePattern("user.hson.13284852._000*").scan(sh,inputFilePath); // 410014.PowhegPythiaEvtGen_P2012_Wt_inclusive_antitop
//  SH::ScanDir().filePattern("user.hson.13284853._000*").scan(sh,inputFilePath); // 410025.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_top
//  SH::ScanDir().filePattern("user.hson.13284856._000*").scan(sh,inputFilePath); // 410026.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_antitop
  // EXOT5 for Diboson
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas12/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13284825._000*").scan(sh,inputFilePath); // 363355.Sherpa_221_NNPDF30NNLO_ZqqZvv
//  SH::ScanDir().filePattern("user.hson.13284826._000*").scan(sh,inputFilePath); // 363356.Sherpa_221_NNPDF30NNLO_ZqqZll
//  SH::ScanDir().filePattern("user.hson.13284828._000*").scan(sh,inputFilePath); // 363357.Sherpa_221_NNPDF30NNLO_WqqZvv
//  SH::ScanDir().filePattern("user.hson.13284829._000*").scan(sh,inputFilePath); // 363358.Sherpa_221_NNPDF30NNLO_WqqZll
//  SH::ScanDir().filePattern("user.hson.13284831._000*").scan(sh,inputFilePath); // 363359.Sherpa_221_NNPDF30NNLO_WpqqWmlv
//  SH::ScanDir().filePattern("user.hson.13284835._000*").scan(sh,inputFilePath); // 363489.Sherpa_221_NNPDF30NNLO_WlvZqq
//  SH::ScanDir().filePattern("user.hson.13284813._000*").scan(sh,inputFilePath); // 363490.Sherpa_221_NNPDF30NNLO_llll
//  SH::ScanDir().filePattern("user.hson.13284816._000*").scan(sh,inputFilePath); // 363491.Sherpa_221_NNPDF30NNLO_lllv
//  SH::ScanDir().filePattern("user.hson.13284818._000*").scan(sh,inputFilePath); // 363492.Sherpa_221_NNPDF30NNLO_llvv
//  SH::ScanDir().filePattern("user.hson.13284821._000*").scan(sh,inputFilePath); // 363493.Sherpa_221_NNPDF30NNLO_lvvv
//  SH::ScanDir().filePattern("user.hson.13284822._000*").scan(sh,inputFilePath); // 363494.Sherpa_221_NNPDF30NNLO_vvvv
  // EXOT5 for Multijet
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas12/hson02/Dataset/MC");
//  SH::ScanDir().filePattern("user.hson.13284858._000*").scan(sh,inputFilePath); // 361020.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ0W
//  SH::ScanDir().filePattern("user.hson.13284861._000*").scan(sh,inputFilePath); // 361021.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ1W
//  SH::ScanDir().filePattern("user.hson.13284864._000*").scan(sh,inputFilePath); // 361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W
//  SH::ScanDir().filePattern("user.hson.13284868._000*").scan(sh,inputFilePath); // 361023.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ3W
//  SH::ScanDir().filePattern("user.hson.13284872._000*").scan(sh,inputFilePath); // 361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W
//  SH::ScanDir().filePattern("user.hson.13284876._000*").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
//  SH::ScanDir().filePattern("user.hson.13284878._000*").scan(sh,inputFilePath); // 361026.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ6W
//  SH::ScanDir().filePattern("user.hson.13284882._000*").scan(sh,inputFilePath); // 361027.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ7W
//  SH::ScanDir().filePattern("user.hson.13284890._000*").scan(sh,inputFilePath); // 361029.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ9W
//  SH::ScanDir().filePattern("user.hson.13284896._000*").scan(sh,inputFilePath); // 361031.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ11W
//  SH::ScanDir().filePattern("DAOD_EXOT5.08602818._000*").scan(sh,inputFilePath); // 361032.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ12W


  // Full Skim Dataset (Data15)
  // Period A ~ G (run 276262 ~ 281075) : 226 files
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas11/hson02/Dataset/Data/skim_v3/Data15_Period_AtoG");
  //SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
  // Period A ~ G (run 281317 ~ 284484) : 218 files
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas11/hson02/Dataset/Data/skim_v3/Data15_Period_HtoJ");
  //SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 

  // Full derivation Dataset for missing skim dataset (run 276416, 280614, 281385 and 284154)
  //const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas11/hson02/Dataset/Data/derivation");
  //SH::ScanDir().filePattern("DAOD_EXOT5.10297922._000*").scan(sh,inputFilePath); // run 276416
  //SH::ScanDir().filePattern("DAOD_EXOT5.10298247._000*").scan(sh,inputFilePath); // run 280614
  //SH::ScanDir().filePattern("DAOD_EXOT5.10298357._000*").scan(sh,inputFilePath); // run 281385
  //SH::ScanDir().filePattern("DAOD_EXOT5.10298467._000*").scan(sh,inputFilePath); // run 284154



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
  std::string slurmJobName = "zee";
  // No delay
  //std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021 -o stdout.%j -e stderr.%j --mail-type=END --mail-user=Hyungsuk.Son@tufts.edu --job-name="+slurmJobName;
  // Submit batch runs with 10min delay
  std::string slurmOptions = "-n 1 --cpus-per-task 1 --mem 32000 -p batch --time=2-2:00:00 --begin=now+2000 --exclude=m4lmem01,alpha018,pcomp18,pmem01,alpha012,pcomp30,pcomp26,omega021,alpha029,alpha028,pcomp01 -o stdout.%j -e stderr.%j --mail-type=END --job-name="+slurmJobName;
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
