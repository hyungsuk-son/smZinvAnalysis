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
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316823._000*").scan(sh,inputFilePath); // 364142.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316824._000*").scan(sh,inputFilePath); // 364143.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316826._000*").scan(sh,inputFilePath); // 364144.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316827._000*").scan(sh,inputFilePath); // 364145.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316828._000*").scan(sh,inputFilePath); // 364146.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316830._000*").scan(sh,inputFilePath); // 364147.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316832._000*").scan(sh,inputFilePath); // 364148.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316835._000*").scan(sh,inputFilePath); // 364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316836._000*").scan(sh,inputFilePath); // 364150.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316837._000*").scan(sh,inputFilePath); // 364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316840._000*").scan(sh,inputFilePath); // 364152.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316841._000*").scan(sh,inputFilePath); // 364153.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316842._000*").scan(sh,inputFilePath); // 364154.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316843._000*").scan(sh,inputFilePath); // 364155.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Zee
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316795._000*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316796._000*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316797._000*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316799._000*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316800._000*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316803._000*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316804._000*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316805._000*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316807._000*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316808._000*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316810._000*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316811._000*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316813._000*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316815._000*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // EXOT5 for Zmumu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316772._000*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316774._000*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316775._000*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316777._000*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316779._000*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316780._000*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316781._000*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316782._000*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316784._000*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("user.hson.14316785._000*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316786._000*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316787._000*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316788._000*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316789._000*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS
  // EXOT5 for Ztautau
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316855._000*").scan(sh,inputFilePath); // 364128.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316857._000*").scan(sh,inputFilePath); // 364129.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316860._000*").scan(sh,inputFilePath); // 364130.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316861._000*").scan(sh,inputFilePath); // 364131.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316862._000*").scan(sh,inputFilePath); // 364132.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316864._000*").scan(sh,inputFilePath); // 364133.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316866._000*").scan(sh,inputFilePath); // 364134.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316867._000*").scan(sh,inputFilePath); // 364135.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316869._000*").scan(sh,inputFilePath); // 364136.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316870._000*").scan(sh,inputFilePath); // 364137.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316872._000*").scan(sh,inputFilePath); // 364138.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316874._000*").scan(sh,inputFilePath); // 364139.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316875._000*").scan(sh,inputFilePath); // 364140.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316876._000*").scan(sh,inputFilePath); // 364141.Sherpa_221_NNPDF30NNLO_Ztautau_MAXHTPTV1000_E_CMS
  // EXOT5 for Wenu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14343032._000*").scan(sh,inputFilePath); // 364170.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316932._000*").scan(sh,inputFilePath); // 364171.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316934._000*").scan(sh,inputFilePath); // 364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316937._000*").scan(sh,inputFilePath); // 364173.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316940._000*").scan(sh,inputFilePath); // 364174.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316943._000*").scan(sh,inputFilePath); // 364175.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316944._000*").scan(sh,inputFilePath); // 364176.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14343033._000*").scan(sh,inputFilePath); // 364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14343035._000*").scan(sh,inputFilePath); // 364178.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316951._000*").scan(sh,inputFilePath); // 364179.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316953._000*").scan(sh,inputFilePath); // 364180.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316957._000*").scan(sh,inputFilePath); // 364181.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316959._000*").scan(sh,inputFilePath); // 364182.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316962._000*").scan(sh,inputFilePath); // 364183.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV1000_E_CMS
  // EXOT5 for Wmunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316884._000*").scan(sh,inputFilePath); // 364156.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316886._000*").scan(sh,inputFilePath); // 364157.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316887._000*").scan(sh,inputFilePath); // 364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316891._000*").scan(sh,inputFilePath); // 364159.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316893._000*").scan(sh,inputFilePath); // 364160.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316897._000*").scan(sh,inputFilePath); // 364161.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316899._000*").scan(sh,inputFilePath); // 364162.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316900._000*").scan(sh,inputFilePath); // 364163.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316902._000*").scan(sh,inputFilePath); // 364164.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316903._000*").scan(sh,inputFilePath); // 364165.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316905._000*").scan(sh,inputFilePath); // 364166.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316907._000*").scan(sh,inputFilePath); // 364167.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316909._000*").scan(sh,inputFilePath); // 364168.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316912._000*").scan(sh,inputFilePath); // 364169.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Wtaunu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14316973._000*").scan(sh,inputFilePath); // 364184.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316974._000*").scan(sh,inputFilePath); // 364185.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316976._000*").scan(sh,inputFilePath); // 364186.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("user.hson.14316977._000*").scan(sh,inputFilePath); // 364187.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316979._000*").scan(sh,inputFilePath); // 364188.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316981._000*").scan(sh,inputFilePath); // 364189.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("user.hson.14316983._000*").scan(sh,inputFilePath); // 364190.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316984._000*").scan(sh,inputFilePath); // 364191.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316986._000*").scan(sh,inputFilePath); // 364192.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("user.hson.14316987._000*").scan(sh,inputFilePath); // 364193.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("user.hson.14316989._000*").scan(sh,inputFilePath); // 364194.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("user.hson.14316991._000*").scan(sh,inputFilePath); // 364195.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("user.hson.14316993._000*").scan(sh,inputFilePath); // 364196.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("user.hson.14316995._000*").scan(sh,inputFilePath); // 364197.Sherpa_221_NNPDF30NNLO_Wtaunu_MAXHTPTV1000_E_CMS
  // EXOT5 for Top
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14317039._0000*").scan(sh,inputFilePath); // 410501_0 (ttbar) 410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad (seperate into 6 batch runs)
//  SH::ScanDir().filePattern("user.hson.14317039._0001*").scan(sh,inputFilePath); // 410501_1 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14317039._0002*").scan(sh,inputFilePath); // 410501_2 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14317039._0003*").scan(sh,inputFilePath); // 410501_3 (ttbar)
//  SH::ScanDir().filePattern("user.hson.14317039._0004*").scan(sh,inputFilePath); // 410501_4 (ttbar)
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_5 (ttbar) // No use
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_6 (ttbar) // No use
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_7 (ttbar) // No use
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 410501_8 (ttbar) // No use
//  SH::ScanDir().filePattern("user.hson.14317042._000*").scan(sh,inputFilePath); // 410011.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_top
//  SH::ScanDir().filePattern("user.hson.14317045._000*").scan(sh,inputFilePath); // 410012.PowhegPythiaEvtGen_P2012_singletop_tchan_lept_antitop
//  SH::ScanDir().filePattern("user.hson.14317046._000*").scan(sh,inputFilePath); // 410013.PowhegPythiaEvtGen_P2012_Wt_inclusive_top
//  SH::ScanDir().filePattern("user.hson.14317049._000*").scan(sh,inputFilePath); // 410014.PowhegPythiaEvtGen_P2012_Wt_inclusive_antitop
//  SH::ScanDir().filePattern("user.hson.14317052._000*").scan(sh,inputFilePath); // 410025.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_top
//  SH::ScanDir().filePattern("user.hson.14317055._000*").scan(sh,inputFilePath); // 410026.PowhegPythiaEvtGen_P2012_SingleTopSchan_noAllHad_antitop
  // EXOT5 for Diboson
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14317013._000*").scan(sh,inputFilePath); // 363494.Sherpa_221_NNPDF30NNLO_vvvv
//  SH::ScanDir().filePattern("user.hson.14317006._000*").scan(sh,inputFilePath); // 364250.Sherpa_222_NNPDF30NNLO_llll
//  SH::ScanDir().filePattern("user.hson.14317008._000*").scan(sh,inputFilePath); // 364253.Sherpa_222_NNPDF30NNLO_lllv
//  SH::ScanDir().filePattern("user.hson.14317009._000*").scan(sh,inputFilePath); // 364254.Sherpa_222_NNPDF30NNLO_llvv
//  SH::ScanDir().filePattern("user.hson.14317012._000*").scan(sh,inputFilePath); // 364255.Sherpa_222_NNPDF30NNLO_lvvv
//  SH::ScanDir().filePattern("user.hson.14317015._000*").scan(sh,inputFilePath); // 363355.Sherpa_221_NNPDF30NNLO_ZqqZvv
//  SH::ScanDir().filePattern("user.hson.14317017._000*").scan(sh,inputFilePath); // 363356.Sherpa_221_NNPDF30NNLO_ZqqZll
//  SH::ScanDir().filePattern("user.hson.14317019._000*").scan(sh,inputFilePath); // 363357.Sherpa_221_NNPDF30NNLO_WqqZvv
//  SH::ScanDir().filePattern("user.hson.14317020._000*").scan(sh,inputFilePath); // 363358.Sherpa_221_NNPDF30NNLO_WqqZll
//  SH::ScanDir().filePattern("user.hson.14317022._000*").scan(sh,inputFilePath); // 363359.Sherpa_221_NNPDF30NNLO_WpqqWmlv
//  SH::ScanDir().filePattern("user.hson.14317024._000*").scan(sh,inputFilePath); // 363360.Sherpa_221_NNPDF30NNLO_WplvWmqq
//  SH::ScanDir().filePattern("user.hson.14317028._000*").scan(sh,inputFilePath); // 363489.Sherpa_221_NNPDF30NNLO_WlvZqq
  // EXOT5 for Multijet
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim_v2");
//  SH::ScanDir().filePattern("user.hson.14317066._000*").scan(sh,inputFilePath); // 361020.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ0W
//  SH::ScanDir().filePattern("user.hson.14317071._000*").scan(sh,inputFilePath); // 361021.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ1W
//  SH::ScanDir().filePattern("user.hson.14317072._000*").scan(sh,inputFilePath); // 361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W
//  SH::ScanDir().filePattern("user.hson.14317076._000*").scan(sh,inputFilePath); // 361023.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ3W
//  SH::ScanDir().filePattern("user.hson.14317078._000*").scan(sh,inputFilePath); // 361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W
//  SH::ScanDir().filePattern("user.hson.14317080._000*").scan(sh,inputFilePath); // 361025.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ5W
//  SH::ScanDir().filePattern("user.hson.14317082._000*").scan(sh,inputFilePath); // 361026.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ6W
//  SH::ScanDir().filePattern("user.hson.14317084._000*").scan(sh,inputFilePath); // 361027.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ7W
//  SH::ScanDir().filePattern("user.hson.14317086._000*").scan(sh,inputFilePath); // 361028.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ8W
//  SH::ScanDir().filePattern("user.hson.14317088._000*").scan(sh,inputFilePath); // 361029.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ9W
//  SH::ScanDir().filePattern("user.hson.14317090._000*").scan(sh,inputFilePath); // 361030.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ10W
//  SH::ScanDir().filePattern("user.hson.14317093._000*").scan(sh,inputFilePath); // 361031.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ11W
//  SH::ScanDir().filePattern("user.hson.14317096._000*").scan(sh,inputFilePath); // 361032.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ12W

  // STDM4 for Zee
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim");
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364114.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364115.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364116.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364117.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364118.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364119.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364120.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364121.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364122.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV140_280_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364123.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364124.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364125.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364126.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364127.Sherpa_221_NNPDF30NNLO_Zee_MAXHTPTV1000_E_CMS
  // STDM4 for Zmumu
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/MC/MC16a/skim");
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364101.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364102.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364103.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364104.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364105.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV70_140_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364106.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364107.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364108.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV140_280_BFilter (derivation)
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364109.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CVetoBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364110.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_CFilterBVeto
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364111.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV280_500_BFilter
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364112.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV500_1000
//  SH::ScanDir().filePattern("*").scan(sh,inputFilePath); // 364113.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV1000_E_CMS


  // Full Skim Dataset (Data15)
  // Period A ~ G (run 276262 ~ 281075) : 313 files
  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data15/Data15_Period_AtoG");
  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 
  // Period H ~ J (run 281317 ~ 284484) : 328 files
//  const char* inputFilePath = gSystem->ExpandPathName ("/cluster/home/h/s/hson02/beaucheminlabHome/Dataset/Data/skim/data15/Data15_Period_HtoJ");
//  SH::ScanDir().filePattern("user.hson.*").scan(sh,inputFilePath); 




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
  std::string slurmJobName = "data15";
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
