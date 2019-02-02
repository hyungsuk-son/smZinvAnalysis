#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <smZInvAnalysis/smZInvAnalysis.h>

#include <TSystem.h>

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODMetaData/FileMetaData.h"
#include "xAODMetaData/FileMetaDataAuxInfo.h"
#include "xAODBase/IParticleHelpers.h"
#include "AthContainers/ConstDataVector.h"


// ASG status code check
#include <AsgTools/MessageCheck.h>
// Correction code check
#include "PATInterfaces/CorrectionCode.h" // to check the return correction code status of tools

#include "xAODCore/ShallowAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/AuxContainerBase.h"

// header files for systematics:
#include "PATInterfaces/SystematicVariation.h" 
#include "PATInterfaces/SystematicsUtil.h"

#include <SampleHandler/MetaNames.h>

struct DescendingPt:std::function<bool(const xAOD::IParticle*, const xAOD::IParticle*)> {
  bool operator()(const xAOD::IParticle* l, const xAOD::IParticle* r)  const {
    return l->pt() > r->pt();
  }
};



// this is needed to distribute the algorithm to the workers
ClassImp(smZInvAnalysis)



smZInvAnalysis :: smZInvAnalysis ()
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode smZInvAnalysis :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  // let's initialize the algorithm to use the xAODRootAccess package
  job.useXAOD ();
  xAOD::Init().ignore(); // call before opening first file

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)
  ANA_CHECK(xAOD::Init());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  // Sum of Weight
  h_sumOfWeights = new TH1D("h_sumOfWeights", "MetaData_EventCount", 3, 0.5, 3.5);
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(1, "sumOfWeights initial");
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(2, "sumOfWeightsSquared initial");
  h_sumOfWeights -> GetXaxis() -> SetBinLabel(3, "nEvents initial");
  wk()->addOutput (h_sumOfWeights);

  // Retrieve Derivation name from MetaData
  h_dataType = new TH1F("h_dataType","dataType",1,0,1);
  h_dataType->SetCanExtend(TH1::kAllAxes);
  wk()->addOutput (h_dataType);


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed

  m_event = wk()->xaodEvent();

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in initialise. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  // check if the event is data or MC
  // (many tools are applied either to data or MC)
  m_isData = true;
  // check if the event is MC
  if(eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION ) ){
    m_isData = false; // can do something with this later
  }

  // Extract MC campaign automatically from Run Number (ex. 284500: mc16a, 300000: mc16c)
  // mc16a : matching 2015+2016 data
  // mc16c : matching 2017 data
  // mc16d : re-reconstruct MC16c (unavailable yet)
  if (!m_isData) { // MC

    uint32_t runNum = eventInfo->runNumber();

    switch(runNum)
    {
      case 284500 :
        m_MC_campaign="mc16a";
        break;
        // This should be switched to mc16d once it is available.
      case 300000 :
        m_MC_campaign="mc16c";
        break;
      default :
        ANA_MSG_ERROR( "Could not determine mc campaign from run number! Impossible to autocongigure PRW. Aborting." );
        return StatusCode::FAILURE;
        break;
    }
    ANA_MSG_INFO( "Determined MC campaign to be " << m_MC_campaign);
  }

  // Retrieve the Dataset name
  m_nameDataset = wk()->metaData()->castString (SH::MetaNames::sampleName());
  std::cout << " Dataset name = " << m_nameDataset << std::endl;

  m_ZtruthChannel = "";
  // Check if this sample is Zee or Zmumu (for dress-, bare-, dress- level analysis in Truth level)
  // Zee (Sherpa, runNumber 364114 ~ 364127)
  if ( m_nameDataset.find("Zee")!=std::string::npos || // for Derivation samples
      m_nameDataset.find("364114")!=std::string::npos || m_nameDataset.find("364115")!=std::string::npos || m_nameDataset.find("364116")!=std::string::npos ||
      m_nameDataset.find("364117")!=std::string::npos || m_nameDataset.find("364118")!=std::string::npos || m_nameDataset.find("364119")!=std::string::npos ||
      m_nameDataset.find("364120")!=std::string::npos || m_nameDataset.find("364121")!=std::string::npos || m_nameDataset.find("364122")!=std::string::npos ||
      m_nameDataset.find("364123")!=std::string::npos || m_nameDataset.find("364124")!=std::string::npos || m_nameDataset.find("364125")!=std::string::npos ||
      m_nameDataset.find("364126")!=std::string::npos || m_nameDataset.find("364127")!=std::string::npos ) { // for my skim samples
    m_ZtruthChannel = "Zee";
  }
  // Zmumu (Sherpa, runNumber 364100 ~ 364113)
  if ( m_nameDataset.find("Zmumu")!=std::string::npos || // for Derivation samples
      m_nameDataset.find("364100")!=std::string::npos || m_nameDataset.find("364101")!=std::string::npos || m_nameDataset.find("364102")!=std::string::npos ||
      m_nameDataset.find("364103")!=std::string::npos || m_nameDataset.find("364104")!=std::string::npos || m_nameDataset.find("364105")!=std::string::npos ||
      m_nameDataset.find("364106")!=std::string::npos || m_nameDataset.find("364107")!=std::string::npos || m_nameDataset.find("364108")!=std::string::npos ||
      m_nameDataset.find("364109")!=std::string::npos || m_nameDataset.find("364110")!=std::string::npos || m_nameDataset.find("364111")!=std::string::npos ||
      m_nameDataset.find("364112")!=std::string::npos || m_nameDataset.find("364113")!=std::string::npos ) { // for my skim samples
    m_ZtruthChannel = "Zmumu";
  }

  // Retrieve the input file name
  m_input_filename = wk()->inputFileName();
  std::cout << " Input file name = " << m_input_filename << std::endl;

  // check if this sample is Derivation or my skimmed file
  m_fileType = "DxAOD";
  if ( m_input_filename.find("mini")!=std::string::npos || m_nameDataset.find("mini")!=std::string::npos ) { // my skimmed mini-xAOD sample name (ex: user.hson.11301992._000020.mini-xAOD.root)
    m_fileType = "skim";
  }
  else if ( m_input_filename.find("TRUTH1")!=std::string::npos ) { // my custom TRUTH1 sample name (ex: user.hson.12002993.EXT0._000001.DAOD_TRUTH1.test.pool.root)
    m_fileType = "truth1";
  }

  // check which denerator used in this dataset (Sherpa or MadGraph)
  m_generatorType = "";
  if ( m_nameDataset.find("Sherpa")!=std::string::npos || m_input_filename.find("Sherpa")!=std::string::npos ) { // ex: mc15_13TeV.364151.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV280_500_CVetoBVeto.merge.DAOD_EXOT5.e5308_s2726_r7772_r7676_p2949
    m_generatorType = "sherpa";
  }
  if ( m_nameDataset.find("MG")!=std::string::npos || m_input_filename.find("MG")!=std::string::npos || m_nameDataset.find("MadGraph")!=std::string::npos) { // ex: user.hson.363156.MGPy8EG.custom.TRUTH1.test10112017_EXT0
    m_generatorType = "madgraph";
  }
  std::cout << " Generator Type name = " <<  m_generatorType << std::endl;

  if (m_fileType == "DxAOD" || m_fileType == "skim") {

    // Event Bookkeepers
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/AnalysisMetadata#Luminosity_Bookkeepers

    // get the MetaData tree once a new file is opened, with
    TTree *MetaData = dynamic_cast<TTree*>(wk()->inputFile()->Get("MetaData"));
    if (!MetaData) {
      Error("fileExecute()", "MetaData not found! Exiting.");
      return EL::StatusCode::FAILURE;
    }
    MetaData->LoadTree(0);
    //MetaData->Print();


    ////////////////////////////
    // To get Derivation info //
    ////////////////////////////
    //----------------------------
    // MetaData information
    //--------------------------- 
    const xAOD::FileMetaData* fileMetaData = 0;
    //const xAOD::FileMetaDataAuxInfo* fileMetaData = 0;
    if( ! m_event->retrieveMetaInput( fileMetaData, "FileMetaData").isSuccess() ){
      Error("fileExecute()", "Failed to retrieve FileMetaData from MetaData. Exiting." );
      return EL::StatusCode::FAILURE;
    }


    // Initialize string variable
    m_dataType = "";
    // Read dataType from FileMetaData and store it to m_dataType
    const bool s = fileMetaData->value(xAOD::FileMetaData::dataType, m_dataType);

    if (s) {
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // Exotic Derivation (EXOT)
        std::string temp (m_dataType, 11, 5); // (ex) m_dataType: StreamDAOD_EXOT5
        m_nameDerivation = temp; // take "EXOT5" in "StreamDAOD_EXOT5"
      }
      if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
        std::string temp (m_dataType, 11, 4); // (ex) m_dataType: StreamDAOD_STDM4
        m_nameDerivation = temp; // only take "STDM" in "StreamDAOD_STDM4"
      }
      std::cout << " data type = " << m_dataType << std::endl;
      std::cout << " Derivation name = " << m_nameDerivation << std::endl;

      // Save a derivation name in a histogram
      h_dataType->Fill(m_dataType.c_str(), 1);
    }

    /*
    // Test (Retrieve beam energy info from FileMetaData using TBranch and TTree)
    TBranch *branch = MetaData->GetBranch("FileMetaDataAuxDyn.beamEnergy");
    branch->Print();

    Float_t m_beamEnergy;
    branch->SetAddress(&m_beamEnergy);
    branch->GetEntry(0);

    std::cout << " beam Energy = " << m_beamEnergy << std::endl;
    */



    //////////////////////////
    // To get Sum of Weight // 
    //////////////////////////

    //check if file is from a DxAOD
    bool m_isDerivation = !MetaData->GetBranch("StreamAOD");

    // For Derivation MC
    if(m_isDerivation && m_fileType == "DxAOD" && !m_isData) {
    // Note that m_isDerivation means Derivation dataset including skim. DxAOD means original Derivation dataset and skim means my skimmed dataset.

      // Now, let's find the actual information
      const xAOD::CutBookkeeperContainer* completeCBC = 0;
      if(!m_event->retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()){
        Error("initializeEvent()","Failed to retrieve CutBookkeepers from MetaData! Exiting.");
        return EL::StatusCode::FAILURE;
      }

      // Now, let's actually find the right one that contains all the needed info...
      const xAOD::CutBookkeeper* allEventsCBK=0;
      int maxcycle = -1;
      for ( auto cbk : *completeCBC ) { 
        if ( cbk->name() == "AllExecutedEvents" && cbk->inputStream() == "StreamAOD" && cbk->cycle() > maxcycle){
          maxcycle = cbk->cycle();
          allEventsCBK = cbk;
        }
      }


      uint64_t nEventsProcessed  = allEventsCBK->nAcceptedEvents();
      double sumOfWeights        = allEventsCBK->sumOfEventWeights();
      double sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();

      h_sumOfWeights -> Fill(1, sumOfWeights);
      h_sumOfWeights -> Fill(2, sumOfWeightsSquared);
      h_sumOfWeights -> Fill(3, nEventsProcessed);

      //Info("execute()", " Event # = %llu, sumOfWeights/nEventsProcessed = %f", eventInfo->eventNumber(), sumOfWeights/double(nEventsProcessed));
    }

    // For skimmed MC
    if (!m_isData && m_fileType == "skim") {

      // Retrieve histograms
      //TH1F *temp_dataType = dynamic_cast<TH1F*>(wk()->inputFile()->Get("h_dataType"));
      //temp_dataType->SetName("temp_dataType");
      TH1D *temp_sumOfWeights = dynamic_cast<TH1D*>(wk()->inputFile()->Get("h_sumOfWeights"));
      temp_sumOfWeights->SetName("temp_sumOfWeights");

      // Retrieve bin Label
      //m_dataType = temp_dataType->GetXaxis()->GetBinLabel(1);


      /*
         if ( m_dataType.find("EXOT")!=std::string::npos ) { // Exotic Derivation (EXOT)
         std::string temp (m_dataType, 11, 5); // (ex) m_dataType: StreamDAOD_EXOT5
         m_nameDerivation = temp; // take "EXOT5" in "StreamDAOD_EXOT5"
         }
         if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
         std::string temp (m_dataType, 11, 4); // (ex) m_dataType: StreamDAOD_STDM4
         m_nameDerivation = temp; // only take "STDM" in "StreamDAOD_STDM4"
         }


      // Save a derivation name in a histogram
      h_dataType->Fill(m_dataType.c_str(), 1);

      std::cout << " data type = " << m_dataType << std::endl;
      */


      // Save Sum of Weights in a histogram
      uint64_t nEventsProcessed  = temp_sumOfWeights->GetBinContent(3);
      double sumOfWeights        = temp_sumOfWeights->GetBinContent(1);
      double sumOfWeightsSquared = temp_sumOfWeights->GetBinContent(2);
      h_sumOfWeights -> Fill(1, sumOfWeights);
      h_sumOfWeights -> Fill(2, sumOfWeightsSquared);
      h_sumOfWeights -> Fill(3, nEventsProcessed);

    }

  } // Fro Derivation or Skim dataset


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  m_event = wk()->xaodEvent();

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in initialise. Exiting." );
    return EL::StatusCode::FAILURE;
  }



  // count number of events
  m_eventCounter = 0;
  // count clean events
  m_numCleanEvents = 0;

  // Enable Local Cutflow test
  // !Caution! When enabling this test, real analysis code will "NOT" be implemented.
  // !Note that I should choose only one channel otherwise the array values will be overwritten by following channel!
  m_useArrayCutflow = false; // For Exotic analysis cutflow comparison
  // Enable analysis cutflow to store into histogram
  m_useBitsetCutflow = true;

  // Event Channel
  m_isZnunu = true;
  m_isZmumu = true;
  m_isZee = true;
  m_isWmunu = true;
  m_isWenu = true;
  m_isZemu = true;

  // Enable Reconstruction level analysis
  m_doReco = true;

  if (m_isData) m_doReco = true;
  if (m_fileType == "truth1") m_doReco = false;

  // Enable Truth level analysis
  m_doTruth = true;

  // Enable Systematics
  m_doSys = false;

  // Scale factor
  m_recoSF = true;
  m_idSF = true;
  m_ttvaSF = true; // for muon
  m_isoMuonSF = true; // for muon iso
  m_isoMuonSFforZ = false; // Muons in Z->mumu are not isolated (Exotic analysis)
  m_elecTrigSF = true; // for electron trigger
  m_muonTrigSFforExotic = false; // for muon trigger (muon trigger is not used for Zmumu channel)
  m_muonTrigSFforSM = false; // for muon trigger (If I use muon triggers for Zmumu channel)
  m_metTrigSF = true; // for MET trigger
  m_isoElectronSF = true; // for electron iso


  // Cut values
  m_LeadLepPtCut = 80000.; ///MeV
  m_SubLeadLepPtCut = 7000.; ///MeV
  m_muonPtCut = 7000.; /// MeV
  m_lepEtaCut = 2.5;
  m_elecPtCut = 7000.; /// MeV
  m_elecEtaCut = 2.47;
  m_photPtCut = 20000.; /// MeV
  m_photEtaCut = 2.47;
  m_mllMin = 66000.; ///MeV
  m_mllMax = 116000.; ///MeV
  m_mTCut = 50000.; ///MeV // SM Analysis
  m_mTMin = 30000.; ///MeV // Exotic Analysis
  m_mTMax = 100000.; ///MeV // Exotic Analysis
  m_monoJetPtCut = 120000.; /// MeV
  m_monoJetEtaCut = 2.4;
  m_diJet1PtCut = 80000.; /// MeV
  m_diJet2PtCut = 50000.; /// MeV
  m_diJetRapCut = 4.4;
  m_CJVptCut = 25000.; ///MeV
  m_metCut = 200000.; ///MeV
  m_mjjCut = 200000.; ///MeV
  m_ORJETdeltaR = 0.4;

  // EXOT5 derivation Skim cut
  // Enable EXOT5 derivation skim for STDM4 (no skim) sample
  m_doSkimEXOT5 = false;
  // Skim cut values
  m_skimUncalibMonoJetPt = 100000.;
  m_skimMonoJetPt = 100000.;
  m_skimLeadingJetPt = 40000.;
  m_skimSubleadingJetPt = 40000.;
  m_skimMjj = 150000.;

  //////////////////////////////////
  // Event Selection for SM study //
  //////////////////////////////////
  // Common cut value
  // Overlap Removal
  sm_doORMuon = false;
  // MET
  sm_metCut = 130000.;
  sm_doPhoton_MET = false; // Add photon objects into real MET definition
  sm_doTau_MET = false; // Add tau objects into real MET definition
  sm_ORJETdeltaR = 0.4;
  // Jet pT
  sm_goodJetPtCut = 30000.;
  // No fiducial cut
  sm_noLep1PtCut = 0.;
  sm_noLep2PtCut = 0.;
  sm_noLepEtaCut = 100.; // Setting to 100. means no eta cut applied.
  // Exclusive
  sm_exclusiveJetPtCut = 130000.;
  sm_exclusiveJetEtaCut = 2.4;
  // Inclusive
  sm_inclusiveJetPtCut = 110000.;
  sm_inclusiveJetEtaCut = 2.4;
  // dPhi(Met,Jet) cut
  sm_dPhiJetMetCut = 0.4;
  // SM study lepton cuts
  sm_lep1PtCut = 50000.;
  sm_lep2PtCut = 7000.;
  sm_lepEtaCut = 2.5;




  // Initialize Cutflow
  if (m_useBitsetCutflow)
    m_BitsetCutflow = new BitsetCutflow(wk());

  // Initialize Cutflow count array
  for (int i=0; i<40; i++) {
    m_eventCutflow[i]=0;
  }

  // Initialize all tools for  Reco level analysis
  if (m_doReco) {

    // GRL
    m_grl = new GoodRunsListSelectionTool("GoodRunsListSelectionTool");
    std::vector<std::string> vecStringGRL;
    // 2015 data
    std::string fullGRLFilePath2015 = PathResolverFindCalibFile("smZInvAnalysis/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml");
    vecStringGRL.push_back(fullGRLFilePath2015);
    // 2016 data
    std::string fullGRLFilePath2016 = PathResolverFindCalibFile("smZInvAnalysis/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml");
    vecStringGRL.push_back(fullGRLFilePath2016);
    ANA_CHECK(m_grl->setProperty( "GoodRunsListVec", vecStringGRL));
    ANA_CHECK(m_grl->setProperty("PassThrough", false)); // if true (default) will ignore result of GRL and will just pass all events
    ANA_CHECK(m_grl->initialize());

    // Initialize and configure trigger tools
    m_trigConfigTool = new TrigConf::xAODConfigTool("xAODConfigTool"); // gives us access to the meta-data
    ANA_CHECK( m_trigConfigTool->initialize() );
    ToolHandle< TrigConf::ITrigConfigTool > trigConfigHandle( m_trigConfigTool );
    // trigger decision tool
    m_trigDecisionTool = new Trig::TrigDecisionTool("TrigDecisionTool");
    ANA_CHECK(m_trigDecisionTool->setProperty( "ConfigTool", trigConfigHandle ) ); // connect the TrigDecisionTool to the ConfigTool
    ANA_CHECK(m_trigDecisionTool->setProperty( "TrigDecisionKey", "xTrigDecision" ) );
    ANA_CHECK(m_trigDecisionTool->initialize() );

    // PileupReweighting Tool
    if (!m_isData) { // For MC
      m_prwTool = new CP::PileupReweightingTool("PrwTool");
      std::vector<std::string> file_conf;
      uint32_t dsid = 0;
      if (!m_isData) dsid = eventInfo->mcChannelNumber();
      // Pile-up config files are in SUSYTools pileup database. http://atlas.web.cern.ch/Atlas/GROUPS/DATABASE/GroupData/dev/SUSYTools/PRW_AUTOCONFIG/files/
      file_conf.push_back(PathResolverFindCalibFile("dev/SUSYTools/PRW_AUTOCONGIF/files/pileup_" + m_MC_campaign + "_dsid" + std::to_string(dsid) + ".root"));
      std::vector<std::string> file_ilumi;
      // 2015 dataset
      file_ilumi.push_back(PathResolverFindCalibFile("smZInvAnalysis/ilumicalc_histograms_None_276262-284484_OflLumi-13TeV-010.root"));
      // 2016 dataset
      file_ilumi.push_back(PathResolverFindCalibFile("smZInvAnalysis/ilumicalc_histograms_None_297730-311481_OflLumi-13TeV-010.root"));
      ANA_CHECK(m_prwTool->setProperty("ConfigFiles", file_conf) );
      ANA_CHECK(m_prwTool->setProperty("LumiCalcFiles", file_ilumi) );
      ANA_CHECK(m_prwTool->setProperty("DataScaleFactor",     1.0 / 1.03) ); // the recommended value for MC16
      ANA_CHECK(m_prwTool->setProperty("DataScaleFactorUP",   1.0) );
      ANA_CHECK(m_prwTool->setProperty("DataScaleFactorDOWN", 1.0 / 1.06) );
      //ANA_CHECK(m_prwTool->setProperty("UnrepresentedDataAction", 2));
      ANA_CHECK(m_prwTool->initialize() );
    }

    // JES Calibration (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ApplyJetCalibrationR21)
    const std::string name_JetCalibTools = "JetCalibTools";
    std::string jetAlgo = "AntiKt4EMTopo"; //String describing your jet collection, for example AntiKt4EMTopo or AntiKt4LCTopo
    std::string config = "JES_data2017_2016_2015_Recommendation_Aug2018_rel21.config"; //Path to global config used to initialize the tool
    std::string calibSeq = "JetArea_Residual_EtaJES_GSC"; //String describing the calibration sequence to apply
    if (!m_isData) calibSeq += "_Smear"; // for MC
    else calibSeq += "_Insitu"; // for Data
     std::string calibArea = "00-04-81"; //Calibration Area tag
    //Call the constructor. The default constructor can also be used if the arguments are set with python configuration instead
    //Initialize the tool
    m_jetCalibration = new JetCalibrationTool(name_JetCalibTools.c_str());
    ANA_CHECK(m_jetCalibration->setProperty("JetCollection", jetAlgo.c_str()));
    ANA_CHECK(m_jetCalibration->setProperty("ConfigFile", config.c_str()));
    ANA_CHECK(m_jetCalibration->setProperty("CalibSequence", calibSeq.c_str()));
    ANA_CHECK(m_jetCalibration->setProperty("CalibArea", calibArea.c_str()));
    ANA_CHECK(m_jetCalibration->setProperty("IsData", m_isData));
    ANA_CHECK(m_jetCalibration->initialize());

    // JES uncertainty (Jet Energy Scale)
    // Twiki (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetEtmissRecommendationsR21)
    // Twiki (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2018SmallR)
    m_jetUncertaintiesTool = new JetUncertaintiesTool("JESProvider");
    // Set the properties, note the use of the CHECK macro to ensure nothing failed
    ANA_CHECK(m_jetUncertaintiesTool->setProperty("JetDefinition", "AntiKt4EMTopo"));
    ANA_CHECK(m_jetUncertaintiesTool->setProperty("MCType", "MC16"));
    ANA_CHECK(m_jetUncertaintiesTool->setProperty("ConfigFile", "rel21/Summer2018/R4_StrongReduction_Scenario1_SimpleJER.config"));
    ANA_CHECK(m_jetUncertaintiesTool->setProperty("CalibArea", "CalibArea-05"));
    ANA_CHECK(m_jetUncertaintiesTool->setProperty("IsData",m_isData));
    // Initialise jet uncertainty tool
    ANA_CHECK(m_jetUncertaintiesTool->initialize());

    // Initialize and configure the jet cleaning tool (LooseBad)
    m_jetCleaningLooseBad = new JetCleaningTool("JetCleaningLooseBad");
    m_jetCleaningLooseBad->msg().setLevel( MSG::DEBUG ); 
    ANA_CHECK(m_jetCleaningLooseBad->setProperty( "CutLevel", "LooseBad"));
    ANA_CHECK(m_jetCleaningLooseBad->setProperty("DoUgly", false));
    ANA_CHECK(m_jetCleaningLooseBad->initialize());

    // Initialize and configure the jet cleaning tool (TightBad)
    m_jetCleaningTightBad = new JetCleaningTool("JetCleaningTightBad");
    m_jetCleaningTightBad->msg().setLevel( MSG::DEBUG ); 
    ANA_CHECK(m_jetCleaningTightBad->setProperty( "CutLevel", "TightBad"));
    ANA_CHECK(m_jetCleaningTightBad->setProperty("DoUgly", false));
    ANA_CHECK(m_jetCleaningTightBad->initialize());

    // Initialise JVT tool
    JetVertexTaggerTool* pjvtag = 0;
    pjvtag = new JetVertexTaggerTool("jvtag");
    m_hjvtagup = ToolHandle<IJetUpdateJvt>("jvtag");
    ANA_CHECK(pjvtag->setProperty("JVTFileName","JetMomentTools/JVTlikelihood_20140805.root"));
    ANA_CHECK(pjvtag->initialize());

    // Initialise Forward JVT tool
    m_fJvtTool.setTypeAndName("JetForwardJvtTool/fJvt");
    m_fJvtTool.setProperty("CentralMaxPt",60e3);
    ANA_CHECK( m_fJvtTool.retrieve() );

    // Initialise Jet JVT Efficiency Tool
    m_jvtefficiencyTool = new CP::JetJvtEfficiency("JvtEfficiencyTool");
    ANA_CHECK(m_jvtefficiencyTool->setProperty("WorkingPoint", "Medium"));
    ANA_CHECK(m_jvtefficiencyTool->setProperty("SFFile","JetJvtEfficiency/Moriond2018/JvtSFFile_EMTopoJets.root"));
    ANA_CHECK(m_jvtefficiencyTool->initialize());

    // Initialise Jet fJVT Efficiency Tool
    m_fjvtefficiencyTool = new CP::JetJvtEfficiency("fJvtEfficiencyTool");
    //ANA_CHECK(m_fjvtefficiencyTool->setProperty("WorkingPoint", "Default"));
    ANA_CHECK(m_fjvtefficiencyTool->setProperty("SFFile","JetJvtEfficiency/Moriond2018/fJvtSFFile.root"));
    ANA_CHECK(m_fjvtefficiencyTool->setProperty("ScaleFactorDecorationName","fJVTSF"));
    ANA_CHECK(m_fjvtefficiencyTool->initialize());


    // Initialize the BJet tools
    std::string taggerName = "MV2c10";
    std::string workingPointName = "FixedCutBEff_70";
    // Initialize the BJetSelectionTool
    m_BJetSelectTool = new BTaggingSelectionTool("BJetSelectionTool");
    ANA_CHECK(m_BJetSelectTool->setProperty("MaxEta", 2.5));
    ANA_CHECK(m_BJetSelectTool->setProperty("MinPt", 30000.));
    ANA_CHECK(m_BJetSelectTool->setProperty("JetAuthor", "AntiKt4EMTopoJets"));
    ANA_CHECK(m_BJetSelectTool->setProperty("TaggerName", taggerName));
    ANA_CHECK(m_BJetSelectTool->setProperty("FlvTagCutDefinitionsFileName", "xAODBTaggingEfficiency/13TeV/2017-21-13TeV-MC16-CDI-2018-02-09_v1.root"));
    ANA_CHECK(m_BJetSelectTool->setProperty("OperatingPoint", workingPointName));
    ANA_CHECK(m_BJetSelectTool->initialize()); 
    // Initialize the BJetEfficiencyTool
    m_BJetEfficiencyTool = new BTaggingEfficiencyTool("BJetEfficiencyTool");
    ANA_CHECK(m_BJetEfficiencyTool->setProperty("JetAuthor", "AntiKt4EMTopoJets"));
    ANA_CHECK(m_BJetEfficiencyTool->setProperty("TaggerName", taggerName));
    ANA_CHECK(m_BJetEfficiencyTool->setProperty("ScaleFactorFileName", "13TeV/2017-21-13TeV-MC16-CDI-2018-02-09_v1.root"));
    ANA_CHECK(m_BJetEfficiencyTool->setProperty("OperatingPoint", workingPointName));
    ANA_CHECK(m_BJetEfficiencyTool->initialize()); 


    // Muon momentum corrections (https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MCPAnalysisConsolidationMC16)
    // Muon calibration and smearing tool (For 2015 and 2016 dataset)
    m_muonCalibrationAndSmearingTool2016 = new CP::MuonCalibrationAndSmearingTool( "MuonCorrectionTool2016" );
    // For 2015 and 2016 datasets
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("Year","Data16"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("StatComb",false));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("SagittaRelease","sagittaBiasDataAll_25_07_17"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("SagittaCorr",true));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("doSagittaMCDistortion",false));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("Release","Recs2017_08_02"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->setProperty("SagittaCorrPhaseSpace",true));
    // Initialize the muon calibration and smearing tool
    m_muonCalibrationAndSmearingTool2016->msg().setLevel( MSG::INFO );
    ANA_CHECK(m_muonCalibrationAndSmearingTool2016->initialize());

    // Muon calibration and smearing tool (For 2017 dataset)
    m_muonCalibrationAndSmearingTool2017 = new CP::MuonCalibrationAndSmearingTool( "MuonCorrectionTool2017" );
    // For 2017 dataset
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("Year","Data17"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("StatComb",false));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("SagittaRelease","sagittaBiasDataAll_30_07_18"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("SagittaCorr",true));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("doSagittaMCDistortion",false));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("Release","Recs2017_08_02"));
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->setProperty("SagittaCorrPhaseSpace",true));
    // Initialize the muon calibration and smearing tool
    m_muonCalibrationAndSmearingTool2017->msg().setLevel( MSG::INFO );
    ANA_CHECK(m_muonCalibrationAndSmearingTool2017->initialize());

    // Muon identification (Medium) for SM study
    m_muonMediumSelection = new CP::MuonSelectionTool( "MuonMediumSelection" );
    //m_muonMediumSelection->msg().setLevel( MSG::VERBOSE );
    m_muonMediumSelection->msg().setLevel( MSG::INFO );
    //m_muonMediumSelection->msg().setLevel( MSG::ERROR );
    // MuQuality: {Tight, Medium, Loose, VeryLoose}, corresponding to 0, 1, 2 and 3 (default is 3 - VeryLoose quality)
    ANA_CHECK(m_muonMediumSelection->setProperty( "MaxEta", 2.5 ));
    ANA_CHECK(m_muonMediumSelection->setProperty( "MuQuality", 1));
    ANA_CHECK(m_muonMediumSelection->initialize());

    // Muon identification (Loose) for Exotic study
    m_muonLooseSelection = new CP::MuonSelectionTool( "MuonLooseSelection" );
    //m_muonLooseSelection->msg().setLevel( MSG::VERBOSE );
    m_muonLooseSelection->msg().setLevel( MSG::INFO );
    //m_muonLooseSelection->msg().setLevel( MSG::ERROR );
    // MuQuality: {Tight, Medium, Loose, VeryLoose}, corresponding to 0, 1, 2 and 3 (default is 3 - VeryLoose quality)
    ANA_CHECK(m_muonLooseSelection->setProperty( "MaxEta", 2.5 ));
    ANA_CHECK(m_muonLooseSelection->setProperty( "MuQuality", 2));
    ANA_CHECK(m_muonLooseSelection->initialize());


    // Muon trigger efficiency scale factors tool
    m_muonTriggerSFTool = new CP::MuonTriggerScaleFactors( "MuonTriggerSFTool" );
    ANA_CHECK(m_muonTriggerSFTool->setProperty("MuonQuality", "Medium")); // if you are selecting loose offline muons
    ANA_CHECK(m_muonTriggerSFTool->setProperty("CalibrationRelease", "180905_TriggerUpdate"));
    ANA_CHECK(m_muonTriggerSFTool->initialize() );

    // Muon reconstruction efficiency scale factors tool
    m_muonEfficiencySFTool = new CP::MuonEfficiencyScaleFactors( "MuonEfficiencySFTool" );
    ANA_CHECK(m_muonEfficiencySFTool->setProperty("WorkingPoint", "Medium") );
    ANA_CHECK(m_muonEfficiencySFTool->setProperty("CalibrationRelease", "180808_SummerUpdate"));
    ANA_CHECK(m_muonEfficiencySFTool->initialize() );

    // Muon isolation scale factor tool
    m_muonIsolationSFTool = new CP::MuonEfficiencyScaleFactors( "MuonIsolationSFTool" );
    ANA_CHECK(m_muonIsolationSFTool->setProperty("WorkingPoint", "FixedCutTightIso") );
    ANA_CHECK(m_muonIsolationSFTool->setProperty("CalibrationRelease", "180808_SummerUpdate"));
    ANA_CHECK(m_muonIsolationSFTool->initialize() );

    // Muon track-to-vertex-association (TTVA) scale factors tool
    m_muonTTVAEfficiencySFTool = new CP::MuonEfficiencyScaleFactors( "MuonTTVAEfficiencySFTool" );
    ANA_CHECK(m_muonTTVAEfficiencySFTool->setProperty("WorkingPoint", "TTVA") );
    ANA_CHECK(m_muonTTVAEfficiencySFTool->setProperty("CalibrationRelease", "180808_SummerUpdate"));
    ANA_CHECK(m_muonTTVAEfficiencySFTool->initialize() );


    // Initialize the electron and photon calibration and smearing tool
    m_egammaCalibrationAndSmearingTool = new CP::EgammaCalibrationAndSmearingTool( "EgammaCorrectionTool" );
    ANA_CHECK(m_egammaCalibrationAndSmearingTool->setProperty( "ESModel", "es2017_R21_v0" ));
    // The tool can still be run without a previous instantiation of the pileup reweighting tool, setting the randomRunNumber property to a value of a run of the 2015 dataset or the 2016 dataset.
    //ANA_CHECK(m_egammaCalibrationAndSmearingTool->setProperty( "randomRunNumber", 123456 ));
    //ANA_CHECK(m_egammaCalibrationAndSmearingTool->setProperty( "randomRunNumber", EgammaCalibPeriodRunNumbersExample::run_2016 )); // For MC15c
    ANA_CHECK(m_egammaCalibrationAndSmearingTool->setProperty( "decorrelationModel", "1NP_v1" )); // specify how the systematics variations are correlated: (1NP_v1: very simplified model)
    ANA_CHECK(m_egammaCalibrationAndSmearingTool->initialize());

    // LH Electron identification (Tight for SM study)
    // initialize the electron selection tool
    m_LHToolTight = new AsgElectronLikelihoodTool ("m_LHToolTight");
    // initialize the primary vertex container for the tool to have access to the number of vertices used to adapt cuts based on the pileup
    ANA_CHECK(m_LHToolTight->setProperty("primaryVertexContainer","PrimaryVertices"));
    // define the config files
    std::string confDirTight_2017 = "ElectronPhotonSelectorTools/offline/mc16_20170828/";
    ANA_CHECK(m_LHToolTight->setProperty("ConfigFile",confDirTight_2017+"ElectronLikelihoodTightOfflineConfig2017_Smooth.conf"));
    // initialize
    ANA_CHECK(m_LHToolTight->initialize());

    // LH Electron identification (Loose for SM study)
    // initialize the electron selection tool
    m_LHToolLoose = new AsgElectronLikelihoodTool ("m_LHToolLoose");
    // initialize the primary vertex container for the tool to have access to the number of vertices used to adapt cuts based on the pileup
    ANA_CHECK(m_LHToolLoose->setProperty("primaryVertexContainer","PrimaryVertices"));
    // define the config files
    std::string confDirLoose_2017 = "ElectronPhotonSelectorTools/offline/mc16_20170828/";
    ANA_CHECK(m_LHToolLoose->setProperty("ConfigFile",confDirLoose_2017+"ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf"));
    // initialize
    ANA_CHECK(m_LHToolLoose->initialize());

    // EGamma Ambiguity tool
    m_egammaAmbiguityTool = new EGammaAmbiguityTool ("EGammaAmbiguityTool");
    ANA_CHECK( m_egammaAmbiguityTool->initialize() );


    // Initialise Electron Efficiency Tool
    m_elecEfficiencySFTool_reco = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_reco");
    std::vector< std::string > corrFileNameList_reco;
    corrFileNameList_reco.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v2/offline/efficiencySF.offline.RecoTrk.root");
    ANA_CHECK(m_elecEfficiencySFTool_reco->setProperty("CorrectionFileNameList", corrFileNameList_reco) );
    ANA_CHECK(m_elecEfficiencySFTool_reco->setProperty("ForceDataType", 1) );
    ANA_CHECK(m_elecEfficiencySFTool_reco->setProperty("CorrelationModel", "TOTAL") );
    ANA_CHECK(m_elecEfficiencySFTool_reco->initialize() );

    m_elecEfficiencySFTool_id = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_id");
    std::vector< std::string > corrFileNameList_id;
    corrFileNameList_id.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/offline/efficiencySF.offline.TightLLH_d0z0_v13.root");
    ANA_CHECK(m_elecEfficiencySFTool_id->setProperty("CorrectionFileNameList", corrFileNameList_id) );
    ANA_CHECK(m_elecEfficiencySFTool_id->setProperty("ForceDataType", 1) );
    ANA_CHECK(m_elecEfficiencySFTool_id->setProperty("CorrelationModel", "TOTAL") );
    ANA_CHECK(m_elecEfficiencySFTool_id->initialize() );

    m_elecEfficiencySFTool_iso = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_iso");
    std::vector< std::string > corrFileNameList_iso;
    corrFileNameList_iso.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/isolation/efficiencySF.Isolation.TightLLH_d0z0_v13_isolFixedCutTight.root");
    ANA_CHECK(m_elecEfficiencySFTool_iso->setProperty("CorrectionFileNameList", corrFileNameList_iso) );
    ANA_CHECK(m_elecEfficiencySFTool_iso->setProperty("ForceDataType", 1) );
    ANA_CHECK(m_elecEfficiencySFTool_iso->setProperty("CorrelationModel", "TOTAL") );
    ANA_CHECK(m_elecEfficiencySFTool_iso->initialize() );

    m_elecEfficiencySFTool_trigSF = new AsgElectronEfficiencyCorrectionTool("AsgElectronEfficiencyCorrectionTool_trigSF");
    std::vector< std::string > corrFileNameList_trigSF;
    corrFileNameList_trigSF.push_back("ElectronEfficiencyCorrection/2015_2017/rel21.2/Moriond_February2018_v1/trigger/efficiency.SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2017_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0.TightLLH_d0z0_v13_isolFixedCutTight.root");
    ANA_CHECK(m_elecEfficiencySFTool_trigSF->setProperty("CorrectionFileNameList", corrFileNameList_trigSF) );
    ANA_CHECK(m_elecEfficiencySFTool_trigSF->setProperty("ForceDataType", 1) );
    ANA_CHECK(m_elecEfficiencySFTool_trigSF->setProperty("CorrelationModel", "TOTAL") );
    ANA_CHECK(m_elecEfficiencySFTool_trigSF->initialize() );





    // Initialize the MC fudge tool
    m_fudgeMCTool = new ElectronPhotonShowerShapeFudgeTool ( "FudgeMCTool" );
    int FFset = 22; // Rel 21
    ANA_CHECK(m_fudgeMCTool->setProperty("Preselection",FFset));
    ANA_CHECK(m_fudgeMCTool->setProperty("FFCalibFile","ElectronPhotonShowerShapeFudgeTool/v2/PhotonFudgeFactors.root")); // Rel 21 
    ANA_CHECK(m_fudgeMCTool->initialize());

    // Photon identification (Tight)
    // create the selector
    m_photonTightIsEMSelector = new AsgPhotonIsEMSelector ( "PhotonTightIsEMSelector" );
    // decide which kind of selection (Loose/Medium/Tight) you want to use
    ANA_CHECK(m_photonTightIsEMSelector->setProperty("isEMMask",egammaPID::PhotonTight));
    // set the file that contains the cuts on the shower shapes (stored in http://atlas.web.cern.ch/Atlas/GROUPS/DATABASE/GroupData/)
    ANA_CHECK(m_photonTightIsEMSelector->setProperty("ConfigFile","ElectronPhotonSelectorTools/offline/20180116/PhotonIsEMTightSelectorCutDefs.conf")); // Rel 21
    // initialise the tool
    ANA_CHECK(m_photonTightIsEMSelector->initialize());

    // Initialise tau smearing tool
    m_tauSmearingTool = new TauAnalysisTools::TauSmearingTool( "TauSmearingTool" );
    m_tauSmearingTool->msg().setLevel( MSG::INFO );
    //m_tauSmearingTool->msg().setLevel( MSG::DEBUG );
    ANA_CHECK(m_tauSmearingTool->initialize());

    // Initialise TauOverlappingElectronLLHDecorator
    m_tauOverlappingElectronLLHDecorator = new TauAnalysisTools::TauOverlappingElectronLLHDecorator("TauOverlappingElectronLLHDecorator"); 
    //m_tauOverlappingElectronLLHDecorator->msg().setLevel( MSG::DEBUG );
    ANA_CHECK(m_tauOverlappingElectronLLHDecorator->initialize());

    // Initialize the tau selection tool
    m_tauSelTool = new TauAnalysisTools::TauSelectionTool( "TauSelectionTool" );
    // define the config files
    std::string confPath = PathResolverFindCalibFile("smZInvAnalysis/recommended_selection_mc16.conf");
    ANA_CHECK(m_tauSelTool->setProperty( "ConfigPath", confPath));
    m_tauSelTool->msg().setLevel( MSG::INFO );
    //m_tauSelTool->msg().setLevel( MSG::DEBUG );
    //m_tauSelTool->msg().setLevel( MSG::VERBOSE );
    // initialize
    ANA_CHECK(m_tauSelTool->initialize());

    // IsolationSelectionTool (FixedCutTight for SM analysis)
    m_isolationFixedCutTightSelectionTool = new CP::IsolationSelectionTool("IsolationFixedCutTightSelectionTool");
    ANA_CHECK(m_isolationFixedCutTightSelectionTool->setProperty("MuonWP","FixedCutTight"));
    ANA_CHECK(m_isolationFixedCutTightSelectionTool->setProperty("ElectronWP","FixedCutTight"));
    ANA_CHECK(m_isolationFixedCutTightSelectionTool->setProperty("PhotonWP","FixedCutTight"));
    ANA_CHECK(m_isolationFixedCutTightSelectionTool->initialize());

    // IsolationSelectionTool (LooseTrackOnly for Exotic analysis)
    m_isolationLooseTrackOnlySelectionTool = new CP::IsolationSelectionTool("IsolationLooseTrackOnlySelectionTool");
    ANA_CHECK(m_isolationLooseTrackOnlySelectionTool->setProperty("MuonWP","LooseTrackOnly"));
    ANA_CHECK(m_isolationLooseTrackOnlySelectionTool->setProperty("ElectronWP","LooseTrackOnly"));
    ANA_CHECK(m_isolationLooseTrackOnlySelectionTool->setProperty("PhotonWP","FixedCutTight"));
    ANA_CHECK(m_isolationLooseTrackOnlySelectionTool->initialize());

    // Initialise Overlap removal tool
    m_toolBox = new ORUtils::ToolBox("ToolBox");
    static const std::string inputLabel = "selected";
    static const std::string outputLabel = outputPassValue? "passOR" : "overlaps";
    ORUtils::ORFlags orFlags("OverlapRemovalTool",inputLabel,outputLabel);
    orFlags.outputPassValue = outputPassValue; // overlap objects are 'true'
    // specify which objects to configure tools
    orFlags.doElectrons = true;
    orFlags.doMuons = sm_doORMuon;
    orFlags.doJets = true;
    orFlags.doTaus = false;
    if ( m_dataType.find("EXOT")!=std::string::npos ) { //EXOT5 derivation
      orFlags.doTaus = false;
    } else if ( m_dataType.find("STDM")!=std::string::npos ) { // STDM4 Derivation
      orFlags.doTaus = false;
    }
    orFlags.doPhotons = false;
    orFlags.boostedLeptons = false;
    orFlags.bJetLabel = "";
    // Get the recommended tool configuration
    ANA_CHECK(ORUtils::recommendedTools(orFlags,*m_toolBox));
    // Special settings
    //ANA_CHECK(m_toolBox->muJetORT.setProperty("NumJetTrk",100000000));
    if (sm_doORMuon) {
      ANA_CHECK(m_toolBox->muJetORT.setProperty("InnerDR",sm_ORJETdeltaR));
      ANA_CHECK(m_toolBox->muJetORT.setProperty("OuterDR",sm_ORJETdeltaR));
    }
    ANA_CHECK(m_toolBox->eleJetORT.setProperty("InnerDR",sm_ORJETdeltaR));
    ANA_CHECK(m_toolBox->eleJetORT.setProperty("OuterDR",sm_ORJETdeltaR));
    // Set message level for all tools
    //m_toolBox->msg().setLevel( MSG::INFO );
    m_toolBox->msg().setLevel( MSG::DEBUG );
    // Connect and initialize all tools
    ANA_CHECK(m_toolBox->initialize());

    // Initialise MET tools
    m_metMaker.setTypeAndName("met::METMaker/metMaker");
    ANA_CHECK(m_metMaker.setProperty("DoRemoveMuonJets", true));
    ANA_CHECK(m_metMaker.setProperty( "DoSetMuonJetEMScale", false));
    ANA_CHECK(m_metMaker.setProperty( "DoMuonEloss", false)); // currently under investigation. recommend false unless you see too many jets being removed
    ANA_CHECK(m_metMaker.setProperty("JetMinWeightedPt", 20000.));
    ANA_CHECK(m_metMaker.setProperty("JetMinEFrac", 0.0));
    //ANA_CHECK(m_metMaker.setProperty("JetJvtMomentName", "Jvt"));  // Don't need to use this option if I am using the standard name "Jvt"
    ANA_CHECK(m_metMaker.setProperty("JetRejectionDec","passFJVT")); // NEW for rel. 20.7: MET with forward JVT 
    //m_metMaker.msg().setLevel( MSG::VERBOSE ); // or DEBUG or VERBOSE
    ANA_CHECK(m_metMaker.retrieve());

    // Initialise MET Systematics Tools
    m_metSystTool.setTypeAndName("met::METSystematicsTool/metSystTool");
    ANA_CHECK(m_metSystTool.setProperty("ConfigSoftTrkFile", "TrackSoftTerms.config") );
    ANA_CHECK(m_metSystTool.retrieve());



    // Get the systematics registry and add the recommended systematics into our list of systematics to run over (+/-1 sigma):
    const CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
    const CP::SystematicSet& recommendedSystematics = registry.recommendedSystematics(); // get list of recommended systematics
    m_sysList = CP::make_systematics_vector(recommendedSystematics);

  } //m_doReco


  ////////////////////////
  // Create Histograms ///
  ////////////////////////

  // For Exotic analysis
  // Publication bins
  // Monojet and VBF MET
  Float_t binsMET[] = {200.,250.,300.,350.,500.,700.,1000.,1400.};
  Int_t nbinMET = sizeof(binsMET)/sizeof(Float_t) - 1;
  // VBF mjj
  Float_t binsMjj[] = {200.,400.,600.,1000.,2000.,3000.,4000.};
  Int_t nbinMjj = sizeof(binsMjj)/sizeof(Float_t) - 1;
  // VBF dPhi(j1,j2)
  Float_t pi = TMath::Pi();
  Float_t binsDPhi[] = {0., pi/(7-1), 2*pi/(7-1), 3*pi/(7-1), 4*pi/(7-1), 5*pi/(7-1), pi}; // where 7 is number of bins
  Int_t nbinDPhi = sizeof(binsDPhi)/sizeof(Float_t) - 1;


  // For SM ratio analysis
  // Custom Binning
  // Exclusive
  Float_t ex_binsMET[] = {130.,150.,175.,200.,225.,250.,300.,350.,400.,450.,550.,650.,1500.};
  Int_t ex_nbinMET = sizeof(ex_binsMET)/sizeof(Float_t) - 1;
  // Inclusive
  Float_t in_binsMET[] = {130.,150.,175.,200.,225.,250.,300.,350.,400.,450.,525.,600.,675.,750.,900.,1050.,1500.};
  Int_t in_nbinMET = sizeof(in_binsMET)/sizeof(Float_t) - 1;




  TH1::SetDefaultSumw2(kTRUE);



  ////////////////////
  // MC Truth level //
  ////////////////////
  if (!m_isData && m_doTruth) { // MC

    if (m_dataType.find("EXOT")!=std::string::npos || m_dataType.find("STDM")!=std::string::npos) { // EXOT5 or STDM4

      if (m_isZmumu) {
        h_channel = "h_zmumu_";
        /////////////////////////////////
        // Truth Z -> mumu + JET EVENT //
        /////////////////////////////////
        h_level = "nominal_";
        addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_noTrig", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_noTrig", 150, 0., 300.);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_metTrig", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_metTrig", 150, 0., 300.);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_muonTrig", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_muonTrig", 150, 0., 300.);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_met_emulmet", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_mjj", nbinMjj, binsMjj);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_dPhijj", nbinDPhi, binsDPhi);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_mll", 150, 0., 300.);
        //Test
        addHist(hMap1D, h_channel+h_level+"truth_test_met_emulmet", 100, 0., 1400.);
        addHist(hMap1D, h_channel+h_level+"truth_test_mll", 150, 0., 300.);
        addHist(hMap1D, h_channel+h_level+"truth_test_met_emulmet_with_MetTrig", 100, 0., 1400.);
        addHist(hMap1D, h_channel+h_level+"truth_test_mll_with_MetTrig", 150, 0., 300.);
        /////////////////////////////////////////////
        // WZ Truth level (For dressed-level jets) //
        /////////////////////////////////////////////
        if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
          h_level = "dress_";
          addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_noTrig", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_noTrig", 150, 0., 300.);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_metTrig", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_metTrig", 150, 0., 300.);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet_muonTrig", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_mll_muonTrig", 150, 0., 300.);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_met_emulmet", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_mjj", nbinMjj, binsMjj);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_dPhijj", nbinDPhi, binsDPhi);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_mll", 150, 0., 300.);
        }
      } // m_isZmumu


      if (m_isZee) {
        h_channel = "h_zee_";
        ///////////////////////////////
        // Truth Z -> ee + JET EVENT //
        ///////////////////////////////
        h_level = "nominal_";
        addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_monojet_mll", 150, 0., 300.);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_met_emulmet", nbinMET, binsMET);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_mjj", nbinMjj, binsMjj);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_dPhijj", nbinDPhi, binsDPhi);
        addHist(hMap1D, h_channel+h_level+"truth_vbf_mll", 150, 0., 300.);
        /////////////////////////////////////////////
        // WZ Truth level (For dressed-level jets) //
        /////////////////////////////////////////////
        if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
          h_level = "dress_";
          addHist(hMap1D, h_channel+h_level+"truth_monojet_met_emulmet", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_monojet_mll", 150, 0., 300.);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_met_emulmet", nbinMET, binsMET);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_mjj", nbinMjj, binsMjj);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_dPhijj", nbinDPhi, binsDPhi);
          addHist(hMap1D, h_channel+h_level+"truth_vbf_mll", 150, 0., 300.);
        }
      } // m_isZee



      //////////////////////////////////////
      // Truth (particle) level Histogram //
      //////////////////////////////////////
      const int channel_num = 1;
      const int prefix_num = 4;
      std::string channel[channel_num] = {"h_zmumu_"};
      std::string PreFix[prefix_num] = {"nominal_truth_","born_truth_","bare_truth_","dress_truth_"};
      for(int i=0; i < channel_num; i++) {
        for(int j=0; j < prefix_num; j++) {
          addHist(hMap1D, channel[i]+PreFix[j]+"MET_mono", nbinMET, binsMET);
          addHist(hMap1D, channel[i]+PreFix[j]+"MET_search", nbinMET, binsMET);
          addHist(hMap1D, channel[i]+PreFix[j]+"Mjj_search", nbinMjj, binsMjj);
          addHist(hMap1D, channel[i]+PreFix[j]+"DeltaPhiAll", nbinDPhi, binsDPhi);
          addHist(hMap1D, channel[i]+PreFix[j]+"Mll_mono", 150, 0., 300.);
          addHist(hMap1D, channel[i]+PreFix[j]+"Mll_search", 150, 0., 300.);
        } 
      }

    } // EXOT5 or STDM4


    //////////////////////////////////
    // SM study in TRUTH1 derivation//
    //////////////////////////////////
    // TRUTH1 or STDM or EOXT Derivation


    if ( !m_isData && ( m_fileType =="truth1" || m_dataType.find("STDM")!=std::string::npos || m_dataType.find("EXOT")!=std::string::npos ) ) {
      is_customDerivation = true;
      // When using STDM derivation samples, do not retrieve my Custom jet collections.
      if (m_dataType.find("STDM")!=std::string::npos || m_dataType.find("EXOT")!=std::string::npos) { // STDM or EXOT
        is_customDerivation = false;
      }
      // prompt muon
      addHist(hMap1D, "SM_study_truth_prompt_muon_pt", 100, 0., 100.);
      if (is_customDerivation)
        addHist(hMap1D, "SM_study_custom_prompt_muon_pt", 100, 0., 100.);
      // prompt electron
      addHist(hMap1D, "SM_study_truth_prompt_electron_pt", 100, 0., 100.);
      if (is_customDerivation)
        addHist(hMap1D, "SM_study_custom_prompt_electron_pt", 100, 0., 100.);

      // Nomianl jet
      addHist(hMap1D, "SM_study_nominal_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_nominal_jet_pt", 100, 0., 1000.);
      addHist(hMap1D, "SM_study_nominal_good_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_nominal_good_jet_pt", 100, 0., 1000.);

      // Overlap removed (OR) jet
      addHist(hMap1D, "SM_study_bare_OR_good_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_bare_OR_good_jet_pt", 100, 0., 1000.);
      addHist(hMap1D, "SM_study_dress_OR_good_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_dress_OR_good_jet_pt", 100, 0., 1000.);

      // Overlap subtracted (OS) jet
      addHist(hMap1D, "SM_study_bare_OS_good_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_bare_OS_good_jet_pt", 100, 0., 1000.);
      addHist(hMap1D, "SM_study_dress_OS_good_jet_n", 40, 0., 40.);
      addHist(hMap1D, "SM_study_dress_OS_good_jet_pt", 100, 0., 1000.);



      if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT


        // FSR photon
        addHist(hMap1D, "SM_study_truth_fsr_photon_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_truth_fsr_photon_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_fsr_photon_from_Z_madgraph_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_fsr_photon_from_Z_sherpa_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_fsr_photon_from_lepton_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_fsr_photon_not_from_lepton_pt", 100, 0., 100.);
        if ( m_ZtruthChannel == "Zmumu" ) { // For Sherpa Zmumu sample
          addHist(hMap1D, "SM_study_prompt_or_fsr_photon_muon_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_prompt_or_fsr_photon_muon_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
          addHist(hMap1D, "SM_study_fsr_photon_muon_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_fsr_photon_muon_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
          addHist(hMap1D, "SM_study_prompt_photon_muon_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_prompt_photon_muon_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
        }
        if ( m_ZtruthChannel == "Zee" ) { // For Sherpa Zee sample
          addHist(hMap1D, "SM_study_prompt_or_fsr_photon_electron_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_prompt_or_fsr_photon_electron_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
          addHist(hMap1D, "SM_study_fsr_photon_electron_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_fsr_photon_electron_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
          addHist(hMap1D, "SM_study_prompt_photon_electron_dr", 100, 0., 5.);
          addHist(hMap2D, "SM_study_prompt_photon_electron_dr_vs_photon_pt", 100, 0., 5., 100, 0., 100.);
        }
        if ( m_ZtruthChannel == "Zmumu" ) { // For Sherpa Zmumu sample
          addHist(hMap1D, "SM_study_truth_fsr_photon_dr_less_04_with_muon_n", 40, 0., 40.);
        }
        if ( m_ZtruthChannel == "Zee" ) { // For Sherpa Zee sample
          addHist(hMap1D, "SM_study_truth_fsr_photon_dr_less_04_with_electron_n", 40, 0., 40.);
        }
        addHist(hMap1D, "SM_study_truth_dressed_photon_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_nondressed_photon_pt", 100, 0., 100.);
        addHist(hMap1D, "SM_study_truth_prompt_dressed_photon_pt", 100, 0., 100.);
        if (is_customDerivation) {
          addHist(hMap1D, "SM_study_custom_fsr_photon_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_fsr_photon_pt", 100, 0., 100.);
          addHist(hMap1D, "SM_study_custom_dressed_photon_pt", 100, 0., 100.);
          addHist(hMap1D, "SM_study_custom_nondressed_photon_pt", 100, 0., 100.);
        }
        // Jet
        addHist(hMap1D, "SM_study_wz_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_wz_jet_pt", 100, 0., 1000.);
        if (is_customDerivation) {
          addHist(hMap1D, "SM_study_custom_dress_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_dress_jet_pt", 100, 0., 1000.);
          addHist(hMap1D, "SM_study_custom_bare_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_bare_jet_pt", 100, 0., 1000.);
          addHist(hMap1D, "SM_study_custom_born_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_born_jet_pt", 100, 0., 1000.);
        }
        addHist(hMap1D, "SM_study_emulated_bare_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_emulated_bare_jet_pt", 100, 0., 1000.);
        addHist(hMap1D, "SM_study_emulated_born_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_emulated_born_jet_pt", 100, 0., 1000.);
        // Jet (Good) pT > 30GeV 
        addHist(hMap1D, "SM_study_wz_good_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_wz_good_jet_pt", 100, 0., 1000.);
        if (is_customDerivation) {
          addHist(hMap1D, "SM_study_custom_dress_good_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_dress_good_jet_pt", 100, 0., 1000.);
          addHist(hMap1D, "SM_study_custom_bare_good_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_bare_good_jet_pt", 100, 0., 1000.);
          addHist(hMap1D, "SM_study_custom_born_good_jet_n", 40, 0., 40.);
          addHist(hMap1D, "SM_study_custom_born_good_jet_pt", 100, 0., 1000.);
        }
        addHist(hMap1D, "SM_study_emulated_bare_good_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_emulated_bare_good_jet_pt", 100, 0., 1000.);
        addHist(hMap1D, "SM_study_emulated_born_good_jet_n", 40, 0., 40.);
        addHist(hMap1D, "SM_study_emulated_born_good_jet_pt", 100, 0., 1000.);
        // dress electron
        addHist(hMap1D, "SM_study_dress_electron_pt_from_Z", 100, 0., 1000.);
        // bare electron
        addHist(hMap1D, "SM_study_bare_electron_pt_from_Z", 100, 0., 1000.);
        // born electron
        addHist(hMap1D, "SM_study_born_electron_pt_from_Z", 100, 0., 1000.);
        // dress muon
        addHist(hMap1D, "SM_study_dress_muon_pt_from_Z", 100, 0., 1000.);
        // bare muon
        addHist(hMap1D, "SM_study_bare_muon_pt_from_Z", 100, 0., 1000.);
        // born muon
        addHist(hMap1D, "SM_study_born_muon_pt_from_Z", 100, 0., 1000.);

      } // Not EXOT

      // Neutrino from Z
      addHist(hMap1D, "SM_study_neutrino_pt_from_Z", 100, 0., 1000.);

      /////////////////////////////////////////////
      // Analysis (Inclusive, Exclusive and VBF) //
      /////////////////////////////////////////////
      if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT

        // Analysis using custom jets (using my custom TRUTH1 derivation)
        if (is_customDerivation) {
          const int channel_n = 2;
          const int level_n = 3;
          const int monojet_n = 2;
          std::string channel[channel_n] = {"zee_","zmumu_"};
          std::string level[level_n] = {"dress_custom_","bare_custom_","born_custom_"};
          std::string monojet[monojet_n] = {"exclusive_","inclusive_"};
          for(int i=0; i < channel_n; i++) {
            for(int j=0; j < level_n; j++) {
              for(int k=0; k < monojet_n; k++) {
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"mll", 150, 0., 300.);
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"met", 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"jet_n", 40, 0., 40.);
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"jet_pt", 100, 0., 1000.);
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"lep1_pt", 140, 0., 1400.);
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"lep2_pt", 140, 0., 1400.);
              } 
            }
          }
        }



        // Analysis using emulated jets (using STDM4 derivation)
        const int channel_n = 2;
        const int level_n = 7;
        const int monojet_n = 4;
        std::string channel[channel_n] = {"zee_","zmumu_"};
        std::string level[level_n] = {"dress_wz_","dress_OR_","dress_OS_","bare_emul_","bare_OR_","bare_OS_","born_emul_"};
        std::string monojet[monojet_n] = {"exclusive_","exclusive_fid_","inclusive_","inclusive_fid_"};
        for(int i=0; i < channel_n; i++) {
          for(int j=0; j < level_n; j++) {
            // Overlap removal and Overlap Subtraction study is not implemented in Z->mumu
            for(int k=0; k < monojet_n; k++) {
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"met", 137, 130., 1500.); // mll cut (66 < mll < 116 GeV)
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"only66mll_met", 137, 130., 1500.); // mll cut ( mll > 66 GeV)
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"fullmll_met", 137, 130., 1500.); // mll cut ( mll > 66 GeV)
              if (monojet[k] == "exclusive_" || monojet[k] == "exclusive_fid_") {
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"MET_mono", ex_nbinMET, ex_binsMET); // mll cut (66 < mll < 116 GeV)
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"only66mll_MET_mono", ex_nbinMET, ex_binsMET); // mll cut ( mll > 66 GeV)
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"fullmll_MET_mono", ex_nbinMET, ex_binsMET); // No mll cut

              }
              if (monojet[k] == "inclusive_" || monojet[k] == "inclusive_fid_") {
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"MET_mono", in_nbinMET, in_binsMET); // mll cut (66 < mll < 116 GeV)
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"only66mll_MET_mono", in_nbinMET, in_binsMET); // mll cut ( mll > 66 GeV)
                addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"fullmll_MET_mono", in_nbinMET, in_binsMET); // No mll cut
              }
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"mll", 150, 0., 300.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"jet_n", 40, 0., 40.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"jet_pt", 100, 0., 1000.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"lep_pt", 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"lep1_pt", 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"lep2_pt", 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+channel[i]+level[j]+monojet[k]+"fullmll", 150, 0., 300.);
              if ( level[j].find("bare")!=std::string::npos || level[j].find("born")!=std::string::npos ) {
                addHist(hMap2D, "SM_study_"+channel[i]+level[j]+monojet[k]+"leadpt_vs_met", 11, 0., 11., 137, 130., 1500.);
              }
            } 
          }
        }

      } // Not EXOT


      // Z->nn
      if (m_isZnunu) {

        std::string channel = "znunu_";
        const int level_n = 1;
        const int monojet_n = 2;
        std::string level[level_n] = {"truth_"};
        std::string monojet[monojet_n] = {"exclusive_","inclusive_"};
        for(int i=0; i < level_n; i++) {
          for(int j=0; j < monojet_n; j++) {
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"mll", 150, 0., 300.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"met", 137, 130., 1500.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"only66mll_met", 137, 130., 1500.); // mll cut ( mll > 66 GeV)
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"fullmll_met", 137, 130., 1500.); // mll cut ( mll > 66 GeV)
            if (monojet[j] == "exclusive_") {
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"MET_mono", ex_nbinMET, ex_binsMET);
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"only66mll_MET_mono", ex_nbinMET, ex_binsMET); // mll cut ( mll > 66 GeV)
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"fullmll_MET_mono", ex_nbinMET, ex_binsMET); // No mll cut
            }
            if (monojet[j] == "inclusive_") {
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"MET_mono", in_nbinMET, in_binsMET);
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"only66mll_MET_mono", in_nbinMET, in_binsMET); // mll cut ( mll > 66 GeV)
              addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"fullmll_MET_mono", in_nbinMET, in_binsMET); // No mll cut
            }
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"jet_n", 40, 0., 40.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"jet_pt", 100, 0., 1000.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"lep_pt", 140, 0., 1400.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"lep1_pt", 140, 0., 1400.);
            addHist(hMap1D, "SM_study_"+channel+level[i]+monojet[j]+"lep2_pt", 140, 0., 1400.);
            addHist(hMap2D, "SM_study_"+channel+level[i]+monojet[j]+"leadpt_vs_met", 11, 0., 11., 137, 130., 1500.);
          }
        }

      }


    } // TRUTH1



  } // MC





  //////////////////////////
  // Reconstruction level //
  //////////////////////////
  if(m_doReco){

    // loop over recommended systematics
    for (const auto &sysList : m_sysList){
      if ((!m_doSys || m_isData) && (sysList).name() != "") continue;
      std::string m_sysName = (sysList).name();

      if (m_doSys && (m_sysName.find("TAUS_")!=std::string::npos || m_sysName.find("PH_")!=std::string::npos )) continue;
      if (m_isZmumu && !m_isZee && !m_isZnunu && m_doSys && ((m_sysName.find("EL_")!=std::string::npos || m_sysName.find("EG_")!=std::string::npos))) continue;
      if (m_isZee && !m_isZmumu && !m_isZnunu && m_doSys && ((m_sysName.find("MUON_")!=std::string::npos || m_sysName.find("MUONS_")!=std::string::npos))) continue;
      if (m_isZnunu && !m_isZmumu && !m_isZee && m_doSys && ((m_sysName.find("MUON_")!=std::string::npos || m_sysName.find("MUONS_")!=std::string::npos || m_sysName.find("EL_")!=std::string::npos || m_sysName.find("EG_")!=std::string::npos)) ) continue;


      // Print the list of systematics
      if(m_sysName=="") std::cout << "Nominal (no syst) "  << std::endl;
      else std::cout << "Systematic: " << m_sysName << std::endl;


      ////////////////////
      // SM ratio Study //
      ////////////////////

      // Analysis plots
      const int sm_channel_n = 5;
      const int sm_level_n = 1;
      const int sm_monojet_n = 2;
      std::string sm_channel[sm_channel_n] = {"znunu_","zee_","zmumu_","wenu_","wmunu_"};
      std::string sm_level[sm_level_n] = {"reco_"};
      std::string sm_monojet[sm_monojet_n] = {"exclusive_","inclusive_"};
      for(int i=0; i < sm_channel_n; i++) {
        for(int j=0; j < sm_level_n; j++) {
          for(int k=0; k < sm_monojet_n; k++) {
            addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met"+m_sysName, 137, 130., 1500.);
            if (sm_channel[i] == "zmumu_" || sm_channel[i] == "zee_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_met"+m_sysName, 137, 130., 1500.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_met"+m_sysName, 137, 130., 1500.);
            }
            if (sm_monojet[k] == "exclusive_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono"+m_sysName, ex_nbinMET, ex_binsMET);
              if (sm_channel[i] == "zee_" || sm_channel[i] == "wenu_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono_OS"+m_sysName, ex_nbinMET, ex_binsMET);
              }
              if (sm_channel[i] == "zmumu_" || sm_channel[i] == "zee_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_MET_mono"+m_sysName, ex_nbinMET, ex_binsMET);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_MET_mono"+m_sysName, ex_nbinMET, ex_binsMET);
              }
            }
            if (sm_monojet[k] == "inclusive_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono"+m_sysName, in_nbinMET, in_binsMET);
              if (sm_channel[i] == "zee_" || sm_channel[i] == "wenu_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono_OS"+m_sysName, in_nbinMET, in_binsMET);
              }
              if (sm_channel[i] == "zmumu_" || sm_channel[i] == "zee_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_MET_mono"+m_sysName, in_nbinMET, in_binsMET);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_MET_mono"+m_sysName, in_nbinMET, in_binsMET);
              }
            }
            addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_n"+m_sysName, 40, 0., 40.);
            if (sm_monojet[k] == "exclusive_") addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"dPhiJetMet"+m_sysName, 20, 0, 3.2);
            if (sm_monojet[k] == "inclusive_") addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"dPhiMinJetMet"+m_sysName, 20, 0, 3.2);
            if (sm_channel[i] == "znunu_" && sm_monojet[k] == "inclusive_") addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"dPhiMinJetMetNoCut"+m_sysName, 20, 0, 3.2);
            if (sm_monojet[k] == "exclusive_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_pt"+m_sysName, 150, 0., 1500.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_phi"+m_sysName, 20, -3.2, 3.2);
            }
            if (sm_monojet[k] == "inclusive_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet1_pt"+m_sysName, 150, 0., 1500.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet2_pt"+m_sysName, 150, 0., 1500.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet3_pt"+m_sysName, 150, 0., 1500.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet1_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet2_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet3_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet1_phi"+m_sysName, 20, -3.2, 3.2);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet2_phi"+m_sysName, 20, -3.2, 3.2);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet3_phi"+m_sysName, 20, -3.2, 3.2);
            }
            if (sm_channel[i] == "zee_" || sm_channel[i] == "wenu_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_n_OS"+m_sysName, 40, 0., 40.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"jet_pt_OS"+m_sysName, 100, 0., 1000.);
            }
            // For unfolding (No MET trigger passed)
            if (sm_channel[i] == "znunu_" || sm_channel[i] == "zmumu_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_noMetTrig"+m_sysName, 137, 130., 1500.);
              if (sm_channel[i] == "zmumu_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_met_noMetTrig"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_met_noMetTrig"+m_sysName, 137, 130., 1500.);
              }
              if (sm_monojet[k] == "exclusive_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono_noMetTrig"+m_sysName, ex_nbinMET, ex_binsMET);
                if (sm_channel[i] == "zmumu_") {
                  addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_MET_mono_noMetTrig"+m_sysName, ex_nbinMET, ex_binsMET);
                  addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_MET_mono_noMetTrig"+m_sysName, ex_nbinMET, ex_binsMET);
                }
              }
              if (sm_monojet[k] == "inclusive_") {
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"MET_mono_noMetTrig"+m_sysName, in_nbinMET, in_binsMET);
                if (sm_channel[i] == "zmumu_") {
                  addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"only66mll_MET_mono_noMetTrig"+m_sysName, in_nbinMET, in_binsMET);
                  addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"fullmll_MET_mono_noMetTrig"+m_sysName, in_nbinMET, in_binsMET);
                }
              }
            }
            // WpT Test (Emulation of WpT (realMET+Lepton))
            if (m_sysName=="" && (sm_channel[i] == "wmunu_" || sm_channel[i] == "wenu_")) {
              if (sm_monojet[k] == "exclusive_") { // No jet cuts
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+"noJetCut_real_MET"+m_sysName, 143, 0., 1430.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+"noJetCut_emul_MET"+m_sysName, 143, 0., 1430.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+"noJetCut_emul_Wpt"+m_sysName, 143, 0., 1430.);
              } // Exclusive and Inclusive jet cut
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"real_MET"+m_sysName, 143, 0., 1430.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"emul_MET"+m_sysName, 143, 0., 1430.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"emul_Wpt"+m_sysName, 143, 0., 1430.);
            }
            if (m_sysName=="" && (sm_channel[i] == "znunu_" || sm_channel[i] == "zmumu_" || sm_channel[i] == "zee_")) { // No systematic
                // Statistical Unc. Test for ratio (scanning leading jet cuts)
              if (sm_monojet[k] == "exclusive_") { // Exclusive
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt130"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt140"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt150"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt160"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt170"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt180"+m_sysName, 137, 130., 1500.);
              }
              if (sm_monojet[k] == "inclusive_") { // Inclusive
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt100"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt110"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt120"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt130"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt140"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt150"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt160"+m_sysName, 137, 130., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"met_LeadjetPt170"+m_sysName, 137, 130., 1500.);
              }
              // Multijet Background Estimation (Multijet enriched control region)
              if (sm_channel[i] == "znunu_" && sm_monojet[k] == "exclusive_") { // Exclusive
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin1"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin2"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin3"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin4"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin5"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin6"+m_sysName, 300, 0., 1500.);
              }
              if (sm_channel[i] == "znunu_" && sm_monojet[k] == "inclusive_") { // Exclusive
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin1"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin2"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin3"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin4"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin5"+m_sysName, 300, 0., 1500.);
                addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"multijetCR_badJetPt_bin6"+m_sysName, 300, 0., 1500.);
              }
            }
            if (sm_channel[i] == "zmumu_" || sm_channel[i] == "zee_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"mll"+m_sysName, 150, 0., 300.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep1_pt"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep2_pt"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep1_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep2_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep1_phi"+m_sysName, 20, -3.2, 3.2);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep2_phi"+m_sysName, 20, -3.2, 3.2);
            }
            if (sm_channel[i] == "wmunu_" || sm_channel[i] == "wenu_") {
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"mT"+m_sysName, 150, 0., 300.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep_pt"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep_eta"+m_sysName, 25, -5., 5.);
              addHist(hMap1D, "SM_study_"+sm_channel[i]+sm_level[j]+sm_monojet[k]+"lep_phi"+m_sysName, 20, -3.2, 3.2);
            }
          }
        }
      }
      // Trigger Efficiency plots (turn-on curve)
      if (m_sysName=="") { // No systematic
        const int eff_channel_n = 3;
        const int eff_level_n = 1;
        const int eff_monojet_n = 2;
        std::string eff_channel[eff_channel_n] = {"znunu_","zmumu_","wmunu_"};
        std::string eff_level[eff_level_n] = {"trig_eff_reco_"};
        std::string eff_monojet[eff_monojet_n] = {"exclusive_","inclusive_"};
        for(int i=0; i < eff_channel_n; i++) {
          for(int j=0; j < eff_level_n; j++) {
            for(int k=0; k < eff_monojet_n; k++) {
              // Before passing Trigger
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe70_mht"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe90_mht_L1XE50"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe110_mht_L1XE50"+m_sysName, 140, 0., 1400.);
              // After passing Trigger
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe70_mht"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe90_mht_L1XE50"+m_sysName, 140, 0., 1400.);
              addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe110_mht_L1XE50"+m_sysName, 140, 0., 1400.);
              // Do not pass reference trigger (Single muon trigger)
              if (eff_channel[i] == "zmumu_") {
                // Before passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe70_mht_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe90_mht_L1XE50_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_for_HLT_xe110_mht_L1XE50_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
                // After passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe70_mht_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe90_mht_L1XE50_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"met_pass_HLT_xe110_mht_L1XE50_NoPassMuTrig"+m_sysName, 140, 0., 1400.);
              }
              // For publication bins
              if (eff_monojet[k] == "exclusive_") {
                // Before passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe70_mht"+m_sysName, ex_nbinMET, ex_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe90_mht_L1XE50"+m_sysName, ex_nbinMET, ex_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe110_mht_L1XE50"+m_sysName, ex_nbinMET, ex_binsMET);
                // After passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe70_mht"+m_sysName, ex_nbinMET, ex_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe90_mht_L1XE50"+m_sysName, ex_nbinMET, ex_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe110_mht_L1XE50"+m_sysName, ex_nbinMET, ex_binsMET);
              }
              if (eff_monojet[k] == "inclusive_") {
                // Before passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe70_mht"+m_sysName, in_nbinMET, in_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe90_mht_L1XE50"+m_sysName, in_nbinMET, in_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_for_HLT_xe110_mht_L1XE50"+m_sysName, in_nbinMET, in_binsMET);
                // After passing Trigger
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe70_mht"+m_sysName, in_nbinMET, in_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe90_mht_L1XE50"+m_sysName, in_nbinMET, in_binsMET);
                addHist(hMap1D, "SM_study_"+eff_channel[i]+eff_level[j]+eff_monojet[k]+"MET_mono_pass_HLT_xe110_mht_L1XE50"+m_sysName, in_nbinMET, in_binsMET);
              }
            }
          }
        }
      }





    /*
    // Jet calibration test
    addHist(hMap1D, "Uncalibrated_jet_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "Calibrated_jet_pt"+m_sysName, 60, 0., 3000.);

      // Muon calibration test
      addHist(hMap1D, "Uncalibrated_muon_pt"+m_sysName, 30, 0., 1500.);
      addHist(hMap1D, "Calibrated_muon_pt"+m_sysName, 30, 0., 1500.);

      // Electron calibration test
      addHist(hMap1D, "Uncalibrated_electron_pt"+m_sysName, 30, 0., 1500.);
      addHist(hMap1D, "Calibrated_electron_pt"+m_sysName, 30, 0., 1500.);

      // Photon calibration test
      addHist(hMap1D, "Uncalibrated_photon_pt"+m_sysName, 30, 0., 1500.);
      addHist(hMap1D, "Calibrated_photon_pt"+m_sysName, 30, 0., 1500.);

      // Tau smearing test
      addHist(hMap1D, "Unsmeared_tau_pt"+m_sysName, 30, 0., 1500.);
      addHist(hMap1D, "Smeared_tau_pt"+m_sysName, 30, 0., 1500.);
      */


      //////////////////
      // Cutflow Test //
      //////////////////
      if (m_sysName=="" && m_useArrayCutflow) {
        addHist(hMap1D, "h_cutflow_vbf_met_emulmet"+m_sysName, nbinMET, binsMET);
        addHist(hMap1D, "h_cutflow_vbf_mjj"+m_sysName, nbinMjj, binsMjj);
        addHist(hMap1D, "h_cutflow_vbf_dPhijj"+m_sysName, nbinDPhi, binsDPhi);
      }


      ///////////////////////////
      // Exotic Reco Histogram //
      ///////////////////////////
      const int channel_num = 3;
      const int prefix_num = 1;
      std::string channel[channel_num] = {"h_znunu_","h_zee_","h_zmumu_"};
      std::string PreFix[prefix_num] = {""};
      for(int i=0; i < channel_num; i++) {
        for(int j=0; j < prefix_num; j++) {
          addHist(hMap1D, channel[i]+PreFix[j]+"MET_mono"+m_sysName, nbinMET, binsMET);
          addHist(hMap1D, channel[i]+PreFix[j]+"MET_search"+m_sysName, nbinMET, binsMET);
          addHist(hMap1D, channel[i]+PreFix[j]+"Mjj_search"+m_sysName, nbinMjj, binsMjj);
          addHist(hMap1D, channel[i]+PreFix[j]+"DeltaPhiAll"+m_sysName, nbinDPhi, binsDPhi);
        } 
      }

      ///////////////////////////////////////
      // Truth-related Reco Historam for Cz//
      ///////////////////////////////////////
      if (!m_isData) { // MC
        const int channel_num = 1;
        const int prefix_num = 6;
        std::string channel[channel_num] = {"h_zmumu_"};
        std::string PreFix[prefix_num] = {"RecoJetsTruthMETTruthCuts_","RecoJetsTruthMETRecoCuts_","GoodJetsTruthMETTruthCuts_","GoodJetsTruthMETRecoCuts_","TruthJetsTruthMETTruthCuts_","TruthJetsTruthMETRecoCuts_"};
        for(int i=0; i < channel_num; i++) {
          for(int j=0; j < prefix_num; j++) {
            addHist(hMap1D, channel[i]+PreFix[j]+"MET_mono"+m_sysName, nbinMET, binsMET);
            addHist(hMap1D, channel[i]+PreFix[j]+"MET_search"+m_sysName, nbinMET, binsMET);
            addHist(hMap1D, channel[i]+PreFix[j]+"Mjj_search"+m_sysName, nbinMjj, binsMjj);
            addHist(hMap1D, channel[i]+PreFix[j]+"DeltaPhiAll"+m_sysName, nbinDPhi, binsDPhi);
          } 
        }
      } // MC

      // Test
      addHist(hMap1D, "h_monojet_truth_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_monojet_truth_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);
      addHist(hMap1D, "h_VBF_truth_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_VBF_truth_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);

      addHist(hMap1D, "h_monojet_ORtruth_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_monojet_ORtruth_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);
      addHist(hMap1D, "h_VBF_ORtruth_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_VBF_ORtruth_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);

      addHist(hMap1D, "h_monojet_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_monojet_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);
      addHist(hMap1D, "h_VBF_goodJetORTruthMuMu_pt"+m_sysName, 60, 0., 3000.);
      addHist(hMap1D, "h_VBF_truthPseudoMETZmumu"+m_sysName, nbinMET, binsMET);



    } // end of systematics loop

  } // m_doReco


  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  // push cutflow bitset to cutflow hist
  if (m_useBitsetCutflow)
    m_BitsetCutflow->PushBitSet();


  m_store = wk()->xaodStore();

  //----------------------------
  // Event information
  //--------------------------- 
  const xAOD::EventInfo* eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info collection in initialise. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  if (m_doReco) {

    // Apply the prwTool first before calling the efficiency correction methods
    if (!m_isData) {
      m_prwTool->apply(*eventInfo, true); // If you specify the false option to getRandomRunNumber (or to apply), the tool will not use the mu-dependency. This is not recommended though!
    }

    // Get run number for both Data and MC
    // The following example assumes that you have instantiated the PileupReweightingTool and called the apply() method.
    unsigned int m_runNumber = 0;
    // case where the event is data
    if (m_isData) {
      ANA_MSG_DEBUG("The current event is a data event. Return m_runNumber instead.");
      m_runNumber = eventInfo->runNumber();
    }
    // case where the event is simulation (MC)
    else {
      if( eventInfo->auxdecor<unsigned int>("RandomRunNumber") ) {
        m_runNumber = eventInfo->auxdecor<unsigned int>("RandomRunNumber");
      } else {
        ANA_MSG_WARNING("Failed to find the RandomRunNumber decoration. Please call the apply() method from the PileupReweightingTool before hand in order to get period dependent SFs. You'll receive SFs from the most recent period.");
        m_runNumber=9999999;
      }
    }

    m_dataYear = "";
    m_run2016Period = "";
    // 2015 Dataset
    if (m_runNumber >= 276262 && m_runNumber <= 284484) {
      m_dataYear = "2015";
    }
    // 2016 Dataset
    else if (m_runNumber >= 297730 && m_runNumber <= 311481) {
      m_dataYear = "2016";
      // Period A ~ D3 (297730~302872)
      if (m_runNumber <= 302872) m_run2016Period = "AtoD3";
      // Period D4 ~ L (302919~311481)
      if (m_runNumber >= 302919) m_run2016Period = "D4toL";
    }
    // 2017 Dataset
    else if (m_runNumber >= 325713 && m_runNumber <= 340453 ) {
      m_dataYear = "2017";
    }


    //std::cout << "[execute] run Number = " << m_runNumber << endl;
    //std::cout << "[execute] m_dataYear = " << m_dataYear << endl;

  } // Get run number when m_doReco


  // Calculate EventWeight
  m_mcEventWeight = 1.;
  float mcWeight = 1.;
  if (!m_isData) {
    mcWeight = eventInfo->mcEventWeight();
    m_mcEventWeight = mcWeight;
  }


  // print every 100 events, so we know where we are:
  //if( (m_eventCounter % 100) ==0 ) Info("execute()", "Event number = %i and Lumi Block number = %i", m_eventCounter, eventInfo->lumiBlock() );
  m_eventCounter++;


  if (m_useArrayCutflow) m_eventCutflow[0]+=1;
  if (m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Total");

  // if data check if event passes GRL
  if(m_isData && m_fileType != "skim"){ // it's data!
    if(!m_grl->passRunLB(*eventInfo)){
      return EL::StatusCode::SUCCESS; // go to next event
    }
  } // end if Data
  if (m_useArrayCutflow) m_eventCutflow[1]+=1;
  if (m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("GRL");

  // Print Lumi Block numbers
  //Info("execute()", "Lumi Block number passing GRL= %i", eventInfo->lumiBlock() );

  // -------------------------------------------------
  // Apply event cleaning to remove events due to 
  // problematic regions of the detector 
  // or incomplete events.
  // Apply to data.
  // -------------------------------------------------
  // reject event if:
  if(m_isData){ // it's data!
    if(   (eventInfo->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error ) 
        || (eventInfo->errorState(xAOD::EventInfo::Tile)==xAOD::EventInfo::Error )
        || (eventInfo->errorState(xAOD::EventInfo::SCT)==xAOD::EventInfo::Error )
        || (eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18) ) )
    {
      return EL::StatusCode::SUCCESS; // go to the next event
    } // end if event flags check
  } // end if the event is data
  if (m_useArrayCutflow) m_eventCutflow[2]+=1;
  if (m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("LAr_Tile_Core");


  // -----------------
  // Batman Cleaning
  // -----------------
  // IsBadBatMan Event Flag and EMEC-IW Saturation Problem
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/HowToCleanJets2016#IsBadBatMan_Event_Flag_and_EMEC
  // It has been observed that there were problems with saturation
  // in the EMEC-IW cells in the high pile-up runs in 2015+2016.
  // ---------------------------------------------------------------
  // reject event if:
  if(m_isData && (m_dataYear == "2015" || m_dataYear == "2016")){ // For only 2015 and 2016 data (do not apply with 2017 data)
    if ((bool) eventInfo->auxdata<char>("DFCommonJets_isBadBatman") ) {
      return EL::StatusCode::SUCCESS; // go to the next event
    }
  }
  if (m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Batman Cleaning");


/*
  // examine the HLT_xe80* chains, see if they passed/failed and their total prescale
  //auto chainGroup = m_trigDecisionTool->getChainGroup("HLT_xe.*");
  auto chainGroup = m_trigDecisionTool->getChainGroup("HLT_xe80_tc_lcw_L1XE50");
  std::map<std::string,int> triggerCounts;
  for(auto &trig : chainGroup->getListOfTriggers()) {
    auto cg = m_trigDecisionTool->getChainGroup(trig);
    std::string thisTrig = trig;
    Info( "execute()", "%30s chain passed(1)/failed(0): %d total chain prescale (L1*HLT): %.1f", thisTrig.c_str(), cg->isPassed(), cg->getPrescale() );
  } // end for loop (c++11 style) over chain group matching "HLT_xe80.*" 
*/


  //-----------------------
  // Trigger Decision
  //-----------------------
  m_met_trig_fire = false;
  m_ele_trig_fire = false;
  m_mu_trig_fire = false;
  // MET Triggers
  if (m_useArrayCutflow) { // For Exotic cutflow study
    m_met_trig_fire = ( (m_dataYear == "2015" && m_trigDecisionTool->isPassed("HLT_xe70")) ||
        (m_dataYear == "2016" && ( m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe100_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe130_mht_L1XE50") )) );
  } else { // For SM study
    m_met_trig_fire = ( (m_dataYear == "2015" && m_trigDecisionTool->isPassed("HLT_xe70_mht")) ||
        ( m_dataYear == "2016" && (m_run2016Period == "AtoD3" &&  m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50")) || (m_run2016Period == "D4toL" && m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50")) ) );
  }

  // Single Electron Triggers
  m_ele_trig_fire = ( (m_dataYear == "2015" && ( ((!m_isData && m_trigDecisionTool->isPassed("HLT_e24_lhmedium_L1EM18VH")) || (m_isData && m_trigDecisionTool->isPassed("HLT_e24_lhmedium_L1EM20VH")) ) || m_trigDecisionTool->isPassed("HLT_e60_lhmedium") || m_trigDecisionTool->isPassed("HLT_e120_lhloose") )) ||
                      (m_dataYear == "2016" && ( m_trigDecisionTool->isPassed("HLT_e26_lhtight_nod0_ivarloose") || m_trigDecisionTool->isPassed("HLT_e60_lhmedium_nod0") || m_trigDecisionTool->isPassed("HLT_e140_lhloose_nod0") )) );

  // Single Muon Triggers
  m_mu_trig_fire = ( (m_dataYear == "2015" && ( m_trigDecisionTool->isPassed("HLT_mu20_iloose_L1MU15") || m_trigDecisionTool->isPassed("HLT_mu50") )) ||
                     (m_dataYear == "2016" && ( m_trigDecisionTool->isPassed("HLT_mu26_ivarmedium") || m_trigDecisionTool->isPassed("HLT_mu50") )) );


  /*
  std::cout << "MET trigger fired : " << m_met_trig_fire << std::endl;
  std::cout << "Electron trigger fired : " << m_ele_trig_fire << std::endl;
  std::cout << "Muon trigger fired : " << m_mu_trig_fire << std::endl;
  */

  //-----------------------------------------------------------
  // For Skim STDM4 samples using EXOT5 derivation skim method
  //-----------------------------------------------------------
  // Skim pass
  passUncalibMonojetCut = false;
  passRecoJetCuts = false;
  passTruthJetCuts = false;


  //------------------------------------
  // Global variable for Cz calulation
  //------------------------------------

  // Truth Zmumu
  m_passTruthNoMetZmumu = false; // Pass truth Zmumu cut, but no MET cut
  m_truthPseudoMETZmumu = -9.F;
  m_truthPseudoMETPhiZmumu = -9.F;












  //--------------------
  // MC Truth selection
  //--------------------


  // real MET
  float truthMET;
  float truthMET_phi;
  // Znunu
  float truth_neutrino1_pt = 0.;
  float truth_neutrino2_pt = 0.;
  float m_truthEmulMETZnunu = 0.;
  float m_truthEmulMETPhiZnunu = 0.;
  // Zmumu
  float truth_mll_muon = 0.;
  float truth_muon1_pt = 0.;
  float truth_muon2_pt = 0.;
  bool pass_truth_dimuonCut = false; // dimuon cut
  bool pass_truth_OSmuon = false; // Opposite sign charge muon
  float m_truthEmulMETZmumu = 0.;
  float m_truthEmulMETPhiZmumu = 0.;
  float m_truthMll_muon = 0.;
  // Zee
  float truth_mll_electron = 0.;
  float truth_electron1_pt = 0.;
  float truth_electron2_pt = 0.;
  bool pass_truth_dielectronCut = false; // dielectron cut
  bool pass_truth_OSelectron = false; // Opposite sign charge electron
  float m_truthEmulMETZee = 0.;
  float m_truthEmulMETPhiZee = 0.;
  float m_truthMll_electron = 0.;
  // Monojet
  float truth_monojet_pt = 0;
  float truth_monojet_phi = 0;
  float truth_monojet_eta = 0;
  float truth_monojet_rapidity = 0;
  float truth_dPhiMonojetMet = 0;
  float truth_dPhiMonojetMet_Zmumu = 0;
  float truth_dPhiMonojetMet_Zee = 0;
  // Dijet
  TLorentzVector truth_jet1;
  TLorentzVector truth_jet2;
  float truth_jet1_pt = 0;
  float truth_jet2_pt = 0;
  float truth_jet3_pt = 0;
  float truth_jet1_phi = 0;
  float truth_jet2_phi = 0;
  float truth_jet3_phi = 0;
  float truth_jet1_eta = 0;
  float truth_jet2_eta = 0;
  float truth_jet3_eta = 0;
  float truth_jet1_rapidity = 0;
  float truth_jet2_rapidity = 0;
  float truth_jet3_rapidity = 0;

  float truth_Jet_ht = 0;
  float truth_dPhiJet1Met = 0;
  float truth_dPhiJet2Met = 0;
  float truth_dPhiJet3Met = 0;
  float truth_dPhiJet1Met_Zmumu = 0;
  float truth_dPhiJet2Met_Zmumu = 0;
  float truth_dPhiJet3Met_Zmumu = 0;
  float truth_dPhiJet1Met_Zee = 0;
  float truth_dPhiJet2Met_Zee = 0;
  float truth_dPhiJet3Met_Zee = 0;

  float truth_mjj = 0;
  bool pass_truth_monoJet = false; // Select monoJet
  bool pass_truth_diJet = false; // Select DiJet
  bool pass_truth_CJV = true; // Central Jet Veto (CJV)
  bool pass_truth_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  bool pass_truth_dPhijetmet_Zmumu = true; // deltaPhi(Jet_i,MET_Zmumu)
  bool pass_truth_dPhijetmet_Zee = true; // deltaPhi(Jet_i,MET_Zee)
  float truth_dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
  float truth_dPhiMinjetmet_Zmumu = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
  float truth_dPhiMinjetmet_Zee = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)

  // For dressed-level jets
  // MonoWZjet
  float truth_monoWZjet_pt = 0;
  float truth_monoWZjet_phi = 0;
  float truth_monoWZjet_eta = 0;
  float truth_monoWZjet_rapidity = 0;
  float truth_dPhiMonoWZjetMet = 0;
  float truth_dPhiMonoWZjetMet_Zmumu = 0;
  float truth_dPhiMonoWZjetMet_Zee = 0;
  // DiWZjet
  TLorentzVector truth_WZjet1;
  TLorentzVector truth_WZjet2;
  float truth_WZjet1_pt = 0;
  float truth_WZjet2_pt = 0;
  float truth_WZjet3_pt = 0;
  float truth_WZjet1_phi = 0;
  float truth_WZjet2_phi = 0;
  float truth_WZjet3_phi = 0;
  float truth_WZjet1_eta = 0;
  float truth_WZjet2_eta = 0;
  float truth_WZjet3_eta = 0;
  float truth_WZjet1_rapidity = 0;
  float truth_WZjet2_rapidity = 0;
  float truth_WZjet3_rapidity = 0;

  float truth_WZJet_ht = 0;
  float truth_dPhiWZJet1Met = 0;
  float truth_dPhiWZJet2Met = 0;
  float truth_dPhiWZJet3Met = 0;
  float truth_dPhiWZJet1Met_Zmumu = 0;
  float truth_dPhiWZJet2Met_Zmumu = 0;
  float truth_dPhiWZJet3Met_Zmumu = 0;
  float truth_dPhiWZJet1Met_Zee = 0;
  float truth_dPhiWZJet2Met_Zee = 0;
  float truth_dPhiWZJet3Met_Zee = 0;

  float truth_WZmjj = 0;
  bool pass_truth_monoWZJet = false; // Select monoJet
  bool pass_truth_diWZJet = false; // Select DiJet
  bool pass_truth_WZ_CJV = true; // Central Jet Veto (CJV)
  bool pass_truth_dPhiWZjetmet = true; // deltaPhi(Jet_i,MET)
  bool pass_truth_dPhiWZjetmet_Zmumu = true; // deltaPhi(Jet_i,MET_Zmumu)
  bool pass_truth_dPhiWZjetmet_Zee = true; // deltaPhi(Jet_i,MET_Zee)
  float truth_dPhiMinWZjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
  float truth_dPhiMinWZjetmet_Zmumu = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
  float truth_dPhiMinWZjetmet_Zee = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)


  /////////////////
  /// deep copy ///
  /////////////////
  // create the new truth container and its auxiliary store.
  xAOD::TruthParticleContainer* m_bornTruthMuon = new xAOD::TruthParticleContainer();
  xAOD::AuxContainerBase* m_bornTruthMuonAux = new xAOD::AuxContainerBase();
  m_bornTruthMuon->setStore( m_bornTruthMuonAux ); //< Connect the two

  xAOD::TruthParticleContainer* m_bornTruthElectron = new xAOD::TruthParticleContainer();
  xAOD::AuxContainerBase* m_bornTruthElectronAux = new xAOD::AuxContainerBase();
  m_bornTruthElectron->setStore( m_bornTruthElectronAux ); //< Connect the two

  xAOD::TruthParticleContainer* m_bareTruthMuon = new xAOD::TruthParticleContainer();
  xAOD::AuxContainerBase* m_bareTruthMuonAux = new xAOD::AuxContainerBase();
  m_bareTruthMuon->setStore( m_bareTruthMuonAux ); //< Connect the two

  xAOD::TruthParticleContainer* m_bareTruthElectron = new xAOD::TruthParticleContainer();
  xAOD::AuxContainerBase* m_bareTruthElectronAux = new xAOD::AuxContainerBase();
  m_bareTruthElectron->setStore( m_bareTruthElectronAux ); //< Connect the two

  m_selectedTruthNeutrino = new xAOD::TruthParticleContainer();
  m_selectedTruthNeutrinoAux = new xAOD::AuxContainerBase();
  m_selectedTruthNeutrino->setStore( m_selectedTruthNeutrinoAux ); //< Connect the two

  m_dressedTruthMuon = new xAOD::TruthParticleContainer();
  m_dressedTruthMuonAux = new xAOD::AuxContainerBase();
  m_dressedTruthMuon->setStore( m_dressedTruthMuonAux ); //< Connect the two

  m_dressedTruthElectron = new xAOD::TruthParticleContainer();
  m_dressedTruthElectronAux = new xAOD::AuxContainerBase();
  m_dressedTruthElectron->setStore( m_dressedTruthElectronAux ); //< Connect the two

  m_selectedTruthTau = new xAOD::TruthParticleContainer();
  m_selectedTruthTauAux = new xAOD::AuxContainerBase();
  m_selectedTruthTau->setStore( m_selectedTruthTauAux ); //< Connect the two

  m_selectedTruthJet = new xAOD::JetContainer();
  m_selectedTruthJetAux = new xAOD::AuxContainerBase();
  m_selectedTruthJet->setStore( m_selectedTruthJetAux ); //< Connect the two

  if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
    m_selectedTruthWZJet = new xAOD::JetContainer();
    m_selectedTruthWZJetAux = new xAOD::AuxContainerBase();
    m_selectedTruthWZJet->setStore( m_selectedTruthWZJetAux ); //< Connect the two
  }

  // EXOT5 Method Skim for STDM4 sample
  xAOD::JetContainer* m_truthJet = new xAOD::JetContainer();
  xAOD::AuxContainerBase* m_truthJetAux = new xAOD::AuxContainerBase();
  m_truthJet->setStore( m_truthJetAux ); //< Connect the two





  // Truth level analysis
  if (!m_isData && m_doTruth && (m_dataType.find("EXOT")!=std::string::npos || m_dataType.find("STDM")!=std::string::npos)) { // MC Truth and EXOT5 or STDM4

    const xAOD::TruthEventContainer* m_truthEvents = nullptr;
    if ( !m_event->retrieve( m_truthEvents, "TruthEvents" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthEvents container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::JetContainer* m_truthJets = nullptr;
    if ( !m_event->retrieve( m_truthJets, "AntiKt4TruthJets" ).isSuccess() ){
      Error("execute()", "Failed to retrieve AntiKt4TruthJets container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::MissingETContainer*  m_truthMET = nullptr;
    if ( !m_event->retrieve( m_truthMET, "MET_Truth" ).isSuccess() ){
      Error("execute()", "Failed to retrieve MET_Truth container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::MissingET* truthmet = (*m_truthMET)["NonInt"];
    truthMET = truthmet->met();
    truthMET_phi = truthmet->phi();

    const xAOD::TruthParticleContainer* m_truthNeutrinos = nullptr;
    if ( !m_event->retrieve( m_truthNeutrinos, m_nameDerivation+"TruthNeutrinos" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthNeutrinos container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::TruthParticleContainer* m_truthMuons = nullptr;
    if ( !m_event->retrieve( m_truthMuons, m_nameDerivation+"TruthMuons" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthMuons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::TruthParticleContainer* m_truthElectrons = nullptr;
    if ( !m_event->retrieve( m_truthElectrons, m_nameDerivation+"TruthElectrons" ).isSuccess() ){
      Error("execute()", "Failed to retrieve truthElectrons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    const xAOD::TruthParticleContainer* m_truthTaus = nullptr;
    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
      if ( !m_event->retrieve( m_truthTaus, m_nameDerivation+"TruthTaus" ).isSuccess() ){
        Error("execute()", "Failed to retrieve TruthTaus container. Exiting." );
        return EL::StatusCode::FAILURE;
      }
    } else {
      if ( !m_event->retrieve( m_truthTaus, "TruthTaus" ).isSuccess() ){
        Error("execute()", "Failed to retrieve TruthTaus container. Exiting." );
        return EL::StatusCode::FAILURE;
      }
    }




    //------------------
    // Truth Particles
    //------------------
    bool isEW = 0;
    for (const auto &truthEvent : *m_truthEvents) { // Interaction loop in an event
      int nPart = truthEvent->nTruthParticles();
      for (int iPart = 0;iPart<nPart;iPart++) { // Particle loop in an interaction
        const xAOD::TruthParticle* particle = truthEvent->truthParticle(iPart);
        int status = 0, pdgId = 0;
        float ppt=0.;
        if (particle) status = particle->status();
        if (particle) pdgId = particle->pdgId();
        if (particle) ppt = particle->pt();

        // Test truth content for a few events
        if (m_eventCounter<20 && particle) {
          //std::cout<<"TruthPart: Evt: "<<m_eventCounter<<", Part: "<<iPart<<", ID: "<<pdgId<<", status: "<<status<<", pt: "<<ppt*0.001<<std::endl;
        }
        if (pdgId == 23 || pdgId == 24 || pdgId == -24) isEW = 1;


        //////////////////
        // Born leptons //
        //////////////////
        if (particle) {
          if ( (fabs(pdgId)==11 ||fabs(pdgId)==13)  && status==11 ){
            // Born in Sherpa: status = 11 (documentation particle (not necessarily physical), e.g. particles from the parton shower, intermediate states, helper particles, etc.)
            // Born in MG: status = 23

            // Find mother
            bool isFromZ = false;
            if (particle->hasProdVtx() && particle->prodVtx() && particle->prodVtx()->nIncomingParticles() != 0 ) {
              const xAOD::TruthVertex * prodVtx = particle->prodVtx();

              auto numIncoming = prodVtx->nIncomingParticles();
              for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
                const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
                int motherAbsPdgId = mother->absPdgId();
                if ( motherAbsPdgId == 23 ) isFromZ = true;
                //if (m_eventCounter<20) std::cout << "mother ID : " << motherAbsPdgId << ", pt : " << ppt << ", which is from Z? : " << isFromZ << std::endl;
              }

            }


            if (fabs(pdgId)==11) {
              xAOD::TruthParticle* bornTruthElectron = new xAOD::TruthParticle();
              bornTruthElectron->makePrivateStore(*particle);
              m_bornTruthElectron->push_back(bornTruthElectron);
            } else if (fabs(pdgId)==13) {
              xAOD::TruthParticle* bornTruthMuon = new xAOD::TruthParticle();
              bornTruthMuon->makePrivateStore(*particle);
              m_bornTruthMuon->push_back(bornTruthMuon);
              //if (m_eventCounter<20) std::cout << "Born muon ID : " << pdgId << ", pt : " << ppt << ", which is from Z? : " << isFromZ << std::endl;
            }

          }
        }


      } // Particle loop end

    } // Interaction loop end

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_bornTruthMuon, "bornTruthMuon" ));
    ANA_CHECK(m_store->record( m_bornTruthMuonAux, "bornTruthMuonAux." ));
    ANA_CHECK(m_store->record( m_bornTruthElectron, "bornTruthElectron" ));
    ANA_CHECK(m_store->record( m_bornTruthElectronAux, "bornTruthElectronAux." ));

    // Sort born Truth leptons
    if (m_bornTruthMuon->size() > 1) std::partial_sort(m_bornTruthMuon->begin(), m_bornTruthMuon->begin()+2, m_bornTruthMuon->end(), DescendingPt());
    if (m_bornTruthElectron->size() > 1) std::partial_sort(m_bornTruthElectron->begin(), m_bornTruthElectron->begin()+2, m_bornTruthElectron->end(), DescendingPt());



    /*
    if (m_eventCounter<20) {
      Info("execute()", "========================================");
      Info("execute()", " Event # = %llu", eventInfo->eventNumber());
      int bornMuCount = 0;
      for (const auto& muon : *m_bornTruthMuon) {
        Info("execute()", " born muon # : %i", bornMuCount);
        Info("execute()", " born muon pt = %.3f GeV", muon->pt() * 0.001);
        Info("execute()", " born muon eta = %.3f", muon->eta());
        Info("execute()", " born muon phi = %.3f", muon->phi());
        bornMuCount++;
      }
    }
    */





    //----------------
    // Truth Neutrinos
    //----------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_neutrino_shallowCopy = xAOD::shallowCopyContainer( *m_truthNeutrinos );
    xAOD::TruthParticleContainer* truth_neutrinoSC = truth_neutrino_shallowCopy.first;

    TLorentzVector truthNeutrinoVector;

    for (const auto &neutrino : *truth_neutrinoSC) {
      truthNeutrinoVector += neutrino->p4();
      if (std::abs(neutrino->auxdata<int>("motherID")) < 111 && std::abs(neutrino->auxdata<int>("motherID")) != 15) {
        xAOD::TruthParticle* truthNeutrino = new xAOD::TruthParticle();
        truthNeutrino->makePrivateStore(*neutrino);
        m_selectedTruthNeutrino->push_back(truthNeutrino);
      }
    } 

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_selectedTruthNeutrino, "selectedTruthNeutrino" ));
    ANA_CHECK(m_store->record( m_selectedTruthNeutrinoAux, "selectedTruthNeutrinoAux." ));

    delete truth_neutrino_shallowCopy.first;
    delete truth_neutrino_shallowCopy.second;

    // Sort Truth neutrino
    if (m_selectedTruthNeutrino->size() > 1) std::partial_sort(m_selectedTruthNeutrino->begin(), m_selectedTruthNeutrino->begin()+2, m_selectedTruthNeutrino->end(), DescendingPt());



    //-------------
    // Truth Muons
    //-------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_muon_shallowCopy = xAOD::shallowCopyContainer( *m_truthMuons );
    xAOD::TruthParticleContainer* truth_muonSC = truth_muon_shallowCopy.first;

    // truth muon selection: note that I don't require pt and eta cut for this container because I will use truth level leptons with pT and eta cuts loosened by 10% of their default cut values.
    for (const auto &muon : *truth_muonSC) {

      // Store bare Muons
      xAOD::TruthParticle* bareMuon = new xAOD::TruthParticle();
      m_bareTruthMuon->push_back( bareMuon );
      *bareMuon = *muon; // copies auxdata from one auxstore to the other

      // Store dressed Muons
      if (std::abs(muon->auxdata<int>("motherID")) < 111 && std::abs(muon->auxdata<int>("motherID")) != 15) {
        TLorentzVector fourVector;
        fourVector.SetPtEtaPhiE(muon->auxdata<float>("pt_dressed"), muon->auxdata<float>("eta_dressed"), muon->auxdata<float>("phi_dressed"), muon->auxdata<float>("e_dressed"));
        //if (m_eventCounter<20) std::cout << "original muon pt = " << muon->pt() << "  muon pt_dressed = " << muon->auxdata<float>("pt_dressed") << endl;
        muon->setE(fourVector.E());
        muon->setPx(fourVector.Px());
        muon->setPy(fourVector.Py());
        muon->setPz(fourVector.Pz());
        xAOD::TruthParticle* dressedMuon = new xAOD::TruthParticle();
        dressedMuon->makePrivateStore(*muon);
        m_dressedTruthMuon->push_back(dressedMuon);
      }

    } 

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_bareTruthMuon, "bareTruthMuon" ));
    ANA_CHECK(m_store->record( m_bareTruthMuonAux, "bareTruthMuonAux." ));
    ANA_CHECK(m_store->record( m_dressedTruthMuon, "dressedTruthMuon" ));
    ANA_CHECK(m_store->record( m_dressedTruthMuonAux, "dressedTruthMuonAux." ));

    delete truth_muon_shallowCopy.first;
    delete truth_muon_shallowCopy.second;

    // Sort truth muons
    if (m_bareTruthMuon->size() > 1) std::partial_sort(m_bareTruthMuon->begin(), m_bareTruthMuon->begin()+2, m_bareTruthMuon->end(), DescendingPt());
    if (m_dressedTruthMuon->size() > 1) std::partial_sort(m_dressedTruthMuon->begin(), m_dressedTruthMuon->begin()+2, m_dressedTruthMuon->end(), DescendingPt());



    /*
    if (m_eventCounter<20) {
      Info("execute()", "-------------------------------");
      int bareMuCount = 0;
      for (const auto& muon : *m_bareTruthMuon) {
        Info("execute()", " bare muon # : %i", bareMuCount);
        Info("execute()", " bare muon pt = %.3f GeV", muon->pt() * 0.001);
        Info("execute()", " bare muon eta = %.3f", muon->eta());
        Info("execute()", " bare muon phi = %.3f", muon->phi());
        bareMuCount++;
      }
      Info("execute()", "-------------------------------");
      int dressedMuCount = 0;
      for (const auto& muon : *m_dressedTruthMuon) {
        Info("execute()", " dressed muon # : %i", dressedMuCount);
        Info("execute()", " dressed muon pt = %.3f GeV", muon->pt() * 0.001);
        Info("execute()", " dressed muon eta = %.3f", muon->eta());
        Info("execute()", " dressed muon phi = %.3f", muon->phi());
        Info("execute()", " dressed muon motherID = %i", muon->auxdata<int>("motherID") );
        dressedMuCount++;
      }
    }
    */


    //-----------------
    // Truth Electrons
    //-----------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_elec_shallowCopy = xAOD::shallowCopyContainer( *m_truthElectrons );
    xAOD::TruthParticleContainer* truth_elecSC = truth_elec_shallowCopy.first;

    // truth electron selection: note that I don't require pt and eta cut for this container because I will use truth level leptons with pT and eta cuts loosened by 10% of their default cut values.
    for (const auto &electron : *truth_elecSC) {

      // Store bare Electrons
      xAOD::TruthParticle* bareElectron = new xAOD::TruthParticle();
      m_bareTruthElectron->push_back( bareElectron );
      *bareElectron = *electron; // copies auxdata from one auxstore to the other

      // Store dressed Electrons
      if (std::abs(electron->auxdata<int>("motherID")) < 111 && std::abs(electron->auxdata<int>("motherID")) != 15) {
        TLorentzVector fourVector;
        fourVector.SetPtEtaPhiE(electron->auxdata<float>("pt_dressed"), electron->auxdata<float>("eta_dressed"), electron->auxdata<float>("phi_dressed"), electron->auxdata<float>("e_dressed"));
        electron->setE(fourVector.E());
        electron->setPx(fourVector.Px());
        electron->setPy(fourVector.Py());
        electron->setPz(fourVector.Pz());
        xAOD::TruthParticle* truthElectron = new xAOD::TruthParticle();
        truthElectron->makePrivateStore(*electron);
        m_dressedTruthElectron->push_back(truthElectron);
      }
    }

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_bareTruthElectron, "bareTruthElectron" ));
    ANA_CHECK(m_store->record( m_bareTruthElectronAux, "bareTruthElectronAux." ));
    ANA_CHECK(m_store->record( m_dressedTruthElectron, "dressedTruthElectron" ));
    ANA_CHECK(m_store->record( m_dressedTruthElectronAux, "dressedTruthElectronAux." ));

    delete truth_elec_shallowCopy.first;
    delete truth_elec_shallowCopy.second;

    // Sort Truth electrons
    if (m_bareTruthElectron->size() > 1) std::partial_sort(m_bareTruthElectron->begin(), m_bareTruthElectron->begin()+2, m_bareTruthElectron->end(), DescendingPt());
    if (m_dressedTruthElectron->size() > 1) std::partial_sort(m_dressedTruthElectron->begin(), m_dressedTruthElectron->begin()+2, m_dressedTruthElectron->end(), DescendingPt());



    //------------
    // Truth Taus
    //------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_tau_shallowCopy = xAOD::shallowCopyContainer( *m_truthTaus );
    xAOD::TruthParticleContainer* truth_tauSC = truth_tau_shallowCopy.first;

    for (const auto &tau : *truth_tauSC) {
      if (tau->auxdata<unsigned int>("classifierParticleOrigin") == 12 || tau->auxdata<unsigned int>("classifierParticleOrigin") == 13) {
        xAOD::TruthParticle* truthTau = new xAOD::TruthParticle();
        truthTau->makePrivateStore(*tau);
        m_selectedTruthTau->push_back(truthTau);
      }
    }

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_selectedTruthTau, "selectedTruthTau" ));
    ANA_CHECK(m_store->record( m_selectedTruthTauAux, "selectedTruthTauAux." ));

    delete truth_tau_shallowCopy.first;
    delete truth_tau_shallowCopy.second;





    //------------
    // Truth Jets
    //------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_jet_shallowCopy = xAOD::shallowCopyContainer( *m_truthJets );
    xAOD::JetContainer* truth_jetSC = truth_jet_shallowCopy.first;

    //if (m_truthNeutrinos->size() > 1) m_truthNeutrinoMET = truthNeutrinoVector.Pt();

    for (const auto &jet : *truth_jetSC) {

      // Store all truth jets for EXOT5 style Skim -> it will later be used for EXOT5 Skim cut
      if ( m_dataType.find("STDM")!=std::string::npos && m_doSkimEXOT5) { // STDM derivation
        xAOD::Jet* truthJet = new xAOD::Jet();
        m_truthJet->push_back( truthJet ); // jet acquires the m_goodJet auxstore
        *truthJet = *jet; // copies auxdata from one auxstore to the other
      }

      jet->auxdata<bool>("RecoJet") = false; // Decorate jet with RecoJet false -> means truth jet
      if (jet->pt() < 20000. || std::abs(jet->rapidity()) > 4.4) continue;
      bool fakeJet = false;
      float dRmin = 9.;
      for (const auto &muon : *m_dressedTruthMuon) {
        if (muon->pt() > 7000. &&  std::abs(muon->eta()) < 2.5){
          float dR = deltaR(jet->eta(), muon->eta(), jet->phi(), muon->phi());
          if (dR < dRmin) dRmin = dR;
          if (dR < m_ORJETdeltaR) fakeJet = true;
        }
      }
      for (const auto &electron : *m_dressedTruthElectron) {
        if (electron->pt() > 7000. &&  std::abs(electron->eta()) < 2.5 && deltaR(jet->eta(), electron->eta(), jet->phi(), electron->phi()) < m_ORJETdeltaR) fakeJet = true;
      }
      if (fakeJet) continue;

      // Store selected truth jets
      xAOD::Jet* selectedTruthJet = new xAOD::Jet();
      selectedTruthJet->makePrivateStore(*jet);
      m_selectedTruthJet->push_back(selectedTruthJet);
    }

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_selectedTruthJet, "selectedTruthJet" ));
    ANA_CHECK(m_store->record( m_selectedTruthJetAux, "selectedTruthJetAux." ));

    delete truth_jet_shallowCopy.first;
    delete truth_jet_shallowCopy.second;

    // Sort Truth jets
    if (m_selectedTruthJet->size() > 1) std::sort(m_selectedTruthJet->begin(), m_selectedTruthJet->end(), DescendingPt());


    /*
    if (m_eventCounter<20) {
      Info("execute()", "-------------------------------");
      int count = 0;
      for (const auto& jet : *m_selectedTruthJet) {
        Info("execute()", " truth jet #: %i", count);
        Info("execute()", " truth jet pt = %.3f GeV", jet->pt() * 0.001);
        Info("execute()", " truth jet eta = %.3f", jet->eta());
        Info("execute()", " truth jet phi = %.3f", jet->phi());
        count++;
      }
    }
    */




    /////////////////////////////////
    // Truth Jet decision for Skim //
    /////////////////////////////////
    // Sort truthJets
    if ( m_dataType.find("STDM")!=std::string::npos && m_doSkimEXOT5) { // STDM derivation
      if (m_truthJet->size() > 1) std::partial_sort(m_truthJet->begin(), m_truthJet->begin()+2, m_truthJet->end(), DescendingPt());

      float truthMjj = 0;
      if (m_truthJet->size() > 1) {
        TLorentzVector truthJet1 = m_truthJet->at(0)->p4();
        TLorentzVector truthJet2 = m_truthJet->at(1)->p4();
        auto truthDijet = truthJet1 + truthJet2;
        truthMjj = truthDijet.M();
      }

      if ((m_truthJet->size() > 0 && m_truthJet->at(0)->pt() > m_skimMonoJetPt) || (m_truthJet->size() > 1 && m_truthJet->at(0)->pt() > m_skimLeadingJetPt && m_truthJet->at(1)->pt() > m_skimSubleadingJetPt && truthMjj > m_skimMjj)) passTruthJetCuts = true;
    }

    // Delete copy containers
    delete m_truthJet;
    delete m_truthJetAux;





    //---------------------------------------
    // Truth WZJets (dressed-level truth jet)
    //---------------------------------------

    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)

      const xAOD::JetContainer* m_truthWZJets = nullptr;
      if ( !m_event->retrieve( m_truthWZJets, "AntiKt4TruthWZJets" ).isSuccess() ){
        Error("execute()", "Failed to retrieve AntiKt4TruthWZJets container. Exiting." );
        return EL::StatusCode::FAILURE;
      }

      /// shallow copy to retrive auxdata variables
      std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_WZjet_shallowCopy = xAOD::shallowCopyContainer( *m_truthWZJets );
      xAOD::JetContainer* truth_WZjetSC = truth_WZjet_shallowCopy.first;

      for (const auto &jet : *truth_WZjetSC) {
        
        //std::cout << "WZjet pt = " << jet->pt()*0.001 << std::endl;

        // Overlap removal
        jet->auxdata<bool>("RecoJet") = false; // Decorate jet with RecoJet false -> means truth jet
        if (jet->pt() < 20000. || std::abs(jet->rapidity()) > 4.4) continue;
        bool fakeJet = false;
        float dRmin = 9.;
        for (const auto &muon : *m_dressedTruthMuon) {
          if (muon->pt() > 7000. &&  std::abs(muon->eta()) < 2.5){
            float dR = deltaR(jet->eta(), muon->eta(), jet->phi(), muon->phi());
            if (dR < dRmin) dRmin = dR;
            if (dR < m_ORJETdeltaR) fakeJet = true;
          }
        }
        for (const auto &electron : *m_dressedTruthElectron) {
          if (electron->pt() > 7000. &&  std::abs(electron->eta()) < 2.5 && deltaR(jet->eta(), electron->eta(), jet->phi(), electron->phi()) < m_ORJETdeltaR) fakeJet = true;
        }
        if (fakeJet) continue;

        // Store selected truth WZ jets
        xAOD::Jet* truthWZJet = new xAOD::Jet();
        m_selectedTruthWZJet->push_back(truthWZJet);
        *truthWZJet = *jet;

      }

      // record your deep copied jet container (and aux container) to the store
      ANA_CHECK(m_store->record( m_selectedTruthWZJet, "selectedTruthWZJet" ));
      ANA_CHECK(m_store->record( m_selectedTruthWZJetAux, "selectedTruthWZJetAux." ));

      delete truth_WZjet_shallowCopy.first;
      delete truth_WZjet_shallowCopy.second;

    } // For STDM derivation


    // Sort Truth WZJets
    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
      if (m_selectedTruthWZJet->size() > 1) std::sort(m_selectedTruthWZJet->begin(), m_selectedTruthWZJet->end(), DescendingPt());
    }



    /*
    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
      if (m_eventCounter<20) {
      Info("execute()", "-------------------------------");
        int count = 0;
        for (const auto& jet : *m_selectedTruthWZJet) {
          Info("execute()", " truth WZjet #: %i", count);
          Info("execute()", " truth WZjet pt = %.3f GeV", jet->pt() * 0.001);
          Info("execute()", " truth WZjet eta = %.3f", jet->eta());
          Info("execute()", " truth WZjet phi = %.3f", jet->phi());
          count++;
        }
      }
    }
    */



    //----------------------
    // Truth born-level Jets
    //----------------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > antiKt4TruthJets_shallowCopy = xAOD::shallowCopyContainer( *m_truthJets );
    xAOD::JetContainer* antiKt4TruthJetSC = antiKt4TruthJets_shallowCopy.first;

    for (const auto &jet : *antiKt4TruthJetSC) {

      //std::cout << "antikt-jet pt = " << jet->pt()*0.001 << std::endl;

      // 4-vector sum of born-level jet excluding born-level muons
      TLorentzVector truth_jet = jet->p4();
      auto bornJet = truth_jet;
      for (const auto& muon : *m_bornTruthMuon) {
        TLorentzVector truth_muon = muon->p4();
        float dR = deltaR(truth_jet.Eta(), truth_muon.Eta(), truth_jet.Phi(), truth_muon.Phi());
        if (dR < 0.4) {
          bornJet = bornJet - truth_muon;
        }
      }

      // 4-vector sum of dress-level jet excluding dress-level muons
      auto dressJet = truth_jet;
      for (const auto& muon : *m_dressedTruthMuon) {
        TLorentzVector truth_muon = muon->p4();
        float dR = deltaR(truth_jet.Eta(), truth_muon.Eta(), truth_jet.Phi(), truth_muon.Phi());
        if (dR < 0.4) {
          //std::cout << "dressed-muon pt = " << truth_muon.Pt()*0.001 << "        <<<<<<<<<<=========================" <<std::endl;
          auto minus_truth_muon = -truth_muon;
          dressJet = dressJet + minus_truth_muon;
        }
      }
      //std::cout << "dress-jet pt = " << dressJet.Pt()*0.001 << std::endl;



/*
      // Overlap removal
      if (jet->pt() < 20000. || std::abs(jet->rapidity()) > 4.4) continue;
      bool fakeJet = false;
      float dRmin = 9.;
      for (const auto &muon : *m_bornTruthMuon) {
        if (muon->pt() > 7000. &&  std::abs(muon->eta()) < 2.5){
          float dR = deltaR(jet->eta(), muon->eta(), jet->phi(), muon->phi());
          if (dR < dRmin) dRmin = dR;
          if (dR < m_ORJETdeltaR) fakeJet = true;
        }
      }
      for (const auto &electron : *m_bornTruthElectron) {
        if (electron->pt() > 7000. &&  std::abs(electron->eta()) < 2.5 && deltaR(jet->eta(), electron->eta(), jet->phi(), electron->phi()) < m_ORJETdeltaR) fakeJet = true;
      }
      if (fakeJet) continue;

      // Store selected born-level truth jets
      xAOD::Jet* selectedBornTruthJet = new xAOD::Jet();
      selectedBornTruthJet->makePrivateStore(*jet);
      m_selectedBornTruthJet->push_back(selectedBornTruthJet);
*/
      

    }
/*
    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_selectedBornTruthJet, "selectedBornTruthJet" ));
    ANA_CHECK(m_store->record( m_selectedBornTruthJetAux, "selectedBornTruthJetAux." ));
*/
    delete antiKt4TruthJets_shallowCopy.first;
    delete antiKt4TruthJets_shallowCopy.second;
/*
    // Sort selected born-level Truth jets
    if (m_selectedBornTruthJet->size() > 1) std::sort(m_selectedBornTruthJet->begin(), m_selectedBornTruthJet->end(), DescendingPt());
*/









    ////////////////////////////////////////
    // Do Truth analysis for each Channel //
    ////////////////////////////////////////
    if (m_isZmumu) {
      doZmumuTruth(m_dressedTruthMuon, m_mcEventWeight, "nominal_truth_", "");
      doZmumuTruth(m_bornTruthMuon, m_mcEventWeight, "born_truth_", "");
      doZmumuTruth(m_bareTruthMuon, m_mcEventWeight, "bare_truth_", "");
      if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
        doZmumuTruth(m_dressedTruthMuon, m_mcEventWeight, "dress_truth_", "");
      }
    }

























    //---------------------------------------------
    // Define Truth Znunu, Zmumu and Zee Selection
    //---------------------------------------------

    // Znunu
    if (m_selectedTruthNeutrino->size() > 1) {

      TLorentzVector truth_neutrino1 = m_selectedTruthNeutrino->at(0)->p4();
      TLorentzVector truth_neutrino2 = m_selectedTruthNeutrino->at(1)->p4();
      truth_neutrino1_pt = truth_neutrino1.Perp();
      truth_neutrino2_pt = truth_neutrino2.Perp();
      auto truth_Znunu = truth_neutrino1 + truth_neutrino2;
      m_truthEmulMETZnunu = truth_Znunu.Pt();
      m_truthEmulMETPhiZnunu = truth_Znunu.Phi();

    } // Znunu selection loop


    // Zmumu
    if (m_dressedTruthMuon->size() > 1) {

      TLorentzVector truth_muon1 = m_dressedTruthMuon->at(0)->p4();
      TLorentzVector truth_muon2 = m_dressedTruthMuon->at(1)->p4();
      truth_muon1_pt = truth_muon1.Perp();
      truth_muon2_pt = truth_muon2.Perp();
      auto truth_Zmumu = truth_muon1 + truth_muon2;
      truth_mll_muon = truth_Zmumu.M();
      m_truthEmulMETZmumu = truth_Zmumu.Pt();
      m_truthEmulMETPhiZmumu = truth_Zmumu.Phi();

      if ( truth_muon1_pt >  m_LeadLepPtCut && truth_muon2_pt > m_SubLeadLepPtCut
          && TMath::Abs(truth_muon1.Eta()) < m_lepEtaCut && TMath::Abs(truth_muon2.Eta()) < m_lepEtaCut ) pass_truth_dimuonCut = true;
      if ( m_dressedTruthMuon->at(0)->pdgId() * m_dressedTruthMuon->at(1)->pdgId() < 0 ) pass_truth_OSmuon = true;

    } // Zmumu selection loop

    // Zee
    if (m_dressedTruthElectron->size() > 1) {

      TLorentzVector truth_electron1 = m_dressedTruthElectron->at(0)->p4();
      TLorentzVector truth_electron2 = m_dressedTruthElectron->at(1)->p4();
      truth_electron1_pt = truth_electron1.Perp();
      truth_electron2_pt = truth_electron2.Perp();
      auto truth_Zee = truth_electron1 + truth_electron2;
      truth_mll_electron = truth_Zee.M();
      m_truthEmulMETZee = truth_Zee.Pt();
      m_truthEmulMETPhiZee = truth_Zee.Phi();

      if ( truth_electron1_pt >  m_LeadLepPtCut && truth_electron2_pt > m_SubLeadLepPtCut
          && TMath::Abs(truth_electron1.Eta()) < m_lepEtaCut && TMath::Abs(truth_electron2.Eta()) < m_lepEtaCut ) pass_truth_dielectronCut = true;
      if ( m_dressedTruthElectron->at(0)->pdgId() * m_dressedTruthElectron->at(1)->pdgId() < 0 ) pass_truth_OSelectron = true;

    } // Zee selection loop





    //------------------------------------------
    // Define Truth Monojet and DiJet Properties
    //------------------------------------------

    ///////////////////////
    // Monojet Selection //
    ///////////////////////
    if (m_selectedTruthJet->size() > 0) {

      truth_monojet_pt = m_selectedTruthJet->at(0)->pt();
      truth_monojet_phi = m_selectedTruthJet->at(0)->phi();
      truth_monojet_eta = m_selectedTruthJet->at(0)->eta();
      truth_monojet_rapidity = m_selectedTruthJet->at(0)->rapidity();


      // Define Monojet
      if ( truth_monojet_pt > m_monoJetPtCut ){
        if ( fabs(truth_monojet_eta) < m_monoJetEtaCut){
          pass_truth_monoJet = true;
        }
      }

      // deltaPhi(truth_monojet,MET) decision
      // For Zmumu
      if (m_isZmumu){
        truth_dPhiMonojetMet_Zmumu = deltaPhi(truth_monojet_phi, m_truthEmulMETPhiZmumu);
      }
      // For Zee
      if (m_isZee){
        truth_dPhiMonojetMet_Zee = deltaPhi(truth_monojet_phi, m_truthEmulMETPhiZmumu);
      }

    } // MonoJet selection 



    /////////////////////
    // DiJet Selection //
    /////////////////////
    if (m_selectedTruthJet->size() > 1) {

      truth_jet1 = m_selectedTruthJet->at(0)->p4();
      truth_jet2 = m_selectedTruthJet->at(1)->p4();
      truth_jet1_pt = m_selectedTruthJet->at(0)->pt();
      truth_jet2_pt = m_selectedTruthJet->at(1)->pt();
      truth_jet1_phi = m_selectedTruthJet->at(0)->phi();
      truth_jet2_phi = m_selectedTruthJet->at(1)->phi();
      truth_jet1_eta = m_selectedTruthJet->at(0)->eta();
      truth_jet2_eta = m_selectedTruthJet->at(1)->eta();
      truth_jet1_rapidity = m_selectedTruthJet->at(0)->rapidity();
      truth_jet2_rapidity = m_selectedTruthJet->at(1)->rapidity();
      auto truth_dijet = truth_jet1 + truth_jet2;
      truth_mjj = truth_dijet.M();

      // Define Dijet
      if ( truth_jet1_pt > m_diJet1PtCut && truth_jet2_pt > m_diJet2PtCut ){
        if ( fabs(truth_jet1_rapidity) < m_diJetRapCut && fabs(truth_jet2_rapidity) < m_diJetRapCut ){
          pass_truth_diJet = true;
        }
      }

      // deltaPhi(Jet1,MET) or deltaPhi(Jet2,MET) decision
      // For Zmumu
      if (m_isZmumu){
        truth_dPhiJet1Met_Zmumu = deltaPhi(truth_jet1_phi, m_truthEmulMETPhiZmumu);
        truth_dPhiJet2Met_Zmumu = deltaPhi(truth_jet2_phi, m_truthEmulMETPhiZmumu);
      }
      // For Zee
      if (m_isZee){
        truth_dPhiJet1Met_Zee = deltaPhi(truth_jet1_phi, m_truthEmulMETPhiZee);
        truth_dPhiJet2Met_Zee = deltaPhi(truth_jet2_phi, m_truthEmulMETPhiZee);
      }

    } // DiJet selection 

    // For jet3
    if (m_selectedTruthJet->size() > 2) {
      truth_jet3_pt = m_selectedTruthJet->at(2)->pt();
      truth_jet3_phi = m_selectedTruthJet->at(2)->phi();
      truth_jet3_eta = m_selectedTruthJet->at(2)->eta();
      truth_jet3_rapidity = m_selectedTruthJet->at(2)->rapidity();
      // deltaPhi(Jet3,MET)
      truth_dPhiJet3Met = deltaPhi(truth_jet3_phi, truthMET_phi);
      truth_dPhiJet3Met_Zmumu = deltaPhi(truth_jet3_phi, m_truthEmulMETPhiZmumu);
      truth_dPhiJet3Met_Zee = deltaPhi(truth_jet3_phi, m_truthEmulMETPhiZee);
    }


    // Define deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)
    if (m_selectedTruthJet->size() > 0) {

      // loop over the jets in the Good Jets Container
      for (const auto& jet : *m_selectedTruthJet) {
        float truth_jet_pt = jet->pt();
        float truth_jet_rapidity = jet->rapidity();
        float truth_jet_phi = jet->phi();

        // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
        if (m_selectedTruthJet->at(0) == jet || m_selectedTruthJet->at(1) == jet || m_selectedTruthJet->at(2) == jet || m_selectedTruthJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
          // For Znunu
          if (m_isZnunu){
            float truth_dPhijetmet = deltaPhi(truth_jet_phi,truthMET_phi);
            if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhijetmet < 0.4 ) pass_truth_dPhijetmet = false;
            truth_dPhiMinjetmet = std::min(truth_dPhiMinjetmet, truth_dPhijetmet);
          }
          // For Zmumu
          if (m_isZmumu){
            float truth_dPhijetmet_Zmumu = deltaPhi(truth_jet_phi,m_truthEmulMETPhiZmumu);
            if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhijetmet_Zmumu < 0.4 ) pass_truth_dPhijetmet_Zmumu = false;
            truth_dPhiMinjetmet_Zmumu = std::min(truth_dPhiMinjetmet_Zmumu, truth_dPhijetmet_Zmumu);
          }
          // For Zee
          if (m_isZee){
            float truth_dPhijetmet_Zee = deltaPhi(truth_jet_phi,m_truthEmulMETPhiZee);
            if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhijetmet_Zee < 0.4 ) pass_truth_dPhijetmet_Zee = false;
            truth_dPhiMinjetmet_Zee = std::min(truth_dPhiMinjetmet_Zee, truth_dPhijetmet_Zee);
          }
        }

        // Central Jet Veto (CJV)
        if ( m_selectedTruthJet->size() > 2 && pass_truth_diJet ){
          if (m_selectedTruthJet->at(0) != jet && m_selectedTruthJet->at(1) != jet){
            if (truth_jet_pt > m_CJVptCut && fabs(truth_jet_rapidity) < m_diJetRapCut) {
              if ( (truth_jet1_rapidity > truth_jet2_rapidity) && (truth_jet_rapidity < truth_jet1_rapidity && truth_jet_rapidity > truth_jet2_rapidity)){
                pass_truth_CJV = false;
              }
              if ( (truth_jet1_rapidity < truth_jet2_rapidity) && (truth_jet_rapidity > truth_jet1_rapidity && truth_jet_rapidity < truth_jet2_rapidity)){
                pass_truth_CJV = false;
              }
            }
          }
        }

        truth_Jet_ht += truth_jet_pt;
      } // Jet loop

    } // End deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)




    //-----------------------------------------------------------------------
    // Define Truth WZ Monojet and DiJet Properties (for dressed-level jets)
    //-----------------------------------------------------------------------


    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)

      ///////////////////////
      // Monojet Selection //
      ///////////////////////
      if (m_selectedTruthWZJet->size() > 0) {

        truth_monoWZjet_pt = m_selectedTruthWZJet->at(0)->pt();
        truth_monoWZjet_phi = m_selectedTruthWZJet->at(0)->phi();
        truth_monoWZjet_eta = m_selectedTruthWZJet->at(0)->eta();
        truth_monoWZjet_rapidity = m_selectedTruthWZJet->at(0)->rapidity();


        // Define MonoWZjet
        if ( truth_monoWZjet_pt > m_monoJetPtCut ){
          if ( fabs(truth_monoWZjet_eta) < m_monoJetEtaCut){
            pass_truth_monoWZJet = true;
          }
        }

        // deltaPhi(truth_monoWZjet,MET) decision
        // For Zmumu
        if (m_isZmumu){
          truth_dPhiMonoWZjetMet_Zmumu = deltaPhi(truth_monoWZjet_phi, m_truthEmulMETPhiZmumu);
        }
        // For Zee
        if (m_isZee){
          truth_dPhiMonoWZjetMet_Zee = deltaPhi(truth_monoWZjet_phi, m_truthEmulMETPhiZmumu);
        }

      } // MonoWZJet selection 



      /////////////////////
      // DiJet Selection //
      /////////////////////
      if (m_selectedTruthWZJet->size() > 1) {

        truth_WZjet1 = m_selectedTruthWZJet->at(0)->p4();
        truth_WZjet2 = m_selectedTruthWZJet->at(1)->p4();
        truth_WZjet1_pt = m_selectedTruthWZJet->at(0)->pt();
        truth_WZjet2_pt = m_selectedTruthWZJet->at(1)->pt();
        truth_WZjet1_phi = m_selectedTruthWZJet->at(0)->phi();
        truth_WZjet2_phi = m_selectedTruthWZJet->at(1)->phi();
        truth_WZjet1_eta = m_selectedTruthWZJet->at(0)->eta();
        truth_WZjet2_eta = m_selectedTruthWZJet->at(1)->eta();
        truth_WZjet1_rapidity = m_selectedTruthWZJet->at(0)->rapidity();
        truth_WZjet2_rapidity = m_selectedTruthWZJet->at(1)->rapidity();
        auto truth_diWZjet = truth_WZjet1 + truth_WZjet2;
        truth_WZmjj = truth_diWZjet.M();

        // Define DiWZjet
        if ( truth_WZjet1_pt > m_diJet1PtCut && truth_WZjet2_pt > m_diJet2PtCut ){
          if ( fabs(truth_WZjet1_rapidity) < m_diJetRapCut && fabs(truth_WZjet2_rapidity) < m_diJetRapCut ){
            pass_truth_diWZJet = true;
          }
        }

        // deltaPhi(Jet1,MET) or deltaPhi(Jet2,MET) decision
        // For Zmumu
        if (m_isZmumu){
          truth_dPhiWZJet1Met_Zmumu = deltaPhi(truth_WZjet1_phi, m_truthEmulMETPhiZmumu);
          truth_dPhiWZJet2Met_Zmumu = deltaPhi(truth_WZjet2_phi, m_truthEmulMETPhiZmumu);
        }
        // For Zee
        if (m_isZee){
          truth_dPhiWZJet1Met_Zee = deltaPhi(truth_WZjet1_phi, m_truthEmulMETPhiZee);
          truth_dPhiWZJet2Met_Zee = deltaPhi(truth_WZjet2_phi, m_truthEmulMETPhiZee);
        }

      } // DiJet selection 

      // For jet3
      if (m_selectedTruthWZJet->size() > 2) {
        truth_WZjet3_pt = m_selectedTruthWZJet->at(2)->pt();
        truth_WZjet3_phi = m_selectedTruthWZJet->at(2)->phi();
        truth_WZjet3_eta = m_selectedTruthWZJet->at(2)->eta();
        truth_WZjet3_rapidity = m_selectedTruthWZJet->at(2)->rapidity();
        // deltaPhi(Jet3,MET)
        truth_dPhiWZJet3Met = deltaPhi(truth_WZjet3_phi, truthMET_phi);
        truth_dPhiWZJet3Met_Zmumu = deltaPhi(truth_WZjet3_phi, m_truthEmulMETPhiZmumu);
        truth_dPhiWZJet3Met_Zee = deltaPhi(truth_WZjet3_phi, m_truthEmulMETPhiZee);
      }


      // Define deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)
      if (m_selectedTruthWZJet->size() > 0) {

        // loop over the jets in the Good Jets Container
        for (const auto& jet : *m_selectedTruthWZJet) {
          float truth_jet_pt = jet->pt();
          float truth_jet_rapidity = jet->rapidity();
          float truth_jet_phi = jet->phi();

          // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
          if (m_selectedTruthWZJet->at(0) == jet || m_selectedTruthWZJet->at(1) == jet || m_selectedTruthWZJet->at(2) == jet || m_selectedTruthWZJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
            // For Znunu
            if (m_isZnunu){
              float truth_dPhiWZjetmet = deltaPhi(truth_jet_phi,truthMET_phi);
              if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhiWZjetmet < 0.4 ) pass_truth_dPhiWZjetmet = false;
              truth_dPhiMinWZjetmet = std::min(truth_dPhiMinWZjetmet, truth_dPhiWZjetmet);
            }
            // For Zmumu
            if (m_isZmumu){
              float truth_dPhiWZjetmet_Zmumu = deltaPhi(truth_jet_phi,m_truthEmulMETPhiZmumu);
              if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhiWZjetmet_Zmumu < 0.4 ) pass_truth_dPhiWZjetmet_Zmumu = false;
              truth_dPhiMinWZjetmet_Zmumu = std::min(truth_dPhiMinWZjetmet_Zmumu, truth_dPhiWZjetmet_Zmumu);
            }
            // For Zee
            if (m_isZee){
              float truth_dPhiWZjetmet_Zee = deltaPhi(truth_jet_phi,m_truthEmulMETPhiZee);
              if ( truth_jet_pt > 30000. && fabs(truth_jet_rapidity) < 4.4 && truth_dPhiWZjetmet_Zee < 0.4 ) pass_truth_dPhiWZjetmet_Zee = false;
              truth_dPhiMinWZjetmet_Zee = std::min(truth_dPhiMinWZjetmet_Zee, truth_dPhiWZjetmet_Zee);
            }
          }

          // Central Jet Veto (CJV)
          if ( m_selectedTruthWZJet->size() > 2 && pass_truth_diWZJet ){
            if (m_selectedTruthWZJet->at(0) != jet && m_selectedTruthWZJet->at(1) != jet){
              if (truth_jet_pt > m_CJVptCut && fabs(truth_jet_rapidity) < m_diJetRapCut) {
                if ( (truth_WZjet1_rapidity > truth_WZjet2_rapidity) && (truth_jet_rapidity < truth_WZjet1_rapidity && truth_jet_rapidity > truth_WZjet2_rapidity)){
                  pass_truth_WZ_CJV = false;
                }
                if ( (truth_WZjet1_rapidity < truth_WZjet2_rapidity) && (truth_jet_rapidity > truth_WZjet1_rapidity && truth_jet_rapidity < truth_WZjet2_rapidity)){
                  pass_truth_WZ_CJV = false;
                }
              }
            }
          }

          truth_WZJet_ht += truth_jet_pt;
        } // Jet loop

      } // End deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)




    } // For STDM derivation



    //-----------------------------
    // Truth Z -> mumu + JET EVENT
    //-----------------------------

    if (m_isZmumu) {

      h_channel = "h_zmumu_";

      /////////////////////////
      // Nominal Truth level //
      /////////////////////////
      h_level = "nominal_";


      // Test
      if (m_dressedTruthMuon->size() == 2 && m_dressedTruthElectron->size() == 0 /* && m_selectedTruthTau->size() == 0 */ ) {
        if (pass_truth_OSmuon) {
          if ( truth_mll_muon > m_mllMin && truth_mll_muon < m_mllMax ) {
            hMap1D[h_channel+h_level+"truth_test_met_emulmet"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
            hMap1D[h_channel+h_level+"truth_test_mll"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
            // MET trigger
            if (m_doReco && m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") ){
              hMap1D[h_channel+h_level+"truth_test_met_emulmet_with_MetTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
              hMap1D[h_channel+h_level+"truth_test_mll_with_MetTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
            }
          }
        }
      }



      if (m_dressedTruthMuon->size() == 2 && m_dressedTruthElectron->size() == 0 /* && m_selectedTruthTau->size() == 0 */ ) {
        if (pass_truth_OSmuon) {
          if ( pass_truth_dimuonCut ) {
            if ( truth_mll_muon > m_mllMin && truth_mll_muon < m_mllMax ) {
              if ( m_truthEmulMETZmumu > m_metCut ) {

                ////////////////////////
                // MonoJet phasespace //
                ////////////////////////
                if (pass_truth_monoJet && pass_truth_dPhijetmet_Zmumu) {
                  // Fill histogram
                  // No trigger passed
                  hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_noTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                  hMap1D[h_channel+h_level+"truth_monojet_mll_noTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                  // MET trigger
                  if (m_doReco && (m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe100_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50") ) ){
                    hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_metTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_monojet_mll_metTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                  }
                  // Muon trigger
                  if (m_doReco && (m_trigDecisionTool->isPassed("HLT_mu24_ivarmedium") || m_trigDecisionTool->isPassed("HLT_mu26_ivarmedium") ) ) {
                    hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_muonTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_monojet_mll_muonTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                  }
                }

                ////////////////////
                // VBF phasespace //
                ////////////////////
                if (pass_truth_diJet && truth_mjj > m_mjjCut && pass_truth_CJV && pass_truth_dPhijetmet_Zmumu) {

                  // Fill histogram
                  hMap1D[h_channel+h_level+"truth_vbf_met_emulmet"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                  hMap1D[h_channel+h_level+"truth_vbf_mjj"]->Fill(truth_mjj * 0.001, m_mcEventWeight);
                  hMap1D[h_channel+h_level+"truth_vbf_dPhijj"]->Fill(deltaPhi(truth_jet1_phi, truth_jet2_phi), m_mcEventWeight);
                  hMap1D[h_channel+h_level+"truth_vbf_mll"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);

                }

              } // MET cut
            } // Mll cut
          } // Dimuon Cut
        } // OS lepton cut
      } // Lepton Veto


      /////////////////////////////////////////////
      // WZ Truth level (For dressed-level jets) //
      /////////////////////////////////////////////

      h_level = "dress_";
      if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
        if (m_dressedTruthMuon->size() == 2 && m_dressedTruthElectron->size() == 0 /* && m_selectedTruthTau->size() == 0 */ ) {
          if (pass_truth_OSmuon) {
            if ( pass_truth_dimuonCut ) {
              if ( truth_mll_muon > m_mllMin && truth_mll_muon < m_mllMax ) {
                if ( m_truthEmulMETZmumu > m_metCut ) {

                  ////////////////////////
                  // MonoJet phasespace //
                  ////////////////////////
                  if (pass_truth_monoWZJet && pass_truth_dPhiWZjetmet_Zmumu) {
                    // Fill histogram
                    // No trigger passed
                    hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_noTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_monojet_mll_noTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                    // MET trigger
                    if (m_doReco && (m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe100_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50") ) ){
                      hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_metTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_monojet_mll_metTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                    }
                    // Muon trigger
                    if (m_doReco && (m_trigDecisionTool->isPassed("HLT_mu24_ivarmedium") || m_trigDecisionTool->isPassed("HLT_mu26_ivarmedium") ) ) {
                      hMap1D[h_channel+h_level+"truth_monojet_met_emulmet_muonTrig"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_monojet_mll_muonTrig"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);
                    }
                  }

                  ////////////////////
                  // VBF phasespace //
                  ////////////////////
                  if (pass_truth_diWZJet && truth_WZmjj > m_mjjCut && pass_truth_WZ_CJV && pass_truth_dPhiWZjetmet_Zmumu) {

                    // Fill histogram
                    hMap1D[h_channel+h_level+"truth_vbf_met_emulmet"]->Fill(m_truthEmulMETZmumu * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_mjj"]->Fill(truth_mjj * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_dPhijj"]->Fill(deltaPhi(truth_jet1_phi, truth_jet2_phi), m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_mll"]->Fill(truth_mll_muon * 0.001, m_mcEventWeight);

                  }

                } // MET cut
              } // Mll cut
            } // Dimuon Cut
          } // OS lepton cut
        } // Lepton Veto
      } // SM Derivation (STDM)




    } // End of Z -> mumu


    //---------------------------
    // Truth Z -> ee + JET EVENT
    //---------------------------

    if (m_isZee) {

      h_channel = "h_zee_";

      /////////////////////////
      // Nominal Truth level //
      /////////////////////////
      h_level = "nominal_";

      //if ( m_doReco && ( m_trigDecisionTool->isPassed("HLT_e26_lhtight_nod0_ivarloose") || m_trigDecisionTool->isPassed("HLT_e60_lhmedium_nod0") || m_trigDecisionTool->isPassed("HLT_e140_lhloose_nod0") ) ) {
        if (m_dressedTruthElectron->size() == 2 && m_dressedTruthMuon->size() == 0 /* && m_selectedTruthTau->size() == 0 */ ) {
          if (pass_truth_OSelectron) {
            if ( pass_truth_dielectronCut ) {
              if ( truth_mll_electron > m_mllMin && truth_mll_electron < m_mllMax ) {
                if ( m_truthEmulMETZee > m_metCut ) {

                  ////////////////////////
                  // MonoJet phasespace //
                  ////////////////////////
                  if (pass_truth_monoJet && pass_truth_dPhijetmet_Zee) {

                    // Fill histogram
                    hMap1D[h_channel+h_level+"truth_monojet_met_emulmet"]->Fill(m_truthEmulMETZee * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_monojet_mll"]->Fill(truth_mll_electron * 0.001, m_mcEventWeight);
                  }

                  ////////////////////
                  // VBF phasespace //
                  ////////////////////
                  if (pass_truth_diJet && truth_mjj > m_mjjCut && pass_truth_CJV && pass_truth_dPhijetmet_Zee) {

                    // Fill histogram
                    hMap1D[h_channel+h_level+"truth_vbf_met_emulmet"]->Fill(m_truthEmulMETZee * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_mjj"]->Fill(truth_mjj * 0.001, m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_dPhijj"]->Fill(deltaPhi(truth_jet1_phi, truth_jet2_phi), m_mcEventWeight);
                    hMap1D[h_channel+h_level+"truth_vbf_mll"]->Fill(truth_mll_electron * 0.001, m_mcEventWeight);

                  }

                } // MET cut
              } // Mll cut
            } // Dielectron Cut
          } // OS lepton cut
        } // Lepton Veto
      //} // Electron trigger



      /////////////////////////////////////////////
      // WZ Truth level (For dressed-level jets) //
      /////////////////////////////////////////////
      h_level = "dress_";

      if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
        //if ( m_doReco && (m_trigDecisionTool->isPassed("HLT_e26_lhtight_nod0_ivarloose") || m_trigDecisionTool->isPassed("HLT_e60_lhmedium_nod0") || m_trigDecisionTool->isPassed("HLT_e140_lhloose_nod0") ) ) {
          if (m_dressedTruthMuon->size() == 0 && m_dressedTruthElectron->size() == 2 /* && m_selectedTruthTau->size() == 0 */ ) {
            if (pass_truth_OSelectron) {
              if ( pass_truth_dielectronCut ) {
                if ( truth_mll_electron > m_mllMin && truth_mll_electron < m_mllMax ) {
                  if ( m_truthEmulMETZee > m_metCut ) {

                    ////////////////////////
                    // MonoJet phasespace //
                    ////////////////////////
                    if (pass_truth_monoWZJet && pass_truth_dPhiWZjetmet_Zee) {

                      // Fill histogram
                      hMap1D[h_channel+h_level+"truth_monojet_met_emulmet"]->Fill(m_truthEmulMETZee * 0.001, m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_monojet_mll"]->Fill(truth_mll_electron * 0.001, m_mcEventWeight);
                    }

                    ////////////////////
                    // VBF phasespace //
                    ////////////////////
                    if (pass_truth_diWZJet && truth_WZmjj > m_mjjCut && pass_truth_WZ_CJV && pass_truth_dPhiWZjetmet_Zee) {

                      // Fill histogram
                      hMap1D[h_channel+h_level+"truth_vbf_met_emulmet"]->Fill(m_truthEmulMETZee * 0.001, m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_vbf_mjj"]->Fill(truth_mjj * 0.001, m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_vbf_dPhijj"]->Fill(deltaPhi(truth_jet1_phi, truth_jet2_phi), m_mcEventWeight);
                      hMap1D[h_channel+h_level+"truth_vbf_mll"]->Fill(truth_mll_electron * 0.001, m_mcEventWeight);

                    }

                  } // MET cut
                } // Mll cut
              } // Dielectron Cut
            } // OS lepton cut
          } // Lepton Veto
        //} // Electron trigger
      }// SM Derivation (STDM)




    } // End of Z -> ee


  } // MC and (EXOT5 or STDM4)

  // End of Truth selection






  //////////////////////////////////
  // SM study in TRUTH1 derivation//
  //////////////////////////////////

  //-----------------------------------------------
  // Truth dressed-level jets in TRUTH1 derivation
  //-----------------------------------------------

  // TRUTH1 or STDM or EOXT Derivation
  if ( !m_isData && m_doTruth && ( m_fileType =="truth1" || m_dataType.find("STDM")!=std::string::npos || m_dataType.find("EXOT")!=std::string::npos ) ) {

    //Info("execute()", "Event number = %i", m_eventCounter );

    ////////////////////////////////////////
    // Truth1 derivation analysis options //
    ////////////////////////////////////////

    bool is_Sherpa = false;
    if (m_generatorType == "sherpa") is_Sherpa = true;

    bool is_MG = false;
    if (m_generatorType == "madgraph") is_MG = true;





    // To store the nominal jets
    xAOD::JetContainer* m_truthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_truthNominalJetAux = new xAOD::AuxContainerBase();
    m_truthNominalJet->setStore( m_truthNominalJetAux ); //< Connect the two

    // To build Emulated bare and born jets
    xAOD::JetContainer* m_truthWZJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_truthWZJetAux = new xAOD::AuxContainerBase();
    m_truthWZJet->setStore( m_truthWZJetAux ); //< Connect the two

    xAOD::JetContainer* m_copyTruthWZJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_copyTruthWZJetAux = new xAOD::AuxContainerBase();
    m_copyTruthWZJet->setStore( m_copyTruthWZJetAux ); //< Connect the two
    
    xAOD::JetContainer* m_truthEmulBareJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_truthEmulBareJetAux = new xAOD::AuxContainerBase();
    m_truthEmulBareJet->setStore( m_truthEmulBareJetAux ); //< Connect the two

    xAOD::JetContainer* m_truthEmulBornJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_truthEmulBornJetAux = new xAOD::AuxContainerBase();
    m_truthEmulBornJet->setStore( m_truthEmulBornJetAux ); //< Connect the two


    // Minimum jet cut (pT < 30GeV)

    // To store the signal Nominal jets
    xAOD::JetContainer* m_goodTruthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodTruthNominalJetAux = new xAOD::AuxContainerBase();
    m_goodTruthNominalJet->setStore( m_goodTruthNominalJetAux ); //< Connect the two

    // To store the signal WZ jets
    xAOD::JetContainer* m_goodTruthWZJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodTruthWZJetAux = new xAOD::AuxContainerBase();
    m_goodTruthWZJet->setStore( m_goodTruthWZJetAux ); //< Connect the two

    // To store the signal custom bare jets
    xAOD::JetContainer* m_goodCustomDressJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodCustomDressJetAux = new xAOD::AuxContainerBase();
    m_goodCustomDressJet->setStore( m_goodCustomDressJetAux ); //< Connect the two

    // To store the signal custom bare jets
    xAOD::JetContainer* m_goodCustomBareJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodCustomBareJetAux = new xAOD::AuxContainerBase();
    m_goodCustomBareJet->setStore( m_goodCustomBareJetAux ); //< Connect the two

    // To store the signal custom born jets
    xAOD::JetContainer* m_goodCustomBornJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodCustomBornJetAux = new xAOD::AuxContainerBase();
    m_goodCustomBornJet->setStore( m_goodCustomBornJetAux ); //< Connect the two

    // To store the signal emulated bare jets
    xAOD::JetContainer* m_goodEmulBareJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodEmulBareJetAux = new xAOD::AuxContainerBase();
    m_goodEmulBareJet->setStore( m_goodEmulBareJetAux ); //< Connect the two

    // To store the signal emulated born jets
    xAOD::JetContainer* m_goodEmulBornJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_goodEmulBornJetAux = new xAOD::AuxContainerBase();
    m_goodEmulBornJet->setStore( m_goodEmulBornJetAux ); //< Connect the two


    // Overlap Removal VS Overlap Subtraction Study
    // To store overlap removed nominal jets from bare-level leptons
    xAOD::JetContainer* m_ORbareTruthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_ORbareTruthNominalJetAux = new xAOD::AuxContainerBase();
    m_ORbareTruthNominalJet->setStore( m_ORbareTruthNominalJetAux ); //< Connect the two
    // To store overlap removed nominal jets from dress-level leptons
    xAOD::JetContainer* m_ORdressTruthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_ORdressTruthNominalJetAux = new xAOD::AuxContainerBase();
    m_ORdressTruthNominalJet->setStore( m_ORdressTruthNominalJetAux ); //< Connect the two
    // To store overlap subtracted nominal jets from bare-level leptons
    xAOD::JetContainer* m_OSbareTruthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_OSbareTruthNominalJetAux = new xAOD::AuxContainerBase();
    m_OSbareTruthNominalJet->setStore( m_OSbareTruthNominalJetAux ); //< Connect the two
    // To store overlap subtracted nominal jets from dressed-level leptons
    xAOD::JetContainer* m_OSdressTruthNominalJet = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_OSdressTruthNominalJetAux = new xAOD::AuxContainerBase();
    m_OSdressTruthNominalJet->setStore( m_OSdressTruthNominalJetAux ); //< Connect the two


    // To store the FSR photons
    xAOD::TruthParticleContainer* m_truthPromptOrFSRPhotons = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthPromptOrFSRPhotonsAux = new xAOD::AuxContainerBase();
    m_truthPromptOrFSRPhotons->setStore( m_truthPromptOrFSRPhotonsAux ); //< Connect the two

    // To store all photons for MadGraph
    xAOD::TruthParticleContainer* m_truthMadGraphPhotons = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthMadGraphPhotonsAux = new xAOD::AuxContainerBase();
    m_truthMadGraphPhotons->setStore( m_truthMadGraphPhotonsAux ); //< Connect the two


    // To store all truth muons
    xAOD::TruthParticleContainer* m_truthMuon = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthMuonAux = new xAOD::AuxContainerBase();
    m_truthMuon->setStore( m_truthMuonAux ); //< Connect the two

    // To store all truth electrons
    xAOD::TruthParticleContainer* m_truthElectron = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthElectronAux = new xAOD::AuxContainerBase();
    m_truthElectron->setStore( m_truthElectronAux ); //< Connect the two

    // To store all truth neutrinos
    xAOD::TruthParticleContainer* m_truthNeutrino = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthNeutrinoAux = new xAOD::AuxContainerBase();
    m_truthNeutrino->setStore( m_truthNeutrinoAux ); //< Connect the two

    // To store the prompt muons (used for WZ jets)
    xAOD::TruthParticleContainer* m_truthPromptMuon = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthPromptMuonAux = new xAOD::AuxContainerBase();
    m_truthPromptMuon->setStore( m_truthPromptMuonAux ); //< Connect the two

    // To store the prompt electrons (used for WZ jets)
    xAOD::TruthParticleContainer* m_truthPromptElectron = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthPromptElectronAux = new xAOD::AuxContainerBase();
    m_truthPromptElectron->setStore( m_truthPromptElectronAux ); //< Connect the two

    // To store the born muons
    xAOD::TruthParticleContainer* m_truthBornMuon = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthBornMuonAux = new xAOD::AuxContainerBase();
    m_truthBornMuon->setStore( m_truthBornMuonAux ); //< Connect the two

    // To store the born electrons
    xAOD::TruthParticleContainer* m_truthBornElectron = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthBornElectronAux = new xAOD::AuxContainerBase();
    m_truthBornElectron->setStore( m_truthBornElectronAux ); //< Connect the two

    // To store the dressed muons from Z
    xAOD::TruthParticleContainer* m_truthDressMuonFromZ = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthDressMuonFromZAux = new xAOD::AuxContainerBase();
    m_truthDressMuonFromZ->setStore( m_truthDressMuonFromZAux ); //< Connect the two

    // To store the bare muons form Z
    xAOD::TruthParticleContainer* m_truthBareMuonFromZ = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthBareMuonFromZAux = new xAOD::AuxContainerBase();
    m_truthBareMuonFromZ->setStore( m_truthBareMuonFromZAux ); //< Connect the two

    // To store the dressed electrons from Z
    xAOD::TruthParticleContainer* m_truthDressElectronFromZ = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthDressElectronFromZAux = new xAOD::AuxContainerBase();
    m_truthDressElectronFromZ->setStore( m_truthDressElectronFromZAux ); //< Connect the two

    // To store the bare electrons form Z
    xAOD::TruthParticleContainer* m_truthBareElectronFromZ = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthBareElectronFromZAux = new xAOD::AuxContainerBase();
    m_truthBareElectronFromZ->setStore( m_truthBareElectronFromZAux ); //< Connect the two

    // To store the neutrinos form Z
    xAOD::TruthParticleContainer* m_truthNeutrinoFromZ = new xAOD::TruthParticleContainer();
    xAOD::AuxContainerBase* m_truthNeutrinoFromZAux = new xAOD::AuxContainerBase();
    m_truthNeutrinoFromZ->setStore( m_truthNeutrinoFromZAux ); //< Connect the two





    // Retrieve TruthJets
    const xAOD::JetContainer* m_truthJetsContainer = nullptr;
    if ( !m_event->retrieve( m_truthJetsContainer, "AntiKt4TruthJets" ).isSuccess() ){
      Error("execute()", "Failed to retrieve AntiKt4TruthJets container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Retrieve TruthWZJets (Not in EXOT5)
    const xAOD::JetContainer* m_truthWZJetsContainer = nullptr;
    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) {
      if ( !m_event->retrieve( m_truthWZJetsContainer, "AntiKt4TruthWZJets" ).isSuccess() ){
        Error("execute()", "Failed to retrieve AntiKt4TruthWZJets container. Exiting." );
        return EL::StatusCode::FAILURE;
      }
    }

    // Retrieve truth photons (Not in EXOT5)
    const xAOD::TruthParticleContainer* m_truthPhotonsContainer = nullptr;
    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) {
      if ( !m_event->retrieve( m_truthPhotonsContainer, m_nameDerivation+"TruthPhotons" ).isSuccess() ){
        Error("execute()", "Failed to retrieve TruthPhotons container. Exiting." );
        return EL::StatusCode::FAILURE;
      }
    }

    // Retrieve truth muons
    const xAOD::TruthParticleContainer* m_truthMuonsContainer = nullptr;
    if ( !m_event->retrieve( m_truthMuonsContainer, m_nameDerivation+"TruthMuons" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthMuons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Retrieve truth electrons
    const xAOD::TruthParticleContainer* m_truthElectronsContainer = nullptr;
    if ( !m_event->retrieve( m_truthElectronsContainer, m_nameDerivation+"TruthElectrons" ).isSuccess() ){
      Error("execute()", "Failed to retrieve truthElectrons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Retrieve truth neutrinos
    const xAOD::TruthParticleContainer* m_truthNeutrinosContainer = nullptr;
    if ( !m_event->retrieve( m_truthNeutrinosContainer, m_nameDerivation+"TruthNeutrinos" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthNeutrinos container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Retrieve truth particles
    const xAOD::TruthParticleContainer* m_truthParticlesContainer = nullptr;
    if ( !m_event->retrieve( m_truthParticlesContainer, "TruthParticles" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthParticless container. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Retrieve truth events
    const xAOD::TruthEventContainer* m_truthEventsContainer = nullptr;
    if ( !m_event->retrieve( m_truthEventsContainer, "TruthEvents" ).isSuccess() ){
      Error("execute()", "Failed to retrieve TruthEvents container. Exiting." );
      return EL::StatusCode::FAILURE;
    }






    //-------------
    // Truth Muons
    //-------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_muon_shallowCopy = xAOD::shallowCopyContainer( *m_truthMuonsContainer );
    xAOD::TruthParticleContainer* truth_muonSC = truth_muon_shallowCopy.first;

    // truth muon selection: note that I don't require pt and eta cut for this container because I will use truth level leptons with pT and eta cuts loosened by 10% of their default cut values.
    for (const auto &muon : *truth_muonSC) {

      // Store all truth Electrons
      xAOD::TruthParticle* truthMuon = new xAOD::TruthParticle();
      m_truthMuon->push_back( truthMuon );
      *truthMuon = *muon; // copies auxdata from one auxstore to the other

      if ( is_customDerivation && (bool) muon->auxdata<char>("IsPromptLepton")) {
        hMap1D["SM_study_custom_prompt_muon_pt"]->Fill(muon->pt() * 0.001, m_mcEventWeight);
      }

      bool isPromptMuon = muon->auxdata<unsigned int>("classifierParticleOrigin") < 5 || muon->auxdata<unsigned int>("classifierParticleOrigin") > 35 ||
                        ( muon->auxdata<unsigned int>("classifierParticleOrigin") > 9 && muon->auxdata<unsigned int>("classifierParticleOrigin") < 23 );

      // Store prompt Muons
      if (isPromptMuon) {

        hMap1D["SM_study_truth_prompt_muon_pt"]->Fill(muon->pt() * 0.001, m_mcEventWeight);
        //std::cout << " Prompt muon pt = " << muon->pt() << endl;

        xAOD::TruthParticle* promptMuon = new xAOD::TruthParticle();
        m_truthPromptMuon->push_back( promptMuon );
        *promptMuon = *muon; // copies auxdata from one auxstore to the other
      }
    }

    // record your deep copied jet container (and aux container) to the store
    //ANA_CHECK(m_store->record( m_truthMuon, "m_truthMuon" ));
    //ANA_CHECK(m_store->record( m_truthMuonAux, "m_truthMuonAux." ));
    //ANA_CHECK(m_store->record( m_truthPromptMuon, "truthPromptMuon" ));
    //ANA_CHECK(m_store->record( m_truthPromptMuonAux, "truthPromptMuonAux." ));

    delete truth_muon_shallowCopy.first;
    delete truth_muon_shallowCopy.second;


    //-----------------
    // Truth Electrons
    //-----------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_elec_shallowCopy = xAOD::shallowCopyContainer( *m_truthElectronsContainer );
    xAOD::TruthParticleContainer* truth_elecSC = truth_elec_shallowCopy.first;

    // truth electron selection: note that I don't require pt and eta cut for this container because I will use truth level leptons with pT and eta cuts loosened by 10% of their default cut values.
    for (const auto &electron : *truth_elecSC) {

      //std::cout << "Truth electron pt = " << electron->pt() << endl;

      // Store all truth Electrons
      xAOD::TruthParticle* truthElectron = new xAOD::TruthParticle();
      m_truthElectron->push_back( truthElectron );
      *truthElectron = *electron; // copies auxdata from one auxstore to the other

      if (is_customDerivation && (bool) electron->auxdata<char>("IsPromptLepton")) {
        hMap1D["SM_study_custom_prompt_electron_pt"]->Fill(electron->pt() * 0.001, m_mcEventWeight);
      }

      bool isPromptElectron = electron->auxdata<unsigned int>("classifierParticleOrigin") < 5 || electron->auxdata<unsigned int>("classifierParticleOrigin") > 35 ||
                        ( electron->auxdata<unsigned int>("classifierParticleOrigin") > 9 && electron->auxdata<unsigned int>("classifierParticleOrigin") < 23 );

      if (isPromptElectron) {

        hMap1D["SM_study_truth_prompt_electron_pt"]->Fill(electron->pt() * 0.001, m_mcEventWeight);
        //std::cout << " Prompt electron pt = " << electron->pt() << endl;

        // Store prompt Electrons
        xAOD::TruthParticle* promptElectron = new xAOD::TruthParticle();
        m_truthPromptElectron->push_back( promptElectron );
        *promptElectron = *electron; // copies auxdata from one auxstore to the other
      }

    }

    // record your deep copied jet container (and aux container) to the store
    //ANA_CHECK(m_store->record( m_truthElectron, "truthElectron" ));
    //ANA_CHECK(m_store->record( m_truthElectronAux, "truthElectronAux." ));
    //ANA_CHECK(m_store->record( m_truthPromptElectron, "truthPromptElectron" ));
    //ANA_CHECK(m_store->record( m_truthPromptElectronAux, "truthPromptElectronAux." ));

    delete truth_elec_shallowCopy.first;
    delete truth_elec_shallowCopy.second;



    //-----------------
    // Truth Neutrinos
    //-----------------
    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_neutrino_shallowCopy = xAOD::shallowCopyContainer( *m_truthNeutrinosContainer );
    xAOD::TruthParticleContainer* truth_neutrinoSC = truth_neutrino_shallowCopy.first;

    // truth neutrino selection: note that I don't require pt and eta cut for this container because I will use truth level leptons with pT and eta cuts loosened by 10% of their default cut values.
    for (const auto &neutrino : *truth_neutrinoSC) {

      // Store all truth Neutrinos
      xAOD::TruthParticle* truthNeutrino = new xAOD::TruthParticle();
      m_truthNeutrino->push_back( truthNeutrino );
      *truthNeutrino = *neutrino; // copies auxdata from one auxstore to the other

    }

    // record your deep copied jet container (and aux container) to the store
    //ANA_CHECK(m_store->record( m_truthNeutrino, "truthNeutrino" ));
    //ANA_CHECK(m_store->record( m_truthNeutrinoAux, "truthNeutrinoAux." ));

    delete truth_neutrino_shallowCopy.first;
    delete truth_neutrino_shallowCopy.second;









    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT


      /*
      if (m_eventCounter<10) {
        std::cout << "==============================================================================" << std::endl;
        std::cout << "Event count = " << m_eventCounter << std::endl;
      }
      */


      //---------------------------------------------
      // Truth Particles (for retrieve born leptons)
      //---------------------------------------------
      for (const auto &truthEvent : *m_truthEventsContainer) { // Interaction loop in an event
        int nPart = truthEvent->nTruthParticles();
        for (int iPart = 0;iPart<nPart;iPart++) { // Particle loop in an interaction
          const xAOD::TruthParticle* particle = truthEvent->truthParticle(iPart);
          int status = 0, pdgId = 0;
          float ppt=0.;
          if (particle) status = particle->status();
          if (particle) pdgId = particle->pdgId();
          if (particle) ppt = particle->pt();

          // Test truth content for a few events
          /*
             if (m_eventCounter<50) {
             if (particle){
          //if ( fabs(pdgId)==11 || fabs(pdgId)==13 || fabs(pdgId)==23 || fabs(pdgId)==22 ){ // For only electron, muon, Z0, photon
          std::cout<<"TruthPart: Evt: "<<m_eventCounter<<", Part: "<<iPart<<", ID: "<<pdgId<<", status: "<<status<<", pt: "<<ppt<<std::endl;
          if (particle->hasProdVtx() && particle->prodVtx() && particle->prodVtx()->nIncomingParticles() != 0 ) {
          const xAOD::TruthVertex * prodVtx = particle->prodVtx();

          auto numIncoming = prodVtx->nIncomingParticles();
          for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
          const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
          int motherPdgId = mother->pdgId();
          std::cout << "mother ID : " << motherPdgId << ", mother pt : " << mother->pt() << std::endl;
          }
          }
          //} // For only electron, muon, Z0, photon
          }
          } // 50 events
          */



          // Find the FSR photon from W/Z decay leptons and from this FSR photon, find the bare lepton.
          //if (m_eventCounter<50) {
          if (particle) {
            if ( fabs(pdgId)==22 && status==1 ){ // For stable photoni
              bool isFromWZ = false;

              // bare lepton container
              std::vector<const xAOD::TruthParticle*> bareLeptons;
              bareLeptons.reserve(10);

              // For emulated jets with MadGraph
              xAOD::TruthParticle* madGraphPhoton = new xAOD::TruthParticle();
              madGraphPhoton->makePrivateStore(*particle);
              madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") = false;
              madGraphPhoton->auxdata<bool>("IsMadGraphFSRPhoton") = false;

              // For MadGraph (FSR photons should have always one mother => particle->prodVtx()->nIncomingParticles() == 1 )
              // -----------------------------------------------------------------------------------------------------------
              if (particle->hasProdVtx() && particle->prodVtx() && particle->prodVtx()->nIncomingParticles() == 1 ) { // Incoming particle should be one single lepton
                const xAOD::TruthVertex * prodVtx = particle->prodVtx();
                auto numIncoming = prodVtx->nIncomingParticles();

                // Find the FSR photon from W/Z decay
                for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
                  const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
                  int motherAbsPdgId = mother->absPdgId();
                  if (motherAbsPdgId == 11 || motherAbsPdgId == 13) {
                    if (mother->hasProdVtx() && mother->prodVtx() && mother->prodVtx()->nIncomingParticles() == 1 ) { // Incoming particle should be one single lepton
                      const xAOD::TruthVertex * motherProdVtx = mother->prodVtx();
                      while (true) {
                        const xAOD::TruthParticle * grandMother = motherProdVtx->incomingParticle( 0 );
                        int grandMotherAbsPdgId = grandMother->absPdgId();
                        if (grandMotherAbsPdgId == 24 || grandMotherAbsPdgId == 23 ) { // W/Z decay
                          isFromWZ = true;
                          break;
                        } else if (motherAbsPdgId == 11 || motherAbsPdgId == 13) {
                          if (grandMother->hasProdVtx() && grandMother->prodVtx() && grandMother->prodVtx()->nIncomingParticles() == 1 ) { // Incoming particle should be one single lepton
                            motherProdVtx = grandMother->prodVtx();
                          } else {
                            break;
                          }

                        } else {
                          break;
                        }
                      } // while
                    }


                  }
                }
                if (isFromWZ) {
                  //std::cout<<"Found Z decay from FSR photon in MadGraph!! : photon ID: "<< pdgId << ", status: "<< status<< ", pt: "<< ppt << std::endl;
                  madGraphPhoton->auxdata<bool>("IsMadGraphFSRPhoton") = true;
                  hMap1D["SM_study_truth_fsr_photon_from_Z_madgraph_pt"]->Fill(ppt * 0.001, m_mcEventWeight);
                }

                // Find the bare lepton 
                if (isFromWZ){ // if the FSR photon is from W/Z boson

                  auto numOutgoing = prodVtx->nOutgoingParticles();
                  if (numOutgoing>1) { // Vertex has at least two decay particles (this photon and letpon)
                    for ( std::size_t sisterIndex = 0; sisterIndex < numOutgoing; ++sisterIndex ){ // Loop over outgoing particles including myself (decaying lepton and photon(me))
                      const xAOD::TruthParticle * sister = prodVtx->outgoingParticle( sisterIndex );
                      int sisterAbsPdgId = sister->absPdgId();
                      int sisterStatus = sister->status();
                      //std::cout<<"Finding Bare lepton from FSR photon (1) : sister ID: "<< sister->pdgId() << ", status: "<< sisterStatus<< ", pt: "<< sister->pt() << std::endl;
                      if (sisterAbsPdgId == 11 || sisterAbsPdgId == 13) { // Leptons among the outgoing particles
                        //std::cout<<"Finding Bare lepton from FSR photon (2) : lepton ID: "<< sister->pdgId() << ", status: "<< sisterStatus<< ", pt: "<< sister->pt() << std::endl;
                        if (sisterStatus == 1) { // stable particle (bare lepton)
                          bareLeptons.push_back(sister);
                          //std::cout<<"Found Bare lepton from FSR photon in MadGraph!! : bare lepton ID: "<< sister->pdgId() << ", status: "<< sisterStatus<< ", pt: "<< sister->pt() << std::endl;
                          // Tag this FSR photon (the sister of the bare lepton) and decide if this photon is dressing her sister (bare lepton)
                          if (particle->p4().DeltaR(sister->p4()) < 0.1) {
                            madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") = true;
                          }
                          //std::cout<<"FSR photon in MadGraph : photon pt : " << particle->pt() << " which is dressing? " << madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") << std::endl;
                          break;
                        } else {
                          if (sister->hasDecayVtx() && sister->decayVtx() && sister->decayVtx()->nOutgoingParticles() > 0 ) { // Unstable lepton's Outgoing particle has at least two particles
                            const xAOD::TruthVertex * sisterDecayVtx = sister->decayVtx();
                            bool isBareLepton = false;
                            int count_descendant = 0; // To prevent falling into infinity loop (when it does not find any bare lepton eventually)
                            do {
                              int isNieceIndex = 0;
                              auto numSisterOutgoing = sisterDecayVtx->nOutgoingParticles();
                              for ( std::size_t nieceIndex = 0; nieceIndex < numSisterOutgoing; ++nieceIndex ){ // Loop over sister's daughter (niece) -> one lepton and photon
                                const xAOD::TruthParticle * niece = sisterDecayVtx->outgoingParticle( nieceIndex );
                                int nieceAbsPdgId = niece->absPdgId();
                                int nieceStatus = niece->status();
                                //std::cout<<"Finding Bare lepton from FSR photon (3) : niece ID: "<< niece->pdgId() << ", status: "<< nieceStatus<< ", pt: "<< niece->pt() << std::endl;
                                if ((nieceAbsPdgId == 11 || nieceAbsPdgId == 13)) { // Leptons among the niece
                                  //std::cout<<"Finding Bare lepton from FSR photon (4) : lepton ID: "<< niece->pdgId() << ", status: "<< nieceStatus<< ", pt: "<< niece->pt() << std::endl;
                                  if (nieceStatus == 1) { // stable lepton (bare lepton)
                                    isBareLepton = true; // Bare lepton found!!
                                    bareLeptons.push_back(niece);
                                    //std::cout<<"Found Bare lepton from FSR photon in MadGraph!! : bare lepton ID: "<< niece->pdgId() << ", status: "<< nieceStatus<< ", pt: "<< niece->pt() << std::endl;
                                    break;
                                  } else isNieceIndex = nieceIndex; // Get niece index for unstable letpon
                                }
                              }
                              if (!isBareLepton) { // If bare lepton is not found among the niece
                                const xAOD::TruthParticle * niece = sisterDecayVtx->outgoingParticle( isNieceIndex );
                                if (niece->hasDecayVtx() && niece->decayVtx() && niece->decayVtx()->nOutgoingParticles() > 0 ) { // Unstable lepton's Outgoing particle has at least two particles
                                  sisterDecayVtx = niece->decayVtx(); // sisterDecayVtx -> nieceDecayVtx
                                }
                                count_descendant++; // Add one if a bare lepton is not found in this loop
                                if (count_descendant > 10) { // If it does not find any bare lepton within 10 loop (descendant),
                                  isBareLepton = true; // last unstable (status = 51) lepton is considered as a bare lepton
                                  bareLeptons.push_back(niece);
                                  //std::cout<<"Found considered Bare lepton from FSR photon in MadGraph!! : bare(?) lepton ID: "<< niece->pdgId() << ", status: "<< niece->status() << ", pt: "<< niece->pt() << std::endl;
                                }
                              }
                            } while (!isBareLepton); // while a neice is not still bare lepton until a bare letpon is found
                          }
                        }
                      } // leptons
                    }
                  }
                } // isFromWZ

              } // For MadGraph

              // For Prompt photons (not FSR) -> Mostly photons from PiZero
              if (!isFromWZ) {
                if (is_MG) {
                  for (const auto &elec : *m_truthPromptElectron) {
                    double deltaR = particle->p4().DeltaR(elec->p4());
                    //std::cout << "Prompt Photon pt = " << particle->pt() << " with dR = " << deltaR << " from " << elec->pdgId() << " with pt " << elec->pt() << std::endl;
                    if (deltaR < 0.1) {
                      madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") = true;
                    }
                  }
                  for (const auto &muon : *m_truthPromptMuon) {
                    double deltaR = particle->p4().DeltaR(muon->p4());
                    //std::cout << "Prompt Photon pt = " << particle->pt() << " with dR = " << deltaR << " from " << muon->pdgId() << " with pt " << muon->pt() << std::endl;
                    if (deltaR < 0.1) {
                      madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") = true;
                    }
                  }
                  //std::cout<<"Prompt photon in MadGraph : photon pt : " << particle->pt() << " which is dressing? " << madGraphPhoton->auxdata<bool>("IsMadGraphDressingPhoton") << std::endl;
                } // For MadGraph
              } // For Prompt photons


              // Store all photons for MadGraph
              m_truthMadGraphPhotons->push_back(madGraphPhoton);




              // For Sherpa (FSR photons should have always two mothers => particle->prodVtx()->nIncomingParticles() > 1 )
              // The following vertex-based identification of W/Z's is needed for SHERPA samples
              // where the W/Z particle is not explicitly in the particle record.
              // -----------------------------------------------------------------------------------------------------------
              if (particle->hasProdVtx() && particle->prodVtx() && particle->prodVtx()->nIncomingParticles() > 1 ) { // Incoming particle should be one single lepton
                int nDecay = 0;
                const xAOD::TruthVertex * prodVtx = particle->prodVtx();
                auto numIncoming = prodVtx->nIncomingParticles();

                // Find the FSR photon from W/Z decay
                // If it is a W or Z then two of those decay products should be lepton / neutrino pair corresponding to the transitions
                // W+ -> l+ nu
                // W- -> l- nu~
                // Z  -> l+ l-
                // Z  -> nu nu~
                for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
                  const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
                  int motherAbsPdgId = mother->absPdgId();
                  if (motherAbsPdgId >10 && motherAbsPdgId < 15) { // lepton/neutrino (e, v_e, mu, v_mu)
                    nDecay++;
                  }
                }
                if (nDecay == 2){
                  isFromWZ = true;
                }

                if (isFromWZ) {
                  hMap1D["SM_study_truth_fsr_photon_from_Z_sherpa_pt"]->Fill(ppt * 0.001, m_mcEventWeight);
                  //std::cout<<"Found Z decay from FSR photon in Sherpa !! : photon ID: "<< pdgId << ", status: "<< status<< ", pt: "<< ppt << std::endl;
                }

                // Find the bare leptons associated with the FSR photons
                if (isFromWZ) {
                  const xAOD::TruthVertex * prodVtx = particle->prodVtx();
                  auto numIncoming = prodVtx->nIncomingParticles();
                  for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
                    if (motherIndex > 0) break; // Each born lepton (mother) has two bare leptons as children. To avoid double count, I only take one born lepton.
                    const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
                    int motherAbsPdgId = mother->absPdgId();
                    if (motherAbsPdgId >10 && motherAbsPdgId < 15) { // lepton/neutrino (e, v_e, mu, v_mu)

                      if (mother->hasDecayVtx() && mother->decayVtx() && mother->decayVtx()->nOutgoingParticles() > 0 ) {
                        const xAOD::TruthVertex * decayVtx = mother->decayVtx();
                        auto numOutgoing = decayVtx->nOutgoingParticles();

                        for ( std::size_t childIndex = 0; childIndex < numOutgoing; ++childIndex ){
                          const xAOD::TruthParticle * child = decayVtx->outgoingParticle( childIndex );
                          int childAbsPdgId = child->absPdgId();
                          int childStatus = child->status();
                          if ( (childAbsPdgId == 11 || childAbsPdgId == 13) && (childStatus == 1) ) {
                            //std::cout<<"Finding Bare lepton from FSR photon (1) : child ID: "<< child->pdgId() << ", status: "<< childStatus << ", pt: "<< child->pt() << std::endl;
                            bareLeptons.push_back(child);
                          }
                        }
                      }

                    }
                  }

                } // isFromWZ

                // Print out stored bare leptons
                for(const auto& lep : bareLeptons) {
                  //std::cout<<"Found Bare lepton from FSR photon in Sherpa : bare lepton ID: "<< lep->pdgId() << ", status: "<< lep->status() << ", pt: "<< lep->pt() << std::endl;
                }

              } // For Sherpa



            } // For stable photon (pdgId==22 && status==1)
          }
          //} // For 50 events




          //////////////////
          // Born leptons //
          //////////////////
          if (particle) {
            if ( (fabs(pdgId)==11 || fabs(pdgId)==13) && ((is_Sherpa && status==11) || (is_MG )) ){
              // Born in Sherpa: status = 11 (documentation particle (not necessarily physical), e.g. particles from the parton shower, intermediate states, helper particles, etc.)
              // Born in MG: status = 23

              // Find mother (to be used for Powheg, MG and Alpgen)
              bool isFromZ = false;
              // Sherpa
              if (is_Sherpa) isFromZ = true;
              // MadGraph
              if (is_MG) {
                if (particle->hasProdVtx() && particle->prodVtx() && particle->prodVtx()->nIncomingParticles() != 0 ) {
                  const xAOD::TruthVertex * prodVtx = particle->prodVtx();

                  auto numIncoming = prodVtx->nIncomingParticles();
                  for ( std::size_t motherIndex = 0; motherIndex < numIncoming; ++motherIndex ){
                    const xAOD::TruthParticle * mother = prodVtx->incomingParticle( motherIndex );
                    int motherAbsPdgId = mother->absPdgId();
                    if ( motherAbsPdgId == 23 ) isFromZ = true;
                    //if (isFromZ) std::cout << " Found Z decay !! :" << " mother ID : " << motherAbsPdgId << ", decay particle ID : " << pdgId << " , pt = " << ppt << " , status = " << status << std::endl;

                  }
                }
              } // MG

              // Store born leptons
              if (isFromZ) {
                if (fabs(pdgId)==11) {
                  // born electron (from Z)
                  xAOD::TruthParticle* bornTruthElectronFromZ = new xAOD::TruthParticle();
                  bornTruthElectronFromZ->makePrivateStore(*particle);
                  m_truthBornElectron->push_back(bornTruthElectronFromZ);
                  hMap1D["SM_study_born_electron_pt_from_Z"]->Fill(ppt * 0.001, m_mcEventWeight);
                  //if (m_eventCounter<50) std::cout << "Born electron ID : " << pdgId << ", pt : " << ppt << std::endl;
                } else if (fabs(pdgId)==13) {
                  // born muon (from Z)
                  xAOD::TruthParticle* bornTruthMuonFromZ = new xAOD::TruthParticle();
                  bornTruthMuonFromZ->makePrivateStore(*particle);
                  m_truthBornMuon->push_back(bornTruthMuonFromZ);
                  hMap1D["SM_study_born_muon_pt_from_Z"]->Fill(ppt * 0.001, m_mcEventWeight);
                  //if (m_eventCounter<50) std::cout << "Born muon ID : " << pdgId << ", pt : " << ppt << ", which is from Z? : " << isFromZ << std::endl;
                }
              }

            }
          } // Born lepton loop end

        } // Particle loop end

      } // Interaction loop end

      // record your deep copied jet container (and aux container) to the store
      //ANA_CHECK(m_store->record( m_truthBornMuon, "truthBornMuon" ));
      //ANA_CHECK(m_store->record( m_truthBornMuonAux, "truthBornMuonAux." ));
      //ANA_CHECK(m_store->record( m_truthBornElectron, "truthBornElectron" ));
      //ANA_CHECK(m_store->record( m_truthBornElectronAux, "truthBornElectronAux." ));



      //-----------------
      // FSR Photons
      //-----------------
      /// shallow copy to retrive auxdata variables
      std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_photon_shallowCopy = xAOD::shallowCopyContainer( *m_truthPhotonsContainer );
      xAOD::TruthParticleContainer* truth_photSC = truth_photon_shallowCopy.first;

      int nFSRphotons = 0;

      for (const auto &photon : *truth_photSC) {

        // Store Prompt or FSR Photons
        bool isPromptPhoton = photon->auxdata<unsigned int>("classifierParticleOrigin") < 5 || photon->auxdata<unsigned int>("classifierParticleOrigin") > 35 ||
          ( photon->auxdata<unsigned int>("classifierParticleOrigin") > 9 && photon->auxdata<unsigned int>("classifierParticleOrigin") < 23 );
        bool isFSRPhoton = photon->auxdata<unsigned int>("classifierParticleOrigin") == 40;


        if (isFSRPhoton) {
          hMap1D["SM_study_truth_fsr_photon_pt"]->Fill(photon->pt() * 0.001, m_mcEventWeight);
          /*
          if (m_eventCounter<50) {
            std::cout << "FSR photon : pT = " << photon->pt()  <<  " , type = " << photon->auxdata<unsigned int>("classifierParticleType") << " , origin = " << 
              photon->auxdata<unsigned int>("classifierParticleOrigin") << " , outcome = " << photon->auxdata<unsigned int>("ParticleOutCome") <<
              " and mother ID  = " << photon->auxdata<int>("motherID") << std::endl;
          }
          */
          if (std::abs(photon->auxdata<int>("motherID")) == 11 || std::abs(photon->auxdata<int>("motherID")) == 13) { // FSR photon from leptons
            hMap1D["SM_study_truth_fsr_photon_from_lepton_pt"]->Fill(photon->pt() * 0.001, m_mcEventWeight);
          } else {
            hMap1D["SM_study_truth_fsr_photon_not_from_lepton_pt"]->Fill(photon->pt() * 0.001, m_mcEventWeight);
          }
          nFSRphotons++;
        }

        if (isPromptPhoton || isFSRPhoton) {
          xAOD::TruthParticle* truthPromptOrFSRPhoton = new xAOD::TruthParticle();
          truthPromptOrFSRPhoton->makePrivateStore(*photon);
          m_truthPromptOrFSRPhotons->push_back(truthPromptOrFSRPhoton);
        }
      }

      hMap1D["SM_study_truth_fsr_photon_n"]->Fill(nFSRphotons, m_mcEventWeight);

      // record your deep copied jet container (and aux container) to the store
      //ANA_CHECK(m_store->record( m_truthPromptOrFSRPhotons, "truthPromptOrFSRPhotons" ));
      //ANA_CHECK(m_store->record( m_truthPromptOrFSRPhotonsAux, "truthPromptOrFSRPhotonsAux." ));

      delete truth_photon_shallowCopy.first;
      delete truth_photon_shallowCopy.second;





      if (is_customDerivation) {
        //--------------------------------------------
        // FSR Photons from TruthParticles container
        //--------------------------------------------
        /// shallow copy to retrive auxdata variables
        std::pair< xAOD::TruthParticleContainer*, xAOD::ShallowAuxContainer* > truth_particles_shallowCopy = xAOD::shallowCopyContainer( *m_truthParticlesContainer );
        xAOD::TruthParticleContainer* truth_particlesSC = truth_particles_shallowCopy.first;

        int nCustomFSRphotons = 0;

        for (const auto &part : *truth_particlesSC) {

          // FSR Photons
          if ((bool) part->auxdata<char>("IsFSRPhoton")) {
            hMap1D["SM_study_custom_fsr_photon_pt"]->Fill(part->pt() * 0.001, m_mcEventWeight);
            nFSRphotons++;
            if ( (bool) part->auxdata<char>("IsFSRPhotonDressed") ) {
              hMap1D["SM_study_custom_dressed_photon_pt"]->Fill(part->pt() * 0.001, m_mcEventWeight);
            } else {
              hMap1D["SM_study_custom_nondressed_photon_pt"]->Fill(part->pt() * 0.001, m_mcEventWeight);
            }

          } // End of FSR photons

        } // End of all truth particles

        hMap1D["SM_study_custom_fsr_photon_n"]->Fill(nFSRphotons, m_mcEventWeight);

        delete truth_particles_shallowCopy.first;
        delete truth_particles_shallowCopy.second;

      } // is_customDerivation






      //////////////////////////////////////////////////////////////////////////////////////////
      // Add variable "IsDressingPhoton" for FSR photons if they are within dR < 0.1 (Sherpa) //
      //////////////////////////////////////////////////////////////////////////////////////////
      int nFSRphotons_smallDRwithMuon = 0;
      int nFSRphotons_smallDRwithElectron = 0;
      for (const auto &phot : *m_truthPromptOrFSRPhotons) {

        //std::cout << " Prompt or FSR photon pt = " << phot->pt() << endl;
        bool isFSRPhoton = phot->auxdata<unsigned int>("classifierParticleOrigin") == 40;

        phot->auxdata<bool>("IsDressingPhoton") = false;

        double dR_min = 10.;

        if ( m_ZtruthChannel == "Zmumu" ) { // For Sherpa Zmumu sample

          for (const auto &muon : *m_truthPromptMuon) {
            double dR = phot->p4().DeltaR(muon->p4());

            if (dR < dR_min) dR_min = dR; // Obtain the nearest dR from muons

            if (dR < 0.1) {
              phot->auxdata<bool>("IsDressingPhoton") = true;
              //std::cout << "dressed photon pt = " << phot->pt() << " with dR = " << dR << " near to a muon with pt " << muon->pt() << std::endl;
            }

          } // prompt muons

          hMap1D["SM_study_prompt_or_fsr_photon_muon_dr"]->Fill(dR_min, m_mcEventWeight);
          hMap2D["SM_study_prompt_or_fsr_photon_muon_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          if (isFSRPhoton) {
            if (dR_min < 0.4) nFSRphotons_smallDRwithMuon++;
            hMap1D["SM_study_fsr_photon_muon_dr"]->Fill(dR_min, m_mcEventWeight);
            hMap2D["SM_study_fsr_photon_muon_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          }
          if (!isFSRPhoton) {
            hMap1D["SM_study_prompt_photon_muon_dr"]->Fill(dR_min, m_mcEventWeight);
            hMap2D["SM_study_prompt_photon_muon_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          }

        } // Zmumu


        if ( m_ZtruthChannel == "Zee" ) { // For Sherpa Zee sample

          for (const auto &elec : *m_truthPromptElectron) {
            double dR = phot->p4().DeltaR(elec->p4());

            if (dR < dR_min) dR_min = dR; // Obtain the nearest dR from electrons

            if (dR < 0.1) {
              phot->auxdata<bool>("IsDressingPhoton") = true;
              //std::cout << "dressed photon pt = " << phot->pt() << " with dR = " << dR << " near to a electron with pt " << elec->pt() << std::endl;
            }

          } // prompt electrons

          hMap1D["SM_study_prompt_or_fsr_photon_electron_dr"]->Fill(dR_min, m_mcEventWeight);
          hMap2D["SM_study_prompt_or_fsr_photon_electron_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          if (isFSRPhoton) {
            if (dR_min < 0.4) nFSRphotons_smallDRwithElectron++;
            hMap1D["SM_study_fsr_photon_electron_dr"]->Fill(dR_min, m_mcEventWeight);
            hMap2D["SM_study_fsr_photon_electron_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          }
          if (!isFSRPhoton) {
            hMap1D["SM_study_prompt_photon_electron_dr"]->Fill(dR_min, m_mcEventWeight);
            hMap2D["SM_study_prompt_photon_electron_dr_vs_photon_pt"]->Fill(dR_min, phot->pt(), m_mcEventWeight);
          }

        } // Zee

      } // m_truthPromptOrFSRPhotons

      if ( m_ZtruthChannel == "Zmumu" ) { // For Sherpa Zmumu sample
        hMap1D["SM_study_truth_fsr_photon_dr_less_04_with_muon_n"]->Fill(nFSRphotons_smallDRwithMuon, m_mcEventWeight);
      }
      if ( m_ZtruthChannel == "Zee" ) { // For Sherpa Zee sample
        hMap1D["SM_study_truth_fsr_photon_dr_less_04_with_electron_n"]->Fill(nFSRphotons_smallDRwithElectron, m_mcEventWeight);
      }


      for (const auto &phot : *m_truthPromptOrFSRPhotons) {

        bool isFSRPhoton = phot->auxdata<unsigned int>("classifierParticleOrigin") == 40;

        // For dressed phtons
        if (phot->auxdata<bool>("IsDressingPhoton")){
          hMap1D["SM_study_truth_dressed_photon_pt"]->Fill(phot->pt() * 0.001, m_mcEventWeight);
        }
        // For non-dressed FSR photons
        if (isFSRPhoton && !phot->auxdata<bool>("IsDressingPhoton")) {
          hMap1D["SM_study_truth_nondressed_photon_pt"]->Fill(phot->pt() * 0.001, m_mcEventWeight);
          //std::cout << "non-dressed FSR photon pt = " << phot->pt() << endl;
        }
        // For Prompt photons (except FSR) associated with leptons
        if (phot->auxdata<bool>("IsDressingPhoton") && !isFSRPhoton ){
          hMap1D["SM_study_truth_prompt_dressed_photon_pt"]->Fill(phot->pt() * 0.001, m_mcEventWeight);
        }
      }


    } // Not EXOT5





    ///////////////////////////////////////
    // Retrieve truthJet for the Z->nunu //
    ///////////////////////////////////////

    /// shallow copy to retrive auxdata variables
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_Jet_shallowCopy = xAOD::shallowCopyContainer( *m_truthJetsContainer );
    xAOD::JetContainer* truth_JetSC = truth_Jet_shallowCopy.first;

    for (const auto &jet : *truth_JetSC) {
      hMap1D["SM_study_nominal_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

      // Store in m_truthNominalJet
      xAOD::Jet* truthNominalJets = new xAOD::Jet();
      truthNominalJets->makePrivateStore(*jet);
      m_truthNominalJet->push_back(truthNominalJets);

    }

    // record your deep copied jet container (and aux container) to the store
    //ANA_CHECK(m_store->record( m_truthNominalJet, "truthNominalJet" ));
    //ANA_CHECK(m_store->record( m_truthNominalJetAux, "truthNominalJetAux." ));

    delete truth_Jet_shallowCopy.first;
    delete truth_Jet_shallowCopy.second;

    hMap1D["SM_study_nominal_jet_n"]->Fill(m_truthNominalJet->size(), m_mcEventWeight);



    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT


      ////////////////////////////////////////////////////////////////////////
      // Duplicate truthWZJet for the emulated bare and born jet collection //
      ////////////////////////////////////////////////////////////////////////

      /// shallow copy to retrive auxdata variables
      // WZ jet
      std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_wzJet_shallowCopy = xAOD::shallowCopyContainer( *m_truthWZJetsContainer );
      xAOD::JetContainer* truth_wzJetSC = truth_wzJet_shallowCopy.first;

      // Deep copy for the duplicated WZJet
      for (const auto &jet : *truth_wzJetSC) {
        hMap1D["SM_study_wz_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        // Store in m_truthWZJet
        xAOD::Jet* truthWZJets = new xAOD::Jet();
        truthWZJets->makePrivateStore(*jet);
        m_truthWZJet->push_back(truthWZJets);

        // Store in m_copyTruthWZJet
        xAOD::Jet* copyWZJets = new xAOD::Jet();
        copyWZJets->makePrivateStore(*jet);
        m_copyTruthWZJet->push_back(copyWZJets);
      }

      // record your deep copied jet container (and aux container) to the store
      //ANA_CHECK(m_store->record( m_truthWZJet, "truthWZJet" ));
      //ANA_CHECK(m_store->record( m_truthWZJetAux, "truthWZJetAux." ));
      //ANA_CHECK(m_store->record( m_copyTruthWZJet, "copyTruthWZJet" ));
      //ANA_CHECK(m_store->record( m_copyTruthWZJetAux, "copyTruthWZJetAux." ));

      delete truth_wzJet_shallowCopy.first;
      delete truth_wzJet_shallowCopy.second;

      hMap1D["SM_study_wz_jet_n"]->Fill(m_truthWZJet->size(), m_mcEventWeight);




      //-------------------------------------------------
      // Create my emulated bare and born jet collection
      //-------------------------------------------------
      for (const auto &jet : *m_copyTruthWZJet) {

        TLorentzVector wz_jet = jet->p4();
        auto bareJet = wz_jet;
        auto bornJet = wz_jet;

        //std::cout << "================================" << std::endl;
        //std::cout << " TruthWZJet pT = " << jet->pt() << endl;

        ////////////////
        // For Sherpa //
        ////////////////
        if (is_Sherpa) {

          for (const auto &phot : *m_truthPromptOrFSRPhotons) {

            bool isFSRPhoton = phot->auxdata<unsigned int>("classifierParticleOrigin") == 40;

            TLorentzVector photonP4 = phot->p4();
            double dR = jet->p4().DeltaR(phot->p4());

            // Calculate bareJet
            // ---------------------
            // For both dressing Prompt and FSR photons
            if (phot->auxdata<bool>("IsDressingPhoton")) {
              if (dR < 0.4) {
                bareJet = bareJet + photonP4;
                //std::cout << " Emulated bare jet pT (now processing) = " << bareJet.Pt() << endl;
              }
            }
            // Calculate bornJet
            // ---------------------
            // For only non-dressing FSR photons
            if (isFSRPhoton && !phot->auxdata<bool>("IsDressingPhoton")) {
              if (dR < 0.4) {
                bornJet = bornJet - photonP4;
                //std::cout << " Emulated born jet pT (now processing) = " << bornJet.Pt() << endl;
              }
            }
            // For dressing Prompt photons
            if (!isFSRPhoton && phot->auxdata<bool>("IsDressingPhoton")) {
              if (dR < 0.4) {
                bornJet = bornJet + photonP4;
              }
            }

          } // Prompt or FSR photons

        } // for Sherpa

        else if (is_MG) {
          //////////////////
          // For MadGraph //
          //////////////////
          for (const auto &phot : *m_truthMadGraphPhotons) {

            TLorentzVector photonP4 = phot->p4();
            double dR = jet->p4().DeltaR(phot->p4());

            // Calculate bareJet
            // ---------------------
            // For both dressing Prompt and FSR photons
            if (phot->auxdata<bool>("IsMadGraphDressingPhoton")) {
              if (dR < 0.4) {
                bareJet = bareJet + photonP4;
                //std::cout << " Emulated bare jet pT (now processing) = " << bareJet.Pt() << endl;
              }
            }
            // Calculate bornJet
            // ---------------------
            // For only non-dressing FSR photons
            if (phot->auxdata<bool>("IsMadGraphFSRPhoton") && !phot->auxdata<bool>("IsMadGraphDressingPhoton")) {
              if (dR < 0.4) {
                bornJet = bornJet - photonP4;
                //std::cout << " Emulated born jet pT (now processing) = " << bornJet.Pt() << endl;
              }
            }
            // For dressing Prompt photons
            if (!phot->auxdata<bool>("IsMadGraphFSRPhoton") && phot->auxdata<bool>("IsMadGraphDressingPhoton")) {
              if (dR < 0.4) {
                bornJet = bornJet + photonP4;
              }
            }

          } // FSR photons

        } // for MadGraph


        //std::cout << " Emulated bare jet pT (completed) = " << bareJet.Pt() << endl;
        //std::cout << " Emulated born jet pT (completed) = " << bornJet.Pt() << endl;

        // Store bareJets
        xAOD::JetFourMom_t bareJetp4 (bareJet.Pt(), bareJet.Eta(), bareJet.Phi(), bareJet.M());
        jet->setJetP4 (bareJetp4); // we've overwritten the 4-momentum
        if (jet->pt() > 5000. && std::abs(jet->eta()) < 5.) { // TRUTH1 jet cuts (defined in TRUTH1.py and CopyTruthJetParticles.cxx)
          xAOD::Jet* bareJets = new xAOD::Jet();
          bareJets->makePrivateStore(*jet);
          m_truthEmulBareJet->push_back(bareJets);
        }

        // Store bornJets
        xAOD::JetFourMom_t bornJetp4 (bornJet.Pt(), bornJet.Eta(), bornJet.Phi(), bornJet.M());
        jet->setJetP4 (bornJetp4); // we've overwritten the 4-momentum
        if (jet->pt() > 5000. && std::abs(jet->eta()) < 5.) { // TRUTH1 jet cuts (defined in TRUTH1.py and CopyTruthJetParticles.cxx)
          xAOD::Jet* bornJets = new xAOD::Jet();
          bornJets->makePrivateStore(*jet);
          m_truthEmulBornJet->push_back(bornJets);
        }

      }  // WZJets

      // record your deep copied jet container (and aux container) to the store
      //ANA_CHECK(m_store->record( m_truthEmulBareJet, "truthEmulBareJet" ));
      //ANA_CHECK(m_store->record( m_truthEmulBareJetAux, "truthEmulBareJetAux." ));
      //ANA_CHECK(m_store->record( m_truthEmulBornJet, "truthEmulBornJet" ));
      //ANA_CHECK(m_store->record( m_truthEmulBornJetAux, "truthEmulBornJetAux." ));




      ///////////////////////////////////////////////////////////////////////////////////
      // Add a new jet to the bare jet when a dressing photon is isolated from any jet //
      ///////////////////////////////////////////////////////////////////////////////////

      ////////////
      // Sherpa //
      ////////////
      if (is_Sherpa) {
        // Tag all isolated (dressing) photons from WZ jets
        for (const auto &phot : *m_truthPromptOrFSRPhotons) {

          phot->auxdata<bool>("IsIsoDressingPhoton") = false;

          // For dressing photons
          if (phot->auxdata<bool>("IsDressingPhoton")) {

            phot->auxdata<bool>("IsIsoDressingPhoton") = true;

            for (const auto &jet : *m_truthWZJet) {

              double dR = jet->p4().DeltaR(phot->p4());

              if (dR < 0.4) {
                phot->auxdata<bool>("IsIsoDressingPhoton") = false;
              } 

            } // WZ jet loop
          } // Dressing photons
        } // Prompt or FSR photons
      } // For Sherpa

      //////////////
      // MadGraph //
      //////////////
      else if (is_MG) {
        // Tag all isolated (dressing) photons from WZ jets
        for (const auto &phot : *m_truthMadGraphPhotons) {

          phot->auxdata<bool>("IsIsoDressingPhoton") = false;

          // For dressing photons
          if (phot->auxdata<bool>("IsMadGraphDressingPhoton")) {

            phot->auxdata<bool>("IsIsoDressingPhoton") = true;

            for (const auto &jet : *m_truthWZJet) {

              double dR = jet->p4().DeltaR(phot->p4());

              if (dR < 0.4) {
                phot->auxdata<bool>("IsIsoDressingPhoton") = false;
              } 

            } // WZ jet loop
          } // Dressing photons
        } // FSR photons
      } // For MadGraph




      // Create a jet container to store additional jets which are far from WZ jets
      xAOD::JetContainer* m_extraBareJet = new xAOD::JetContainer();
      xAOD::AuxContainerBase* m_extraBareJetAux = new xAOD::AuxContainerBase();
      m_extraBareJet->setStore( m_extraBareJetAux ); //< Connect the two

      // record your deep copied jet container (and aux container) to the store
      //ANA_CHECK(m_store->record( m_extraBareJet, "extraBareJet" ));
      //ANA_CHECK(m_store->record( m_extraBareJetAux, "extraBareJetAux." ));



      //////////////////////////////////////////////////////////////////////////////
      // Loop over isolated (dressing) Prompt and FSR photons and create new jets //
      //////////////////////////////////////////////////////////////////////////////
      // /////////
      // Sherpa //
      // /////////
      if (is_Sherpa) {
        for (const auto &phot : *m_truthPromptOrFSRPhotons) {

          if (phot->auxdata<bool>("IsIsoDressingPhoton")) {

            TLorentzVector photonP4 = phot->p4();

            // First photon creates a new jet
            if (m_extraBareJet->size() == 0) {

              for (const auto &jet : *m_copyTruthWZJet) {
                // Add a photon 4-momentum to the extra bare Jet vector
                xAOD::JetFourMom_t newJetp4 (phot->pt(), phot->eta(), phot->phi(), phot->m());
                jet->setJetP4 (newJetp4); // we've overwritten the 4-momentum
                xAOD::Jet* newJets = new xAOD::Jet();
                newJets->makePrivateStore(*jet);
                m_extraBareJet->push_back(newJets);
                break;
              }

            } else {

              // From the second photon, check if there is jet (just added) near this photon
              bool isIsoPhotonFromNewjets = true;
              for (const auto &jet : *m_extraBareJet) {
                double dR = jet->p4().DeltaR(phot->p4());
                if (dR < 0.4) {
                  isIsoPhotonFromNewjets = false;
                }
              }

              // If there is a new jet located near this photon, add this photon 4-momentum to the new jet
              if (!isIsoPhotonFromNewjets) {
                for (const auto &jet : *m_extraBareJet) {
                  double dR = jet->p4().DeltaR(phot->p4());
                  if (dR < 0.4) {
                    // 4-vector sum (jet + photon)
                    TLorentzVector jetP4 = jet->p4();
                    auto bareJet = jetP4 + photonP4;
                    xAOD::JetFourMom_t addedJetp4 (bareJet.Pt(), bareJet.Eta(), bareJet.Phi(), bareJet.M());
                    jet->setJetP4 (addedJetp4); // we've overwritten the 4-momentum
                  }
                }

                // If there is no jet found near this photon, create new jet using this photon 4-momentum
              } else {

                // Add newJets to Emulated Bare Jet vector
                for (const auto &jet : *m_copyTruthWZJet) {
                  xAOD::JetFourMom_t newJetp4 (phot->pt(), phot->eta(), phot->phi(), phot->m());
                  jet->setJetP4 (newJetp4); // we've overwritten the 4-momentum
                  xAOD::Jet* newJets = new xAOD::Jet();
                  newJets->makePrivateStore(*jet);
                  m_extraBareJet->push_back(newJets);
                  break;
                }

              }

            }

          } // Isolated dressing photons from WZ jets

        } // Prompt or FSR photons

      } // For Sherpa
      // ///////////
      // MadGraph //
      // ///////////
      if (is_MG) {
        for (const auto &phot : *m_truthMadGraphPhotons) {

          if (phot->auxdata<bool>("IsIsoDressingPhoton")) {

            TLorentzVector photonP4 = phot->p4();

            // First photon creates a new jet
            if (m_extraBareJet->size() == 0) {

              for (const auto &jet : *m_copyTruthWZJet) {
                // Add a photon 4-momentum to the extra bare Jet vector
                xAOD::JetFourMom_t newJetp4 (phot->pt(), phot->eta(), phot->phi(), phot->m());
                jet->setJetP4 (newJetp4); // we've overwritten the 4-momentum
                xAOD::Jet* newJets = new xAOD::Jet();
                newJets->makePrivateStore(*jet);
                m_extraBareJet->push_back(newJets);
                break;
              }

            } else {

              // From the second photon, check if there is jet (just added) near this photon
              bool isIsoPhotonFromNewjets = true;
              for (const auto &jet : *m_extraBareJet) {
                double dR = jet->p4().DeltaR(phot->p4());
                if (dR < 0.4) {
                  isIsoPhotonFromNewjets = false;
                }
              }

              // If there is a new jet located near this photon, add this photon 4-momentum to the new jet
              if (!isIsoPhotonFromNewjets) {
                for (const auto &jet : *m_extraBareJet) {
                  double dR = jet->p4().DeltaR(phot->p4());
                  if (dR < 0.4) {
                    // 4-vector sum (jet + photon)
                    TLorentzVector jetP4 = jet->p4();
                    auto bareJet = jetP4 + photonP4;
                    xAOD::JetFourMom_t addedJetp4 (bareJet.Pt(), bareJet.Eta(), bareJet.Phi(), bareJet.M());
                    jet->setJetP4 (addedJetp4); // we've overwritten the 4-momentum
                  }
                }

                // If there is no jet found near this photon, create new jet using this photon 4-momentum
              } else {

                // Add newJets to Emulated Bare Jet vector
                for (const auto &jet : *m_copyTruthWZJet) {
                  xAOD::JetFourMom_t newJetp4 (phot->pt(), phot->eta(), phot->phi(), phot->m());
                  jet->setJetP4 (newJetp4); // we've overwritten the 4-momentum
                  xAOD::Jet* newJets = new xAOD::Jet();
                  newJets->makePrivateStore(*jet);
                  m_extraBareJet->push_back(newJets);
                  break;
                }

              }

            }

          } // Isolated dressing photons from WZ jets

        } // FSR photons

      } // For MadGraph



      // Store created  extra bare jets to Emulated Bare jet container
      if (m_extraBareJet->size() > 0) {
        for (const auto &jet : *m_extraBareJet) {
          if (jet->pt() > 5000. && std::abs(jet->eta()) < 5.) { // TRUTH1 jet cuts (defined in TRUTH1.py and CopyTruthJetParticles.cxx)
            xAOD::Jet* extraBareJets = new xAOD::Jet();
            extraBareJets->makePrivateStore(*jet);
            m_truthEmulBareJet->push_back(extraBareJets);
          }
        }
      }


      delete m_extraBareJet;
      delete m_extraBareJetAux;




      ////////////////////////////////////
      // For TruthWZJet validation test //
      ////////////////////////////////////
      for (const auto &phot : *m_truthPromptOrFSRPhotons) {

        // For dressed FSR photons
        if (phot->auxdata<bool>("IsDressingPhoton")) {

          for (const auto &jet : *m_truthWZJet) {

            double dR = jet->p4().DeltaR(phot->p4());

            if (dR < 0.4) {
              //std::cout << "An dressed FSR photon (pT = " << phot->pt() << " ) near a WZ jet (pT = " << jet->pt() << " ) "<< std::endl;
              if (phot->pt() == jet->pt()) {
                //std::cout << "!!!Found!!! An dressed FSR photon (pT = " << phot->pt() << " ) created a WZ jet (pT = " << jet->pt() << " ) "<< std::endl;
              }
            } 

          } // jet loop

        } // For dressed FSR photons

      } // FSR photon loop






      //////////////////////////
      // Retrieve Custom Jets //
      //////////////////////////

      /// shallow copy to retrive auxdata variables
      if (is_customDerivation) {

        // Retrieve my custom jets
        // Dress-level
        const xAOD::JetContainer* m_truthDressJets = nullptr;
        if ( !m_event->retrieve( m_truthDressJets, "AntiKt4TruthDressJets" ).isSuccess() ){
          Error("execute()", "Failed to retrieve AntiKt4TruthDressJets container. Exiting." );
          return EL::StatusCode::FAILURE;
        }
        // Bare-level
        const xAOD::JetContainer* m_truthBareJets = nullptr;
        if ( !m_event->retrieve( m_truthBareJets, "AntiKt4TruthBareJets" ).isSuccess() ){
          Error("execute()", "Failed to retrieve AntiKt4TruthBareJets container. Exiting." );
          return EL::StatusCode::FAILURE;
        }
        // Born-level
        const xAOD::JetContainer* m_truthBornJets = nullptr;
        if ( !m_event->retrieve( m_truthBornJets, "AntiKt4TruthBornJets" ).isSuccess() ){
          Error("execute()", "Failed to retrieve AntiKt4TruthBornJets container. Exiting." );
          return EL::StatusCode::FAILURE;
        }

        // Dress-level shallow copy
        std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_dressJet_shallowCopy = xAOD::shallowCopyContainer( *m_truthDressJets );
        xAOD::JetContainer* truth_dressJetSC = truth_dressJet_shallowCopy.first;
        // Bare level shallow copy
        std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_bareJet_shallowCopy = xAOD::shallowCopyContainer( *m_truthBareJets );
        xAOD::JetContainer* truth_bareJetSC = truth_bareJet_shallowCopy.first;
        // Born level shallow copy
        std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > truth_bornJet_shallowCopy = xAOD::shallowCopyContainer( *m_truthBornJets );
        xAOD::JetContainer* truth_bornJetSC = truth_bornJet_shallowCopy.first;

        // Dress-level jet
        int nCustomDressJet = 0.;
        for (const auto &jet : *truth_dressJetSC) {
          hMap1D["SM_study_custom_dress_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
          nCustomDressJet++;
          if (jet->pt() > 25000.){
            hMap1D["SM_study_custom_dress_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
            // Store in m_goodCustomDressJet
            xAOD::Jet* goodCustomDressJets = new xAOD::Jet();
            goodCustomDressJets->makePrivateStore(*jet);
            m_goodCustomDressJet->push_back(goodCustomDressJets);
          }
        } // Dress Jet
        // record your deep copied jet container (and aux container) to the store
        //ANA_CHECK(m_store->record( m_goodCustomDressJet, "goodCustomDressJet" ));
        //ANA_CHECK(m_store->record( m_goodCustomDressJetAux, "goodCustomDressJetAux." ));

        hMap1D["SM_study_custom_dress_jet_n"]->Fill(nCustomDressJet, m_mcEventWeight);
        hMap1D["SM_study_custom_dress_good_jet_n"]->Fill(m_goodCustomDressJet->size(), m_mcEventWeight);

        // Bare-level jet
        int nCustomBareJet = 0.;
        for (const auto &jet : *truth_bareJetSC) {
          hMap1D["SM_study_custom_bare_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
          nCustomBareJet++;
          if (jet->pt() > 25000.){
            hMap1D["SM_study_custom_bare_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
            // Store in m_goodCustomBareJet
            xAOD::Jet* goodCustomBareJets = new xAOD::Jet();
            goodCustomBareJets->makePrivateStore(*jet);
            m_goodCustomBareJet->push_back(goodCustomBareJets);
          }
        } // Bare Jet
        // record your deep copied jet container (and aux container) to the store
        //ANA_CHECK(m_store->record( m_goodCustomBareJet, "goodCustomBareJet" ));
        //ANA_CHECK(m_store->record( m_goodCustomBareJetAux, "goodCustomBareJetAux." ));

        hMap1D["SM_study_custom_bare_jet_n"]->Fill(nCustomBareJet, m_mcEventWeight);
        hMap1D["SM_study_custom_bare_good_jet_n"]->Fill(m_goodCustomBareJet->size(), m_mcEventWeight);

        // Born-level jet
        int nCustomBornJet = 0.;
        for (const auto &jet : *truth_bornJetSC) {
          hMap1D["SM_study_custom_born_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
          nCustomBornJet++;
          if (jet->pt() > 25000.){
            hMap1D["SM_study_custom_born_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
            // Store in m_goodCustomBornJet
            xAOD::Jet* goodCustomBornJets = new xAOD::Jet();
            goodCustomBornJets->makePrivateStore(*jet);
            m_goodCustomBornJet->push_back(goodCustomBornJets);
          }
        } // Born Jet
        // record your deep copied jet container (and aux container) to the store
        //ANA_CHECK(m_store->record( m_goodCustomBornJet, "goodCustomBornJet" ));
        //ANA_CHECK(m_store->record( m_goodCustomBornJetAux, "goodCustomBornJetAux." ));

        hMap1D["SM_study_custom_born_jet_n"]->Fill(nCustomBornJet, m_mcEventWeight);
        hMap1D["SM_study_custom_born_good_jet_n"]->Fill(m_goodCustomBornJet->size(), m_mcEventWeight);


        delete truth_dressJet_shallowCopy.first;
        delete truth_dressJet_shallowCopy.second;

        delete truth_bareJet_shallowCopy.first;
        delete truth_bareJet_shallowCopy.second;

        delete truth_bornJet_shallowCopy.first;
        delete truth_bornJet_shallowCopy.second;

      }


    } // Not EXOT5


    /////////////////////
    // TRUTH1 analysis //
    /////////////////////



    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT

      //----------------------------------------------------
      // Select signal or Z boson bare and dressed Electrons
      //----------------------------------------------------
      for (const auto &electron : *m_truthElectron) {

        // Store bare electrons from Z Boson
        if (electron->auxdata<unsigned int>("classifierParticleOrigin") == 13){
          xAOD::TruthParticle* bareElectronFromZ = new xAOD::TruthParticle();
          m_truthBareElectronFromZ->push_back( bareElectronFromZ );
          *bareElectronFromZ = *electron; // copies auxdata from one auxstore to the other
          hMap1D["SM_study_bare_electron_pt_from_Z"]->Fill(electron->pt() * 0.001, m_mcEventWeight);
          //if (m_eventCounter<50) std::cout << "Bare electron pT from Z= " << electron->pt() << std::endl;
        }

        // Store signal dressed Electrons
        TLorentzVector fourVector;
        fourVector.SetPtEtaPhiE(electron->auxdata<float>("pt_dressed"), electron->auxdata<float>("eta_dressed"), electron->auxdata<float>("phi_dressed"), electron->auxdata<float>("e_dressed"));
        electron->setE(fourVector.E());
        electron->setPx(fourVector.Px());
        electron->setPy(fourVector.Py());
        electron->setPz(fourVector.Pz());
        // Store dressed electrons from Z Boson
        if (electron->auxdata<unsigned int>("classifierParticleOrigin") == 13){
          xAOD::TruthParticle* dressElectronFromZ = new xAOD::TruthParticle();
          m_truthDressElectronFromZ->push_back( dressElectronFromZ );
          *dressElectronFromZ = *electron; // copies auxdata from one auxstore to the other
          hMap1D["SM_study_dress_electron_pt_from_Z"]->Fill(electron->pt() * 0.001, m_mcEventWeight);
          //std::cout << "Dressed electron pT from Z= " << electron->pt() << std::endl;
        }
      }

      // Sort truth electrons
      if (m_truthBornElectron->size() > 1) std::partial_sort(m_truthBornElectron->begin(), m_truthBornElectron->begin()+2, m_truthBornElectron->end(), DescendingPt());
      if (m_truthBareElectronFromZ->size() > 1) std::partial_sort(m_truthBareElectronFromZ->begin(), m_truthBareElectronFromZ->begin()+2, m_truthBareElectronFromZ->end(), DescendingPt());
      if (m_truthDressElectronFromZ->size() > 1) std::partial_sort(m_truthDressElectronFromZ->begin(), m_truthDressElectronFromZ->begin()+2, m_truthDressElectronFromZ->end(), DescendingPt());


      //----------------------------------------------------
      // Select signal or Z boson bare and dressed Muons
      //----------------------------------------------------
      for (const auto &muon : *m_truthMuon) {

        // Store bare muons from Z Boson
        if (muon->auxdata<unsigned int>("classifierParticleOrigin") == 13){
          xAOD::TruthParticle* bareMuonFromZ = new xAOD::TruthParticle();
          m_truthBareMuonFromZ->push_back( bareMuonFromZ );
          *bareMuonFromZ = *muon; // copies auxdata from one auxstore to the other
          hMap1D["SM_study_bare_muon_pt_from_Z"]->Fill(muon->pt() * 0.001, m_mcEventWeight);
          //std::cout << "Bare muon pT from Z= " << muon->pt() << std::endl;
        }

        // Store signal dressed Muons
        TLorentzVector fourVector;
        fourVector.SetPtEtaPhiE(muon->auxdata<float>("pt_dressed"), muon->auxdata<float>("eta_dressed"), muon->auxdata<float>("phi_dressed"), muon->auxdata<float>("e_dressed"));
        muon->setE(fourVector.E());
        muon->setPx(fourVector.Px());
        muon->setPy(fourVector.Py());
        muon->setPz(fourVector.Pz());
        // Store dressed muons from Z Boson
        if (muon->auxdata<unsigned int>("classifierParticleOrigin") == 13){
          xAOD::TruthParticle* dressMuonFromZ = new xAOD::TruthParticle();
          m_truthDressMuonFromZ->push_back( dressMuonFromZ );
          *dressMuonFromZ = *muon; // copies auxdata from one auxstore to the other
          hMap1D["SM_study_dress_muon_pt_from_Z"]->Fill(muon->pt() * 0.001, m_mcEventWeight);
          //std::cout << "Dressed muon pT from Z= " << muon->pt() << std::endl;
        }
      }

      // Sort truth muons
      if (m_truthBornMuon->size() > 1) std::partial_sort(m_truthBornMuon->begin(), m_truthBornMuon->begin()+2, m_truthBornMuon->end(), DescendingPt());
      if (m_truthBareMuonFromZ->size() > 1) std::partial_sort(m_truthBareMuonFromZ->begin(), m_truthBareMuonFromZ->begin()+2, m_truthBareMuonFromZ->end(), DescendingPt());
      if (m_truthDressMuonFromZ->size() > 1) std::partial_sort(m_truthDressMuonFromZ->begin(), m_truthDressMuonFromZ->begin()+2, m_truthDressMuonFromZ->end(), DescendingPt());



    } // Not EXOT5



    //----------------------------------------------------
    // Select signal or Z boson Neutrinos
    //----------------------------------------------------
    for (const auto &neutrino : *m_truthNeutrino) {

      // Store neutrinos from Z Boson
      if (neutrino->auxdata<unsigned int>("classifierParticleOrigin") == 13){
        xAOD::TruthParticle* neutrinoFromZ = new xAOD::TruthParticle();
        m_truthNeutrinoFromZ->push_back( neutrinoFromZ );
        *neutrinoFromZ = *neutrino; // copies auxdata from one auxstore to the other
        hMap1D["SM_study_neutrino_pt_from_Z"]->Fill(neutrino->pt() * 0.001, m_mcEventWeight);
        //std::cout << "neutrino pT from Z = " << neutrino->pt() << std::endl;
      }

    }


    // Sort truth neutrinos
    if (m_truthNeutrinoFromZ->size() > 1) std::partial_sort(m_truthNeutrinoFromZ->begin(), m_truthNeutrinoFromZ->begin()+2, m_truthNeutrinoFromZ->end(), DescendingPt());

    //for (const auto &neutrino : *m_truthNeutrinoFromZ) {
    //  std::cout << "sorted neutrino pT from Z = " << neutrino->pt() << std::endl;
    //}


    //------------------
    // Select good Jets
    //------------------

    // Nominal jet
    for (const auto &jet : *m_truthNominalJet) {

      if (jet->pt() < sm_goodJetPtCut || std::abs(jet->rapidity()) > 4.4) continue;

      hMap1D["SM_study_nominal_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
      // Store in m_goodTruthNominalJet
      xAOD::Jet* goodNominalJets = new xAOD::Jet();
      goodNominalJets->makePrivateStore(*jet);
      m_goodTruthNominalJet->push_back(goodNominalJets);

    } // Nominal jet

    hMap1D["SM_study_nominal_good_jet_n"]->Fill(m_goodTruthNominalJet->size(), m_mcEventWeight);


    //----------------------
    // Overlap Removal (OR)
    //----------------------
    // AntiKt4TruthJets : Exclude neutrinos and muons in the clustering, Include all electrons and all photons
    // --------------------------
    // For bare-level electorn
    for (const auto &jet : *m_truthNominalJet) {

      if (jet->pt() < sm_goodJetPtCut) continue;
      //std::cout << "[OR bare loop] truth good Jet pT = " << jet->pt() * 0.001 << std::endl;

      // Remove jets near the bare-level leptons (dR<0.4)
      bool orJetFromBareLep = false;
      for (const auto &electron : *m_truthBareElectronFromZ) {
        // where good electrons (i.e. pT > 7GeV, |eta| < 2.4) are used because we will use these electrons in reco level.
        if (electron->pt() > sm_lep2PtCut &&  std::abs(electron->eta()) < sm_lepEtaCut && deltaR(jet->eta(), electron->eta(), jet->phi(), electron->phi()) < sm_ORJETdeltaR) orJetFromBareLep = true;
      }

      if (orJetFromBareLep) continue;

      hMap1D["SM_study_bare_OR_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
      // Store in m_ORbareTruthNominalJet
      xAOD::Jet* ORbareNominalJets = new xAOD::Jet();
      ORbareNominalJets->makePrivateStore(*jet);
      m_ORbareTruthNominalJet->push_back(ORbareNominalJets);

    } // Nominal jet

    hMap1D["SM_study_bare_OR_good_jet_n"]->Fill(m_ORbareTruthNominalJet->size(), m_mcEventWeight);

    // --------------------------
    // For dress-level electorn
    for (const auto &jet : *m_truthNominalJet) {

      if (jet->pt() < sm_goodJetPtCut) continue;
      //std::cout << "[OR dress loop] truth good Jet pT = " << jet->pt() * 0.001 << std::endl;

      // Remove jets near the dress-level leptons (dR<0.4)
      bool orJetFromDressLep = false;
      for (const auto &electron : *m_truthDressElectronFromZ) {
        // where good electrons (i.e. pT > 7GeV, |eta| < 2.4) are used because we will use these electrons in reco level.
        if (electron->pt() > sm_lep2PtCut &&  std::abs(electron->eta()) < sm_lepEtaCut && deltaR(jet->eta(), electron->eta(), jet->phi(), electron->phi()) < sm_ORJETdeltaR) orJetFromDressLep = true;
      }

      if (orJetFromDressLep) continue;

      hMap1D["SM_study_dress_OR_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
      // Store in m_ORdressTruthNominalJet
      xAOD::Jet* ORdressNominalJets = new xAOD::Jet();
      ORdressNominalJets->makePrivateStore(*jet);
      m_ORdressTruthNominalJet->push_back(ORdressNominalJets);

    } // Nominal jet

    hMap1D["SM_study_dress_OR_good_jet_n"]->Fill(m_ORbareTruthNominalJet->size(), m_mcEventWeight);


    //-------------------------
    // Overlap subtraction (OS)
    //-------------------------
    // AntiKt4TruthJets : Exclude neutrinos and muons in the clustering, Include all electrons and all photons
    // --------------------------
    // For bare-level electorn

    // Copy good Truth jet container to rewrite overlap-subracted jets
    xAOD::JetContainer* m_copyGoodTruthJetForBareOS = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_copyGoodTruthJetForBareOSAux = new xAOD::AuxContainerBase();
    m_copyGoodTruthJetForBareOS->setStore( m_copyGoodTruthJetForBareOSAux ); //< Connect the two
    // Nominal good jet
    for (const auto &jet : *m_goodTruthNominalJet) {
      //std::cout << "[OS bare loop] truth good Jet pT = " << jet->pt() * 0.001 << std::endl;
      // Store in m_copyGoodTruthJetForBareOS
      xAOD::Jet* copyGoodTruthJetForBareOS = new xAOD::Jet();
      copyGoodTruthJetForBareOS->makePrivateStore(*jet);
      m_copyGoodTruthJetForBareOS->push_back(copyGoodTruthJetForBareOS);
    } // Nominal good jet

    // where good electrons (i.e. pT > 7GeV, |eta| < 2.4) are used because we will use these electrons in reco level.
    TLorentzVector bareElec1P4;
    TLorentzVector bareElec2P4;
    // Determine which jet is the most close to the lepton1
    float minDRbareElec1 = 1000.;
    if (m_truthBareElectronFromZ->size() > 0) {
      bareElec1P4 = m_truthBareElectronFromZ->at(0)->p4();
      if (m_truthBareElectronFromZ->at(0)->pt() > sm_lep2PtCut &&  std::abs(m_truthBareElectronFromZ->at(0)->eta()) < sm_lepEtaCut) {
        for (const auto &jet : *m_copyGoodTruthJetForBareOS) {
          float dR = deltaR(jet->eta(), m_truthBareElectronFromZ->at(0)->eta(), jet->phi(), m_truthBareElectronFromZ->at(0)->phi());
          if ( dR < minDRbareElec1 ) minDRbareElec1 = dR;
          //std::cout << " dR = " << dR << " , min dR between jet and electron1 = " << minDRbareElec1 << std::endl;
        }
      }
    }
    // Determine which jet is the most close to the lepton2
    float minDRbareElec2 = 1000.;
    if (m_truthBareElectronFromZ->size() > 1) {
      bareElec2P4 = m_truthBareElectronFromZ->at(1)->p4();
      if (m_truthBareElectronFromZ->at(1)->pt() > sm_lep2PtCut &&  std::abs(m_truthBareElectronFromZ->at(1)->eta()) < sm_lepEtaCut) {
        for (const auto &jet : *m_copyGoodTruthJetForBareOS) {
          float dR = deltaR(jet->eta(), m_truthBareElectronFromZ->at(1)->eta(), jet->phi(), m_truthBareElectronFromZ->at(1)->phi());
          if ( dR < minDRbareElec2 ) minDRbareElec2 = dR;
          //std::cout << " dR = " << dR << " , min dR between jet and electron2 = " << minDRbareElec2 << std::endl;
        }
      }
    }

    for (const auto &jet : *m_copyGoodTruthJetForBareOS) {
      // Subtract lepton 4 momentum from jets near the bare-level lepton (dR<0.4)
      TLorentzVector nominal_jet = jet->p4();
      auto subtracted_jet = nominal_jet;
      // Subtract electron1 from the jet if this jet is closest to the electron1
      if (m_truthBareElectronFromZ->size() > 0) {
        if (m_truthBareElectronFromZ->at(0)->pt() > sm_lep2PtCut &&  std::abs(m_truthBareElectronFromZ->at(0)->eta()) < sm_lepEtaCut) {
          float dR = deltaR(jet->eta(), m_truthBareElectronFromZ->at(0)->eta(), jet->phi(), m_truthBareElectronFromZ->at(0)->phi());
          //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron1 = " << minDRbareElec1 << std::endl;
          if ( dR < sm_ORJETdeltaR ) {
            if ( dR == minDRbareElec1 ) {
              // Implement subtraction
              subtracted_jet = subtracted_jet - bareElec1P4;
              //std::cout << " !! Found overlapped jet with electron 1 with dR = " << dR << std::endl;
            }
          }
        }
      }
      // Subtract electron2 from the jet if this jet is closest to the electron2
      if (m_truthBareElectronFromZ->size() > 1) {
        if (m_truthBareElectronFromZ->at(1)->pt() > sm_lep2PtCut &&  std::abs(m_truthBareElectronFromZ->at(1)->eta()) < sm_lepEtaCut) {
          float dR = deltaR(jet->eta(), m_truthBareElectronFromZ->at(1)->eta(), jet->phi(), m_truthBareElectronFromZ->at(1)->phi());
          //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron2 = " << minDRbareElec2 << std::endl;
          if ( dR < sm_ORJETdeltaR ) {
            if ( dR == minDRbareElec2 ) {
              //std::cout << " !! Found overlapped jet with electron 2 with dR = " << dR << std::endl;
              // Implement subtraction
              subtracted_jet = subtracted_jet - bareElec2P4;
            }
          }
        }
      }
      // Store bare-level lepton subtracted Jets
      xAOD::JetFourMom_t bareSubtracedJetP4 (subtracted_jet.Pt(), subtracted_jet.Eta(), subtracted_jet.Phi(), subtracted_jet.M());
      jet->setJetP4 (bareSubtracedJetP4); // we've overwritten the 4-momentum
      if (jet->pt() < 5000. || std::abs(jet->eta()) > 5.) continue; // TRUTH1 jet cuts (defined in TRUTH1.py and CopyTruthJetParticles.cxx)
      if (jet->pt() < sm_goodJetPtCut) continue; // veto secondary jets with i.e. 25GeV

      hMap1D["SM_study_bare_OS_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
      // Store in m_OSbareTruthNominalJet
      xAOD::Jet* bareSubtracedJets = new xAOD::Jet();
      bareSubtracedJets->makePrivateStore(*jet);
      m_OSbareTruthNominalJet->push_back(bareSubtracedJets);
    } // copied good truth jet
    hMap1D["SM_study_bare_OS_good_jet_n"]->Fill(m_OSbareTruthNominalJet->size(), m_mcEventWeight);

    delete m_copyGoodTruthJetForBareOS;
    delete m_copyGoodTruthJetForBareOSAux;


    // --------------------------
    // For dress-level electorn

    // Copy good Truth jet container to rewrite overlap-subracted jets
    xAOD::JetContainer* m_copyGoodTruthJetForDressOS = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_copyGoodTruthJetForDressOSAux = new xAOD::AuxContainerBase();
    m_copyGoodTruthJetForDressOS->setStore( m_copyGoodTruthJetForDressOSAux ); //< Connect the two
    // Nominal good jet
    for (const auto &jet : *m_goodTruthNominalJet) {
      //std::cout << "[OS dress loop] truth good Jet pT = " << jet->pt() * 0.001 << std::endl;
      // Store in m_copyGoodTruthJetForDressOS
      xAOD::Jet* copyGoodTruthJetForDressOS = new xAOD::Jet();
      copyGoodTruthJetForDressOS->makePrivateStore(*jet);
      m_copyGoodTruthJetForDressOS->push_back(copyGoodTruthJetForDressOS);
    } // Nominal good jet

    // where good electrons (i.e. pT > 7GeV, |eta| < 2.4) are used because we will use these electrons in reco level.
    TLorentzVector dressElec1P4;
    TLorentzVector dressElec2P4;
    // Determine which jet is the most close to the lepton1
    float minDRdressElec1 = 1000.;
    if (m_truthDressElectronFromZ->size() > 0) {
      dressElec1P4 = m_truthDressElectronFromZ->at(0)->p4();
      if (m_truthDressElectronFromZ->at(0)->pt() > sm_lep2PtCut &&  std::abs(m_truthDressElectronFromZ->at(0)->eta()) < sm_lepEtaCut) {
        for (const auto &jet : *m_copyGoodTruthJetForDressOS) {
          float dR = deltaR(jet->eta(), m_truthDressElectronFromZ->at(0)->eta(), jet->phi(), m_truthDressElectronFromZ->at(0)->phi());
          if ( dR < minDRdressElec1 ) minDRdressElec1 = dR;
          //std::cout << " dR = " << dR << " , min dR between jet and electron1 = " << minDRdressElec1 << std::endl;
        }
      }
    }
    // Determine which jet is the most close to the lepton2
    float minDRdressElec2 = 1000.;
    if (m_truthDressElectronFromZ->size() > 1) {
      dressElec2P4 = m_truthDressElectronFromZ->at(1)->p4();
      if (m_truthDressElectronFromZ->at(1)->pt() > sm_lep2PtCut &&  std::abs(m_truthDressElectronFromZ->at(1)->eta()) < sm_lepEtaCut) {
        for (const auto &jet : *m_copyGoodTruthJetForDressOS) {
          float dR = deltaR(jet->eta(), m_truthDressElectronFromZ->at(1)->eta(), jet->phi(), m_truthDressElectronFromZ->at(1)->phi());
          if ( dR < minDRdressElec2 ) minDRdressElec2 = dR;
          //std::cout << " dR = " << dR << " , min dR between jet and electron2 = " << minDRdressElec2 << std::endl;
        }
      }
    }

    for (const auto &jet : *m_copyGoodTruthJetForDressOS) {
      // Subtract lepton 4 momentum from jets near the dress-level lepton (dR<0.4)
      TLorentzVector nominal_jet = jet->p4();
      auto subtracted_jet = nominal_jet;
      // Subtract electron1 from the jet if this jet is closest to the electron1
      if (m_truthDressElectronFromZ->size() > 0) {
        if (m_truthDressElectronFromZ->at(0)->pt() > sm_lep2PtCut &&  std::abs(m_truthDressElectronFromZ->at(0)->eta()) < sm_lepEtaCut) {
          float dR = deltaR(jet->eta(), m_truthDressElectronFromZ->at(0)->eta(), jet->phi(), m_truthDressElectronFromZ->at(0)->phi());
          //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron1 = " << minDRdressElec1 << std::endl;
          if ( dR < sm_ORJETdeltaR ) {
            if ( dR == minDRdressElec1 ) {
              // Implement subtraction (mixed with removal)
              // In the truth level, dressed electron could have larger momentum than jet pT because FSR photons can be involved in the electron momentum.
              // In this case, subtraction cannot remove the jet, rather create a different jet with the opposite direction.
              // So if the jet pT is greater than the dressed electron, we will implement the overlap subtraction.
              // Otherwise, we will implement the overlap removal.
              if ( subtracted_jet.Pt() > dressElec1P4.Pt() ) {
                subtracted_jet = subtracted_jet - dressElec1P4; // subtraction
              } else { // Test if dressed electron pT is greater than jet pT when dressing photon is outside the jet cone (dR < 0.4)
                //std::cout << " !!!!!!!!!! [With Electron1] Found a dressing FSR photon outside jet !!!!!!!!!!! " << std::endl;
                //std::cout << " truth good Jet pT = " << jet->pt() * 0.001 << " , dress electron pT = " << dressElec1P4.Pt() * 0.001 << std::endl;
                for (const auto &phot : *m_truthPromptOrFSRPhotons) {
                  // For dressing FSR photons
                  if (phot->auxdata<bool>("IsDressingPhoton")) {
                    double dR_dressElePho = phot->p4().DeltaR(m_truthDressElectronFromZ->at(0)->p4());
                    if (dR_dressElePho < 0.1) {
                      for (const auto &electron : *m_truthBareElectronFromZ) {
                        double dR_bareElePho = phot->p4().DeltaR(electron->p4());
                        if (dR_bareElePho < 0.1) {
                          //std::cout << " dressing FSR photon pT = " << phot->pt() * 0.001 << " , bare electron pT = " << electron->pt() * 0.001 << std::endl;
                        }
                      }
                    }
                  }
                } // FSR photon loop
                // Implement removal
                subtracted_jet = subtracted_jet - subtracted_jet; // removal
              }
              //std::cout << " !! Found overlapped jet with electron 1 with dR = " << dR << std::endl;
            }
          }
        }
      } // end electron1 subtraction
      // Subtract electron2 from the jet if this jet is closest to the electron2
      if (m_truthDressElectronFromZ->size() > 1) {
        if (m_truthDressElectronFromZ->at(1)->pt() > sm_lep2PtCut &&  std::abs(m_truthDressElectronFromZ->at(1)->eta()) < sm_lepEtaCut) {
          float dR = deltaR(jet->eta(), m_truthDressElectronFromZ->at(1)->eta(), jet->phi(), m_truthDressElectronFromZ->at(1)->phi());
          //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron2 = " << minDRdressElec2 << std::endl;
          if ( dR < sm_ORJETdeltaR ) {
            if ( dR == minDRdressElec2 ) {
              //std::cout << " !! Found overlapped jet with electron 2 with dR = " << dR << std::endl;
              // Implement subtraction (mixed with removal)
              if ( subtracted_jet.Pt() > dressElec2P4.Pt() ) {
                subtracted_jet = subtracted_jet - dressElec2P4; // subtraction
              } else { // Test if dressed electron pT is greater than (maybe subtracted with electron 1) jet pT when dressing photon is outside the jet cone (dR < 0.4)
                //std::cout << " !!!!!!!!!! [With Electron2] Found a dressing FSR photon outside jet !!!!!!!!!!! " << std::endl;
                //std::cout << " good or subtracted (with electron1) Jet pT = " << subtracted_jet.Pt() * 0.001 << " , dress electron pT = " << dressElec2P4.Pt() * 0.001 << std::endl;
                for (const auto &phot : *m_truthPromptOrFSRPhotons) {
                  // For dressing FSR photons
                  if (phot->auxdata<bool>("IsDressingPhoton")) {
                    double dR_dressElePho = phot->p4().DeltaR(m_truthDressElectronFromZ->at(1)->p4());
                    if (dR_dressElePho < 0.1) {
                      for (const auto &electron : *m_truthBareElectronFromZ) {
                        double dR_bareElePho = phot->p4().DeltaR(electron->p4());
                        if (dR_bareElePho < 0.1) {
                          //std::cout << " dressing FSR photon pT = " << phot->pt() * 0.001 << " , bare electron pT = " << electron->pt() * 0.001 << std::endl;
                        }
                      }
                    }
                  } 
                } // FSR photon loop
                // Implement removal
                subtracted_jet = subtracted_jet - subtracted_jet; // removal
              }
            }
          }
        }
      } // end electron2 subtraction
      // Store dress-level lepton subtracted Jets
      xAOD::JetFourMom_t dressSubtracedJetP4 (subtracted_jet.Pt(), subtracted_jet.Eta(), subtracted_jet.Phi(), subtracted_jet.M());
      jet->setJetP4 (dressSubtracedJetP4); // we've overwritten the 4-momentum
      if (jet->pt() < 5000. || std::abs(jet->eta()) > 5.) continue; // TRUTH1 jet cuts (defined in TRUTH1.py and CopyTruthJetParticles.cxx)
      if (jet->pt() < sm_goodJetPtCut) continue; // veto secondary jets with i.e. 25GeV

      hMap1D["SM_study_dress_OS_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);
      // Store in m_OSdressTruthNominalJet
      xAOD::Jet* dressSubtracedJets = new xAOD::Jet();
      dressSubtracedJets->makePrivateStore(*jet);
      m_OSdressTruthNominalJet->push_back(dressSubtracedJets);
    } // copied good truth jets

    hMap1D["SM_study_dress_OS_good_jet_n"]->Fill(m_OSdressTruthNominalJet->size(), m_mcEventWeight);

    delete m_copyGoodTruthJetForDressOS;
    delete m_copyGoodTruthJetForDressOSAux;










    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // Not EXOT

      // WZ jet
      for (const auto &jet : *m_truthWZJet) {

        if (jet->pt() < sm_goodJetPtCut || std::abs(jet->rapidity()) > 4.4) continue;

        hMap1D["SM_study_wz_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        // Store in m_goodTruthWZJet
        xAOD::Jet* goodWZJets = new xAOD::Jet();
        goodWZJets->makePrivateStore(*jet);
        m_goodTruthWZJet->push_back(goodWZJets);

      } // WZ jet

      hMap1D["SM_study_wz_good_jet_n"]->Fill(m_goodTruthWZJet->size(), m_mcEventWeight);

      // Emulated Bare jet
      for (const auto &jet : *m_truthEmulBareJet) {

        hMap1D["SM_study_emulated_bare_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        if (jet->pt() < sm_goodJetPtCut || std::abs(jet->rapidity()) > 4.4) continue;

        hMap1D["SM_study_emulated_bare_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        // Store in m_goodEmulBareJet
        xAOD::Jet* goodEmulBareJets = new xAOD::Jet();
        goodEmulBareJets->makePrivateStore(*jet);
        m_goodEmulBareJet->push_back(goodEmulBareJets);

      } // Bare Jet

      hMap1D["SM_study_emulated_bare_jet_n"]->Fill(m_truthEmulBareJet->size(), m_mcEventWeight);
      hMap1D["SM_study_emulated_bare_good_jet_n"]->Fill(m_goodEmulBareJet->size(), m_mcEventWeight);

      // Emulated Born jet
      for (const auto &jet : *m_truthEmulBornJet) {

        hMap1D["SM_study_emulated_born_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        if (jet->pt() < sm_goodJetPtCut || std::abs(jet->rapidity()) > 4.4) continue;

        hMap1D["SM_study_emulated_born_good_jet_pt"]->Fill(jet->pt() * 0.001, m_mcEventWeight);

        // Store in m_goodEmulBonrJet
        xAOD::Jet* goodEmulBornJets = new xAOD::Jet();
        goodEmulBornJets->makePrivateStore(*jet);
        m_goodEmulBornJet->push_back(goodEmulBornJets);

      } // Born Jet

      hMap1D["SM_study_emulated_born_jet_n"]->Fill(m_truthEmulBornJet->size(), m_mcEventWeight);
      hMap1D["SM_study_emulated_born_good_jet_n"]->Fill(m_goodEmulBornJet->size(), m_mcEventWeight);


    } // Not EXOT5



    /////////////////////
    // Sort truth jets //
    /////////////////////
    if (m_goodTruthNominalJet->size() > 1) std::sort(m_goodTruthNominalJet->begin(), m_goodTruthNominalJet->end(), DescendingPt());
    if (m_goodTruthWZJet->size() > 1) std::sort(m_goodTruthWZJet->begin(), m_goodTruthWZJet->end(), DescendingPt());
    if (m_goodCustomDressJet->size() > 1) std::sort(m_goodCustomDressJet->begin(), m_goodCustomDressJet->end(), DescendingPt());
    if (m_goodCustomBareJet->size() > 1) std::sort(m_goodCustomBareJet->begin(), m_goodCustomBareJet->end(), DescendingPt());
    if (m_goodEmulBareJet->size() > 1) std::sort(m_goodEmulBareJet->begin(), m_goodEmulBareJet->end(), DescendingPt());
    if (m_goodCustomBornJet->size() > 1) std::sort(m_goodCustomBornJet->begin(), m_goodCustomBornJet->end(), DescendingPt());
    if (m_goodEmulBornJet->size() > 1) std::sort(m_goodEmulBornJet->begin(), m_goodEmulBornJet->end(), DescendingPt());

    if (m_ORbareTruthNominalJet->size() > 1) std::sort(m_ORbareTruthNominalJet->begin(), m_ORbareTruthNominalJet->end(), DescendingPt());
    if (m_ORdressTruthNominalJet->size() > 1) std::sort(m_ORdressTruthNominalJet->begin(), m_ORdressTruthNominalJet->end(), DescendingPt());
    if (m_OSbareTruthNominalJet->size() > 1) std::sort(m_OSbareTruthNominalJet->begin(), m_OSbareTruthNominalJet->end(), DescendingPt());
    if (m_OSdressTruthNominalJet->size() > 1) std::sort(m_OSdressTruthNominalJet->begin(), m_OSdressTruthNominalJet->end(), DescendingPt());








    if ( !(m_dataType.find("EXOT")!=std::string::npos) ) { // If not EXOT

      if (m_isZee) {
        std::string sm_channel = "zee";
        //////////////////////////
        // Dress level analysis //
        //////////////////////////

        // Exclusive
        doZllEmulTruth(m_truthDressElectronFromZ, m_goodTruthWZJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_exclusive_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_goodTruthWZJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthDressElectronFromZ, m_goodTruthWZJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_inclusive_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_goodTruthWZJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_inclusive_fid_" );

        // Overlap removal VS Overlap subtraction (dress-level electron and overlap-removed or subtracted AntiKt4TruthJets with dress-level electron)
        // Non fiducial
        doZllEmulTruth(m_truthDressElectronFromZ, m_ORdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_exclusive_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_OSdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_exclusive_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_ORdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_inclusive_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_OSdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_inclusive_" );
        // Fiducial
        doZllEmulTruth(m_truthDressElectronFromZ, m_ORdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_exclusive_fid_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_OSdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_exclusive_fid_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_ORdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_inclusive_fid_" );
        doZllEmulTruth(m_truthDressElectronFromZ, m_OSdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_inclusive_fid_" );



        // Using the Custom Jets
        // -----------------------
        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthDressElectronFromZ, m_goodCustomDressJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthDressElectronFromZ, m_goodCustomDressJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_dress_custom_inclusive_" );

        } // For custom derivation



        /////////////////////////
        // Bare level analysis //
        /////////////////////////

        // Using the Custom Jets

        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthBareElectronFromZ, m_goodCustomBareJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthBareElectronFromZ, m_goodCustomBareJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_custom_inclusive_" );

        } // For custom derivation

        // Using the Emulated Jets
        // --------------------------

        // Exclusive
        doZllEmulTruth(m_truthBareElectronFromZ, m_goodEmulBareJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_exclusive_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_goodEmulBareJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthBareElectronFromZ, m_goodEmulBareJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_inclusive_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_goodEmulBareJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_inclusive_fid_" );

        // Overlap removal VS Overlap subtraction (bare-level electron and overlap-removed or subtracted AntiKt4TruthJets with bare-level electron)
        // Non fiducial
        doZllEmulTruth(m_truthBareElectronFromZ, m_ORbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_exclusive_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_OSbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_exclusive_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_ORbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_inclusive_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_OSbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_inclusive_" );
        // Fiducial
        doZllEmulTruth(m_truthBareElectronFromZ, m_ORbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_exclusive_fid_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_OSbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_exclusive_fid_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_ORbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_inclusive_fid_" );
        doZllEmulTruth(m_truthBareElectronFromZ, m_OSbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_inclusive_fid_" );


        /////////////////////////
        // Born level analysis //
        /////////////////////////

        // Using the Custom Jets
        // -----------------------
        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthBornElectron, m_goodCustomBornJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_born_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthBornElectron, m_goodCustomBornJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_born_custom_inclusive_" );

        } // For custom derivation

        // Using the Emulated Jets

        // Exclusive
        doZllEmulTruth(m_truthBornElectron, m_goodEmulBornJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_exclusive_" );
        doZllEmulTruth(m_truthBornElectron, m_goodEmulBornJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_born_emul_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthBornElectron, m_goodEmulBornJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_inclusive_" );
        doZllEmulTruth(m_truthBornElectron, m_goodEmulBornJet, sm_lep1PtCut, sm_lep2PtCut, m_elecEtaCut , m_mcEventWeight, sm_channel, "_born_emul_inclusive_fid_" );


      } //Zee


      if (m_isZmumu) {
        std::string sm_channel = "zmumu";
        /////////////////////////////
        // Dressed level analysis  //
        /////////////////////////////

        // Exclusive
        doZllEmulTruth(m_truthDressMuonFromZ, m_goodTruthWZJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_exclusive_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_goodTruthWZJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthDressMuonFromZ, m_goodTruthWZJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_inclusive_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_goodTruthWZJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_wz_inclusive_fid_" );

        // Overlap removal VS Overlap subtraction (dress-level electron and overlap-removed or subtracted AntiKt4TruthJets with dress-level electron)
        // Non fiducial
        doZllEmulTruth(m_truthDressMuonFromZ, m_ORdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_exclusive_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_OSdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_exclusive_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_ORdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_inclusive_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_OSdressTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_inclusive_" );
        // Fiducial
        doZllEmulTruth(m_truthDressMuonFromZ, m_ORdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_exclusive_fid_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_OSdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_exclusive_fid_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_ORdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_OR_inclusive_fid_" );
        doZllEmulTruth(m_truthDressMuonFromZ, m_OSdressTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_OS_inclusive_fid_" );



        // Using the Custom Jets
        // -------------------------
        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthDressMuonFromZ, m_goodCustomDressJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthDressMuonFromZ, m_goodCustomDressJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_dress_custom_inclusive_" );

        } // For custom derivation



        /////////////////////////
        // Bare level analysis //
        /////////////////////////

        // Using the Custom Jets
        // ----------------------
        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthBareMuonFromZ, m_goodCustomBareJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthBareMuonFromZ, m_goodCustomBareJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_custom_inclusive_" );

        } // For custom derivation

        // Using the Emulated Jets

        // Exclusive
        doZllEmulTruth(m_truthBareMuonFromZ, m_goodEmulBareJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_exclusive_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_goodEmulBareJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthBareMuonFromZ, m_goodEmulBareJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_inclusive_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_goodEmulBareJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_emul_inclusive_fid_" );

        // Overlap removal VS Overlap subtraction (bare-level electron and overlap-removed or subtracted AntiKt4TruthJets with bare-level electron)
        // Non fiducial
        doZllEmulTruth(m_truthBareMuonFromZ, m_ORbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_exclusive_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_OSbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_exclusive_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_ORbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_inclusive_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_OSbareTruthNominalJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_inclusive_" );
        // Fiducial
        doZllEmulTruth(m_truthBareMuonFromZ, m_ORbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_exclusive_fid_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_OSbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_exclusive_fid_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_ORbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_OR_inclusive_fid_" );
        doZllEmulTruth(m_truthBareMuonFromZ, m_OSbareTruthNominalJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_bare_OS_inclusive_fid_" );



        /////////////////////////
        // Born level analysis //
        /////////////////////////

        // Using the Custom Jets
        // ------------------------
        if (is_customDerivation) {

          // Exclusive
          doZllEmulTruth(m_truthBornMuon, m_goodCustomBornJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_born_custom_exclusive_" );
          // Inclusive
          doZllEmulTruth(m_truthBornMuon, m_goodCustomBornJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_born_custom_inclusive_" );

        } // For custom derivation


        // Using the Emulated Jets

        // Exclusive
        doZllEmulTruth(m_truthBornMuon, m_goodEmulBornJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_exclusive_" );
        doZllEmulTruth(m_truthBornMuon, m_goodEmulBornJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_exclusive_fid_" );
        // Inclusive
        doZllEmulTruth(m_truthBornMuon, m_goodEmulBornJet, sm_noLep1PtCut, sm_noLep2PtCut, sm_noLepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_inclusive_" );
        doZllEmulTruth(m_truthBornMuon, m_goodEmulBornJet, sm_lep1PtCut, sm_lep2PtCut, sm_lepEtaCut , m_mcEventWeight, sm_channel, "_born_emul_inclusive_fid_" );

      } // Zmumu

    } // Not EXOT5



    if (m_isZnunu) {
      std::string sm_channel = "znunu";

      // Exclusive
      doZnunuEmulTruth(m_truthNeutrinoFromZ, m_goodTruthNominalJet, m_mcEventWeight, sm_channel, "_truth_exclusive_" );
      // Inclusive
      doZnunuEmulTruth(m_truthNeutrinoFromZ, m_goodTruthNominalJet, m_mcEventWeight, sm_channel, "_truth_inclusive_" );

    } // Znunu



    ///////////////////////////////////
    // Delete deep copied containers //
    ///////////////////////////////////
    delete m_truthNominalJet;
    delete m_truthNominalJetAux;
    delete m_truthWZJet;
    delete m_truthWZJetAux;
    delete m_copyTruthWZJet;
    delete m_copyTruthWZJetAux;
    delete m_truthEmulBareJet;
    delete m_truthEmulBareJetAux;
    delete m_truthEmulBornJet;
    delete m_truthEmulBornJetAux;
    delete m_goodTruthNominalJet;
    delete m_goodTruthNominalJetAux;
    delete m_goodTruthWZJet;
    delete m_goodTruthWZJetAux;
    delete m_goodCustomDressJet;
    delete m_goodCustomDressJetAux;
    delete m_goodCustomBareJet;
    delete m_goodCustomBareJetAux;
    delete m_goodCustomBornJet;
    delete m_goodCustomBornJetAux;
    delete m_goodEmulBareJet;
    delete m_goodEmulBareJetAux;
    delete m_goodEmulBornJet;
    delete m_goodEmulBornJetAux;
    delete m_ORbareTruthNominalJet;
    delete m_ORbareTruthNominalJetAux;
    delete m_ORdressTruthNominalJet;
    delete m_ORdressTruthNominalJetAux;
    delete m_OSbareTruthNominalJet;
    delete m_OSbareTruthNominalJetAux;
    delete m_OSdressTruthNominalJet;
    delete m_OSdressTruthNominalJetAux;
    delete m_truthPromptOrFSRPhotons;
    delete m_truthPromptOrFSRPhotonsAux;
    delete m_truthMadGraphPhotons;
    delete m_truthMadGraphPhotonsAux;
    delete m_truthMuon;
    delete m_truthMuonAux;
    delete m_truthElectron;
    delete m_truthElectronAux;
    delete m_truthNeutrino;
    delete m_truthNeutrinoAux;
    delete m_truthPromptMuon;
    delete m_truthPromptMuonAux;
    delete m_truthPromptElectron;
    delete m_truthPromptElectronAux;
    delete m_truthBornMuon;
    delete m_truthBornMuonAux;
    delete m_truthBornElectron;
    delete m_truthBornElectronAux;
    delete m_truthDressMuonFromZ;
    delete m_truthDressMuonFromZAux;
    delete m_truthBareMuonFromZ;
    delete m_truthBareMuonFromZAux;
    delete m_truthDressElectronFromZ;
    delete m_truthDressElectronFromZAux;
    delete m_truthBareElectronFromZ;
    delete m_truthBareElectronFromZAux;
    delete m_truthNeutrinoFromZ;
    delete m_truthNeutrinoFromZAux;




  } // For TRUTH1 derivation












  // Enable reconstruction level analysis
  if(!m_doReco){
    return EL::StatusCode::SUCCESS; // go to next event
  }




  //---------------------
  // EVENT SELECTION
  //---------------------
  //---------------------
  // Retrive vertex object and select events with at least one good primary vertex with at least 2 tracks
  //---------------------
  const xAOD::VertexContainer* vertices(0);
  /// retrieve arguments: container type, container key
  if ( !m_event->retrieve( vertices, "PrimaryVertices" ).isSuccess() ){ 
    Error("execute()","Failed to retrieve PrimaryVertices container. Exiting.");
    return EL::StatusCode::FAILURE;
  }
  const xAOD::Vertex* primVertex = 0;
  for (const auto &vtx : *vertices) {
    if (vtx->vertexType() == xAOD::VxType::PriVtx) {
      primVertex = vtx;
    }
  }

  //--------------
  // Preseletion
  //--------------
  if (vertices->size() < 1 || !primVertex) {
    Info("execute()", "WARNING: no primary vertex found! Skipping event.");
    return EL::StatusCode::SUCCESS;
  }
  if (primVertex->nTrackParticles() < 2) return EL::StatusCode::SUCCESS;
  m_numCleanEvents++;
  if (m_useArrayCutflow) m_eventCutflow[3]+=1;
  if (m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Primary vertex");




  /*
  // Test: Select a few events
  if (!(eventInfo->eventNumber() ==  1399885 || eventInfo->eventNumber() == 1403557 || eventInfo->eventNumber() == 1417073 || eventInfo->eventNumber() == 1694215 || eventInfo->eventNumber() == 155667)){
    return EL::StatusCode::SUCCESS; // go to next event
  }
  std::cout << std::endl;
  std::cout << eventInfo->eventNumber() << std::endl;
  */



  //------
  // JETS
  //------

  static std::string jetType = "AntiKt4EMTopoJets";

  /// full copy 
  // get jet container of interest
  const xAOD::JetContainer* m_jets(0);
  if ( !m_event->retrieve( m_jets, jetType ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Failed to retrieve Jet container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //------------
  // muons
  //------------

  /// full copy 
  // get muon container of interest
  const xAOD::MuonContainer* m_muons(0);
  if ( !m_event->retrieve( m_muons, "Muons" ).isSuccess() ){ /// retrieve arguments: container$
    Error("execute()", "Failed to retrieve Muons container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //------------
  // ELECTRONS
  //------------

  /// full copy 
  // get electron container of interest
  const xAOD::ElectronContainer* m_electrons(0);
  if ( !m_event->retrieve( m_electrons, "Electrons" ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Failed to retrieve Electron container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //------------
  // PHOTONS
  //------------

  /// full copy 
  // get photon container of interest
  const xAOD::PhotonContainer* m_photons(0);
  if ( !m_event->retrieve( m_photons, "Photons" ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Failed to retrieve Photon container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //------------
  // TAUS
  //------------

  /// full copy 
  // get tau container of interest
  const xAOD::TauJetContainer* m_taus(0);
  if ( !m_event->retrieve( m_taus, "TauJets" ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Failed to retrieve Tau container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  //=============================
  // Create MissingETContainers 
  //=============================

  //retrieve the original containers
  const xAOD::MissingETContainer* m_metCore(0);
  std::string coreMetKey = "MET_Core_" + jetType;
  coreMetKey.erase(coreMetKey.length() - 4); //this removes the Jets from the end of the jetType
  if ( !m_event->retrieve( m_metCore, coreMetKey ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Unable to retrieve MET core container: " );
    return EL::StatusCode::FAILURE;
  }

  //retrieve the MET association map
  const xAOD::MissingETAssociationMap* m_metMap = 0;
  std::string metAssocKey = "METAssoc_" + jetType;
  metAssocKey.erase(metAssocKey.length() - 4 );//this removes the Jets from the end of the jetType
  if ( !m_event->retrieve( m_metMap, metAssocKey ).isSuccess() ){ // retrieve arguments: container type, container key
    Error("execute()", "Unable to retrieve MissingETAssociationMap: " );
    return EL::StatusCode::FAILURE;
  }





  /////////////////
  /// deep copy ///
  /////////////////
  // create the new container and its auxiliary store.

  // All calibrated jets for MET building
  m_allJet = new xAOD::JetContainer();
  m_allJetAux = new xAOD::AuxContainerBase();
  m_allJet->setStore( m_allJetAux ); //< Connect the two

  // Reco jets for Skim
  xAOD::JetContainer* m_recoJet = new xAOD::JetContainer();
  xAOD::AuxContainerBase* m_recoJetAux = new xAOD::AuxContainerBase();
  m_recoJet->setStore( m_recoJetAux ); //< Connect the two

  // Good jets
  m_goodJet = new xAOD::JetContainer();
  m_goodJetAux = new xAOD::AuxContainerBase();
  m_goodJet->setStore( m_goodJetAux ); //< Connect the two

  // Good jets where the overlap removal applied using the truth muons for Znunu channel
  m_goodJetORTruthNuNu = new xAOD::JetContainer();
  m_goodJetORTruthNuNuAux = new xAOD::AuxContainerBase();
  m_goodJetORTruthNuNu->setStore( m_goodJetORTruthNuNuAux ); //< Connect the two

  // Good jets where the overlap removal applied using the truth muons for Zmumu channel
  m_goodJetORTruthMuMu = new xAOD::JetContainer();
  m_goodJetORTruthMuMuAux = new xAOD::AuxContainerBase();
  m_goodJetORTruthMuMu->setStore( m_goodJetORTruthMuMuAux ); //< Connect the two

  // Good jets where the overlap removal applied using the truth muons for Zee channel
  m_goodJetORTruthElEl = new xAOD::JetContainer();
  m_goodJetORTruthElElAux = new xAOD::AuxContainerBase();
  m_goodJetORTruthElEl->setStore( m_goodJetORTruthElElAux ); //< Connect the two

  // Good jets (Overlap Subtracted)
  m_goodOSJet = new xAOD::JetContainer();
  m_goodOSJetAux = new xAOD::AuxContainerBase();
  m_goodOSJet->setStore( m_goodOSJetAux ); //< Connect the two


  // Good muons
  m_goodMuon = new xAOD::MuonContainer();
  m_goodMuonAux = new xAOD::AuxContainerBase();
  m_goodMuon->setStore( m_goodMuonAux ); //< Connect the two

  // Good muons for Zmumu
  m_goodMuonForZ = new xAOD::MuonContainer();
  m_goodMuonForZAux = new xAOD::AuxContainerBase();
  m_goodMuonForZ->setStore( m_goodMuonForZAux ); //< Connect the two

  // Good electrons
  m_baselineElectron = new xAOD::ElectronContainer();
  m_baselineElectronAux = new xAOD::AuxContainerBase();
  m_baselineElectron->setStore( m_baselineElectronAux ); //< Connect the two

  // Good electrons
  m_goodElectron = new xAOD::ElectronContainer();
  m_goodElectronAux = new xAOD::AuxContainerBase();
  m_goodElectron->setStore( m_goodElectronAux ); //< Connect the two

  // Good photons
  m_goodPhoton = new xAOD::PhotonContainer();
  m_goodPhotonAux = new xAOD::AuxContainerBase();
  m_goodPhoton->setStore( m_goodPhotonAux ); //< Connect the two

  // Good taus
  m_goodTau = new xAOD::TauJetContainer();
  m_goodTauAux = new xAOD::AuxContainerBase();
  m_goodTau->setStore( m_goodTauAux ); //< Connect the two

  // Create a MissingETContainer with its aux store for each systematic
  m_met = new xAOD::MissingETContainer();
  m_metAux = new xAOD::MissingETAuxContainer();
  m_met->setStore(m_metAux);






  //-----------------------
  // Systematics Start
  //-----------------------
  // loop over recommended systematic

  for (const auto &sysList : m_sysList){
    std::string m_sysName = (sysList).name();
    if ((!m_doSys || m_isData) && m_sysName != "") continue;

    if (m_doSys && (m_sysName.find("TAUS_")!=std::string::npos || m_sysName.find("PH_")!=std::string::npos )) continue;
    if (m_isZmumu && !m_isZee && !m_isZnunu && m_doSys && ((m_sysName.find("EL_")!=std::string::npos || m_sysName.find("EG_")!=std::string::npos))) continue;
    if (m_isZee && !m_isZmumu && !m_isZnunu && m_doSys && ((m_sysName.find("MUON_")!=std::string::npos || m_sysName.find("MUONS_")!=std::string::npos))) continue;
    if (m_isZnunu && !m_isZmumu && !m_isZee && m_doSys && ((m_sysName.find("MUON_")!=std::string::npos || m_sysName.find("MUONS_")!=std::string::npos || m_sysName.find("EL_")!=std::string::npos || m_sysName.find("EG_")!=std::string::npos)) ) continue;

    // Print the list of systematics
    //if(m_sysName=="") std::cout << "Nominal (no syst) "  << std::endl;
    //else std::cout << "Systematic: " << m_sysName << std::endl;

    // apply recommended systematic for JetUncertaintiesTool
    if (m_jetUncertaintiesTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure JetUncertaintiesTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for JvtEfficiencyTool
    if (m_jvtefficiencyTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure JvtEfficiencyTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for fJvtEfficiencyTool
    if (m_fjvtefficiencyTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure fJvtEfficiencyTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    /*
    // apply recommended systematic for BJetEfficiencyTool
    if (m_BJetEfficiencyTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure BJetEfficiencyTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok
    */

    // apply recommended systematic for MuonCalibrationAndSmearingTool (For 2015 and 2016 dataset)
    if (m_dataYear == "2015" || m_dataYear == "2016") {
      if( m_muonCalibrationAndSmearingTool2016->applySystematicVariation( sysList ) != CP::SystematicCode::Ok ) {
        Error("execute()", "Cannot configure muon calibration tool for systematic" );
        continue; // go to next systematic
      } // end check that systematic applied ok
    }

    // apply recommended systematic for MuonCalibrationAndSmearingTool (For 2017 dataset)
    if (m_dataYear == "2017") {
      if( m_muonCalibrationAndSmearingTool2017->applySystematicVariation( sysList ) != CP::SystematicCode::Ok ) {
        Error("execute()", "Cannot configure muon calibration tool for systematic" );
        continue; // go to next systematic
      } // end check that systematic applied ok
    }

    // apply recommended systematic for MuonEfficiencySFTool
    if (m_muonEfficiencySFTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure MuonEfficiencySFTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for MuonIsolationSFTool
    if (m_muonIsolationSFTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure MuonIsolationSFTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for MuonTTVAEfficiencySFTool
    if (m_muonTTVAEfficiencySFTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure MuonTTVAEfficiencySFTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for EgammaCalibrationAndSmearingTool
    if (m_egammaCalibrationAndSmearingTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure EgammaCalibrationAndSmearingTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for ElectronEfficiencyCorrectionToolRecoSF
    if (m_elecEfficiencySFTool_reco->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure ElectronEfficiencyCorrectionToolRecoSF for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for ElectronEfficiencyCorrectionToolId
    if (m_elecEfficiencySFTool_id->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure ElectronEfficiencyCorrectionToolId for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for ElectronEfficiencyCorrectionToolIsoSF
    if (m_elecEfficiencySFTool_iso->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure ElectronEfficiencyCorrectionToolIsoSF for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for ElectronEfficiencyCorrectionToolTriggerSF
    if (m_elecEfficiencySFTool_trigSF->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure ElectronEfficiencyCorrectionToolTriggerSF for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for TauSmearingTool
    if (m_tauSmearingTool->applySystematicVariation(sysList) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure TauSmearingTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for MetSystTool
    if (m_metSystTool->applySystematicVariation( sysList ) != CP::SystematicCode::Ok) {
      Error("execute()", "Cannot configure METSystematicsTool for systematics");
      continue; // go to next systematic
    } // end check that systematic applied ok

    // apply recommended systematic for PileupReweightingTool
    if (!m_isData) { // MC
      if (m_prwTool->applySystematicVariation( sysList ) != CP::SystematicCode::Ok) {
        Error("execute()", "Cannot configure PileupReweightingTool for systematics");
        continue; // go to next systematic
      } // end check that systematic applied ok
    }





    float print_mcWeight = mcWeight;
    float print_puweight = 1.;

    //---------------------
    // Pile-up reweighting
    //---------------------
    if (!m_isData) {

      // Apply the prwTool first before calling the efficiency correction methods
      m_prwTool->apply(*eventInfo, true); // If you specify the false option to getRandomRunNumber (or to apply), the tool will not use the mu-dependency. This is not recommended though!

      //float pu_weight = m_prwTool->getCombinedWeight(*eventInfo); // Get Pile-up weight
      float pu_weight = eventInfo->auxdecor<float>("PileupWeight"); // Get Pile-up weight
      print_puweight = pu_weight;
      m_mcEventWeight = mcWeight * pu_weight; // Multiply by mcWeight instead of m_mcEventWeight in order to reset m_mcEventWeight for PRW systematics
      // Get random run number
      // I can get the runNumber which is weighted by integrated luminosity using "getRandomRunnumber" in PileupReweightingTool.
      //unsigned int randomRunNumber = m_prwTool->getRandomRunNumber( *eventInfo, false ); // If you specify the false option to getRandomRunNumber (or to apply), the tool will not use the mu-dependency. This is not recommended though! 

      // Print out pileup reweighting
      /*
         Info("execute()", "================================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());
         Info("execute()", " MC Weight = %f", print_mcWeight);
         Info("execute()", " PU Weight = %f", print_puweight);
         Info("execute()", " mcEventWeight (mc_weight * pu_weight) = %f", m_mcEventWeight);
      */
    }



    ////////////////////////////////
    // Clear deep copy containers //
    ////////////////////////////////
    m_allJet->clear();
    m_goodJet->clear();
    m_goodJetORTruthNuNu->clear();
    m_goodJetORTruthMuMu->clear();
    m_goodJetORTruthElEl->clear();
    m_goodOSJet->clear();
    m_goodMuon->clear();
    m_goodMuonForZ->clear();
    m_baselineElectron->clear();
    m_goodElectron->clear();
    m_goodPhoton->clear();
    m_goodTau->clear();
    m_met->clear();



    //------------
    // MUONS
    //------------

    /// shallow copy for muon calibration and smearing tool
    // create a shallow copy of the muons container for MET building
    std::pair< xAOD::MuonContainer*, xAOD::ShallowAuxContainer* > muons_shallowCopy = xAOD::shallowCopyContainer( *m_muons );
    xAOD::MuonContainer* m_muonSC = muons_shallowCopy.first;

    ANA_CHECK(m_store->record( muons_shallowCopy.first, "CalibMuons"+m_sysName ));
    ANA_CHECK(m_store->record( muons_shallowCopy.second, "CalibMuons"+m_sysName+"Aux."));

    // Decorate objects with ElementLink to their originals -- this is needed to retrieve the contribution of each object to the MET terms.
    // You should make sure that you use the tag xAODBase-00-00-22, which is available from AnalysisBase-2.0.11.
    // The method is defined in the header file xAODBase/IParticleHelpers.h
    bool setLinksMuon = xAOD::setOriginalObjectLink(*m_muons,*m_muonSC);
    if(!setLinksMuon) {
      Error("execute()", "Failed to set original object links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }

    // iterate over our shallow copy
    for (const auto& muon : *m_muonSC) { // C++11 shortcut
      //Info("execute()", "  original muon pt = %.2f GeV", muon->pt() * 0.001); // just to print out something
      //hMap1D["Uncalibrated_muon_pt"+m_sysName]->Fill(muon->pt() * 0.001);

      // Combined (CB) or Segment-tagged (ST) muons (excluding Stand-alone (SA), Calorimeter-tagged (CaloTag) muons etc..)
      if (muon->muonType() != xAOD::Muon_v1::Combined && muon->muonType() != xAOD::Muon_v1::SegmentTagged) continue;

      // Muon calibration and smearing tool (For 2015 and 2016 dataset)
      if (m_dataYear == "2015" || m_dataYear == "2016") {
        if(m_muonCalibrationAndSmearingTool2016->applyCorrection(*muon) == CP::CorrectionCode::Error){ // apply correction and check return code
          // Can have CorrectionCode values of Ok, OutOfValidityRange, or Error. Here only checking for Error.
          // If OutOfValidityRange is returned no modification is made and the original muon values are taken.
          Error("execute()", "MuonCalibrationAndSmearingTool returns Error CorrectionCode");
        }
      }

      // Muon calibration and smearing tool (For 2017 dataset)
      if (m_dataYear == "2017") {
        if(m_muonCalibrationAndSmearingTool2017->applyCorrection(*muon) == CP::CorrectionCode::Error){ // apply correction and check return code
          // Can have CorrectionCode values of Ok, OutOfValidityRange, or Error. Here only checking for Error.
          // If OutOfValidityRange is returned no modification is made and the original muon values are taken.
          Error("execute()", "MuonCalibrationAndSmearingTool returns Error CorrectionCode");
        }
      }


      //Info("execute()", "  corrected muon pt = %.2f GeV", muon->pt() * 0.001);  
      //hMap1D["Calibrated_muon_pt"+m_sysName]->Fill(muon->pt() * 0.001);

      // Muon pT cut
      if (muon->pt() < m_muonPtCut ) continue;

      // MuonSelectionTool
      if (m_useArrayCutflow) { // Loose for Exotic analysis
        if(!m_muonLooseSelection->accept(*muon)) continue;
      } else { // Medium for SM study
        if(!m_muonMediumSelection->accept(*muon)) continue;
      }

      // d0 / z0 cuts applied
      const xAOD::TrackParticle* tp = muon->primaryTrackParticle();

      // zo cut
      //float z0sintheta = 1e8;
      //if (primVertex) z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( muon->p4().Theta() );
      float z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( tp->theta() );
      if (std::fabs(z0sintheta) > 0.5) continue;

      // d0 significance (Transverse impact parameter)
      double d0sig = xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );
      if (std::abs(d0sig) > 3.0) continue;

      // For overlap removal with electron
      muon->auxdata<bool>("brem") = false;

      // decorate selected objects for overlap removal tool
      muon->auxdecor<char>("selected") = true;

      // Store good Muons for Zmumu
      xAOD::Muon* goodMuonForZ = new xAOD::Muon();
      m_goodMuonForZ->push_back( goodMuonForZ );
      *goodMuonForZ = *muon; // copies auxdata from one auxstore to the other

      // Isolation requirement
      if (m_useArrayCutflow) { // LooseTrackOnly for Exotic analysis
        if (!m_isolationLooseTrackOnlySelectionTool->accept(*muon)) continue;
        //if (muon->pt() > 10000. && muon->pt() < 500000. && !m_isolationFixedCutTightSelectionTool->accept(*muon)) continue;
      } else { // FixedCutTight for SM study
        if (!m_isolationFixedCutTightSelectionTool->accept(*muon)) continue;
      }

      // Store good Muons
      xAOD::Muon* goodMuon = new xAOD::Muon();
      m_goodMuon->push_back( goodMuon );
      *goodMuon = *muon; // copies auxdata from one auxstore to the other

    } // end for loop over shallow copied muons

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodMuonForZ, "goodMuonForZ"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodMuonForZAux, "goodMuonForZ"+m_sysName+"Aux." ));
    ANA_CHECK(m_store->record( m_goodMuon, "goodMuon"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodMuonAux, "goodMuon"+m_sysName+"Aux." ));

    //delete muons_shallowCopy.first;
    //delete muons_shallowCopy.second;



    //------------
    // ELECTRONS
    //------------
    /// shallow copy for electron calibration tool
    // create a shallow copy of the electrons container for MET building
    std::pair< xAOD::ElectronContainer*, xAOD::ShallowAuxContainer* > electrons_shallowCopy = xAOD::shallowCopyContainer( *m_electrons );
    xAOD::ElectronContainer* m_elecSC = electrons_shallowCopy.first;

    ANA_CHECK(m_store->record( electrons_shallowCopy.first, "CalibElectrons"+m_sysName ));
    ANA_CHECK(m_store->record( electrons_shallowCopy.second, "CalibElectrons"+m_sysName+"Aux."));

    // Decorate objects with ElementLink to their originals -- this is needed to retrieve the contribution of each object to the MET terms.
    // You should make sure that you use the tag xAODBase-00-00-22, which is available from AnalysisBase-2.0.11.
    // The method is defined in the header file xAODBase/IParticleHelpers.h
    bool setLinksElec = xAOD::setOriginalObjectLink(*m_electrons,*m_elecSC);
    if(!setLinksElec) {
      Error("execute()", "Failed to set original object links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }

    // iterate over our shallow copy
    for (const auto& electron : *m_elecSC) { // C++11 shortcut
      //Info("execute()", "  original electron pt = %.2f GeV", electron->pt() * 0.001); // just to print out something
      //hMap1D["Uncalibrated_electron_pt"+m_sysName]->Fill(electron->pt() * 0.001);
      // Egamma Calibration and Smearing Tool
      if(m_egammaCalibrationAndSmearingTool->applyCorrection(*electron) == CP::CorrectionCode::Error){ // apply correction and check return code
        // Can have CorrectionCode values of Ok, OutOfValidityRange, or Error. Here only checking for Error.
        // If OutOfValidityRange is returned no modification is made and the original electron values are taken.
        Error("execute()", "EgammaCalibrationAndSmearingTool returns Error CorrectionCode");
      }
      //Info("execute()", "  corrected electron pt = %.2f GeV", electron->pt() * 0.001);  
      //hMap1D["Calibrated_electron_pt"+m_sysName]->Fill(electron->pt() * 0.001);

      // Eta cut
      double Eta = electron->caloCluster()->etaBE(2);
      if ( std::abs(Eta) > m_elecEtaCut || (std::abs(Eta) > 1.37 && std::abs(Eta) < 1.52)) continue;

      // pT cut
      if (electron->pt() < m_elecPtCut ) continue; /// veto electron

      // goodOQ(object quality cut) : Bad Electron Cluster
      // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaIdentificationRun2#Object_quality_cut
      if( !electron->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON) ) continue;

      // Ambiguity tool
      if( m_egammaAmbiguityTool->accept(*electron)) {
        const static SG::AuxElement::Decorator<uint8_t> acc("ambiguityType");
        //ANA_MSG_INFO("Ambiguity Type: " << static_cast<int> (acc(*electron)));
        static_cast<int> (acc(*electron));
        const static SG::AuxElement::Decorator<ElementLink<xAOD::EgammaContainer> > ELink ("ambigutityElementLink");
        if(ELink(*electron).isValid()){
          const xAOD::Photon* overlapPhoton = static_cast<const xAOD::Photon*> (*ELink(*electron));
          ANA_MSG_INFO("Overlap photon pt: " << overlapPhoton->pt());
        }
      }

      // LH Electron identification(Tight)
      if (m_generatorType != "madgraph") { // Sherpa
        if (m_useArrayCutflow) { // Loose for Exotic analysis
          if (!m_LHToolLoose->accept(electron)) continue;
        } else { // Tight for SM study
          if (!m_LHToolTight->accept(electron)) continue;
        }
      } else { // MadGraph
        if (!(bool)electron->auxdata<char>("Tight")) continue;
      }
      //Info("execute()", "  Tight electron pt = %.2f GeV", electron->pt() * 0.001);  


      // d0 / z0 cuts applied
      // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Electron_d0_and_z0_cut_definitio
      const xAOD::TrackParticle *tp = electron->trackParticle() ; //your input track particle from the electron

      // zo cut
      float z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( tp->theta() );
      if (std::fabs(z0sintheta) > 0.5) continue;

      // Store baseline Electrons (For Multijet QCD background estimation, Reverse cuts (Failed d0, Failed Iso) )
      xAOD::Electron* baselineElectron = new xAOD::Electron();
      m_baselineElectron->push_back( baselineElectron );
      *baselineElectron = *electron; // copies auxdata from one auxstore to the other

      // d0 significance (Transverse impact parameter)
      double d0sig = xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );
      if (std::abs(d0sig) > 5.0) continue;


      // Isolation requirement
      if (m_useArrayCutflow) { // LooseTrackOnly for Exotic analysis
        if (!m_isolationLooseTrackOnlySelectionTool->accept(*electron)) continue;
        //Info("execute()", "  selected electron pt = %.2f GeV", electron->pt() * 0.001);  
      } else { // FixedCutTight for SM study
        if (!m_isolationFixedCutTightSelectionTool->accept(*electron)) continue;
      }
      //Info("execute()", "  Isolated electron pt = %.2f GeV", electron->pt() * 0.001);  


      // decorate selected objects for overlap removal tool
      electron->auxdecor<char>("selected") = true;

      // Store good Electrons
      xAOD::Electron* goodElectron = new xAOD::Electron();
      m_goodElectron->push_back( goodElectron );
      *goodElectron = *electron; // copies auxdata from one auxstore to the other

    } // end for loop over shallow copied electrons

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_baselineElectron, "baselineElectron"+m_sysName ));
    ANA_CHECK(m_store->record( m_baselineElectronAux, "baselineElectron"+m_sysName+"Aux." ));

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodElectron, "goodElectron"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodElectronAux, "goodElectron"+m_sysName+"Aux." ));

    //delete electrons_shallowCopy.first;
    //delete electrons_shallowCopy.second;



    //------------
    // PHOTONS
    //------------
    /// shallow copy for photon calibration tool
    // create a shallow copy of the photons container for MET building
    std::pair< xAOD::PhotonContainer*, xAOD::ShallowAuxContainer* > photons_shallowCopy = xAOD::shallowCopyContainer( *m_photons );
    xAOD::PhotonContainer* m_photSC = photons_shallowCopy.first;

    ANA_CHECK(m_store->record( photons_shallowCopy.first, "CalibPhotons"+m_sysName ));
    ANA_CHECK(m_store->record( photons_shallowCopy.second, "CalibPhotons"+m_sysName+"Aux."));

    // Decorate objects with ElementLink to their originals -- this is needed to retrieve the contribution of each object to the MET terms.
    // You should make sure that you use the tag xAODBase-00-00-22, which is available from AnalysisBase-2.0.11.
    // The method is defined in the header file xAODBase/IParticleHelpers.h
    bool setLinksPhoton = xAOD::setOriginalObjectLink(*m_photons,*m_photSC);
    if(!setLinksPhoton) {
      Error("execute()", "Failed to set original object links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }

    // iterate over our shallow copy
    for (const auto& photon : *m_photSC) { // C++11 shortcut

      // Photon author cuts
      uint16_t author =  photon->author();
      if (!(author && xAOD::EgammaParameters::AuthorPhoton) && !(author && xAOD::EgammaParameters::AuthorAmbiguous)) continue;

      //Info("execute()", "  original photon pt = %.2f GeV", photon->pt() * 0.001); // just to print out something
      //hMap1D["Uncalibrated_photon_pt"+m_sysName]->Fill(photon->pt() * 0.001);

      // MC fudge tool
      if (!m_isData && m_generatorType != "madgraph"){
        if(m_fudgeMCTool->applyCorrection(*photon) == CP::CorrectionCode::Error){ // apply correction and check return code
          Error("execute()", "ElectronPhotonShowerShapeFudgeTool returns Error CorrectionCode");
        }
      } // is_MC

      // Egamma Calibration and Smearing Tool
      if(m_egammaCalibrationAndSmearingTool->applyCorrection(*photon) == CP::CorrectionCode::Error){ // apply correction and check return code
        // Can have CorrectionCode values of Ok, OutOfValidityRange, or Error. Here only checking for Error.
        // If OutOfValidityRange is returned no modification is made and the original photon values are taken.
        Error("execute()", "EgammaCalibrationAndSmearingTool returns Error CorrectionCode");
      }
      //Info("execute()", "  corrected photon pt = %.2f GeV", photon->pt() * 0.001);  
      //hMap1D["Calibrated_photon_pt"+m_sysName]->Fill(photon->pt() * 0.001);

      // Eta cut
      double Eta = photon->caloCluster()->etaBE(2);
      //if ( std::abs(Eta) >= m_photEtaCut || (std::abs(Eta) >= 1.37 && std::abs(Eta) <= 1.52)) return EL::StatusCode::SUCCESS;
      if ( std::abs(Eta) > m_photEtaCut ) continue;

      // pT cut
      if (photon->pt() < m_photPtCut) continue; /// veto photon
      //Info("execute()", "  Selected photon pt from new Photon Container = %.2f GeV", (photon->pt() * 0.001));

      // goodOQ(object quality cut) : Bad photon Cluster
      // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaIdentificationRun2#Object_quality_cut
      if( !photon->isGoodOQ(xAOD::EgammaParameters::BADCLUSPHOTON) ) continue;

      // Recomputing the photon ID flags
      if (!m_photonTightIsEMSelector->accept(photon)) continue;

      // Isolation requirement
      if (m_useArrayCutflow) { // LooseTrackOnly for Exotic analysis
        if (!m_isolationLooseTrackOnlySelectionTool->accept(*photon)) continue;
      } else { // FixedCutTight for SM study
        if (!m_isolationFixedCutTightSelectionTool->accept(*photon)) continue;
      }


      // decorate selected objects for overlap removal tool
      photon->auxdecor<char>("selected") = true;

      // Store good Photons
      xAOD::Photon* goodPhoton = new xAOD::Photon();
      m_goodPhoton->push_back( goodPhoton );
      *goodPhoton = *photon; // copies auxdata from one auxstore to the other

    } // end for loop over shallow copied photons

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodPhoton, "goodPhoton"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodPhotonAux, "goodPhoton"+m_sysName+"Aux." ));

    //delete photons_shallowCopy.first;
    //delete photons_shallowCopy.second;




    //------------
    // TAUS
    //------------
    /// shallow copy for tau calibration tool
    // create a shallow copy of the taus container for MET building
    std::pair< xAOD::TauJetContainer*, xAOD::ShallowAuxContainer* > tau_shallowCopy = xAOD::shallowCopyContainer( *m_taus );
    xAOD::TauJetContainer* m_tauSC = tau_shallowCopy.first;

    ANA_CHECK(m_store->record( tau_shallowCopy.first, "CalibTaus"+m_sysName ));
    ANA_CHECK(m_store->record( tau_shallowCopy.second, "CalibTaus"+m_sysName+"+Aux."));

    // Decorate objects with ElementLink to their originals -- this is needed to retrieve the contribution of each object to the MET terms.
    // You should make sure that you use the tag xAODBase-00-00-22, which is available from AnalysisBase-2.0.11.
    // The method is defined in the header file xAODBase/IParticleHelpers.h
    bool setLinksTau = xAOD::setOriginalObjectLink(*m_taus,*m_tauSC);
    if(!setLinksTau) {
      Error("execute()", "Failed to set original object links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }

    // iterate over our shallow copy
    for (const auto& taujet : *m_tauSC) { // C++11 shortcut

      //hMap1D["Unsmeared_tau_pt"+m_sysName]->Fill(taujet->pt() * 0.001);


      /*
      // TauOverlappingElectronLLHDecorator
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation
        m_tauOverlappingElectronLLHDecorator->decorate(*taujet);
      }
      // Test TauOverlappingElectronLLHDecorator tool
      Info("execute()", "========================================");
      Info("execute()", " Event # = %llu", eventInfo->eventNumber());
      Info("execute()", " tau pt = %.3f GeV", taujet->pt() * 0.001);
      Info("execute()", " tau eta = %.3f", taujet->eta());
      Info("execute()", " tau phi = %.3f", taujet->phi());
      // LLH score of the matched electron
      // The score should in general be between -4 and 2, where large values indicate the reco-tau to be faked by an electron. Taus with no matched electron are assigned a value of -4.
      float ele_match_lhscore = taujet->auxdata<float>("ele_match_lhscore");
      Info("execute()", " [TauOverlappingElectronLLHDecorator] ele_match_lhscore = %.2f", ele_match_lhscore);
      int ele_olr_pass = (bool)taujet->auxdata<char>("ele_olr_pass");
      Info("execute()", " [TauOverlappingElectronLLHDecorator] ele_olr_pass = %i", ele_olr_pass);
      // element link to the matched electron
      auto electronLink = taujet->auxdata< ElementLink< xAOD::ElectronContainer > >("electronLink");
      if (electronLink.isValid()) {
        const xAOD::Electron* matchedElectron = *electronLink;
        Info("execute()",
            "Tau was matched to a reconstructed electron , which has pt=%g GeV, eta=%g, phi=%g, m=%g",
            matchedElectron->p4().Pt() * 0.001,
            matchedElectron->p4().Eta(),
            matchedElectron->p4().Phi(),
            matchedElectron->p4().M());
      }
      else
        Info("execute()", "Tau was not matched to truth jet" );
      */



      // Tau Smearing (for MC)
      if( (bool) taujet->auxdata<char>("IsTruthMatched") && !m_isData){ // it's MC and truth matched Tau!
        if(m_tauSmearingTool->applyCorrection(*taujet) == CP::CorrectionCode::Error){ // apply correction and check return code
          Error("execute()", "TauSmearingTool returns Error CorrectionCode");
        }
      }

      //hMap1D["Smeared_tau_pt"+m_sysName]->Fill(taujet->pt() * 0.001);

      // TauSelectionTool (Loose)
      // Tau selection tool "ONLY" for EXOT5 derivation
      // because aux item "trackLinks" is missing in the STDM4 derivation samples
      if ( m_dataType.find("EXOT")!=std::string::npos || m_useArrayCutflow) { // EXOT Derivation OR Cutflow test purpose
        if(!m_tauSelTool->accept(taujet)) continue;
      }

      // decorate selected objects for overlap removal tool
      taujet->auxdecor<char>("selected") = true;

      // Store good Taus
      xAOD::TauJet* goodTau = new xAOD::TauJet();
      m_goodTau->push_back( goodTau );
      *goodTau = *taujet; // copies auxdata from one auxstore to the other

    } // end for loop over shallow copied taus

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodTau, "goodTau"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodTauAux, "goodTau"+m_sysName+"Aux." ));

    //delete tau_shallowCopy.first;
    //delete tau_shallowCopy.second;


    /////////////////////////////////
    // Sort Good Muon and Electron // 
    /////////////////////////////////
    if (m_goodMuon->size() > 1) std::partial_sort(m_goodMuon->begin(), m_goodMuon->begin()+2, m_goodMuon->end(), DescendingPt());
    if (m_goodMuonForZ->size() > 1) std::partial_sort(m_goodMuonForZ->begin(), m_goodMuonForZ->begin()+2, m_goodMuonForZ->end(), DescendingPt());
    if (m_baselineElectron->size() > 1) std::partial_sort(m_baselineElectron->begin(), m_baselineElectron->begin()+2, m_baselineElectron->end(), DescendingPt());
    if (m_goodElectron->size() > 1) std::partial_sort(m_goodElectron->begin(), m_goodElectron->begin()+2, m_goodElectron->end(), DescendingPt());




    //------
    // JETS
    //------

    /// shallow copy for jet calibration tool
    // create a shallow copy of the jets container for MET building
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > jets_shallowCopy = xAOD::shallowCopyContainer( *m_jets );
    xAOD::JetContainer* m_jetSC = jets_shallowCopy.first;

    ANA_CHECK(m_store->record( jets_shallowCopy.first, "CalibJets"+m_sysName ));
    ANA_CHECK(m_store->record( jets_shallowCopy.second, "CalibJets"+m_sysName+"Aux."));

    // Decorate objects with ElementLink to their originals -- this is needed to retrieve the contribution of each object to the MET terms.
    // You should make sure that you use the tag xAODBase-00-00-22, which is available from AnalysisBase-2.0.11.
    // The method is defined in the header file xAODBase/IParticleHelpers.h
    bool setLinksJet = xAOD::setOriginalObjectLink(*m_jets,*m_jetSC);
    if(!setLinksJet) {
      Error("execute()", "Failed to set original object links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }

    // iterate over our shallow copy
    for (const auto& jets : *m_jetSC) { // C++11 shortcut
      bool badJet = false; 
      //Info("execute()", "  original jet pt = %.2f GeV", jets->pt() * 0.001);
      //hMap1D["Uncalibrated_jet_pt"+m_sysName]->Fill(jets->pt() * 0.001);

      // According to https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEtmissRecommendationsMC15

      // EXOT5 derivation Skim cut
      if ( m_dataType.find("STDM")!=std::string::npos && m_sysName=="" && m_doSkimEXOT5) { // STDM derivation
        if (jets->pt() > m_skimUncalibMonoJetPt) passUncalibMonojetCut = true;
      }

      // JES calibration
      if ( !m_jetCalibration->applyCalibration(*jets).isSuccess() ){
        Error("execute()", "Failed to apply calibration to Jet objects. Exiting." );
        return EL::StatusCode::FAILURE;
      }
      //Info("execute()", "  calibrated jet pt = %.2f GeV", jets->pt() * 0.001);
      //hMap1D["Calibrated_jet_pt"+m_sysName]->Fill(jets->pt() * 0.001);

      // Store reco calibrated Jets for Skim
      if ( m_dataType.find("STDM")!=std::string::npos && m_sysName=="" && m_doSkimEXOT5) { // STDM derivation
        xAOD::Jet* recoJet = new xAOD::Jet();
        m_recoJet->push_back( recoJet ); // jet acquires the m_goodJet auxstore
        *recoJet = *jets; // copies auxdata from one auxstore to the other
      }

      // JES correction (Apply to both Data and MC, impacting on the JES uncertainties for MC, but also controling how the JER uncertainties are applied for Data)
      if ( m_jetUncertaintiesTool->applyCorrection(*jets) != CP::CorrectionCode::Ok){ // apply correction and check return code
        Error("execute()", "Failed to apply JES correction to Jet objects. Exiting." );
        return EL::StatusCode::FAILURE;
      }

      // Update JVT (jet Vertex Taggger)
      float newjvt = m_hjvtagup->updateJvt(*jets);
      //Info("execute()", "  updated JVT = %.2f", newjvt);
      jets->auxdata<float>("Jvt") = newjvt; //add JVT variable to the (nonconst) jet container
      jets->auxdata<bool>("RecoJet") = true; //add RecoJet variable to the jet container to distinguish from TruthJet later

      // Good Jet Selection
      if (jets->pt() < sm_goodJetPtCut || std::abs(jets->eta()) > 4.5 || std::abs(jets->rapidity()) > 4.4) badJet = true;
      // Define Detector eta for Jvt cut
      // Variable JetEMScaleMomentum is not stored in STDM4 derivation, but EMScale is the same as ConstituentScale. So I can use ConstituentScale for STDM4.
      float jet_EMScale_eta = 0;
      // For EXOT5 derivation
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT derivation
        jet_EMScale_eta = jets->jetP4(xAOD::JetEMScaleMomentum).eta();
        //jet_EMScale_eta = jets->auxdata<float>("JetEMScaleMomentum_eta");
        //std::cout << "jet: JetEMScaleMomentum_eta = " << jets->auxdata<float>("JetEMScaleMomentum_eta") << std::endl;
        //std::cout << "jet: Constituent scale pT = " << jets->jetP4(xAOD::JetConstitScaleMomentum).pt() * 0.001 << " GeV" << std::endl;
        //std::cout << "jet: EM scale pT = " << jets->jetP4(xAOD::JetEMScaleMomentum).pt() * 0.001 << " GeV" << std::endl;
        //std::cout << "jet: Four-momentum at the pile-up subtracted scale pT = " << jets->auxdata<float>("JetPileupScaleMomentum_pt") * 0.001 << " GeV" << std::endl;
        // variable not avaialble in Rel.21 //std::cout << "jet: Origin Constituent scale pT = " << jets->auxdata<float>("JetOriginConstitScaleMomentum_pt") * 0.001 << " GeV" << std::endl;
      }
      // For STDM4 derivation
      if ( m_dataType.find("STDM")!=std::string::npos ) { // STDM derivation
        jet_EMScale_eta = jets->jetP4(xAOD::JetConstitScaleMomentum).eta();
        //std::cout << "jet: Jet EM scale pT = " << jets->jetP4(xAOD::JetConstitScaleMomentum).pt() * 0.001 << " GeV" << std::endl;
      }
      // Jvt cut
      //if (jets->pt() < 60000. && std::fabs(jet_EMScale_eta) < 2.4 && newjvt < 0.59) badJet = true;
      // Use JVT Efficiency tool
      //if (jets->pt() < 60000. && std::fabs(jet_EMScale_eta) < 2.4 && !m_jvtefficiencyTool->passesJvtCut(*jets)) badJet = true;
      //if (jets->pt() < 60000. && std::fabs(jets->eta()) < 2.4 && !m_jvtefficiencyTool->passesJvtCut(*jets)) badJet = true;
      if (!m_jvtefficiencyTool->passesJvtCut(*jets)) badJet = true;


      // decorate selected objects for overlap removal tool
      if (!badJet) {
        jets->auxdecor<char>("selected") = true;
      } else {
        jets->auxdata<bool>("badjet") = true; // Add variable to the container
      }

      // Store all calibrated Jets for MET building
      xAOD::Jet* allJet = new xAOD::Jet();
      m_allJet->push_back( allJet ); // jet acquires the m_goodJet auxstore
      *allJet = *jets; // copies auxdata from one auxstore to the other

    } // end for loop over shallow copied jets

    // Apply fJVT (forward JVT) tool
    // decorate jet container with forward JVT decision
    // the fJVT tool wants to modify the jet containers.
    m_fJvtTool->modify(*m_allJet);//where the "m_allJet" is a calibrated jet container with calibrated JVT values
    /*
    Info("execute()", "Event number = %i and Lumi Block number = %i", m_eventCounter, eventInfo->lumiBlock() );
    // iterate over deep copy
    for (const auto& jets : *m_allJet) { // C++11 shortcut
      std::cout << "deep copy jet: JVT value = " << jets->auxdata<float>("Jvt") << std::endl;
      std::cout << "deep copy jet: pass fJVT = " << (bool) jets->auxdata<char>("passFJVT") << std::endl;
    }
    */

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_allJet, "allJet"+m_sysName ));
    ANA_CHECK(m_store->record( m_allJetAux, "allJet"+m_sysName+"Aux." ));

    //delete jets_shallowCopy.first;
    //delete jets_shallowCopy.second;


    // Run muon-to-jet ghost association
    // ghost associate the muons to the jets (needed by MET muon-jet OR later)
    met::addGhostMuonsToJets(*m_muons, *m_allJet);




    /////////////////////////////////////////////////////////////
    // Reco Jet decision for Skim for STDM4 derivation samples //
    /////////////////////////////////////////////////////////////
    // Sort recoJets
    if ( m_dataType.find("STDM")!=std::string::npos && m_sysName=="" && m_doSkimEXOT5) { // STDM derivation
      if (m_recoJet->size() > 1) std::partial_sort(m_recoJet->begin(), m_recoJet->begin()+2, m_recoJet->end(), DescendingPt());

      float mjj = 0;
      if (m_recoJet->size() > 1) {
        TLorentzVector jet1 = m_recoJet->at(0)->p4();
        TLorentzVector jet2 = m_recoJet->at(1)->p4();
        auto dijet = jet1 + jet2;
        mjj = dijet.M();
      }

      if ((m_recoJet->size() > 0 && m_recoJet->at(0)->pt() > m_skimMonoJetPt) || (m_recoJet->size() > 1 && m_recoJet->at(0)->pt() > m_skimLeadingJetPt && m_recoJet->at(1)->pt() > m_skimSubleadingJetPt && mjj > m_skimMjj)) passRecoJetCuts = true;
    }

    if (m_sysName=="") {
      // Delete copy containers
      delete m_recoJet;
      delete m_recoJetAux;
    }


    ////////////////////////
    // Good Jet selection //
    ////////////////////////
    // all jets (calibrated) in shallow copy loop
    for (const auto& jets : *m_allJet) { // C++11 shortcut

      // Good Jet selection
      if ( jets->auxdata<bool>("badjet")  ) continue;

      // pass fJVT (pT < 60GeV , |eta| > 2.5) : pT < 60GeV is defined in the initial m_fJvtTool
      if ( std::fabs(jets->eta()) > 2.5 && !(bool)jets->auxdata<char>("passFJVT") ) continue;

      // Store good Jets
      xAOD::Jet* goodJet = new xAOD::Jet();
      m_goodJet->push_back( goodJet ); // jet acquires the m_goodJet auxstore
      *goodJet = *jets; // copies auxdata from one auxstore to the other


      ////////////////////////////////////////////////
      // Define Reco jet for Cz (Correction factor) //
      ////////////////////////////////////////////////
      bool passJetORTruthNuNu = true;
      bool passJetORTruthMuMu = true;
      bool passJetORTruthElEl = true;
      // The overlap removal is applied using truth level leptons with pT and eta cuts loosened by 10% of their default values
      if (!m_isData) {
        // Overlap removal using Truth muon
        for (const auto& muon : *m_dressedTruthMuon) { // C++11 shortcut
          if (muon->pt() > m_SubLeadLepPtCut*0.9 &&  std::abs(muon->eta()) < m_lepEtaCut*1.1 && deltaR(jets->eta(), muon->eta(), jets->phi(), muon->phi()) < m_ORJETdeltaR) passJetORTruthMuMu = false;
          if (deltaR(jets->eta(), muon->eta(), jets->phi(), muon->phi()) < m_ORJETdeltaR) passJetORTruthNuNu = false;
        }
        // Overlap removal using Truth electron
        for (const auto& electron : *m_dressedTruthElectron) { // C++11 shortcut
          if (electron->pt() > m_SubLeadLepPtCut*0.9 &&  std::abs(electron->eta()) < m_lepEtaCut*1.1 && deltaR(jets->eta(), electron->eta(), jets->phi(), electron->phi()) < m_ORJETdeltaR) passJetORTruthElEl = false;
          if (deltaR(jets->eta(), electron->eta(), jets->phi(), electron->phi()) < m_ORJETdeltaR) passJetORTruthNuNu = false;
        }
        // Overlap removal using Truth tau
        for (const auto &tau : *m_selectedTruthTau) {
          if (deltaR(jets->eta(), tau->eta(), jets->phi(), tau->phi()) < m_ORJETdeltaR) passJetORTruthNuNu = false;
        }

        // Store Reco Jets for Cz
        if (passJetORTruthNuNu) {
          xAOD::Jet* goodJetORTruthNuNu = new xAOD::Jet();
          m_goodJetORTruthNuNu->push_back( goodJetORTruthNuNu );
          *goodJetORTruthNuNu = *jets; // copies auxdata from one auxstore to the other
        }
        if (passJetORTruthMuMu) {
          xAOD::Jet* goodJetORTruthMuMu = new xAOD::Jet();
          m_goodJetORTruthMuMu->push_back( goodJetORTruthMuMu );
          *goodJetORTruthMuMu = *jets; // copies auxdata from one auxstore to the other
        }
        if (passJetORTruthElEl) {
          xAOD::Jet* goodJetORTruthElEl = new xAOD::Jet();
          m_goodJetORTruthElEl->push_back( goodJetORTruthElEl );
          *goodJetORTruthElEl = *jets; // copies auxdata from one auxstore to the other
        }
      } // End of Cz definition

    }

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodJet, "goodJet"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodJetAux, "goodJet"+m_sysName+"Aux." ));

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodJetORTruthNuNu, "goodJetORTruthNuNu"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodJetORTruthNuNuAux, "goodJetORTruthNuNu"+m_sysName+"Aux." ));

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodJetORTruthMuMu, "goodJetORTruthMuMu"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodJetORTruthMuMuAux, "goodJetORTruthMuMu"+m_sysName+"Aux." ));

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodJetORTruthElEl, "goodJetORTruthElEl"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodJetORTruthElElAux, "goodJetORTruthElEl"+m_sysName+"Aux." ));






    //------------------------------------
    // Overlap subtraction (OS) Reco-level
    //------------------------------------
    // AntiKt4EMTopoJets : Include all electrons and all photons
    // --------------------------
    // For good electorn

    // Copy good Reco jet container to rewrite overlap-subracted jets
    xAOD::JetContainer* m_copyGoodRecoJetForBareOS = new xAOD::JetContainer();
    xAOD::AuxContainerBase* m_copyGoodRecoJetForBareOSAux = new xAOD::AuxContainerBase();
    m_copyGoodRecoJetForBareOS->setStore( m_copyGoodRecoJetForBareOSAux ); //< Connect the two
    // Nominal good jet
    for (const auto &jet : *m_goodJet) {
      //std::cout << "[OS bare loop] reco good Jet pT = " << jet->pt() * 0.001 << std::endl;
      // Store in m_copyGoodRecoJetForBareOS
      xAOD::Jet* copyGoodRecoJetForBareOS = new xAOD::Jet();
      copyGoodRecoJetForBareOS->makePrivateStore(*jet);
      m_copyGoodRecoJetForBareOS->push_back(copyGoodRecoJetForBareOS);
    } // Nominal good jet

    // Use good electrons
    TLorentzVector goodElec1P4;
    TLorentzVector goodElec2P4;
    // Determine which jet is the most close to the lepton1
    float minDRgoodElec1 = 1000.;
    if (m_goodElectron->size() > 0) {
      goodElec1P4 = m_goodElectron->at(0)->p4();
      for (const auto &jet : *m_copyGoodRecoJetForBareOS) {
        float dR = deltaR(jet->eta(), m_goodElectron->at(0)->eta(), jet->phi(), m_goodElectron->at(0)->phi());
        if ( dR < minDRgoodElec1 ) minDRgoodElec1 = dR;
        //std::cout << " dR = " << dR << " , min dR between jet and electron1 = " << minDRgoodElec1 << std::endl;
      }
    }
    // Determine which jet is the most close to the lepton2
    float minDRgoodElec2 = 1000.;
    if (m_goodElectron->size() > 1) {
      goodElec2P4 = m_goodElectron->at(1)->p4();
      for (const auto &jet : *m_copyGoodRecoJetForBareOS) {
        float dR = deltaR(jet->eta(), m_goodElectron->at(1)->eta(), jet->phi(), m_goodElectron->at(1)->phi());
        if ( dR < minDRgoodElec2 ) minDRgoodElec2 = dR;
        //std::cout << " dR = " << dR << " , min dR between jet and electron2 = " << minDRgoodElec2 << std::endl;
      }
    }

    for (const auto &jet : *m_copyGoodRecoJetForBareOS) {
      // Subtract lepton 4 momentum from jets near the bare-level lepton (dR<0.4)
      TLorentzVector nominal_jet = jet->p4();
      auto subtracted_jet = nominal_jet;
      // Subtract electron1 from the jet if this jet is closest to the electron1
      if (m_goodElectron->size() > 0) {
        float dR = deltaR(jet->eta(), m_goodElectron->at(0)->eta(), jet->phi(), m_goodElectron->at(0)->phi());
        //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron1 = " << minDRgoodElec1 << std::endl;
        if ( dR < sm_ORJETdeltaR ) {
          if ( dR == minDRgoodElec1 ) {
            // Implement subtraction
            subtracted_jet = subtracted_jet - goodElec1P4;
            //std::cout << " !! Found overlapped jet with electron 1 with dR = " << dR << std::endl;
          }
        }
      }
      // Subtract electron2 from the jet if this jet is closest to the electron2
      if (m_goodElectron->size() > 1) {
        float dR = deltaR(jet->eta(), m_goodElectron->at(1)->eta(), jet->phi(), m_goodElectron->at(1)->phi());
        //std::cout << " In the jet loop, dR = " << dR << " , min dR between jet and electron2 = " << minDRgoodElec2 << std::endl;
        if ( dR < sm_ORJETdeltaR ) {
          if ( dR == minDRgoodElec2 ) {
            //std::cout << " !! Found overlapped jet with electron 2 with dR = " << dR << std::endl;
            // Implement subtraction
            subtracted_jet = subtracted_jet - goodElec2P4;
          }
        }
      }
      // Overwrite subtracted Jets
      xAOD::JetFourMom_t subtracedJetP4 (subtracted_jet.Pt(), subtracted_jet.Eta(), subtracted_jet.Phi(), subtracted_jet.M());
      jet->setJetP4 (subtracedJetP4); // we've overwritten the 4-momentum

      // Good Jet Selection
      if (jet->pt() < sm_goodJetPtCut || std::abs(jet->eta()) > 4.5 || std::abs(jet->rapidity()) > 4.4) continue;

      // Store subrated Jets in the m_goodOSJet container
      xAOD::Jet* goodOSJet = new xAOD::Jet();
      m_goodOSJet->push_back( goodOSJet ); // jet acquires the m_goodOSJet auxstore
      *goodOSJet = *jet; // copies auxdata from one auxstore to the other

    } // copied good reco jet

    // record your deep copied jet container (and aux container) to the store
    ANA_CHECK(m_store->record( m_goodOSJet, "goodOSJet"+m_sysName ));
    ANA_CHECK(m_store->record( m_goodOSJetAux, "goodOSJet"+m_sysName+"Aux." ));

    ///////////////////////
    // Sort Good OS Jets // 
    ///////////////////////
    if (m_goodOSJet->size() > 1) std::sort(m_goodOSJet->begin(), m_goodOSJet->end(), DescendingPt());

    delete m_copyGoodRecoJetForBareOS;
    delete m_copyGoodRecoJetForBareOSAux;



















    //----------------------------------------------------
    // Decorate overlapped objects using official OR Tool
    //----------------------------------------------------

    if ( !m_toolBox->masterTool->removeOverlaps(m_goodElectron, m_goodMuon, m_goodJet, m_goodTau, m_goodPhoton).isSuccess() ){
      Error("execute()", "Failed to apply the overlap removal to all objects. Exiting." );
      return EL::StatusCode::FAILURE;
    }

    // Now, dump all of the results
    if (m_sysName == "") {

      // electrons
      for(auto electron : *m_goodElectron){
        if(electron->auxdecor<char>("overlaps") != outputPassValue ) {
          nOverlapElectrons++;
          //Info("execute()", "  EventNumber : %i |  Overlap electron pt = %.2f GeV", EventNumber, (electron->pt() * 0.001));
        }
        nInputElectrons++;
      }
      // muons
      for(auto muon : *m_goodMuon){
        if(muon->auxdecor<char>("overlaps") != outputPassValue ) nOverlapMuons++;
        nInputMuons++;
      }
      // jets
      for (auto jets : *m_goodJet) {
        if(jets->auxdecor<char>("overlaps") != outputPassValue ){
          nOverlapJets++;
          //Info("execute()", "  EventNumber : %i |  Overlap jet pt = %.2f GeV", EventNumber, (jets->pt() * 0.001));
        }
        nInputJets++;
      }
      // taus
      for(auto taujet : *m_goodTau){
        if(taujet->auxdecor<char>("overlaps") != outputPassValue ) nOverlapTaus++;
        nInputTaus++;
      }
      // photons
      for(auto photon : *m_goodPhoton){
        if(photon->auxdecor<char>("overlaps")!= outputPassValue ) nOverlapPhotons++;
        nInputPhotons++;
      }

    }


    ///////////////////////////////
    // Decision of the Brem muon //
    ///////////////////////////////
    for (const auto& muon : *m_goodMuonForZ) {
      for (const auto& jet : *m_goodJet) { // C++11 shortcut
        if (deltaR(jet->eta(), muon->eta(), jet->phi(), muon->phi()) < m_ORJETdeltaR) {

          int ntrks = 0;
          float sumpt = 0.;
          std::vector<int> ntrks_vec = jet->auxdata<std::vector<int> >("NumTrkPt500");
          std::vector<float> sumpt_vec = jet->auxdata<std::vector<float> >("SumPtTrkPt500");
          if (ntrks_vec.size() > 0) {
            ntrks = ntrks_vec[primVertex->index()];
            sumpt = sumpt_vec[primVertex->index()];
          }
          if (ntrks < 5 || (muon->pt()/jet->pt() > 0.5 && muon->pt()/sumpt > 0.7)) {
            muon->auxdata<bool>("brem") = true;
          }
        }
      }
    }


    ////////////////////
    // Sort Good Jets // 
    ////////////////////
    if (m_goodJet->size() > 1) std::sort(m_goodJet->begin(), m_goodJet->end(), DescendingPt());



    ////////////////////////////////////////////
    // Overlap removal manually for VBF study //
    ////////////////////////////////////////////
    // For Z->mumu events, the non-isolated muons that overlap with jets are only rejected if they have pT<20GeV
    // This is done before the erasing of the overlapping objects is done
    int i=0;
    for (const auto& muon : *m_goodMuonForZ) {
      for (const auto& jet : *m_goodJet) { // C++11 shortcut
        if (deltaR(jet->eta(), muon->eta(), jet->phi(), muon->phi()) < m_ORJETdeltaR) {
          if (muon->pt() < 20000.) {
            m_goodMuonForZ->erase(m_goodMuonForZ->begin()+i);
            --i;
            break;
          } //else  muon->auxdata<bool>("overlap") = true;
        }
      } // break to here
      ++i;
    }




    //////////////////////////////////////////////
    // Overlap removal officially for VBF study //
    //////////////////////////////////////////////
    m_goodMuon->erase(std::remove_if(std::begin(*m_goodMuon), std::end(*m_goodMuon), [](xAOD::Muon* muon) {return muon->auxdecor<char>("overlaps");}), std::end(*m_goodMuon));
    m_goodElectron->erase(std::remove_if(std::begin(*m_goodElectron), std::end(*m_goodElectron), [](xAOD::Electron* electron) {return electron->auxdecor<char>("overlaps");}), std::end(*m_goodElectron));
    m_goodJet->erase(std::remove_if(std::begin(*m_goodJet), std::end(*m_goodJet), [](xAOD::Jet* jets) {return jets->auxdecor<char>("overlaps");}), std::end(*m_goodJet));
    m_goodTau->erase(std::remove_if(std::begin(*m_goodTau), std::end(*m_goodTau), [](xAOD::TauJet* taujet) {return taujet->auxdecor<char>("overlaps");}), std::end(*m_goodTau));
    m_goodPhoton->erase(std::remove_if(std::begin(*m_goodPhoton), std::end(*m_goodPhoton), [](xAOD::Photon* photon) {return photon->auxdecor<char>("overlaps");}), std::end(*m_goodPhoton));




    ////////////////////////////////////////////
    // Overlap removal manually for VBF study //
    ////////////////////////////////////////////
    // Electron overlap removal with non-isolated muons
    // loop round electrons and remove if it is close to a muon that has bremmed and likely faked an electron
    if (m_useArrayCutflow) {
      int j=0;
      for (const auto &electron : *m_goodElectron) {
        for (const auto &muon : *m_goodMuonForZ) {
          if (deltaR(electron->caloCluster()->etaBE(2),muon->eta(),electron->phi(),muon->phi()) < 0.3 && muon->auxdata<bool>("brem") == true) {
            m_goodElectron->erase(m_goodElectron->begin()+j);
            j--;
            break;
          }
        } // break to here
        j++;
      }
      // Tau overlap removal with non-isolated muons
      // loop round taus and remove if it is close to a muon that has bremmed and likely faked an electron
      int jj=0;
      for (const auto &tau : *m_goodTau) {
        for (const auto &muon : *m_goodMuonForZ) {
          if (deltaR(tau->eta(),muon->eta(),tau->phi(),muon->phi()) < 0.3) {
            m_goodTau->erase(m_goodTau->begin()+jj);
            jj--;
            break;
          }
        } // break to here
        jj++;
      }
    }



    // ----------------------------
    // EXOT5 derivation Event Skim
    // ----------------------------
    if ( m_dataType.find("STDM")!=std::string::npos && m_doSkimEXOT5 ) { // STDM derivation
      bool acceptSkimEvent = passUncalibMonojetCut || passRecoJetCuts || passTruthJetCuts;
      if (!acceptSkimEvent) continue; // go to the next systematic
    }


    //---------------
    // Jet Cleaning
    //---------------
    // Note that Jet Cleaning should be implemented after Overlap removal
    if (!m_useArrayCutflow) { // Implement Event Cleaning "ONLY" when doing real analysis
                              // Do not use this for local cutflow test because the test will do the event cleaning later by order of culflow
      
      bool isBadJet = false;
      // iterate over our deep copy
      for (const auto& jets : *m_goodJet) { // C++11 shortcut
        if ( !m_jetCleaningLooseBad->accept(*jets) ) isBadJet = true;
      }
      if (isBadJet) continue; // go to the next systematic

    }


    //---------------
    // b-jet Veto
    // --------------
    // b-jet counting
    int n_bJet = 0;
    if (m_goodJet->size() > 0) {
      // loop over the jets in the Good Jets Container
      for (const auto& jets : *m_goodJet) {
        // Find BTagged jets
        if (m_BJetSelectTool->accept(*jets)) {
          n_bJet ++;
        }
      }
    }
    // Veto b-jet event
    if (n_bJet > 0) continue; // go to the next systematic







    // Sort Good Jets
    //if (m_goodJet->size() > 1) std::partial_sort(m_goodJet->begin(), m_goodJet->begin()+2, m_goodJet->end(), DescendingPt());
    if (m_goodJetORTruthNuNu->size() > 1) std::sort(m_goodJetORTruthNuNu->begin(), m_goodJetORTruthNuNu->end(), DescendingPt());
    if (m_goodJetORTruthMuMu->size() > 1) std::sort(m_goodJetORTruthMuMu->begin(), m_goodJetORTruthMuMu->end(), DescendingPt());
    if (m_goodJetORTruthElEl->size() > 1) std::sort(m_goodJetORTruthElEl->begin(), m_goodJetORTruthElEl->end(), DescendingPt());



    // This real MET will be used for mT calculation in the function for Wmunu MET efficiency (void smZInvAnalysis::doWmunuSMReco)
    //==============//
    // MET building //
    //==============//

    // do CST or TST
    //std::string softTerm = "SoftClus";
    std::string softTerm = "PVSoftTrk";

    // Real MET
    float MET = -9e9;
    float MET_phi = -9e9;

    //=============================
    // Create MissingETContainers 
    //=============================

    //retrieve the original containers
    const xAOD::MissingETContainer* m_metCore(0);
    std::string coreMetKey = "MET_Core_" + jetType;
    coreMetKey.erase(coreMetKey.length() - 4); //this removes the Jets from the end of the jetType
    if ( !m_event->retrieve( m_metCore, coreMetKey ).isSuccess() ){ // retrieve arguments: container type, container key
      Error("execute()", "Unable to retrieve MET core container: " );
      return EL::StatusCode::FAILURE;
    }

    //retrieve the MET association map
    const xAOD::MissingETAssociationMap* m_metMap = 0;
    std::string metAssocKey = "METAssoc_" + jetType;
    metAssocKey.erase(metAssocKey.length() - 4 );//this removes the Jets from the end of the jetType
    if ( !m_event->retrieve( m_metMap, metAssocKey ).isSuccess() ){ // retrieve arguments: container type, container key
      Error("execute()", "Unable to retrieve MissingETAssociationMap: " );
      return EL::StatusCode::FAILURE;
    }


    // It is necessary to reset the selected objects before every MET calculation
    m_met->clear();
    m_metMap->resetObjSelectionFlags();



    //===========================
    // For rebuild the real MET
    //===========================


    // Electron
    //-----------------
    /// Creat New Hard Object Containers
    // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
    ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

    // iterate over our shallow copy
    for (const auto& electron : *m_goodElectron) { // C++11 shortcut
      // For MET rebuilding
      m_MetElectrons.push_back( electron );
    } // end for loop over shallow copied electrons
    //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

    // For real MET
    m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
        xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
        m_met,                                      //filling this met container
        m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
        m_metMap);                                  //and this association map


    if (sm_doPhoton_MET) {
      // Photon
      //-----------------
      /// Creat New Hard Object Containers
      // [For MET building] filter the Photon container m_photons, placing selected photons into m_MetPhotons
      ConstDataVector<xAOD::PhotonContainer> m_MetPhotons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Photon>

      // iterate over our shallow copy
      for (const auto& photon : *m_goodPhoton) { // C++11 shortcut
        // For MET rebuilding
        m_MetPhotons.push_back( photon );
      } // end for loop over shallow copied photons

      // For real MET
      m_metMaker->rebuildMET("RefPhoton",           //name of metPhotons in metContainer
          xAOD::Type::Photon,                       //telling the rebuilder that this is photon met
          m_met,                                    //filling this met container
          m_MetPhotons.asDataVector(),              //using these metPhotons that accepted our cuts
          m_metMap);                                //and this association map
    }


    // Only implement at EXOT5 derivation
    // METRebuilder will use "trackLinks" aux data in Tau container
    // However STDM4 derivation does not contain a aux data "trackLinks" in Tau container
    // So one cannot build the real MET using Tau objects
    if ( sm_doTau_MET && m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation
      // TAUS
      //-----------------
      /// Creat New Hard Object Containers
      // [For MET building] filter the TauJet container m_taus, placing selected taus into m_MetTaus
      ConstDataVector<xAOD::TauJetContainer> m_MetTaus(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::TauJet>

      // iterate over our shallow copy
      for (const auto& taujet : *m_goodTau) { // C++11 shortcut
        // For MET rebuilding
        m_MetTaus.push_back( taujet );
      } // end for loop over shallow copied taus

      // For real MET
      m_metMaker->rebuildMET("RefTau",           //name of metTaus in metContainer
          xAOD::Type::Tau,                       //telling the rebuilder that this is tau met
          m_met,                                 //filling this met container
          m_MetTaus.asDataVector(),              //using these metTaus that accepted our cuts
          m_metMap);                             //and this association map
    }

    // Muon
    //-----------------
    /// Creat New Hard Object Containers
    // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
    ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

    // iterate over our shallow copy
    for (const auto& muon : *m_goodMuon) { // C++11 shortcut
      // For MET rebuilding
      m_MetMuons.push_back( muon );
    } // end for loop over shallow copied muons
    // For real MET
    m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
        xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
        m_met,                                  //filling this met container
        m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
        m_metMap);                              //and this association map




    // JET
    //-----------------
    //Now time to rebuild jetMet and get the soft term
    //This adds the necessary soft term for both CST and TST
    //these functions create an xAODMissingET object with the given names inside the container
    // For real MET
    m_metMaker->rebuildJetMET("RefJet",          //name of jet met
        "SoftClus",        //name of soft cluster term met
        "PVSoftTrk",       //name of soft track term met
        m_met,             //adding to this new met container
        m_allJet,          //using this jet collection to calculate jet met
        m_metCore,         //core met container
        m_metMap,          //with this association map
        true);             //apply jet jvt cut




    /////////////////////////////
    // Soft term uncertainties //
    /////////////////////////////
    if (!m_isData) {
      // Get the track soft term (For real MET)
      xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
      if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
        Error("execute()", "METSystematicsTool returns Error CorrectionCode");
      }
    }


    ///////////////
    // MET Build //
    ///////////////
    //m_metMaker->rebuildTrackMET("RefJetTrk", softTerm, m_met, m_jetSC, m_metCore, m_metMap, true);

    //this builds the final track or cluster met sums, using systematic varied container
    //In the future, you will be able to run both of these on the same container to easily output CST and TST

    // For real MET
    m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());




    ///////////////////
    // Fill real MET //
    ///////////////////

    MET = ((*m_met)["Final"]->met());
    MET_phi = ((*m_met)["Final"]->phi());










    /////////////////////////////////////////
    // Do Analysis with differenct Channel //
    /////////////////////////////////////////
    if (!m_useArrayCutflow) { // Do not analysis with local cutflow test!!
                              // If "Local cutflow test" is enabled, Event jet cleaning will not be applied by above code,
                              // because event cleaning should be implemented after MET cut following our cutflow order.

      // For Exotic study
      if (m_isZnunu) doZnunuExoticReco(m_metCore, m_metMap, m_mcEventWeight, m_sysName);
      if (m_isZmumu) doZmumuExoticReco(m_metCore, m_metMap, m_muons, m_muonSC, m_mcEventWeight, m_sysName);
      if (m_isZee) doZeeExoticReco(m_metCore, m_metMap, m_elecSC, m_mcEventWeight, m_sysName);

      // For SM study
      if (m_isZnunu) {
        // Exclusive
        doZnunuSMReco(m_metCore, m_metMap, m_mcEventWeight, "_reco_exclusive_", m_sysName);
        // Inclusive
        doZnunuSMReco(m_metCore, m_metMap, m_mcEventWeight, "_reco_inclusive_", m_sysName);
      }
      if (m_isZmumu) {
        // Exclusive
        doZmumuSMReco(m_metCore, m_metMap, m_muons, m_muonSC, m_mcEventWeight, "_reco_exclusive_", m_sysName);
        // Inclusive
        doZmumuSMReco(m_metCore, m_metMap, m_muons, m_muonSC, m_mcEventWeight, "_reco_inclusive_", m_sysName);
      }
      if (m_isZee) {
        // Exclusive
        doZeeSMReco(m_metCore, m_metMap, m_elecSC, m_mcEventWeight, "_reco_exclusive_", m_sysName);
        // Inclusive
        doZeeSMReco(m_metCore, m_metMap, m_elecSC, m_mcEventWeight, "_reco_inclusive_", m_sysName);
      }
      if (m_isWmunu) {
        // Exclusive
        doWmunuSMReco(m_metCore, m_metMap, MET, MET_phi, m_mcEventWeight, "_reco_exclusive_", m_sysName);
        // Inclusive
        doWmunuSMReco(m_metCore, m_metMap, MET, MET_phi, m_mcEventWeight, "_reco_inclusive_", m_sysName);
      }
      if (m_isWenu) {
        // Exclusive
        doWenuSMReco(m_metCore, m_metMap, MET, MET_phi, m_elecSC, m_goodElectron, m_mcEventWeight, "_reco_exclusive_", m_sysName);
        // Inclusive
        doWenuSMReco(m_metCore, m_metMap, MET, MET_phi, m_elecSC, m_goodElectron, m_mcEventWeight, "_reco_inclusive_", m_sysName);
      }


    }




    ///////////////////////
    // Local Cutflow Test//
    ///////////////////////

    //---------------
    // Znunu Channel
    //---------------
    if ( m_sysName=="" && m_useArrayCutflow && m_isZnunu ) {


      //==============//
      // MET building //
      //==============//

      // do CST or TST
      //std::string softTerm = "SoftClus";
      std::string softTerm = "PVSoftTrk";

      // Real MET
      float MET = -9e9;
      float MET_phi = -9e9;

      //=============================
      // Create MissingETContainers 
      //=============================

      //retrieve the original containers
      const xAOD::MissingETContainer* m_metCore(0);
      std::string coreMetKey = "MET_Core_" + jetType;
      coreMetKey.erase(coreMetKey.length() - 4); //this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metCore, coreMetKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MET core container: " );
        return EL::StatusCode::FAILURE;
      }

      //retrieve the MET association map
      const xAOD::MissingETAssociationMap* m_metMap = 0;
      std::string metAssocKey = "METAssoc_" + jetType;
      metAssocKey.erase(metAssocKey.length() - 4 );//this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metMap, metAssocKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MissingETAssociationMap: " );
        return EL::StatusCode::FAILURE;
      }


      // It is necessary to reset the selected objects before every MET calculation
      m_met->clear();
      m_metMap->resetObjSelectionFlags();



      //===========================
      // For rebuild the real MET
      //===========================

      // Only implement at EXOT5 derivation
      // METRebuilder will use "trackLinks" aux data in Tau container
      // However STDM4 derivation does not contain a aux data "trackLinks" in Tau container
      // So one cannot build the real MET using Tau objects
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation

        // Electron
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
        ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

        // iterate over our shallow copy
        for (const auto& electron : *m_goodElectron) { // C++11 shortcut
          // For MET rebuilding
          m_MetElectrons.push_back( electron );
        } // end for loop over shallow copied electrons
        //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

        // For real MET
        m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
            xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
            m_met,                                      //filling this met container
            m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
            m_metMap);                                  //and this association map


        /*
        // Photon
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Photon container m_photons, placing selected photons into m_MetPhotons
        ConstDataVector<xAOD::PhotonContainer> m_MetPhotons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Photon>

        // iterate over our shallow copy
        for (const auto& photon : *m_goodPhoton) { // C++11 shortcut
        // For MET rebuilding
        m_MetPhotons.push_back( photon );
        } // end for loop over shallow copied photons

        // For real MET
        m_metMaker->rebuildMET("RefPhoton",           //name of metPhotons in metContainer
        xAOD::Type::Photon,                       //telling the rebuilder that this is photon met
        m_met,                                    //filling this met container
        m_MetPhotons.asDataVector(),              //using these metPhotons that accepted our cuts
        m_metMap);                                //and this association map

*/

        // TAUS
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the TauJet container m_taus, placing selected taus into m_MetTaus
        ConstDataVector<xAOD::TauJetContainer> m_MetTaus(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::TauJet>

        // iterate over our shallow copy
        for (const auto& taujet : *m_goodTau) { // C++11 shortcut
          // For MET rebuilding
          m_MetTaus.push_back( taujet );
        } // end for loop over shallow copied taus

        // For real MET
        m_metMaker->rebuildMET("RefTau",           //name of metTaus in metContainer
            xAOD::Type::Tau,                       //telling the rebuilder that this is tau met
            m_met,                                 //filling this met container
            m_MetTaus.asDataVector(),              //using these metTaus that accepted our cuts
            m_metMap);                             //and this association map


        // Muon
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
        ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

        // iterate over our shallow copy
        for (const auto& muon : *m_goodMuon) { // C++11 shortcut
          // For MET rebuilding
          m_MetMuons.push_back( muon );
        } // end for loop over shallow copied muons
        // For real MET
        m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
            xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
            m_met,                                  //filling this met container
            m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
            m_metMap);                              //and this association map




        // JET
        //-----------------
        //Now time to rebuild jetMet and get the soft term
        //This adds the necessary soft term for both CST and TST
        //these functions create an xAODMissingET object with the given names inside the container
        // For real MET
        m_metMaker->rebuildJetMET("RefJet",          //name of jet met
            "SoftClus",        //name of soft cluster term met
            "PVSoftTrk",       //name of soft track term met
            m_met,             //adding to this new met container
            m_allJet,          //using this jet collection to calculate jet met
            m_metCore,         //core met container
            m_metMap,          //with this association map
            true);             //apply jet jvt cut




        /////////////////////////////
        // Soft term uncertainties //
        /////////////////////////////
        if (!m_isData) {
          // Get the track soft term (For real MET)
          xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
          if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
            Error("execute()", "METSystematicsTool returns Error CorrectionCode");
          }
        }


        ///////////////
        // MET Build //
        ///////////////
        //m_metMaker->rebuildTrackMET("RefJetTrk", softTerm, m_met, m_jetSC, m_metCore, m_metMap, true);

        //this builds the final track or cluster met sums, using systematic varied container
        //In the future, you will be able to run both of these on the same container to easily output CST and TST

        // For real MET
        m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());




        ///////////////////
        // Fill real MET //
        ///////////////////

        MET = ((*m_met)["Final"]->met());
        MET_phi = ((*m_met)["Final"]->phi());


      } // Only using EXOT5 derivation



      //------------------
      // Pass MET Trigger
      //------------------
      /*
      if (m_isData) { // Data
        if (!(m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe100_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50") ) ) continue; // go to next systematic
      }
      */
      if (!m_met_trig_fire) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[4]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("MET Trigger");




      //----------
      // MET cut
      //----------
      if ( MET < m_metCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[5]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("MET cut");





      // Event Jet cleaning
      bool isBadJet = false;
      //------------------
      // Bad Jet Decision
      //------------------
      // iterate over our deep copy
      for (const auto& jets : *m_goodJet) { // C++11 shortcut
        if ( !m_jetCleaningLooseBad->accept(*jets) ) isBadJet = true;
      }
      if (isBadJet) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[6]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet Cleaning");





      //-------------------------------------
      // Define Monojet and DiJet Properties
      //-------------------------------------

      // Monojet
      float monojet_pt = 0;
      float monojet_phi = 0;
      float monojet_eta = 0;
      float monojet_rapidity = 0;
      float dPhiMonojetMet = 0;
      // Dijet
      TLorentzVector jet1;
      TLorentzVector jet2;
      float jet1_pt = 0;
      float jet2_pt = 0;
      float jet3_pt = 0;
      float jet1_phi = 0;
      float jet2_phi = 0;
      float jet3_phi = 0;
      float jet1_eta = 0;
      float jet2_eta = 0;
      float jet3_eta = 0;
      float jet1_rapidity = 0;
      float jet2_rapidity = 0;
      float jet3_rapidity = 0;

      float goodJet_ht = 0;
      float dPhiJet1Met = 0;
      float dPhiJet2Met = 0;
      float dPhiJet3Met = 0;

      float mjj = 0;
      bool pass_monoJet = false; // Select monoJet
      bool pass_diJet = false; // Select DiJet
      bool pass_CJV = true; // Central Jet Veto (CJV)
      bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)


      ///////////////////////
      // Monojet Selection //
      ///////////////////////
      if (m_goodJet->size() > 0) {

        monojet_pt = m_goodJet->at(0)->pt();
        monojet_phi = m_goodJet->at(0)->phi();
        monojet_eta = m_goodJet->at(0)->eta();
        monojet_rapidity = m_goodJet->at(0)->rapidity();


        // Define Monojet
        if ( monojet_pt > m_monoJetPtCut ){
          if ( fabs(monojet_eta) < m_monoJetEtaCut){
            if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
              pass_monoJet = true;
              //Info("execute()", "  Leading jet pt = %.2f GeV", monojet_pt * 0.001);
            }
          }
        }

        // deltaPhi(monojet,MET) decision
        // For Znunu
        dPhiMonojetMet = deltaPhi(monojet_phi, MET_phi);

      } // MonoJet selection 



      /////////////////////
      // DiJet Selection //
      /////////////////////
      if (m_goodJet->size() > 1) {

        jet1 = m_goodJet->at(0)->p4();
        jet2 = m_goodJet->at(1)->p4();
        jet1_pt = m_goodJet->at(0)->pt();
        jet2_pt = m_goodJet->at(1)->pt();
        jet1_phi = m_goodJet->at(0)->phi();
        jet2_phi = m_goodJet->at(1)->phi();
        jet1_eta = m_goodJet->at(0)->eta();
        jet2_eta = m_goodJet->at(1)->eta();
        jet1_rapidity = m_goodJet->at(0)->rapidity();
        jet2_rapidity = m_goodJet->at(1)->rapidity();
        auto dijet = jet1 + jet2;
        mjj = dijet.M();

        //Info("execute()", "  jet1 = %.2f GeV, jet2 = %.2f GeV", jet1_pt * 0.001, jet2_pt * 0.001);
        //Info("execute()", "  mjj = %.2f GeV", mjj * 0.001);

        // Define Dijet
        if ( jet1_pt > m_diJet1PtCut && jet2_pt > m_diJet2PtCut ){
          if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
            pass_diJet = true;
          }
        }

        // deltaPhi(Jet1,MET) or deltaPhi(Jet2,MET) decision
        // For Znunu
        dPhiJet1Met = deltaPhi(jet1_phi, MET_phi);
        dPhiJet2Met = deltaPhi(jet2_phi, MET_phi);

      } // DiJet selection 



      // For jet3
      if (m_goodJet->size() > 2) {
        jet3_pt = m_goodJet->at(2)->pt();
        jet3_phi = m_goodJet->at(2)->phi();
        jet3_eta = m_goodJet->at(2)->eta();
        jet3_rapidity = m_goodJet->at(2)->rapidity();
        // deltaPhi(Jet3,MET)
        dPhiJet3Met = deltaPhi(jet3_phi, MET_phi);
      }

      // Define deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)
      if (m_goodJet->size() > 0) {

        // loop over the jets in the Good Jets Container
        for (const auto& jet : *m_goodJet) {
          float good_jet_pt = jet->pt();
          float good_jet_rapidity = jet->rapidity();
          float good_jet_phi = jet->phi();

          // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
          if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet || m_goodJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
            // For Znunu
            float dPhijetmet = deltaPhi(good_jet_phi,MET_phi);
            if ( good_jet_pt > 30000. && fabs(good_jet_rapidity) < 4.4 && dPhijetmet < 0.4 ) pass_dPhijetmet = false;
            dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
          }

          // Central Jet Veto (CJV)
          if ( m_goodJet->size() > 2 && pass_diJet ){
            if (m_goodJet->at(0) != jet && m_goodJet->at(1) != jet){
              //cout << "m_goodJet->at(0) = " << m_goodJet->at(0) << " jet = " << jet << endl;
              if (good_jet_pt > m_CJVptCut && fabs(good_jet_rapidity) < m_diJetRapCut) {
                if ( (jet1_rapidity > jet2_rapidity) && (good_jet_rapidity < jet1_rapidity && good_jet_rapidity > jet2_rapidity)){
                  pass_CJV = false;
                }
                if ( (jet1_rapidity < jet2_rapidity) && (good_jet_rapidity > jet1_rapidity && good_jet_rapidity < jet2_rapidity)){
                  pass_CJV = false;
                }
                /* //Valentinos' way (same result as mine)
                   float rapLow  = std::min(jet1_rapidity, jet2_rapidity);
                   float rapHigh = std::max(jet1_rapidity, jet2_rapidity);
                   if (good_jet_rapidity > rapLow && good_jet_rapidity < rapHigh) pass_CJV = false;
                   */
              }
            }
          }

          //Info("execute()", "  Znunu Signal Jet pt = %.2f GeV, eta = %.2f", good_pt_jet * 0.001, good_eta_jet);
          goodJet_ht += good_jet_pt;
        } // Jet loop

      } // End deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)




      //--------------------------
      // Leading jet pass TightBad
      //--------------------------
      if ( m_goodJet->size() > 0 && !m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ) continue; // go to next systematic
      if ( m_goodJet->size() < 1 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[7]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Leading Jet");



      //-----------------
      // At least 2 jets
      //-----------------
      if ( m_goodJet->size() < 2 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[8]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("At least 2 jets");



      //---------------------------------------------------
      // At least 2 jets with pT1>80GeV, pT2>50GeV, |y|<4.4
      //---------------------------------------------------
      if ( !pass_diJet ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[9]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("VBF jet pt cut");






      //---------
      // Mjj cut
      //---------
      if ( mjj < m_mjjCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[10]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Mjj cut");

      //--------------
      // Jet-MET veto
      //--------------
      if ( !pass_dPhijetmet ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[11]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet-MET veto");



      //------------------------
      // Additional lepton Veto 
      //------------------------
      // Muon veto
      if ( m_goodMuon->size() > 0  ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[12]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("muon veto");
      // Electron veto
      if ( m_goodElectron->size() > 0  ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[13]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("electron veto");
      // Tau veto "ONLY" available in EXOT5 derivation
      // because aux item "trackLinks" is missing in the STDM4 derivation samples
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation
        if ( m_goodTau->size() > 0  ) continue; // go to next systematic
      }
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[14]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("tau veto");

      //------------------
      // Central Jet Veto
      //------------------
      if ( !pass_CJV ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[15]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("CJV cut");



    } // End of Cutflow (Znunu channel)



    //---------------
    // Zmumu Channel
    //---------------
    if ( m_sysName=="" && m_useArrayCutflow && m_isZmumu ) {


      //==============//
      // MET building //
      //==============//

      // do CST or TST
      //std::string softTerm = "SoftClus";
      std::string softTerm = "PVSoftTrk";

      // Real MET
      float MET = -9e9;
      float MET_phi = -9e9;
      // Zmumu MET
      float MET_Zmumu = -9e9;
      float MET_Zmumu_phi = -9e9;

      //=============================
      // Create MissingETContainers 
      //=============================

      //retrieve the original containers
      const xAOD::MissingETContainer* m_metCore(0);
      std::string coreMetKey = "MET_Core_" + jetType;
      coreMetKey.erase(coreMetKey.length() - 4); //this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metCore, coreMetKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MET core container: " );
        return EL::StatusCode::FAILURE;
      }

      //retrieve the MET association map
      const xAOD::MissingETAssociationMap* m_metMap = 0;
      std::string metAssocKey = "METAssoc_" + jetType;
      metAssocKey.erase(metAssocKey.length() - 4 );//this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metMap, metAssocKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MissingETAssociationMap: " );
        return EL::StatusCode::FAILURE;
      }


      // It is necessary to reset the selected objects before every MET calculation
      m_met->clear();
      m_metMap->resetObjSelectionFlags();



      //===========================
      // For rebuild the real MET
      //===========================

      // Only implement at EXOT5 derivation
      // METRebuilder will use "trackLinks" aux data in Tau container
      // However STDM4 derivation does not contain a aux data "trackLinks" in Tau container
      // So one cannot build the real MET using Tau objects
      if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation

        // Electron
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
        ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

        // iterate over our shallow copy
        for (const auto& electron : *m_goodElectron) { // C++11 shortcut
          // For MET rebuilding
          m_MetElectrons.push_back( electron );
        } // end for loop over shallow copied electrons
        //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

        // For real MET
        m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
            xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
            m_met,                                      //filling this met container
            m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
            m_metMap);                                  //and this association map


        /*
        // Photon
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Photon container m_photons, placing selected photons into m_MetPhotons
        ConstDataVector<xAOD::PhotonContainer> m_MetPhotons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Photon>

        // iterate over our shallow copy
        for (const auto& photon : *m_goodPhoton) { // C++11 shortcut
        // For MET rebuilding
        m_MetPhotons.push_back( photon );
        } // end for loop over shallow copied photons

        // For real MET
        m_metMaker->rebuildMET("RefPhoton",           //name of metPhotons in metContainer
        xAOD::Type::Photon,                       //telling the rebuilder that this is photon met
        m_met,                                    //filling this met container
        m_MetPhotons.asDataVector(),              //using these metPhotons that accepted our cuts
        m_metMap);                                //and this association map

*/

        // TAUS
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the TauJet container m_taus, placing selected taus into m_MetTaus
        ConstDataVector<xAOD::TauJetContainer> m_MetTaus(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::TauJet>

        // iterate over our shallow copy
        for (const auto& taujet : *m_goodTau) { // C++11 shortcut
          // For MET rebuilding
          m_MetTaus.push_back( taujet );
        } // end for loop over shallow copied taus

        // For real MET
        m_metMaker->rebuildMET("RefTau",           //name of metTaus in metContainer
            xAOD::Type::Tau,                       //telling the rebuilder that this is tau met
            m_met,                                 //filling this met container
            m_MetTaus.asDataVector(),              //using these metTaus that accepted our cuts
            m_metMap);                             //and this association map


        // Muon
        //-----------------
        /// Creat New Hard Object Containers
        // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
        ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

        // iterate over our shallow copy
        for (const auto& muon : *m_goodMuon) { // C++11 shortcut
          // For MET rebuilding
          m_MetMuons.push_back( muon );
        } // end for loop over shallow copied muons
        // For real MET
        m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
            xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
            m_met,                                  //filling this met container
            m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
            m_metMap);                              //and this association map




        // JET
        //-----------------
        //Now time to rebuild jetMet and get the soft term
        //This adds the necessary soft term for both CST and TST
        //these functions create an xAODMissingET object with the given names inside the container
        // For real MET
        m_metMaker->rebuildJetMET("RefJet",          //name of jet met
            "SoftClus",        //name of soft cluster term met
            "PVSoftTrk",       //name of soft track term met
            m_met,             //adding to this new met container
            m_allJet,          //using this jet collection to calculate jet met
            m_metCore,         //core met container
            m_metMap,          //with this association map
            true);             //apply jet jvt cut




        /////////////////////////////
        // Soft term uncertainties //
        /////////////////////////////
        if (!m_isData) {
          // Get the track soft term (For real MET)
          xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
          if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
            Error("execute()", "METSystematicsTool returns Error CorrectionCode");
          }
        }


        ///////////////
        // MET Build //
        ///////////////
        //m_metMaker->rebuildTrackMET("RefJetTrk", softTerm, m_met, m_jetSC, m_metCore, m_metMap, true);

        //this builds the final track or cluster met sums, using systematic varied container
        //In the future, you will be able to run both of these on the same container to easily output CST and TST

        // For real MET
        m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());




        ///////////////////
        // Fill real MET //
        ///////////////////

        MET = ((*m_met)["Final"]->met());
        MET_phi = ((*m_met)["Final"]->phi());


      } // Only using EXOT5 derivation



      //===================================================================
      // For rebuild the emulated MET for Zmumu (by marking Muon invisible)
      //===================================================================

      // It is necessary to reset the selected objects before every MET calculation
      m_met->clear();
      m_metMap->resetObjSelectionFlags();


      // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for muon FSR

      // Muon
      //-----------------
      /// Creat New Hard Object Containers
      // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
      //
      // For emulated MET (No muons)
      // Make a empty container for invisible muons
      ConstDataVector<xAOD::MuonContainer> m_EmptyMuons(SG::VIEW_ELEMENTS);
      //m_EmptyMuons.clear();
      m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
          xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
          m_met,                         //filling this met container
          m_EmptyMuons.asDataVector(),            //using these metMuons that accepted our cuts
          m_metMap);                     //and this association map
      // Make a container for invisible muons
      ConstDataVector<xAOD::MuonContainer> m_invisibleMuons(SG::VIEW_ELEMENTS);
      for (const auto& muon : *m_muonSC) { // C++11 shortcut
        for (const auto& goodmuon : *m_goodMuonForZ) { // C++11 shortcut
          // Check if muons are matched to good muons
          if (muon->pt() == goodmuon->pt() && muon->eta() == goodmuon->eta() && muon->phi() == goodmuon->phi()){
            // Put good muons in
            m_invisibleMuons.push_back( muon );
          }
        }
      }
      // Mark muons invisible
      m_metMaker->markInvisible(m_invisibleMuons.asDataVector(), m_metMap, m_met);


      // Test addGhostMuonsToJets
      // "GhostMuon”is an aux variable that the jets are decorated with inside addGhostMuonsToJets
      /*
      Info("execute()", "====================================");
      Info("execute()", " Event # = %llu", eventInfo->eventNumber());
      int count = 0;
      for (const auto& jet: *m_allJet) {
        Info("execute()", " jet #: %i", count);
        Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
        Info("execute()", " jet eta = %.3f", jet->eta());
        Info("execute()", " jet phi = %.3f", jet->phi());
        Info("execute()", " jet jvt = %.3f", jet->auxdata<float>("Jvt"));
        Info("execute()", " jet forwardjvt = %i", jet->auxdata<char>("passFJVT"));

        std::vector<const xAOD::Muon*> muons_in_jet;
        if ( jet->getAssociatedObjects("GhostMuon", muons_in_jet) ) {
          Info("execute()", " ghostMuons size = %lu", muons_in_jet.size());
        }
        count++;
      }
      */


      // JET
      //-----------------
      //Now time to rebuild jetMet and get the soft term
      //This adds the necessary soft term for both CST and TST
      //these functions create an xAODMissingET object with the given names inside the container

      // For emulated MET marking muons invisible
      m_metMaker->rebuildJetMET("RefJet",          //name of jet met
          "SoftClus",           //name of soft cluster term met
          "PVSoftTrk",          //name of soft track term met
          m_met,       //adding to this new met container
          m_allJet,                //using this jet collection to calculate jet met
          m_metCore,   //core met container
          m_metMap,    //with this association map
          true);                //apply jet jvt cut

      /////////////////////////////
      // Soft term uncertainties //
      /////////////////////////////
      if (!m_isData) {
        // Get the track soft term for Zmumu (For emulated MET marking muons invisible)
        xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
        if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
          Error("execute()", "METSystematicsTool returns Error CorrectionCode");
        }
      }

      ///////////////
      // MET Build //
      ///////////////
      // For emulated MET for Zmumu marking muons invisible
      m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

      //////////////////////////////////////////////////////////////
      // Fill emulated MET for Zmumu (by marking muons invisible) //
      //////////////////////////////////////////////////////////////
      MET_Zmumu = ((*m_met)["Final"]->met());
      MET_Zmumu_phi = ((*m_met)["Final"]->phi());





      //------------------
      // Pass MET Trigger
      //------------------
      /*
      if (m_isData) { // Data
        if (!(m_trigDecisionTool->isPassed("HLT_xe80_tc_lcw_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe90_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe100_mht_L1XE50") || m_trigDecisionTool->isPassed("HLT_xe110_mht_L1XE50") ) ) continue; // go to next systematic
      }
      */
      if (!m_met_trig_fire) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[4]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("MET Trigger");



      //----------------------------------
      // Define Zmumu and Wmunu Selection
      //----------------------------------

      // Zmumu
      float mll_muon = 0.;
      bool pass_dimuonPtCut = false; // dimuon pT cut
      bool pass_OSmuon = false; // Opposite sign charge muon
      bool pass_SSmuon = false; // Same sign charge muon
      int numExtra = 0;

      // For Zmumu Selection
      if (m_goodMuonForZ->size() > 1) {

        TLorentzVector muon1 = m_goodMuonForZ->at(0)->p4();
        TLorentzVector muon2 = m_goodMuonForZ->at(1)->p4();
        float muon1_pt = m_goodMuonForZ->at(0)->pt();
        float muon2_pt = m_goodMuonForZ->at(1)->pt();
        float muon1_charge = m_goodMuonForZ->at(0)->charge();
        float muon2_charge = m_goodMuonForZ->at(1)->charge();
        auto Zll_muon = muon1 + muon2;
        mll_muon = Zll_muon.M();

        //Info("execute()", "  muon1 = %.2f GeV, muon2 = %.2f GeV", muon1_pt * 0.001, muon2_pt * 0.001);
        //Info("execute()", "  mll (Zmumu) = %.2f GeV", mll_muon * 0.001);

        if ( muon1_pt >  m_LeadLepPtCut ) pass_dimuonPtCut = true;
        if ( muon1_charge * muon2_charge < 0 ) pass_OSmuon = true;
        if ( muon1_charge * muon2_charge > 0 ) pass_SSmuon = true;
        //Info("execute()", "  muon1 charge = %f, muon2 charge = %f, pass_OSmuon = %d, pass_SSmuon = %d", muon1_charge, muon2_charge, pass_OSmuon, pass_SSmuon);

        uint index1 = m_goodMuonForZ->at(0)->index();
        uint index2 = m_goodMuonForZ->at(1)->index();
        for (const auto &muon : *m_goodMuon) {
          if (muon->index() != index1 && muon->index() != index2) numExtra++;
          //std::cout << muon->index() << " " << index1 << " " << index2 << std::endl;
        }

      } // Zmumu selection loop




      //if ( eventInfo->eventNumber() == 1417207 ) {
      /*
         Info("execute()", "====================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());
         int count = 0;
         for (const auto& jet : *m_allJet) {
         Info("execute()", " jet #: %i", count);
         Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
         Info("execute()", " jet eta = %.3f", jet->eta());
         Info("execute()", " jet phi = %.3f", jet->phi());
         Info("execute()", " jet jvt = %.3f", jet->auxdata<float>("Jvt"));
         Info("execute()", " jet forwardjvt = %i", jet->auxdata<char>("passFJVT"));
         count++;
         }
         */


      /*
      //Info("execute()", " Good Event number = %i", m_eventCutflow[6]);
      int muonCount = 0;
      for (const auto& muon : *m_goodMuonForZ) {
      Info("execute()", " muon # : %i", muonCount);
      Info("execute()", " muon pt = %.3f GeV", muon->pt() * 0.001);
      Info("execute()", " muon eta = %.3f", muon->eta());
      Info("execute()", " muon phi = %.3f", muon->phi());
      //Info("execute()", " muon charge = %.3f", muon->charge());
      muonCount++;

      // d0 / z0 cuts applied
      const xAOD::TrackParticle* tp = muon->primaryTrackParticle();
      // zo cut
      //float z0sintheta = 1e8;
      //if (primVertex) z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( muon->p4().Theta() );
      float z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( tp->theta() );
      // d0 significance (Transverse impact parameter)
      double d0sig = xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );

      Info("execute()", " muon z0sintheta = %.3f", z0sintheta);
      Info("execute()", " muon d0sig = %.3f", d0sig);
      }

      //Info("execute()", " Mll = %.3f GeV", mll_muon * 0.001);
      */

      /*
         Info("execute()", " MET = %.3f GeV", ((*m_met)["Final"]->met()) * 0.001);
      //Info("execute()", " RefElectron = %.3f GeV", ((*m_met)["RefElectron"]->met()) * 0.001);
      //Info("execute()", " RefPhoton = %.3f GeV", ((*m_met)["RefPhoton"]->met()) * 0.001);
      //Info("execute()", " RefTau = %.3f GeV", ((*m_met)["RefTau"]->met()) * 0.001);
      Info("execute()", " RefMuon = %.3f GeV", ((*m_met)["RefMuon"]->met()) * 0.001);
      Info("execute()", " Invisibles = %.3f GeV", ((*m_met)["Invisibles"]->met()) * 0.001);
      Info("execute()", " RefJet = %.3f GeV", ((*m_met)["RefJet"]->met()) * 0.001);
      Info("execute()", " SoftClus = %.3f GeV", ((*m_met)["SoftClus"]->met()) * 0.001);
      Info("execute()", " PVSoftTrk = %.3f GeV", ((*m_met)["PVSoftTrk"]->met()) * 0.001);

      int muCount = 0;
      for (const auto& muon : *m_muons) {
      Info("execute()", " muon # : %i", muCount);
      Info("execute()", " muon pt = %.3f GeV", muon->pt() * 0.001);
      Info("execute()", " muon eta = %.3f", muon->eta());
      Info("execute()", " muon phi = %.3f", muon->phi());
      muCount++;
      }
      */

      //}




      /*
         if (m_goodElectron->size() > 0 || m_goodElectron->size() > 0 ) {
         Info("execute()", "====================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());

         int elecCount = 0;
         for (const auto& elec : *m_goodElectron) {
         Info("execute()", " elec # : %i", elecCount);
         Info("execute()", " elec pt = %.3f GeV", elec->pt() * 0.001);
         Info("execute()", " elec eta = %.3f", elec->eta());
         Info("execute()", " elec phi = %.3f", elec->phi());
         elecCount++;
         }
         int tauCount = 0;
         for (const auto& tau : *m_goodTau) {
         Info("execute()", " tau # : %i", tauCount);
         Info("execute()", " tau pt = %.3f GeV", tau->pt() * 0.001);
         Info("execute()", " tau eta = %.3f", tau->eta());
         Info("execute()", " tau phi = %.3f", tau->phi());
         tauCount++;
         }
         }
         */




      //-----------------
      // At least 2 muons
      //-----------------
      if (m_goodMuonForZ->size() < 2) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[5]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("At least 2 muons");


      /*
         Info("execute()", "=====================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());
         int muonCount = 0;
         for (const auto& muon : *m_goodMuonForZ) {
         muonCount++;
         if (m_isolationLooseTrackOnlySelectionTool->accept(*muon)) {
         Info("execute()", " muon # : %i , Isolated", muonCount);
         } else Info("execute()", " muon # : %i", muonCount);
         Info("execute()", " muon pt = %.3f GeV", muon->pt() * 0.001);
         Info("execute()", " muon eta = %.3f", muon->eta());
         Info("execute()", " muon phi = %.3f", muon->phi());

      // d0 / z0 cuts applied
      const xAOD::TrackParticle* tp = muon->primaryTrackParticle();
      // zo cut
      //float z0sintheta = 1e8;
      //if (primVertex) z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( muon->p4().Theta() );
      float z0sintheta = ( tp->z0() + tp->vz() - primVertex->z() ) * TMath::Sin( tp->theta() );
      // d0 significance (Transverse impact parameter)
      double d0sig = xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );

      Info("execute()", " muon z0sintheta = %.3f", z0sintheta);
      Info("execute()", " muon d0sig = %.3f", d0sig);
      }
      */





      //-------------------------------------------------------------------
      // Opposite sign muons with at least one pT > 80GeV && 66 < mll < 116
      //-------------------------------------------------------------------
      if ( !pass_dimuonPtCut ) continue; // go to next systematic
      if ( !pass_OSmuon ) continue; // go to next systematic
      if ( mll_muon < m_mllMin || mll_muon > m_mllMax ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[6]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("dimuon and mll");


      /*
      Info("execute()", "=====================================");
      Info("execute()", " Event # = %llu", eventInfo->eventNumber());
      //Info("execute()", " Good Event number = %i", m_eventCutflow[6]);
      Info("execute()", " MET = %.3f GeV", ((*m_met)["Final"]->met()) * 0.001);
      //Info("execute()", " RefElectron = %.3f GeV", ((*m_met)["RefElectron"]->met()) * 0.001);
      //Info("execute()", " RefPhoton = %.3f GeV", ((*m_met)["RefPhoton"]->met()) * 0.001);
      //Info("execute()", " RefTau = %.3f GeV", ((*m_met)["RefTau"]->met()) * 0.001);
      Info("execute()", " RefMuon = %.3f GeV", ((*m_met)["RefMuon"]->met()) * 0.001);
      Info("execute()", " Invisibles = %.3f GeV", ((*m_met)["Invisibles"]->met()) * 0.001);
      Info("execute()", " RefJet = %.3f GeV", ((*m_met)["RefJet"]->met()) * 0.001);
      Info("execute()", " SoftClus = %.3f GeV", ((*m_met)["SoftClus"]->met()) * 0.001);
 
      int count = 0;
      for (const auto& jet : *m_allJet) {
        Info("execute()", " jet # : %i", count);
        Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
        Info("execute()", " jet eta = %.3f", jet->eta());
        Info("execute()", " jet phi = %.3f", jet->phi());
        Info("execute()", " jet jvt = %.3f", jet->auxdata<float>("Jvt"));
        Info("execute()", " jet forwardjvt = %i", jet->auxdata<char>("passFJVT"));
        count++;
      }
      */


      //----------------------
      // Additional Muon Veto
      //----------------------
      //if (numExtra > 0) continue; // go to next systematic



      //----------
      // MET cut
      //----------
      if ( MET_Zmumu < m_metCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[7]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("MET cut");





      // Event Jet cleaning
      bool isBadJet = false;
      //------------------
      // Bad Jet Decision
      //------------------
      // iterate over our deep copy
      for (const auto& jets : *m_goodJet) { // C++11 shortcut
        if ( !m_jetCleaningLooseBad->accept(*jets) ) isBadJet = true;
      }
      if (isBadJet) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[8]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet Cleaning");





      //-------------------------------------
      // Define Monojet and DiJet Properties
      //-------------------------------------

      // Monojet
      float monojet_pt = 0;
      float monojet_phi = 0;
      float monojet_eta = 0;
      float monojet_rapidity = 0;
      float dPhiMonojetMet_Zmumu = 0;
      // Dijet
      TLorentzVector jet1;
      TLorentzVector jet2;
      float jet1_pt = 0;
      float jet2_pt = 0;
      float jet3_pt = 0;
      float jet1_phi = 0;
      float jet2_phi = 0;
      float jet3_phi = 0;
      float jet1_eta = 0;
      float jet2_eta = 0;
      float jet3_eta = 0;
      float jet1_rapidity = 0;
      float jet2_rapidity = 0;
      float jet3_rapidity = 0;

      float goodJet_ht = 0;
      float dPhiJet1Met_Zmumu = 0;
      float dPhiJet2Met_Zmumu = 0;
      float dPhiJet3Met_Zmumu = 0;

      float mjj = 0;
      bool pass_monoJet = false; // Select monoJet
      bool pass_diJet = false; // Select DiJet
      bool pass_CJV = true; // Central Jet Veto (CJV)
      bool pass_dPhijetmet_Zmumu = true; // deltaPhi(Jet_i,MET_Zmumu)
      float dPhiMinjetmet_Zmumu = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)


      ///////////////////////
      // Monojet Selection //
      ///////////////////////
      if (m_goodJet->size() > 0) {

        monojet_pt = m_goodJet->at(0)->pt();
        monojet_phi = m_goodJet->at(0)->phi();
        monojet_eta = m_goodJet->at(0)->eta();
        monojet_rapidity = m_goodJet->at(0)->rapidity();


        // Define Monojet
        if ( monojet_pt > m_monoJetPtCut ){
          if ( fabs(monojet_eta) < m_monoJetEtaCut){
            if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
              pass_monoJet = true;
              //Info("execute()", "  Leading jet pt = %.2f GeV", monojet_pt * 0.001);
            }
          }
        }

        // deltaPhi(monojet,MET) decision
        // For Zmumu
        dPhiMonojetMet_Zmumu = deltaPhi(monojet_phi, MET_Zmumu_phi);

      } // MonoJet selection 



      /////////////////////
      // DiJet Selection //
      /////////////////////
      if (m_goodJet->size() > 1) {

        jet1 = m_goodJet->at(0)->p4();
        jet2 = m_goodJet->at(1)->p4();
        jet1_pt = m_goodJet->at(0)->pt();
        jet2_pt = m_goodJet->at(1)->pt();
        jet1_phi = m_goodJet->at(0)->phi();
        jet2_phi = m_goodJet->at(1)->phi();
        jet1_eta = m_goodJet->at(0)->eta();
        jet2_eta = m_goodJet->at(1)->eta();
        jet1_rapidity = m_goodJet->at(0)->rapidity();
        jet2_rapidity = m_goodJet->at(1)->rapidity();
        auto dijet = jet1 + jet2;
        mjj = dijet.M();

        //Info("execute()", "  jet1 = %.2f GeV, jet2 = %.2f GeV", jet1_pt * 0.001, jet2_pt * 0.001);
        //Info("execute()", "  mjj = %.2f GeV", mjj * 0.001);

        // Define Dijet
        if ( jet1_pt > m_diJet1PtCut && jet2_pt > m_diJet2PtCut ){
          if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
            pass_diJet = true;
          }
        }

        // deltaPhi(Jet1,MET) or deltaPhi(Jet2,MET) decision
        // For Zmumu
        dPhiJet1Met_Zmumu = deltaPhi(jet1_phi, MET_Zmumu_phi);
        dPhiJet2Met_Zmumu = deltaPhi(jet2_phi, MET_Zmumu_phi);

      } // DiJet selection 



      // For jet3
      if (m_goodJet->size() > 2) {
        jet3_pt = m_goodJet->at(2)->pt();
        jet3_phi = m_goodJet->at(2)->phi();
        jet3_eta = m_goodJet->at(2)->eta();
        jet3_rapidity = m_goodJet->at(2)->rapidity();
        // deltaPhi(Jet3,MET)
        dPhiJet3Met_Zmumu = deltaPhi(jet3_phi, MET_Zmumu_phi);
      }

      // Define deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)
      if (m_goodJet->size() > 0) {

        // loop over the jets in the Good Jets Container
        for (const auto& jet : *m_goodJet) {
          float good_jet_pt = jet->pt();
          float good_jet_rapidity = jet->rapidity();
          float good_jet_phi = jet->phi();

          // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
          if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet || m_goodJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
            // For Zmumu
            float dPhijetmet_Zmumu = deltaPhi(good_jet_phi,MET_Zmumu_phi);
            //Info("execute()", " [Zmumu] Event # = %llu", eventInfo->eventNumber());
            //Info("execute()", " [Zmumu] dPhi = %.2f", dPhijetmet_Zmumu);
            if ( good_jet_pt > 30000. && fabs(good_jet_rapidity) < 4.4 && dPhijetmet_Zmumu < 0.4 ) pass_dPhijetmet_Zmumu = false;
            dPhiMinjetmet_Zmumu = std::min(dPhiMinjetmet_Zmumu, dPhijetmet_Zmumu);
            //Info("execute()", " [Zmumu] dPhi_min = %.2f", dPhiMinjetmet_Zmumu);
          }

          // Central Jet Veto (CJV)
          if ( m_goodJet->size() > 2 && pass_diJet ){
            if (m_goodJet->at(0) != jet && m_goodJet->at(1) != jet){
              //cout << "m_goodJet->at(0) = " << m_goodJet->at(0) << " jet = " << jet << endl;
              if (good_jet_pt > m_CJVptCut && fabs(good_jet_rapidity) < m_diJetRapCut) {
                if ( (jet1_rapidity > jet2_rapidity) && (good_jet_rapidity < jet1_rapidity && good_jet_rapidity > jet2_rapidity)){
                  pass_CJV = false;
                }
                if ( (jet1_rapidity < jet2_rapidity) && (good_jet_rapidity > jet1_rapidity && good_jet_rapidity < jet2_rapidity)){
                  pass_CJV = false;
                }
                /* //Valentinos' way (same result as mine)
                   float rapLow  = std::min(jet1_rapidity, jet2_rapidity);
                   float rapHigh = std::max(jet1_rapidity, jet2_rapidity);
                   if (good_jet_rapidity > rapLow && good_jet_rapidity < rapHigh) pass_CJV = false;
                   */
              }
            }
          }

          //Info("execute()", "  Znunu Signal Jet pt = %.2f GeV, eta = %.2f", good_pt_jet * 0.001, good_eta_jet);
          goodJet_ht += good_jet_pt;
        } // Jet loop

      } // End deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)



      /*
      Info("execute()", "====================================");
      Info("execute()", " Event # = %llu", eventInfo->eventNumber());
      int count = 0;
      for (const auto& jet : *m_goodJet) {
        Info("execute()", " jet # : %i", count);
        Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
        Info("execute()", " jet eta = %.3f", jet->eta());
        Info("execute()", " jet phi = %.3f", jet->phi());
        Info("execute()", " jet jvt = %.3f", jet->auxdata<float>("Jvt"));
        Info("execute()", " jet forwardjvt = %i", jet->auxdata<char>("passFJVT"));
        Info("execute()", " pass tight cleaning? %d", (bool)m_jetCleaningTightBad->accept(*jet));
        count++;
      }
      */



      //--------------------------
      // Leading jet pass TightBad
      //--------------------------
      if ( m_goodJet->size() > 0 && !m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ) continue; // go to next systematic
      if ( m_goodJet->size() < 1 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[9]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Leading Jet");




      /*
         Info("execute()", "====================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());
         int count = 0;
         for (const auto& jet : *m_goodJet) {
         Info("execute()", " jet # : %i", count);
         Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
         Info("execute()", " jet eta = %.3f", jet->eta());
         Info("execute()", " jet phi = %.3f", jet->phi());
         Info("execute()", " jet jvt = %.3f", jet->auxdata<float>("Jvt"));
         Info("execute()", " jet forwardjvt = %i", jet->auxdata<char>("passFJVT"));
         count++;
         }
         */




      //-----------------
      // At least 2 jets
      //-----------------
      if ( m_goodJet->size() < 2 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[10]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("At least 2 jets");



      //---------------------------------------------------
      // At least 2 jets with pT1>80GeV, pT2>50GeV, |y|<4.4
      //---------------------------------------------------
      if ( !pass_diJet ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[11]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("VBF jet pt cut");






      //---------
      // Mjj cut
      //---------
      if ( mjj < m_mjjCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[12]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Mjj cut");

      //--------------
      // Jet-MET veto
      //--------------
      if ( !pass_dPhijetmet_Zmumu ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[13]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet-MET veto");



      /*
         Info("execute()", "====================================");
         Info("execute()", " Event # = %llu", eventInfo->eventNumber());
         Info("execute()", " electron size : %lu", m_goodElectron->size());
         Info("execute()", " tau size : %lu", m_goodTau->size());
         int muonCount = 0;
         for (const auto& muon : *m_goodMuonForZ) {
         Info("execute()", " muon # : %i", muonCount);
         Info("execute()", " muon pt = %.3f GeV", muon->pt() * 0.001);
         Info("execute()", " muon eta = %.3f", muon->eta());
         Info("execute()", " muon phi = %.3f", muon->phi());
         Info("execute()", " muon isol = %i", int(m_isolationLooseTrackOnlySelectionTool->accept(*muon)) );
      //Info("execute()", " muon charge = %.3f", muon->charge());
      muonCount++;
      }

      int elecCount = 0;
      for (const auto& elec : *m_goodElectron) {
      Info("execute()", " elec # : %i", elecCount);
      Info("execute()", " elec pt = %.3f GeV", elec->pt() * 0.001);
      Info("execute()", " elec eta = %.3f", elec->eta());
      Info("execute()", " elec phi = %.3f", elec->phi());
      elecCount++;
      }
      int tauCount = 0;
      for (const auto& tau : *m_goodTau) {
      if(!m_tauSelTool->accept(tau)) continue;
      Info("execute()", " tau # : %i", tauCount);
      Info("execute()", " tau pt = %.3f GeV", tau->pt() * 0.001);
      Info("execute()", " tau eta = %.3f", tau->eta());
      Info("execute()", " tau phi = %.3f", tau->phi());
      tauCount++;
      }

*/




      //------------------------
      // Additional lepton Veto 
      //------------------------
      // Additional isolated muon veto
      bool add_iso_muon = false;
      for (const auto& muon : *m_goodMuonForZ) {
        if (m_goodMuonForZ->at(0) == muon || m_goodMuonForZ->at(1) == muon) continue; // Except leading/subleading muon
        if (m_isolationLooseTrackOnlySelectionTool->accept(*muon)) add_iso_muon = true; // For additional isolated muon
      }
      if (add_iso_muon) continue; // go to next systematic
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("additional iso muon veto");
      // Electron veto
      if ( m_goodElectron->size() > 0  ) continue; // go to next systematic
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("electron veto");
      // Tau veto "ONLY" available in EXOT5 derivation
      // because aux item "trackLinks" is missing in the STDM4 derivation samples
      if ( m_dataType.find("EXOT")!=std::string::npos  || m_useArrayCutflow) { // EXOT Derivation OR Cutflow test purpose
        if ( m_goodTau->size() > 0  ) continue; // go to next systematic
      }
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[14]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("tau veto");

      //------------------
      // Central Jet Veto
      //------------------
      if ( !pass_CJV ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[15]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("CJV cut");


      ////////////////////
      // VBF phasespace //
      ////////////////////
      hMap1D["h_cutflow_vbf_met_emulmet"+m_sysName]->Fill(MET_Zmumu * 0.001, m_mcEventWeight);
      hMap1D["h_cutflow_vbf_mjj"+m_sysName]->Fill(mjj * 0.001, m_mcEventWeight);
      hMap1D["h_cutflow_vbf_dPhijj"+m_sysName]->Fill(deltaPhi(jet1_phi, jet2_phi), m_mcEventWeight);


    } // End of Cutflow (Zmumu channel)



    //---------------
    // Zee Channel
    //---------------
    if ( m_sysName=="" && m_useArrayCutflow && m_isZee ) {


      //==============//
      // MET building //
      //==============//

      // do CST or TST
      //std::string softTerm = "SoftClus";
      std::string softTerm = "PVSoftTrk";

      // Zee MET
      float MET_Zee = -9e9;
      float MET_Zee_phi = -9e9;

      //=============================
      // Create MissingETContainers 
      //=============================

      //retrieve the original containers
      const xAOD::MissingETContainer* m_metCore(0);
      std::string coreMetKey = "MET_Core_" + jetType;
      coreMetKey.erase(coreMetKey.length() - 4); //this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metCore, coreMetKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MET core container: " );
        return EL::StatusCode::FAILURE;
      }

      //retrieve the MET association map
      const xAOD::MissingETAssociationMap* m_metMap = 0;
      std::string metAssocKey = "METAssoc_" + jetType;
      metAssocKey.erase(metAssocKey.length() - 4 );//this removes the Jets from the end of the jetType
      if ( !m_event->retrieve( m_metMap, metAssocKey ).isSuccess() ){ // retrieve arguments: container type, container key
        Error("execute()", "Unable to retrieve MissingETAssociationMap: " );
        return EL::StatusCode::FAILURE;
      }


      // It is necessary to reset the selected objects before every MET calculation
      m_met->clear();
      m_metMap->resetObjSelectionFlags();


      //=====================================================================
      // For rebuild the emulated MET for Zee (by marking Electron invisible)
      //=====================================================================

      // It is necessary to reset the selected objects before every MET calculation
      m_met->clear();
      m_metMap->resetObjSelectionFlags();


      // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for electron FSR

      // Electron
      //-----------------
      /// Creat New Hard Object Containers
      // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
      //
      // For emulated MET (No electrons)
      // Make a container for invisible electrons
      ConstDataVector<xAOD::ElectronContainer> m_invisibleElectrons(SG::VIEW_ELEMENTS);
      for (const auto& electron : *m_elecSC) { // C++11 shortcut
        for (const auto& goodelectron : *m_goodElectron) { // C++11 shortcut
          // Check if electrons are matched to good electrons
          if (electron->pt() == goodelectron->pt() && electron->eta() == goodelectron->eta() && electron->phi() == goodelectron->phi()){
            // Put good electrons in
            m_invisibleElectrons.push_back( electron );
          }
        }
      }
      // Mark electrons invisible (No electrons)
      m_metMaker->markInvisible(m_invisibleElectrons.asDataVector(), m_metMap, m_met);

      // JET
      //-----------------
      //Now time to rebuild jetMet and get the soft term
      //This adds the necessary soft term for both CST and TST
      //these functions create an xAODMissingET object with the given names inside the container

      // For emulated MET marking muons invisible
      m_metMaker->rebuildJetMET("RefJet",          //name of jet met
          "SoftClus",           //name of soft cluster term met
          "PVSoftTrk",          //name of soft track term met
          m_met,       //adding to this new met container
          m_allJet,                //using this jet collection to calculate jet met
          m_metCore,   //core met container
          m_metMap,    //with this association map
          true);                //apply jet jvt cut

      /////////////////////////////
      // Soft term uncertainties //
      /////////////////////////////
      if (!m_isData) {
        // Get the track soft term for Zee (For emulated MET marking electrons invisible)
        xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
        if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
          Error("execute()", "METSystematicsTool returns Error CorrectionCode");
        }
      }

      ///////////////
      // MET Build //
      ///////////////
      // For emulated MET for Zee marking electrons invisible
      m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

      ////////////////////////////////////////////////////////////////
      // Fill emulated MET for Zee (by marking electrons invisible) //
      ////////////////////////////////////////////////////////////////
      MET_Zee = ((*m_met)["Final"]->met());
      MET_Zee_phi = ((*m_met)["Final"]->phi());





      //-----------------------
      // Pass Electron Trigger
      //-----------------------
      if (!(m_trigDecisionTool->isPassed("HLT_e24_lhtight_nod0_ivarloose") || m_trigDecisionTool->isPassed("HLT_e24_lhmedium_nod0_L1EM20VH") || m_trigDecisionTool->isPassed("HLT_e26_lhtight_nod0_ivarloose") || m_trigDecisionTool->isPassed("HLT_e60_lhmedium_nod0") || m_trigDecisionTool->isPassed("HLT_e140_lhloose_nod0") ) ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[4]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Electron Trigger");



      //-------------
      // Define Zee
      //-------------

      // Zee
      float mll_electron = 0.;
      bool pass_dielectronPtCut = false; // dielectron pT cut
      bool pass_OSelectron = false; // Opposite sign charge electron
      bool pass_SSelectron = false; // Same sign charge electron
      int numExtra = 0;

      // For Zee Selection
      if (m_goodElectron->size() > 1) {

        TLorentzVector electron1 = m_goodElectron->at(0)->p4();
        TLorentzVector electron2 = m_goodElectron->at(1)->p4();
        float electron1_pt = m_goodElectron->at(0)->pt();
        float electron2_pt = m_goodElectron->at(1)->pt();
        float electron1_charge = m_goodElectron->at(0)->charge();
        float electron2_charge = m_goodElectron->at(1)->charge();
        auto Zll_electron = electron1 + electron2;
        mll_electron = Zll_electron.M();

        //Info("execute()", "  electron1 = %.2f GeV, electron2 = %.2f GeV", electron1_pt * 0.001, electron2_pt * 0.001);
        //Info("execute()", "  mll (Zee) = %.2f GeV", mll_electron * 0.001);

        if ( electron1_pt >  m_LeadLepPtCut ) pass_dielectronPtCut = true;
        if ( electron1_charge * electron2_charge < 0 ) pass_OSelectron = true;
        if ( electron1_charge * electron2_charge > 0 ) pass_SSelectron = true;
        //Info("execute()", "  electron1 charge = %f, electron2 charge = %f, pass_OSelectron = %d, pass_SSelectron = %d", electron1_charge, electron2_charge, pass_OSelectron, pass_SSelectron);

        uint index1 = m_goodElectron->at(0)->index();
        uint index2 = m_goodElectron->at(1)->index();
        for (const auto &electron : *m_goodMuon) {
          if (electron->index() != index1 && electron->index() != index2) numExtra++;
          //std::cout << electron->index() << " " << index1 << " " << index2 << std::endl;
        }

      } // Zee selection loop




      //----------------------
      // At least 2 electrons
      //----------------------
      if (m_goodElectron->size() < 2) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[5]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("At least 2 electrons");



      //-------------------------------------------------------------------
      // Opposite sign electrons with at least one pT > 80GeV && 66 < mll < 116
      //-------------------------------------------------------------------
      if ( !pass_dielectronPtCut ) continue; // go to next systematic
      if ( !pass_OSelectron ) continue; // go to next systematic
      if ( mll_electron < m_mllMin || mll_electron > m_mllMax ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[6]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("dielectron and mll");



      //----------
      // MET cut
      //----------
      if ( MET_Zee < m_metCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[7]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("MET cut");




      // Event Jet cleaning
      bool isBadJet = false;
      //------------------
      // Bad Jet Decision
      //------------------
      // iterate over our deep copy
      for (const auto& jets : *m_goodJet) { // C++11 shortcut
        if ( !m_jetCleaningLooseBad->accept(*jets) ) isBadJet = true;
      }
      if (isBadJet) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[8]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet Cleaning");





      //-------------------------------------
      // Define Monojet and DiJet Properties
      //-------------------------------------

      // Monojet
      float monojet_pt = 0;
      float monojet_phi = 0;
      float monojet_eta = 0;
      float monojet_rapidity = 0;
      float dPhiMonojetMet_Zee = 0;
      // Dijet
      TLorentzVector jet1;
      TLorentzVector jet2;
      float jet1_pt = 0;
      float jet2_pt = 0;
      float jet3_pt = 0;
      float jet1_phi = 0;
      float jet2_phi = 0;
      float jet3_phi = 0;
      float jet1_eta = 0;
      float jet2_eta = 0;
      float jet3_eta = 0;
      float jet1_rapidity = 0;
      float jet2_rapidity = 0;
      float jet3_rapidity = 0;

      float goodJet_ht = 0;
      float dPhiJet1Met_Zee = 0;
      float dPhiJet2Met_Zee = 0;
      float dPhiJet3Met_Zee = 0;

      float mjj = 0;
      bool pass_monoJet = false; // Select monoJet
      bool pass_diJet = false; // Select DiJet
      bool pass_CJV = true; // Central Jet Veto (CJV)
      bool pass_dPhijetmet_Zee = true; // deltaPhi(Jet_i,MET_Zee)
      float dPhiMinjetmet_Zee = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)


      ///////////////////////
      // Monojet Selection //
      ///////////////////////
      if (m_goodJet->size() > 0) {

        monojet_pt = m_goodJet->at(0)->pt();
        monojet_phi = m_goodJet->at(0)->phi();
        monojet_eta = m_goodJet->at(0)->eta();
        monojet_rapidity = m_goodJet->at(0)->rapidity();


        // Define Monojet
        if ( monojet_pt > m_monoJetPtCut ){
          if ( fabs(monojet_eta) < m_monoJetEtaCut){
            if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
              pass_monoJet = true;
              //Info("execute()", "  Leading jet pt = %.2f GeV", monojet_pt * 0.001);
            }
          }
        }

        // deltaPhi(monojet,MET) decision
        // For Zee
        dPhiMonojetMet_Zee = deltaPhi(monojet_phi, MET_Zee_phi);

      } // MonoJet selection 



      /////////////////////
      // DiJet Selection //
      /////////////////////
      if (m_goodJet->size() > 1) {

        jet1 = m_goodJet->at(0)->p4();
        jet2 = m_goodJet->at(1)->p4();
        jet1_pt = m_goodJet->at(0)->pt();
        jet2_pt = m_goodJet->at(1)->pt();
        jet1_phi = m_goodJet->at(0)->phi();
        jet2_phi = m_goodJet->at(1)->phi();
        jet1_eta = m_goodJet->at(0)->eta();
        jet2_eta = m_goodJet->at(1)->eta();
        jet1_rapidity = m_goodJet->at(0)->rapidity();
        jet2_rapidity = m_goodJet->at(1)->rapidity();
        auto dijet = jet1 + jet2;
        mjj = dijet.M();

        //Info("execute()", "  jet1 = %.2f GeV, jet2 = %.2f GeV", jet1_pt * 0.001, jet2_pt * 0.001);
        //Info("execute()", "  mjj = %.2f GeV", mjj * 0.001);

        // Define Dijet
        if ( jet1_pt > m_diJet1PtCut && jet2_pt > m_diJet2PtCut ){
          if ( m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ){ //Tight Leading Jet 
            pass_diJet = true;
          }
        }

        // deltaPhi(Jet1,MET) or deltaPhi(Jet2,MET) decision
        // For Zee
        dPhiJet1Met_Zee = deltaPhi(jet1_phi, MET_Zee_phi);
        dPhiJet2Met_Zee = deltaPhi(jet2_phi, MET_Zee_phi);

      } // DiJet selection 



      // For jet3
      if (m_goodJet->size() > 2) {
        jet3_pt = m_goodJet->at(2)->pt();
        jet3_phi = m_goodJet->at(2)->phi();
        jet3_eta = m_goodJet->at(2)->eta();
        jet3_rapidity = m_goodJet->at(2)->rapidity();
        // deltaPhi(Jet3,MET)
        dPhiJet3Met_Zee = deltaPhi(jet3_phi, MET_Zee_phi);
      }

      // Define deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)
      if (m_goodJet->size() > 0) {

        // loop over the jets in the Good Jets Container
        for (const auto& jet : *m_goodJet) {
          float good_jet_pt = jet->pt();
          float good_jet_rapidity = jet->rapidity();
          float good_jet_phi = jet->phi();

          // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
          if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet || m_goodJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
            // For Zee
            float dPhijetmet_Zee = deltaPhi(good_jet_phi,MET_Zee_phi);
            //Info("execute()", " [Zee] Event # = %llu", eventInfo->eventNumber());
            //Info("execute()", " [Zee] dPhi = %.2f", dPhijetmet_Zee);
            if ( good_jet_pt > 30000. && fabs(good_jet_rapidity) < 4.4 && dPhijetmet_Zee < 0.4 ) pass_dPhijetmet_Zee = false;
            dPhiMinjetmet_Zee = std::min(dPhiMinjetmet_Zee, dPhijetmet_Zee);
            //Info("execute()", " [Zee] dPhi_min = %.2f", dPhiMinjetmet_Zee);
          }

          // Central Jet Veto (CJV)
          if ( m_goodJet->size() > 2 && pass_diJet ){
            if (m_goodJet->at(0) != jet && m_goodJet->at(1) != jet){
              //cout << "m_goodJet->at(0) = " << m_goodJet->at(0) << " jet = " << jet << endl;
              if (good_jet_pt > m_CJVptCut && fabs(good_jet_rapidity) < m_diJetRapCut) {
                if ( (jet1_rapidity > jet2_rapidity) && (good_jet_rapidity < jet1_rapidity && good_jet_rapidity > jet2_rapidity)){
                  pass_CJV = false;
                }
                if ( (jet1_rapidity < jet2_rapidity) && (good_jet_rapidity > jet1_rapidity && good_jet_rapidity < jet2_rapidity)){
                  pass_CJV = false;
                }
                /* //Valentinos' way (same result as mine)
                   float rapLow  = std::min(jet1_rapidity, jet2_rapidity);
                   float rapHigh = std::max(jet1_rapidity, jet2_rapidity);
                   if (good_jet_rapidity > rapLow && good_jet_rapidity < rapHigh) pass_CJV = false;
                   */
              }
            }
          }

          //Info("execute()", "  Znunu Signal Jet pt = %.2f GeV, eta = %.2f", good_pt_jet * 0.001, good_eta_jet);
          goodJet_ht += good_jet_pt;
        } // Jet loop

      } // End deltaPhi(Jet_i,MET) cut and Central Jet Veto (CJV)



      //--------------------------
      // Leading jet pass TightBad
      //--------------------------
      if ( m_goodJet->size() > 0 && !m_jetCleaningTightBad->accept( *m_goodJet->at(0) ) ) continue; // go to next systematic
      if ( m_goodJet->size() < 1 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[9]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Leading Jet");



      //-----------------
      // At least 2 jets
      //-----------------
      if ( m_goodJet->size() < 2 ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[10]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("At least 2 jets");



      //---------------------------------------------------
      // At least 2 jets with pT1>80GeV, pT2>50GeV, |y|<4.4
      //---------------------------------------------------
      if ( !pass_diJet ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[11]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("VBF jet pt cut");






      //---------
      // Mjj cut
      //---------
      if ( mjj < m_mjjCut ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[12]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Mjj cut");

      //--------------
      // Jet-MET veto
      //--------------
      if ( !pass_dPhijetmet_Zee ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[13]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("Jet-MET veto");




      //------------------------
      // Additional lepton Veto 
      //------------------------
      // Exact two electron
      if ( m_goodElectron->size() > 2  ) continue; // go to next systematic
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("exact two electron");
      // Muon veto
      if ( m_goodMuon->size() > 0  ) continue; // go to next systematic
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("muon veto");
      // Tau veto "ONLY" available in EXOT5 derivation
      // because aux item "trackLinks" is missing in the STDM4 derivation samples
      if ( m_dataType.find("EXOT")!=std::string::npos  || m_useArrayCutflow) { // EXOT Derivation OR Cutflow test purpose
        if ( m_goodTau->size() > 0  ) continue; // go to next systematic
      }
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[14]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("tau veto");

      //------------------
      // Central Jet Veto
      //------------------
      if ( !pass_CJV ) continue; // go to next systematic
      if (m_sysName == "" && m_useArrayCutflow) m_eventCutflow[15]+=1;
      if (m_sysName == "" && m_useBitsetCutflow) m_BitsetCutflow->FillCutflow("CJV cut");


      ////////////////////
      // VBF phasespace //
      ////////////////////
      hMap1D["h_cutflow_vbf_met_emulmet"+m_sysName]->Fill(MET_Zee * 0.001, m_mcEventWeight);
      hMap1D["h_cutflow_vbf_mjj"+m_sysName]->Fill(mjj * 0.001, m_mcEventWeight);
      hMap1D["h_cutflow_vbf_dPhijj"+m_sysName]->Fill(deltaPhi(jet1_phi, jet2_phi), m_mcEventWeight);





    } // End of Cutflow (Zee channel)






  } // end for loop over systematics



  /////////////////////
  // clear the store //
  /////////////////////
  // At the end of each iteration/event, clear the store
  m_store->clear();
  // to make sure there are no objects from the current event floating around when the next event is opened.


  ////////////////////////////
  // Delete copy containers //
  ////////////////////////////
  delete m_met;
  delete m_metAux;

  // Below containers can be used for Reco level in the exotic analysis to calculate the unfolding.
  // If truth analysis is not implemented, "m_store->record" is not implemented.
  // That means these containers are not automatically dealt in the code. I should manually delete these container.
  if (!m_doTruth || m_isData) {
    delete m_bornTruthMuon;
    delete m_bornTruthMuonAux;
    delete m_bornTruthElectron;
    delete m_bornTruthElectronAux;
    delete m_bareTruthMuon;
    delete m_bareTruthMuonAux;
    delete m_bareTruthElectron;
    delete m_bareTruthElectronAux;
    delete m_selectedTruthNeutrino;
    delete m_selectedTruthNeutrinoAux;
    delete m_dressedTruthMuon;
    delete m_dressedTruthMuonAux;
    delete m_dressedTruthElectron;
    delete m_dressedTruthElectronAux;
    delete m_selectedTruthTau;
    delete m_selectedTruthTauAux;
    delete m_selectedTruthJet;
    delete m_selectedTruthJetAux;
    if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
      delete m_selectedTruthWZJet;
      delete m_selectedTruthWZJetAux;
    }
    delete m_truthJet;
    delete m_truthJetAux;
  }




  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  ANA_CHECK_SET_TYPE (EL::StatusCode); // set type of return code you are expecting (add to top of each function once)

  // push cutflow for last event
  if (m_useBitsetCutflow) m_BitsetCutflow->PushBitSet();

  // BitsetCutflow
  if(m_useBitsetCutflow && m_BitsetCutflow){
    delete m_BitsetCutflow;
    m_BitsetCutflow = 0;
  }



  if (m_doReco) {

    // GRL
    if (m_grl) {
      delete m_grl;
      m_grl = 0;
    }

    // PileupReweighting Tool
    if (!m_isData) { // For MC
      if(m_prwTool){
        delete m_prwTool;
        m_prwTool = 0;
      }
    }

    // cleaning up trigger tools
    if( m_trigConfigTool ) {
      delete m_trigConfigTool;
      m_trigConfigTool = 0;
    }
    if( m_trigDecisionTool ) {
      delete m_trigDecisionTool;
      m_trigDecisionTool = 0;
    }

    // JES Calibration
    if(m_jetCalibration){
      delete m_jetCalibration;
      m_jetCalibration = 0;
    }

    // JES uncertainty
    if(m_jetUncertaintiesTool){
      delete m_jetUncertaintiesTool;
      m_jetUncertaintiesTool = 0;
    }

    // Jet Cleaning (LooseBad)
    if( m_jetCleaningLooseBad ) {
      delete m_jetCleaningLooseBad;
      m_jetCleaningLooseBad = 0;
    }

    // Jet Cleaning (TightBad)
    if( m_jetCleaningTightBad ) {
      delete m_jetCleaningTightBad;
      m_jetCleaningTightBad = 0;
    }

    // Jet JVT Efficiency Tool
    if( m_jvtefficiencyTool ) {
      delete m_jvtefficiencyTool;
      m_jvtefficiencyTool = 0;
    }

    // Jet fJVT Efficiency Tool
    if( m_fjvtefficiencyTool ) {
      delete m_fjvtefficiencyTool;
      m_fjvtefficiencyTool = 0;
    }

    // b-tag selection Tool
    if(m_BJetSelectTool){
      delete m_BJetSelectTool;
      m_BJetSelectTool = 0;
    }

    // b-tag efficiency Tool
    if(m_BJetEfficiencyTool){
      delete m_BJetEfficiencyTool;
      m_BJetEfficiencyTool = 0;
    }

    // Muon calibration and smearing
    if(m_muonCalibrationAndSmearingTool2016){
      delete m_muonCalibrationAndSmearingTool2016;
      m_muonCalibrationAndSmearingTool2016 = 0;
    }
    if(m_muonCalibrationAndSmearingTool2017){
      delete m_muonCalibrationAndSmearingTool2017;
      m_muonCalibrationAndSmearingTool2017 = 0;
    }

    // Muon selection tool
    if(m_muonMediumSelection){
      delete m_muonMediumSelection;
      m_muonMediumSelection = 0;
    }
    if(m_muonLooseSelection){
      delete m_muonLooseSelection;
      m_muonLooseSelection = 0;
    }

    // Muon trigger efficiency scale factors
    if(m_muonTriggerSFTool){
      delete m_muonTriggerSFTool;
      m_muonTriggerSFTool = 0;
    }

    // Muon reconstruction efficiency scale factors
    if(m_muonEfficiencySFTool){
      delete m_muonEfficiencySFTool;
      m_muonEfficiencySFTool = 0;
    }

    // Muon isolation scale factor
    if(m_muonIsolationSFTool){
      delete m_muonIsolationSFTool;
      m_muonIsolationSFTool = 0;
    }

    // Muon track-to-vertex-association (TTVA) scale factors
    if(m_muonTTVAEfficiencySFTool){
      delete m_muonTTVAEfficiencySFTool;
      m_muonTTVAEfficiencySFTool = 0;
    }

    // Egamma calibration and smearing
    if(m_egammaCalibrationAndSmearingTool){
      delete m_egammaCalibrationAndSmearingTool;
      m_egammaCalibrationAndSmearingTool = 0;
    }

    // Electron identification tool
    if(m_LHToolTight){
      delete m_LHToolTight;
      m_LHToolTight = 0;
    }
    if(m_LHToolLoose){
      delete m_LHToolLoose;
      m_LHToolLoose = 0;
    }

    // EGamma Ambiguity tool
    if(m_egammaAmbiguityTool){
      delete m_egammaAmbiguityTool;
      m_egammaAmbiguityTool = 0;
    }

    // Electron Efficiency Tool
    if(m_elecEfficiencySFTool_reco){
      delete m_elecEfficiencySFTool_reco;
      m_elecEfficiencySFTool_reco = 0;
    }

    if(m_elecEfficiencySFTool_id){
      delete m_elecEfficiencySFTool_id;
      m_elecEfficiencySFTool_id = 0;
    }

    if(m_elecEfficiencySFTool_iso){
      delete m_elecEfficiencySFTool_iso;
      m_elecEfficiencySFTool_iso = 0;
    }

    if(m_elecEfficiencySFTool_trigSF){
      delete m_elecEfficiencySFTool_trigSF;
      m_elecEfficiencySFTool_trigSF = 0;
    }


    // MC fudge tool
    if(m_fudgeMCTool){
      delete m_fudgeMCTool;
      m_fudgeMCTool = 0;
    }

    // Photon identification tool
    if(m_photonTightIsEMSelector){
      delete m_photonTightIsEMSelector;
      m_photonTightIsEMSelector = 0;
    }

    // Tau Smearing Tool
    if(m_tauSmearingTool){
      delete m_tauSmearingTool;
      m_tauSmearingTool = 0;
    }

    // TauOverlappingElectronLLHDecorator
    if(m_tauOverlappingElectronLLHDecorator){
      delete m_tauOverlappingElectronLLHDecorator;
      m_tauOverlappingElectronLLHDecorator = 0;
    }

    // Tau Selection Tool
    if(m_tauSelTool){
      delete m_tauSelTool;
      m_tauSelTool = 0;
    }

    // IsolationSelectionTool
    if(m_isolationFixedCutTightSelectionTool){
      delete m_isolationFixedCutTightSelectionTool;
      m_isolationFixedCutTightSelectionTool = 0;
    }
    if(m_isolationLooseTrackOnlySelectionTool){
      delete m_isolationLooseTrackOnlySelectionTool;
      m_isolationLooseTrackOnlySelectionTool = 0;
    }

    // Overlap Removal Tool
    if(m_toolBox){
      delete m_toolBox;
      m_toolBox = 0;
    }




    // print out the number of Overlap removal
    Info("finalize()", "=======================================================");
    Info("finalize()", "Object count summaries: nOverlapObjects / nInputObjects");
    Info("finalize()", "Number overlap electrons:    %i / %i", nOverlapElectrons, nInputElectrons);
    Info("finalize()", "Number overlap muons:    %i / %i", nOverlapMuons, nInputMuons);
    Info("finalize()", "Number overlap jets:    %i / %i", nOverlapJets, nInputJets);
    Info("finalize()", "Number overlap taus:    %i / %i", nOverlapTaus, nInputTaus);
    Info("finalize()", "Number overlap photons:    %i / %i", nOverlapPhotons, nInputPhotons);
    Info("finalize()", "=======================================================");


    // print out the final number of clean events
    Info("finalize()", "Number of clean events = %i", m_numCleanEvents);


    // print out the final number of clean events
    Info("finalize()", "Number of clean events = %i", m_numCleanEvents);

    // print out Cutflow
    if (m_useArrayCutflow) {
      Info("finalize()", "================================================");
      Info("finalize()", "===============  Base Cutflow  =================");
      for(int i=0; i<16; ++i) {
        int j = i+1;
        Info("finalize()", "Event cutflow (%i) = %i", j, m_eventCutflow[i]);
      }
    }

  } // m_doReco

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode smZInvAnalysis :: addHist(std::map<std::string, TH1*> &hMap, std::string tag,
    int bins, double min, double max) {

  std::string label = tag;
  TH1* h_temp = new TH1F(label.c_str(), "", bins, min, max);
  wk()->addOutput (h_temp);
  hMap[label] = h_temp;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode smZInvAnalysis :: addHist(std::map<std::string, TH1*> &hMap, std::string tag,
    int bins, Float_t binArray[]) {

  std::string label = tag;
  TH1* h_temp = new TH1F(label.c_str(), "", bins, binArray);
  wk()->addOutput (h_temp);
  hMap[label] = h_temp;

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode smZInvAnalysis :: addHist(std::map<std::string, TH2*> &hMap, std::string tag,
    int binsX, double minX, double maxX, int binsY, double minY, double maxY) {

  std::string label = tag;
  TH2* h_temp = new TH2F(label.c_str(), "", binsX, minX, maxX, binsY, minY, maxY);
  wk()->addOutput (h_temp);
  hMap[label] = h_temp;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode smZInvAnalysis :: addHist(std::map<std::string, TH2*> &hMap, std::string tag,
    int binsX, Float_t binArrayX[], int binsY, Float_t binArrayY[]) {

  std::string label = tag;
  TH2* h_temp = new TH2F(label.c_str(), "", binsX, binArrayX, binsY, binArrayY);
  wk()->addOutput (h_temp);
  hMap[label] = h_temp;

  return EL::StatusCode::SUCCESS;
}





float smZInvAnalysis :: deltaPhi(float phi1, float phi2) {

  float dPhi = std::fabs(phi1 - phi2);

  if(dPhi > TMath::Pi())
    dPhi = TMath::TwoPi() - dPhi;

  return dPhi;

}



float smZInvAnalysis :: deltaR(float eta1, float eta2, float phi1, float phi2) {

  float dEta = eta1 - eta2;
  float dPhi = deltaPhi(phi1,phi2);

  return TMath::Sqrt(dEta*dEta + dPhi*dPhi);

}


bool smZInvAnalysis::passMonojet(const xAOD::JetContainer* goodJet, const float& metPhi){

   if (goodJet->size() < 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = goodJet->at(0)->pt();
  float monojet_eta = goodJet->at(0)->eta();

  // Define Monojet
  if ( goodJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *goodJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < m_monoJetPtCut || fabs(monojet_eta) > m_monoJetEtaCut ) return false;
  
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *goodJet) {
    float good_jet_pt = jet->pt();
    float good_jet_phi = jet->phi();

    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    if (goodJet->at(0) == jet || goodJet->at(1) == jet || goodJet->at(2) == jet || goodJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
      float dPhijetmet = deltaPhi(good_jet_phi,metPhi);
      if ( good_jet_pt > 30000. && dPhijetmet < 0.4 ) pass_dPhijetmet = false;
    }

  }

  if ( !pass_dPhijetmet ) return false;

  // Pass Monojet phasespace
  return true;

}


bool smZInvAnalysis::passVBF(const xAOD::JetContainer* goodJet, const float& metPhi){

  if (!passDijet(goodJet, metPhi)) return false;

  // Define Central Jet Veto (CJV)
  float jet1_rapidity = goodJet->at(0)->rapidity();
  float jet2_rapidity = goodJet->at(1)->rapidity();
  bool pass_CJV = true;

  for (const auto& jet : *goodJet) {
    float good_jet_pt = jet->pt();
    float good_jet_rapidity = jet->rapidity();

    // Central Jet Veto (CJV)
    if (goodJet->at(0) != jet && goodJet->at(1) != jet){
      //cout << "goodJet->at(0) = " << goodJet->at(0) << " jet = " << jet << endl;
      if (good_jet_pt > m_CJVptCut && fabs(good_jet_rapidity) < m_diJetRapCut) {
        if ( (jet1_rapidity > jet2_rapidity) && (good_jet_rapidity < jet1_rapidity && good_jet_rapidity > jet2_rapidity)){
          pass_CJV = false;
        }
        if ( (jet1_rapidity < jet2_rapidity) && (good_jet_rapidity > jet1_rapidity && good_jet_rapidity < jet2_rapidity)){
          pass_CJV = false;
        }
        /* //Valentinos' way (same result as mine)
           float rapLow  = std::min(jet1_rapidity, jet2_rapidity);
           float rapHigh = std::max(jet1_rapidity, jet2_rapidity);
           if (good_jet_rapidity > rapLow && good_jet_rapidity < rapHigh) pass_CJV = false;
           */
      }
    }

  } // Jet loop

  // Pass VBF phasespace
  return pass_CJV;

}



bool smZInvAnalysis::passDijet(const xAOD::JetContainer* goodJet, const float& metPhi){

  if (goodJet->size() < 2) return false;

  //-------------------------
  // Define DiJet Properties
  //-------------------------

  // DiJet Selection
  TLorentzVector jet1 = goodJet->at(0)->p4();
  TLorentzVector jet2 = goodJet->at(1)->p4();
  float jet1_pt = goodJet->at(0)->pt();
  float jet2_pt = goodJet->at(1)->pt();
  auto dijet = jet1 + jet2;
  float mjj = dijet.M();

  // Define Dijet
  if ( goodJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *goodJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( jet1_pt < m_diJet1PtCut || jet2_pt < m_diJet2PtCut ) return false;

  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *goodJet) {
    float good_jet_pt = jet->pt();
    float good_jet_phi = jet->phi();

    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    if (goodJet->at(0) == jet || goodJet->at(1) == jet || goodJet->at(2) == jet || goodJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
      float dPhijetmet = deltaPhi(good_jet_phi,metPhi);
      if ( good_jet_pt > 30000. && dPhijetmet < 0.4 ) pass_dPhijetmet = false;
    }

  }
  if ( !pass_dPhijetmet ) return false;

  // Mjj cut
  if ( mjj < m_mjjCut ) return false;

  // Pass VBF phasespace
  return true;

}


void smZInvAnalysis::plotMonojet(const xAOD::JetContainer* goodJet, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName) {
  if (!passMonojet(goodJet, metPhi)) return;

  hMap1D[hist_prefix+"MET_mono"+sysName]->Fill(met*0.001, mcEventWeight);


  /*
  if (hist_prefix.find("nominal_truth")!=std::string::npos){
    Info("execute()", "=====================================");
    Info("execute()", "Truth MET = %.3f GeV", met * 0.001);
    Info("execute()", "Truth MET Phi = %.3f", metPhi);
    int count = 0;
    for (const auto& jet : *goodJet) {
      Info("execute()", " jet # : %i", count);
      Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
      Info("execute()", " jet eta = %.3f", jet->eta());
      Info("execute()", " jet phi = %.3f", jet->phi());
      count++;
    }
  }
  */

  /*
  if (hist_prefix.find("TruthJetsTruthMETTruthCuts")!=std::string::npos){
    Info("execute()", "=====================================");
    Info("execute()", "Truth MET = %.3f GeV", met * 0.001);
    Info("execute()", "Truth MET Phi = %.3f", metPhi);
    int count = 0;
    for (const auto& jet : *goodJet) {
      Info("execute()", " jet # : %i", count);
      Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
      Info("execute()", " jet eta = %.3f", jet->eta());
      Info("execute()", " jet phi = %.3f", jet->phi());
      count++;
    }
  }
  */




}


void smZInvAnalysis::plotVBF(const xAOD::JetContainer* goodJet, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName) {
  if (!passVBF(goodJet, metPhi)) return;

  // Define diJet properties
  TLorentzVector jet1 = goodJet->at(0)->p4();
  TLorentzVector jet2 = goodJet->at(1)->p4();
  float jet1_phi = goodJet->at(0)->phi();
  float jet2_phi = goodJet->at(1)->phi();
  auto dijet = jet1 + jet2;
  float mjj = dijet.M();
  float dphijj = deltaPhi(jet1_phi, jet2_phi);

  hMap1D[hist_prefix+"MET_search"+sysName]->Fill(met*0.001, mcEventWeight);
  hMap1D[hist_prefix+"Mjj_search"+sysName]->Fill(mjj*0.001, mcEventWeight);
  hMap1D[hist_prefix+"DeltaPhiAll"+sysName]->Fill(dphijj, mcEventWeight);

}



void smZInvAnalysis::doZnunuExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& mcEventWeight, std::string sysName){

  h_channel = "h_znunu_";

  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;



  //===========================
  // For rebuild the real MET
  //===========================


  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

  // iterate over our shallow copy
  for (const auto& electron : *m_goodElectron) { // C++11 shortcut
    // For MET rebuilding
    m_MetElectrons.push_back( electron );
  } // end for loop over shallow copied electrons
  //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

  // For real MET
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                                      //filling this met container
      m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
      metMap);                                  //and this association map


  /*
  // Photon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Photon container m_photons, placing selected photons into m_MetPhotons
  ConstDataVector<xAOD::PhotonContainer> m_MetPhotons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Photon>

  // iterate over our shallow copy
  for (const auto& photon : *m_goodPhoton) { // C++11 shortcut
  // For MET rebuilding
  m_MetPhotons.push_back( photon );
  } // end for loop over shallow copied photons

  // For real MET
  m_metMaker->rebuildMET("RefPhoton",           //name of metPhotons in metContainer
  xAOD::Type::Photon,                       //telling the rebuilder that this is photon met
  m_met,                                    //filling this met container
  m_MetPhotons.asDataVector(),              //using these metPhotons that accepted our cuts
  metMap);                                //and this association map
  */


  // Only implement at EXOT5 derivation
  // METRebuilder will use "trackLinks" aux data in Tau container
  // However STDM4 derivation does not contain a aux data "trackLinks" in Tau container
  // So one cannot build the real MET using Tau objects
  if ( m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation

    // TAUS
    //-----------------
    /// Creat New Hard Object Containers
    // [For MET building] filter the TauJet container m_taus, placing selected taus into m_MetTaus
    ConstDataVector<xAOD::TauJetContainer> m_MetTaus(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::TauJet>

    // iterate over our shallow copy
    for (const auto& taujet : *m_goodTau) { // C++11 shortcut
      // For MET rebuilding
      m_MetTaus.push_back( taujet );
    } // end for loop over shallow copied taus

    // For real MET
    m_metMaker->rebuildMET("RefTau",           //name of metTaus in metContainer
        xAOD::Type::Tau,                       //telling the rebuilder that this is tau met
        m_met,                                 //filling this met container
        m_MetTaus.asDataVector(),              //using these metTaus that accepted our cuts
        metMap);                             //and this association map

  } // Only using EXOT5 derivation


  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    // For MET rebuilding
    m_MetMuons.push_back( muon );
  } // end for loop over shallow copied muons
  // For real MET
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                                  //filling this met container
      m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
      metMap);                              //and this association map



  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For real MET
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term (For real MET)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For real MET
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  /////////////////////////////
  // Fill real MET for Znunu //
  /////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());


  //------------------
  // Pass MET Trigger
  //------------------
  if (!m_met_trig_fire) return;


  //----------
  // MET cut
  //----------
  if ( MET < m_metCut ) return;



  //--------------
  // Lepton Veto 
  //--------------
  // Muon veto
  if ( m_goodMuon->size() > 0  ) return;
  // Electron veto
  if ( m_goodElectron->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because tau selection tool is unavailable without "trackLinks" aux data in Tau container)
  if ( m_dataType.find("EXOT")!=std::string::npos ) { // SM Derivation (EXOT)
    if ( m_goodTau->size() > 0  ) return;
  }


  ////////////////////////////
  // Plot Reco Znunu Signal //
  ////////////////////////////
  plotMonojet(m_goodJet, MET, MET_phi, mcEventWeight, h_channel, sysName);
  plotVBF(m_goodJet, MET, MET_phi, mcEventWeight, h_channel, sysName);

}




void smZInvAnalysis::doZmumuExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::MuonContainer* muons, const xAOD::MuonContainer* muonSC, const float& mcEventWeight, std::string sysName){

  h_channel = "h_zmumu_";

  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;

  //===================================================================
  // For rebuild the emulated MET for Zmumu (by marking Muon invisible)
  //===================================================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for muon FSR

  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  //
  // For emulated MET (No muons)
  // Make a empty container for invisible muons
  ConstDataVector<xAOD::MuonContainer> m_EmptyMuons(SG::VIEW_ELEMENTS);
  m_EmptyMuons.clear();
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                         //filling this met container
      m_EmptyMuons.asDataVector(),            //using these metMuons that accepted our cuts
      metMap);                     //and this association map

  // Make a container for invisible muons
  ConstDataVector<xAOD::MuonContainer> m_invisibleMuons(SG::VIEW_ELEMENTS);
  for (const auto& muon : *muonSC) { // C++11 shortcut
    for (const auto& goodmuon : *m_goodMuonForZ) { // C++11 shortcut
      // Check if muons are matched to good muons
      if (muon->pt() == goodmuon->pt() && muon->eta() == goodmuon->eta() && muon->phi() == goodmuon->phi()){
        // Put good muons in
        m_invisibleMuons.push_back( muon );
      }
    }
  }

  // Mark muons invisible
  m_metMaker->markInvisible(m_invisibleMuons.asDataVector(), metMap, m_met);


  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Zmumu (For emulated MET marking muons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Zmumu marking muons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  //////////////////////////////////////////////////////////////
  // Fill emulated MET for Zmumu (by marking muons invisible) //
  //////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());



  /////////////////////////////////
  // Calculate muon SF for Zmumu //
  /////////////////////////////////
  float mcEventWeight_Zmumu = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Zmumu original mcEventWeight = %.3f ", mcEventWeight);
    mcEventWeight_Zmumu = mcEventWeight_Zmumu * GetTotalMuonSF(*m_goodMuonForZ, m_recoSF, m_isoMuonSFforZ, m_ttvaSF, m_muonTrigSFforExotic);
    //Info("execute()", " Zmumu mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Zmumu);
  }


 // Test
   if (passMonojet(m_selectedTruthJet, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_monojet_truth_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_monojet_truth_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }
  if (passVBF(m_selectedTruthJet, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_VBF_truth_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_VBF_truth_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }

  if (passMonojet(m_goodJetORTruthMuMu, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_monojet_ORtruth_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_monojet_ORtruth_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }
  if (passVBF(m_goodJetORTruthMuMu, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_VBF_ORtruth_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_VBF_ORtruth_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }

  if (passMonojet(m_goodJet, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_monojet_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_monojet_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }
  if (passVBF(m_goodJet, m_truthPseudoMETPhiZmumu)){
    for (const auto& jet : *m_goodJetORTruthMuMu) {
      hMap1D["h_VBF_goodJetORTruthMuMu_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
    }
    hMap1D["h_VBF_truthPseudoMETZmumu"+sysName]->Fill(m_truthPseudoMETZmumu * 0.001, mcEventWeight_Zmumu);
  }










  /////////////////////////////////////////////////////////
  // Plot Particle (truth) level dilepton channel for Cz //
  /////////////////////////////////////////////////////////
  if (!m_isData) {
    if (m_passTruthNoMetZmumu &&  m_truthPseudoMETZmumu > m_metCut ) { // pass particle (truth) level dilepton && particle level MET cut
      plotMonojet(m_goodJetORTruthMuMu, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"RecoJetsTruthMETTruthCuts_", sysName);
      plotVBF(m_goodJetORTruthMuMu, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"RecoJetsTruthMETTruthCuts_", sysName);
      //Test
      //Use goodJet
      plotMonojet(m_goodJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"GoodJetsTruthMETTruthCuts_", sysName);
      plotVBF(m_goodJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"GoodJetsTruthMETTruthCuts_", sysName);
      //Use truthJet
      plotMonojet(m_selectedTruthJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"TruthJetsTruthMETTruthCuts_", sysName);
      plotVBF(m_selectedTruthJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"TruthJetsTruthMETTruthCuts_", sysName);

      /*
      Info("execute()", "=====================================");
      Info("execute()", "Truth MET = %.3f GeV", m_truthPseudoMETZmumu * 0.001);
      Info("execute()", "Truth MET Phi = %.3f", m_truthPseudoMETPhiZmumu);
      int count = 0;
      for (const auto& jet : *m_selectedTruthJet) {
        Info("execute()", " jet # : %i", count);
        Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
        Info("execute()", " jet eta = %.3f", jet->eta());
        Info("execute()", " jet phi = %.3f", jet->phi());
        count++;
      }
      */

    }
  } // MC




  //-----------------
  // At least 2 muons
  //-----------------
  if (m_goodMuonForZ->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  bool pass_OS = false; // Opposite sign charge lepton
  bool pass_SS = false; // Same sign charge lepton
  int numExtra = 0;

  TLorentzVector lepton1 = m_goodMuonForZ->at(0)->p4();
  TLorentzVector lepton2 = m_goodMuonForZ->at(1)->p4();
  float lepton1_pt = lepton1.Pt();
  float lepton2_pt = lepton2.Pt();
  float lepton1_charge = m_goodMuonForZ->at(0)->charge();
  float lepton2_charge = m_goodMuonForZ->at(1)->charge();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();

  if ( lepton1_charge * lepton2_charge < 0 ) pass_OS = true;
  if ( lepton1_charge * lepton2_charge > 0 ) pass_SS = true;

  uint index1 = m_goodMuonForZ->at(0)->index();
  uint index2 = m_goodMuonForZ->at(1)->index();
  for (const auto &muon : *m_goodMuon) {
    if (muon->index() != index1 && muon->index() != index2) numExtra++;
    //std::cout << muon->index() << " " << index1 << " " << index2 << std::endl;
  }

  //----------------------
  // Additional Muon Veto
  //----------------------
  if (numExtra > 0) return;


  //---------------
  // Dilepton cut
  //---------------
  if ( lepton1_pt <  m_LeadLepPtCut || lepton2_pt < m_SubLeadLepPtCut ) return;


  //----------------------------------------------------------------------
  // Opposite sign leptons with at least one pT > 80GeV && 66 < mll < 116
  //----------------------------------------------------------------------
  if ( !pass_OS ) return;
  if ( mll < m_mllMin || mll > m_mllMax ) return;



  ///////////////////////////////////////////////////////
  // Plot Reconstruction level dilepton channel for Cz //
  ///////////////////////////////////////////////////////
  if (!m_isData) {
    if ( m_truthPseudoMETZmumu > m_metCut ) { // pass reconstruction level dilepton && particle (truth) level MET cut
      plotMonojet(m_goodJetORTruthMuMu, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"RecoJetsTruthMETRecoCuts_", sysName);
      plotVBF(m_goodJetORTruthMuMu, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"RecoJetsTruthMETRecoCuts_", sysName);
      //Test
      //Use goodJet
      plotMonojet(m_goodJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"GoodJetsTruthMETRecoCuts_", sysName);
      plotVBF(m_goodJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"GoodJetsTruthMETRecoCuts_", sysName);
      //Use truthJet
      plotMonojet(m_selectedTruthJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"TruthJetsTruthMETRecoCuts_", sysName);
      plotVBF(m_selectedTruthJet, m_truthPseudoMETZmumu, m_truthPseudoMETPhiZmumu, mcEventWeight_Zmumu, h_channel+"TruthJetsTruthMETRecoCuts_", sysName);
    }
  } // MC




  //------------------
  // Pass MET Trigger
  //------------------
  if (!m_met_trig_fire) return;


  //----------
  // MET cut
  //----------
  if ( MET < m_metCut ) return;



  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Additional isolated muon veto
  bool add_iso_muon = false;
  for (const auto& muon : *m_goodMuonForZ) {
    if (m_goodMuonForZ->at(0) == muon || m_goodMuonForZ->at(1) == muon) continue;
    if (m_isolationLooseTrackOnlySelectionTool->accept(*muon)) add_iso_muon = true; // For additional isolated muon
  }
  if (add_iso_muon) return;
  // Electron veto
  if ( m_goodElectron->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because tau selection tool is unavailable without "trackLinks" aux data in Tau container)
  if ( m_dataType.find("EXOT")!=std::string::npos ) { // SM Derivation (EXOT)
    if ( m_goodTau->size() > 0  ) return;
  }

  //-----------------
  // Exact 2 muons
  //-----------------
  if (m_goodMuonForZ->size() > 2) return;



  ////////////////////////////
  // Plot Reco Zmumu Signal //
  ////////////////////////////
  plotMonojet(m_goodJet, MET, MET_phi, mcEventWeight_Zmumu, h_channel, sysName);
  plotVBF(m_goodJet, MET, MET_phi, mcEventWeight_Zmumu, h_channel, sysName);

}




void smZInvAnalysis::doZeeExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::ElectronContainer* elecSC, const float& mcEventWeight, std::string sysName){

  h_channel = "h_zee_";

  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;

  //=====================================================================
  // For rebuild the emulated MET for Zee (by marking Electron invisible)
  //=====================================================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for electron FSR

  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  //
  // For emulated MET (No electrons)
  // Make a container for invisible electrons
  ConstDataVector<xAOD::ElectronContainer> m_invisibleElectrons(SG::VIEW_ELEMENTS);
  for (const auto& electron : *elecSC) { // C++11 shortcut
    for (const auto& goodelectron : *m_goodElectron) { // C++11 shortcut
      // Check if electrons are matched to good electrons
      if (electron->pt() == goodelectron->pt() && electron->eta() == goodelectron->eta() && electron->phi() == goodelectron->phi()){
        // Put good electrons in
        m_invisibleElectrons.push_back( electron );
      }
    }
  }
  // Mark electrons invisible (No electrons)
  m_metMaker->markInvisible(m_invisibleElectrons.asDataVector(), metMap, m_met);


  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Zee (For emulated MET marking electrons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Zee marking electrons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  ////////////////////////////////////////////////////////////////
  // Fill emulated MET for Zee (by marking electrons invisible) //
  ////////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());




  //-----------------
  // At least 2 electrons
  //-----------------
  if (m_goodElectron->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  bool pass_OS = false; // Opposite sign charge lepton
  bool pass_SS = false; // Same sign charge lepton

  TLorentzVector lepton1 = m_goodElectron->at(0)->p4();
  TLorentzVector lepton2 = m_goodElectron->at(1)->p4();
  float lepton1_pt = lepton1.Pt();
  float lepton2_pt = lepton2.Pt();
  float lepton1_charge = m_goodElectron->at(0)->charge();
  float lepton2_charge = m_goodElectron->at(1)->charge();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();

  if ( lepton1_charge * lepton2_charge < 0 ) pass_OS = true;
  if ( lepton1_charge * lepton2_charge > 0 ) pass_SS = true;

  uint index1 = m_goodElectron->at(0)->index();
  uint index2 = m_goodElectron->at(1)->index();


  //---------------
  // Dilepton cut
  //---------------
  if ( lepton1_pt <  m_LeadLepPtCut || lepton2_pt < m_SubLeadLepPtCut ) return;


  //----------------------------------------------------------------------
  // Opposite sign leptons with at least one pT > 80GeV && 66 < mll < 116
  //----------------------------------------------------------------------
  if ( !pass_OS ) return;
  if ( mll < m_mllMin || mll > m_mllMax ) return;



  //------------------------------
  // Pass Single Electron Triggers
  //------------------------------
  if (!m_ele_trig_fire) return;


  //----------
  // MET cut
  //----------
  if ( MET < m_metCut ) return;


  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Additional isolated electron veto
  bool add_iso_electron = false;
  for (const auto& electron : *m_goodElectron) {
    if (m_goodElectron->at(0) == electron || m_goodElectron->at(1) == electron) continue;
    if (m_isolationLooseTrackOnlySelectionTool->accept(*electron)) add_iso_electron = true; // For additional isolated electron
  }
  if (add_iso_electron) return;
  // Muon veto
  if ( m_goodMuonForZ->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because tau selection tool is unavailable without "trackLinks" aux data in Tau container)
  if ( m_dataType.find("EXOT")!=std::string::npos ) { // SM Derivation (EXOT)
    if ( m_goodTau->size() > 0  ) return;
  }

  //-----------------
  // Exact 2 electrons
  //-----------------
  if (m_goodElectron->size() > 2) return;



  ///////////////////////////////////
  // Calculate electron SF for Zee //
  ///////////////////////////////////
  float mcEventWeight_Zee = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Zee original mcEventWeight = %.3f ", mcEventWeight);
    mcEventWeight_Zee = mcEventWeight_Zee * GetTotalElectronSF(*m_goodElectron, m_recoSF, m_idSF, m_isoElectronSF, m_elecTrigSF);
    //Info("execute()", " Zee mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Zee);
  }




  //////////////////////////
  // Plot Reco Zee Signal //
  //////////////////////////
  plotMonojet(m_goodJet, MET, MET_phi, mcEventWeight_Zee, h_channel, sysName);
  plotVBF(m_goodJet, MET, MET_phi, mcEventWeight_Zee, h_channel, sysName);

}





void smZInvAnalysis::doZmumuTruth(const xAOD::TruthParticleContainer* truthMuon, const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  h_channel = "h_zmumu_";

  //------------------------------
  // Define Truth Zmumu Selection
  //------------------------------

  //-----------------
  // At least 2 muons
  //-----------------
  if (truthMuon->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  TLorentzVector lepton1 = truthMuon->at(0)->p4();
  TLorentzVector lepton2 = truthMuon->at(1)->p4();
  float lepton1_pt = lepton1.Perp();
  float lepton2_pt = lepton2.Perp();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();
  float MET = Zll.Pt();
  float METPhi = Zll.Phi();


  // MET for Cz calculation
  if ( hist_prefix == "nominal_truth_" ) {
    m_truthPseudoMETZmumu = Zll.Pt();
    m_truthPseudoMETPhiZmumu = Zll.Phi();
  }


  //---------------
  // Dilepton cut
  //---------------
  if ( lepton1_pt <  m_LeadLepPtCut || lepton2_pt < m_SubLeadLepPtCut ) return;
  if ( TMath::Abs(lepton1.Eta()) > m_lepEtaCut || TMath::Abs(lepton2.Eta()) > m_lepEtaCut ) return;
  
  //----------------------------------------------------------------------
  // Opposite sign leptons with at least one pT > 80GeV && 66 < mll < 116
  //----------------------------------------------------------------------
  if ( truthMuon->at(0)->pdgId() * truthMuon->at(1)->pdgId() > 0 ) return;
  if ( mll < m_mllMin || mll > m_mllMax ) return;


  // Pass truth Zmumu cut, but no MET cut for Cz calculation
  if ( hist_prefix == "nominal_truth_" ) {
  m_passTruthNoMetZmumu = true;
  }

  //----------
  // MET cut
  //----------
  if ( MET < m_metCut ) return;


  /*
  Info("execute()", "=====================================");
  Info("execute()", "Truth MET = %.3f GeV", MET * 0.001);
  Info("execute()", "Truth MET Phi = %.3f", METPhi);
  int count = 0;
  for (const auto& jet : *m_selectedTruthJet) {
    Info("execute()", " jet # : %i", count);
    Info("execute()", " jet pt = %.3f GeV", jet->pt() * 0.001);
    Info("execute()", " jet eta = %.3f", jet->eta());
    Info("execute()", " jet phi = %.3f", jet->phi());
    count++;
  }
  */


  //////////
  // Plot //
  //////////
  if ( hist_prefix == "nominal_truth_" ) {
    plotMonojet(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    plotVBF(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    if (passMonojet(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_mono"]->Fill(mll * 0.001, mcEventWeight);
    if (passVBF(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_search"]->Fill(mll * 0.001, mcEventWeight);
  }
  if ( hist_prefix == "born_truth_" ) {
    plotMonojet(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    plotVBF(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    if (passMonojet(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_mono"]->Fill(mll * 0.001, mcEventWeight);
    if (passVBF(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_search"]->Fill(mll * 0.001, mcEventWeight);
  }
  if ( hist_prefix == "bare_truth_" ) {
    plotMonojet(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    plotVBF(m_selectedTruthJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
    if (passMonojet(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_mono"]->Fill(mll * 0.001, mcEventWeight);
    if (passVBF(m_selectedTruthJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_search"]->Fill(mll * 0.001, mcEventWeight);
  }
  if ( m_dataType.find("STDM")!=std::string::npos ) { // SM Derivation (STDM)
    if ( hist_prefix == "dress_truth_" ) {
      plotMonojet(m_selectedTruthWZJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
      plotVBF(m_selectedTruthWZJet, MET, METPhi, mcEventWeight, h_channel+hist_prefix, sysName);
      if (passMonojet(m_selectedTruthWZJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_mono"]->Fill(mll * 0.001, mcEventWeight);
      if (passVBF(m_selectedTruthWZJet, METPhi)) hMap1D[h_channel+hist_prefix+"Mll_search"]->Fill(mll * 0.001, mcEventWeight);
    }
  }


}




void smZInvAnalysis::doZnunuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  std::string channel = "znunu";


  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;


  //===========================
  // For rebuild the real MET
  //===========================


  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

  // iterate over our shallow copy
  for (const auto& electron : *m_goodElectron) { // C++11 shortcut
    // For MET rebuilding
    m_MetElectrons.push_back( electron );
  } // end for loop over shallow copied electrons
  //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

  // For real MET
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                                      //filling this met container
      m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
      metMap);                                  //and this association map


  if (sm_doPhoton_MET) {
    // Photon
    //-----------------
    /// Creat New Hard Object Containers
    // [For MET building] filter the Photon container m_photons, placing selected photons into m_MetPhotons
    ConstDataVector<xAOD::PhotonContainer> m_MetPhotons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Photon>

    // iterate over our shallow copy
    for (const auto& photon : *m_goodPhoton) { // C++11 shortcut
      // For MET rebuilding
      m_MetPhotons.push_back( photon );
    } // end for loop over shallow copied photons

    // For real MET
    m_metMaker->rebuildMET("RefPhoton",           //name of metPhotons in metContainer
        xAOD::Type::Photon,                       //telling the rebuilder that this is photon met
        m_met,                                    //filling this met container
        m_MetPhotons.asDataVector(),              //using these metPhotons that accepted our cuts
        metMap);                                //and this association map
  }



  // Only implement at EXOT5 derivation
  // METRebuilder will use "trackLinks" aux data in Tau container
  // However STDM4 derivation does not contain a aux data "trackLinks" in Tau container
  // So one cannot build the real MET using Tau objects
  if ( sm_doTau_MET && m_dataType.find("EXOT")!=std::string::npos ) { // EXOT Derivation

    // TAUS
    //-----------------
    /// Creat New Hard Object Containers
    // [For MET building] filter the TauJet container m_taus, placing selected taus into m_MetTaus
    ConstDataVector<xAOD::TauJetContainer> m_MetTaus(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::TauJet>

    // iterate over our shallow copy
    for (const auto& taujet : *m_goodTau) { // C++11 shortcut
      // For MET rebuilding
      m_MetTaus.push_back( taujet );
    } // end for loop over shallow copied taus

    // For real MET
    m_metMaker->rebuildMET("RefTau",           //name of metTaus in metContainer
        xAOD::Type::Tau,                       //telling the rebuilder that this is tau met
        m_met,                                 //filling this met container
        m_MetTaus.asDataVector(),              //using these metTaus that accepted our cuts
        metMap);                             //and this association map

  } // Only using EXOT5 derivation


  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    // For MET rebuilding
    m_MetMuons.push_back( muon );
  } // end for loop over shallow copied muons
  // For real MET
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                                  //filling this met container
      m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
      metMap);                              //and this association map



  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For real MET
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term (For real MET)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For real MET for Znunu
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  /////////////////////////////
  // Fill real MET for Znunu //
  /////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());


  //-------------
  // Lepton veto
  //-------------

  // Muon veto
  if (m_goodMuon->size() > 0 ) return;
  // Electron veto
  if ( m_goodElectron->size() > 0 ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because STDM4 derivation does not contain a aux data "trackLinks" in Tau container, I could not use tau selection tool
  //if ( m_goodTau->size() > 0 ) return;





  /////////////////////////////
  // Trigger Efficiency plot //
  /////////////////////////////
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      // Pass exclusive jet cut
      if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
        // Efficiency plot
        // For 2015 Data
        if (m_dataYear == "2015") { // for HLT_xe70_mht
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // For 2016 Data Period A ~ D3 (297730~302872)
        if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // For 2016 Data Period D4 ~ L  (302919~311481)
        if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // Pass MET triggers
        if (m_met_trig_fire) {
          // For 2015 Data
          if (m_dataYear == "2015") { // HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
        }
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      // Pass inclusive jet cut
      if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
        // Efficiency plot
        // For 2015 Data
        if (m_dataYear == "2015") { // for HLT_xe70_mht
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // For 2016 Data Period A ~ D3 (297730~302872)
        if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // For 2016 Data Period D4 ~ L  (302919~311481)
        if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
        }
        // Pass MET triggers
        if (m_met_trig_fire) {
          // For 2015 Data
          if (m_dataYear == "2015") { // HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
        }
      }
    }
  } // No systematic






  //----------
  // MET cut
  //----------
  if ( MET < sm_metCut ) return;



  ////////////////////////////////////////////////
  // Plot for Unfolding (No MET trigger passed) //
  ////////////////////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight); // For publication binning
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight); // For publication binning
    }
  }



  //------------------
  // Pass MET Trigger
  //------------------
  if (!m_met_trig_fire) return;




  ////////////////////////////////////////
  // Calculate MET Trigger SF for Znunu //
  ////////////////////////////////////////
  float mcEventWeight_Znunu = mcEventWeight;
  if (!m_isData && m_metTrigSF) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      mcEventWeight_Znunu = mcEventWeight_Znunu * GetMetTrigSF(MET,"exclusive","znunu");
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      mcEventWeight_Znunu = mcEventWeight_Znunu * GetMetTrigSF(MET,"inclusive","znunu");
    }
  }




  //////////
  // Plot //
  //////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Znunu);
      // Leading jet distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"+sysName]->Fill(m_goodJet->at(0)->pt() * 0.001, mcEventWeight_Znunu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_eta"+sysName]->Fill(m_goodJet->at(0)->eta(), mcEventWeight_Znunu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_phi"+sysName]->Fill(m_goodJet->at(0)->phi(), mcEventWeight_Znunu);
      // dPhi(jet,MET)
      float dPhijetmet = deltaPhi(m_goodJet->at(0)->phi(),MET_phi);
      hMap1D["SM_study_"+channel+hist_prefix+"dPhiJetMet"+sysName]->Fill(dPhijetmet, mcEventWeight_Znunu);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Znunu);
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMet"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Znunu);
        if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet){
          // 1st Leading jet distribution
          if (m_goodJet->at(0) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Znunu);
          }
          // 2nd leading jet distribution
          if (m_goodJet->size() > 1 && m_goodJet->at(1) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Znunu);
          }
          // 3rd leading jet distribution
          if (m_goodJet->size() > 2 && m_goodJet->at(2) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Znunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Znunu);
          }
        }
      }
    }
    // dPhiMin(jet,met) distribution without dPhi(jet,met) cut
    if (passInclusiveRecoJetNoDPhiJetMET(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMetNoCut"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Znunu);
      }
    }
  }

  // Test Statistical uncertainty for ratio (using various leading jet cuts)
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passExclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passExclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passExclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passExclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passExclusiveRecoJet(m_goodJet, 180000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt180"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveRecoJet(m_goodJet, 100000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt100"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 110000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt110"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 120000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt120"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
      if (passInclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Znunu);
    }
  }


  // Multijet Background estimation
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveMultijetCR(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
         if ( MET > 130000. && MET < 150000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin1"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
         if ( MET > 150000. && MET < 175000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin2"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
         if ( MET > 175000. && MET < 200000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin3"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
         if ( MET > 200000. && MET < 225000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin4"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
         if ( MET > 225000. && MET < 250000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin5"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
         if ( MET > 250000. && MET < 300000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin6"+sysName]->Fill(m_goodJet->at(1)->pt() * 0.001, mcEventWeight_Znunu);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveMultijetCR(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
        float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
        //int jet_num = 0;
        for (const auto &jet : *m_goodJet) {
          // dPhi(jet,MET)
          float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
          dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
          //std::cout << "jet # = " << jet_num << ", dPhijetmet = " << dPhijetmet << std::endl;
          //jet_num ++;
        }
        //std::cout << "dPhiMinjetmet = " << dPhiMinjetmet << std::endl;
        int badjet_num = 0;
        for (const auto &jet : *m_goodJet) {
          float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
          if (dPhiMinjetmet != dPhijetmet) badjet_num++;
          else break;
        }
        //std::cout << "badjet_num = " << badjet_num << std::endl;
        if ( MET > 130000. && MET < 150000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin1"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
        if ( MET > 150000. && MET < 175000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin2"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
        if ( MET > 175000. && MET < 200000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin3"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
        if ( MET > 200000. && MET < 225000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin4"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
        if ( MET > 225000. && MET < 250000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin5"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
        if ( MET > 250000. && MET < 300000. ) hMap1D["SM_study_"+channel+hist_prefix+"multijetCR_badJetPt_bin6"+sysName]->Fill(m_goodJet->at(badjet_num)->pt() * 0.001, mcEventWeight_Znunu);
      }
    }
  }



}






void smZInvAnalysis::doZmumuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::MuonContainer* muons, const xAOD::MuonContainer* muonSC, const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  std::string channel = "zmumu";

  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;

  //===================================================================
  // For rebuild the emulated MET for Zmumu (by marking Muon invisible)
  //===================================================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for muon FSR

  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  //
  // For emulated MET (No muons)
  // Make a empty container for invisible muons
  ConstDataVector<xAOD::MuonContainer> m_EmptyMuons(SG::VIEW_ELEMENTS);
  m_EmptyMuons.clear();
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                         //filling this met container
      m_EmptyMuons.asDataVector(),            //using these metMuons that accepted our cuts
      metMap);                     //and this association map

  // Make a container for invisible muons
  ConstDataVector<xAOD::MuonContainer> m_invisibleMuons(SG::VIEW_ELEMENTS);
  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    m_invisibleMuons.push_back( muon );
  } // end for loop over shallow copied muons

  // Mark muons invisible
  m_metMaker->markInvisible(m_invisibleMuons.asDataVector(), metMap, m_met);


  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Zmumu (For emulated MET marking muons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Zmumu marking muons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  //////////////////////////////////////////////////////////////
  // Fill emulated MET for Zmumu (by marking muons invisible) //
  //////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());



  //-----------------
  // At least 2 muons
  //-----------------
  if (m_goodMuon->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  bool pass_OS = false; // Opposite sign charge lepton
  bool pass_SS = false; // Same sign charge lepton

  TLorentzVector lepton1 = m_goodMuon->at(0)->p4();
  TLorentzVector lepton2 = m_goodMuon->at(1)->p4();
  float lepton1_pt = lepton1.Pt();
  float lepton2_pt = lepton2.Pt();
  float lepton1_charge = m_goodMuon->at(0)->charge();
  float lepton2_charge = m_goodMuon->at(1)->charge();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();

  if ( lepton1_charge * lepton2_charge < 0 ) pass_OS = true;
  if ( lepton1_charge * lepton2_charge > 0 ) pass_SS = true;

  //---------------
  // Dilepton cut
  //---------------
  if ( lepton1_pt <  sm_lep1PtCut || lepton2_pt < sm_lep2PtCut ) return;


  //-----------------------
  // Opposite sign leptons
  //-----------------------
  if ( !pass_OS ) return;



  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Electron veto
  if ( m_goodElectron->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because STDM4 derivation does not contain a aux data "trackLinks" in Tau container, I could not use tau selection tool
  //if ( m_goodTau->size() > 0  ) return;

  //-----------------
  // Exact 2 muons
  //-----------------
  if (m_goodMuon->size() > 2) return;



  /////////////////////////////
  // Trigger Efficiency plot //
  /////////////////////////////
  if (sysName=="") { // No systematic
    if (mll > m_mllMin && mll < m_mllMax) { // Mll cut (66Gev < mll < 116GeV)
      // Exclusive
      if ( hist_prefix.find("exclusive")!=std::string::npos ) {
        // Pass exclusive jet cut
        if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
          // Efficiency plot
          // For 2015 Data
          if (m_dataYear == "2015") { // for HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // Pass MET triggers
          if (m_met_trig_fire) {
            // For 2015 Data
            if (m_dataYear == "2015") { // HLT_xe70_mht
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
            // For 2016 Data Period A ~ D3 (297730~302872)
            if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
            // For 2016 Data Period D4 ~ L  (302919~311481)
            if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
          }
        }
      }
      // Inclusive
      if ( hist_prefix.find("inclusive")!=std::string::npos ) {
        // Pass inclusive jet cut
        if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
          // Efficiency plot
          // For 2015 Data
          if (m_dataYear == "2015") { // for HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
            if ( m_mu_trig_fire ) {
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
          // Pass MET triggers
          if (m_met_trig_fire) {
            // For 2015 Data
            if (m_dataYear == "2015") { // HLT_xe70_mht
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
            // For 2016 Data Period A ~ D3 (297730~302872)
            if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
            // For 2016 Data Period D4 ~ L  (302919~311481)
            if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50_NoPassMuTrig"+sysName]->Fill(MET * 0.001, mcEventWeight);
              if ( m_mu_trig_fire ) {
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
                hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              }
            }
          }
        }
      }
    } // Mll cut
  } // No systematic



  //----------
  // MET cut
  //----------
  if ( MET < sm_metCut ) return;


  /////////////////////////////////
  // Calculate muon SF for Zmumu //
  /////////////////////////////////
  float mcEventWeight_Zmumu = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Zmumu original mcEventWeight = %.3f ", mcEventWeight);
    mcEventWeight_Zmumu = mcEventWeight_Zmumu * GetTotalMuonSF(*m_goodMuon, m_recoSF, m_isoMuonSF, m_ttvaSF, m_muonTrigSFforSM);
    //Info("execute()", " Zmumu mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Zmumu);
  }


  ///////////////////////////////////////////////////////////
  // Plot for Unfolding (No MET trigger passed)            //
  // which will be used for MC events in Unfolding formula //
  ///////////////////////////////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66GeV)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
      if (mll > m_mllMin && mll < m_mllMax) { // Mll cut (66Gev < mll < 116GeV)
        hMap1D["SM_study_"+channel+hist_prefix+"met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66Gev)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
      if (mll > m_mllMin && mll < m_mllMax) { // Mll cut (66Gev < mll < 116GeV)
        hMap1D["SM_study_"+channel+hist_prefix+"met_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_noMetTrig"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
    }
  }





  //------------------
  // Pass MET Trigger
  //------------------
  if (!m_met_trig_fire) return;



  ////////////////////////////////////////
  // Calculate MET Trigger SF for Zmumu //
  ////////////////////////////////////////
  if (!m_isData && m_metTrigSF) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      mcEventWeight_Zmumu = mcEventWeight_Zmumu * GetMetTrigSF(MET,"exclusive","zmumu");
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      mcEventWeight_Zmumu = mcEventWeight_Zmumu * GetMetTrigSF(MET,"inclusive","zmumu");
    }
  }



  /////////////////////////////////
  // Mll released MET histograms //
  // (1) no Mll cut              //
  // (2) Mll > 66 GeV            //
  /////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66GeV)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66Gev)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      }
    }
  }






  //-------------------------------
  // Mll cut (66Gev < mll < 116GeV)
  //-------------------------------
  if ( mll < m_mllMin || mll > m_mllMax ) return;



  ///////////////////
  // Analysis Plot //
  ///////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Zmumu);
      // Leading jet distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"+sysName]->Fill(m_goodJet->at(0)->pt() * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_eta"+sysName]->Fill(m_goodJet->at(0)->eta(), mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_phi"+sysName]->Fill(m_goodJet->at(0)->phi(), mcEventWeight_Zmumu);
      // Leading lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"+sysName]->Fill(lepton1.Perp() * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_eta"+sysName]->Fill(lepton1.Eta(), mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_phi"+sysName]->Fill(lepton1.Phi(), mcEventWeight_Zmumu);
      // 2nd leading lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"+sysName]->Fill(lepton2.Perp() * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_eta"+sysName]->Fill(lepton2.Eta(), mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_phi"+sysName]->Fill(lepton2.Phi(), mcEventWeight_Zmumu);
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"+sysName]->Fill(mll * 0.001, mcEventWeight_Zmumu);
      // dPhi(jet,MET)
      float dPhijetmet = deltaPhi(m_goodJet->at(0)->phi(),MET_phi);
      hMap1D["SM_study_"+channel+hist_prefix+"dPhiJetMet"+sysName]->Fill(dPhijetmet, mcEventWeight_Zmumu);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Zmumu);
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMet"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Zmumu);
        if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet){
          // 1st Leading jet distribution
          if (m_goodJet->at(0) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zmumu);
          }
          // 2nd leading jet distribution
          if (m_goodJet->size() > 1 && m_goodJet->at(1) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zmumu);
          }
          // 3rd leading jet distribution
          if (m_goodJet->size() > 2 && m_goodJet->at(2) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zmumu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zmumu);
          }
        }
      }
      // Leading lepton distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"+sysName]->Fill(lepton1.Perp() * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_eta"+sysName]->Fill(lepton1.Eta(), mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_phi"+sysName]->Fill(lepton1.Phi(), mcEventWeight_Zmumu);
      // 2nd leading lepton distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"+sysName]->Fill(lepton2.Perp() * 0.001, mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_eta"+sysName]->Fill(lepton2.Eta(), mcEventWeight_Zmumu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_phi"+sysName]->Fill(lepton2.Phi(), mcEventWeight_Zmumu);
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"+sysName]->Fill(mll * 0.001, mcEventWeight_Zmumu);
    }
  }

  // Test Statistical uncertainty for ratio (using various leading jet cuts)
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passExclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passExclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passExclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passExclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passExclusiveRecoJet(m_goodJet, 180000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt180"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveRecoJet(m_goodJet, 100000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt100"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 110000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt110"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 120000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt120"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
      if (passInclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Zmumu);
    }
  }






}





void smZInvAnalysis::doZeeSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::ElectronContainer* elecSC, const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  std::string channel = "zee";

  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;

  //=====================================================================
  // For rebuild the emulated MET for Zee (by marking Electron invisible)
  //=====================================================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Not adding Electron, Photon, Tau objects as we veto on additional leptons and photons might be an issue for electron FSR

  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  //
  // For emulated MET (No electrons)
  // Make a container for invisible electrons
  ConstDataVector<xAOD::ElectronContainer> m_invisibleElectrons(SG::VIEW_ELEMENTS);
  for (const auto& electron : *elecSC) { // C++11 shortcut
    for (const auto& goodelectron : *m_goodElectron) { // C++11 shortcut
      // Check if electrons are matched to good electrons
      if (electron->pt() == goodelectron->pt() && electron->eta() == goodelectron->eta() && electron->phi() == goodelectron->phi()){
        // Put good electrons in
        m_invisibleElectrons.push_back( electron );
      }
    }
  }
  // Mark electrons invisible (No electrons)
  m_metMaker->markInvisible(m_invisibleElectrons.asDataVector(), metMap, m_met);


  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Zee (For emulated MET marking electrons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Zee marking electrons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  ////////////////////////////////////////////////////////////////
  // Fill emulated MET for Zee (by marking electrons invisible) //
  ////////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());




  //-----------------
  // At least 2 electrons
  //-----------------
  if (m_goodElectron->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  bool pass_OS = false; // Opposite sign charge lepton
  bool pass_SS = false; // Same sign charge lepton

  TLorentzVector lepton1 = m_goodElectron->at(0)->p4();
  TLorentzVector lepton2 = m_goodElectron->at(1)->p4();
  float lepton1_pt = lepton1.Pt();
  float lepton2_pt = lepton2.Pt();
  float lepton1_charge = m_goodElectron->at(0)->charge();
  float lepton2_charge = m_goodElectron->at(1)->charge();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();

  if ( lepton1_charge * lepton2_charge < 0 ) pass_OS = true;
  if ( lepton1_charge * lepton2_charge > 0 ) pass_SS = true;


  //---------------
  // Dilepton cut
  //---------------
  if ( lepton1_pt <  sm_lep1PtCut || lepton2_pt < sm_lep2PtCut ) return;


  //-----------------------
  // Opposite sign leptons
  //-----------------------
  if ( !pass_OS ) return;


  //-------------------------------
  // Pass Single Electron Triggers
  //-------------------------------
  if (!m_ele_trig_fire) return;


  //----------
  // MET cut
  //----------
  if ( MET < sm_metCut ) return;


  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Additional isolated electron veto
  bool add_iso_electron = false;
  for (const auto& electron : *m_goodElectron) {
    if (m_goodElectron->at(0) == electron || m_goodElectron->at(1) == electron) continue;
    if (m_isolationFixedCutTightSelectionTool->accept(*electron)) add_iso_electron = true; // For additional isolated electron
  }
  if (add_iso_electron) return;
  // Muon veto
  if ( m_goodMuon->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because STDM4 derivation does not contain a aux data "trackLinks" in Tau container, I could not use tau selection tool
  //if ( m_goodTau->size() > 0  ) return;

  //-----------------
  // Exact 2 electrons
  //-----------------
  if (m_goodElectron->size() > 2) return;




  ///////////////////////////////////
  // Calculate electron SF for Zee //
  ///////////////////////////////////
  float mcEventWeight_Zee = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Zee original mcEventWeight = %.3f ", mcEventWeight);
    mcEventWeight_Zee = mcEventWeight_Zee * GetTotalElectronSF(*m_goodElectron, m_recoSF, m_idSF, m_isoElectronSF, m_elecTrigSF);
    //Info("execute()", " Zee mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Zee);
  }




  /////////////////////////////////
  // Mll released MET histograms //
  // (1) no Mll cut              //
  // (2) Mll > 66 GeV            //
  /////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66GeV)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      }
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      if (mll > m_mllMin) { // Mll cut (mll > 66Gev)
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      }
    }
  }




  //-------------------------------
  // Mll cut (66Gev < mll < 116GeV)
  //-------------------------------
  if ( mll < m_mllMin || mll > m_mllMax ) return;



  //////////
  // Plot //
  //////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      hMap1D["SM_study_"+channel+hist_prefix+"mll"+sysName]->Fill(mll * 0.001, mcEventWeight_Zee);
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Zee);
      // Leading jet distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"+sysName]->Fill(m_goodJet->at(0)->pt() * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_eta"+sysName]->Fill(m_goodJet->at(0)->eta(), mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_phi"+sysName]->Fill(m_goodJet->at(0)->phi(), mcEventWeight_Zee);
      // Leading lepton distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"+sysName]->Fill(lepton1.Perp() * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_eta"+sysName]->Fill(lepton1.Eta(), mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_phi"+sysName]->Fill(lepton1.Phi(), mcEventWeight_Zee);
      // 2nd leading lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"+sysName]->Fill(lepton2.Perp() * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_eta"+sysName]->Fill(lepton2.Eta(), mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_phi"+sysName]->Fill(lepton2.Phi(), mcEventWeight_Zee);
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"+sysName]->Fill(mll * 0.001, mcEventWeight_Zee);
      // dPhi(jet,MET)
      float dPhijetmet = deltaPhi(m_goodJet->at(0)->phi(),MET_phi);
      hMap1D["SM_study_"+channel+hist_prefix+"dPhiJetMet"+sysName]->Fill(dPhijetmet, mcEventWeight_Zee);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binngin
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Zee);
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMet"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Zee);
        if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet){
          // 1st Leading jet distribution
          if (m_goodJet->at(0) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zee);
          }
          // 2nd leading jet distribution
          if (m_goodJet->size() > 1 && m_goodJet->at(1) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zee);
          }
          // 3rd leading jet distribution
          if (m_goodJet->size() > 2 && m_goodJet->at(2) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Zee);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Zee);
          }
        }
      }

      // Leading lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"+sysName]->Fill(lepton1.Perp() * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_eta"+sysName]->Fill(lepton1.Eta(), mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_phi"+sysName]->Fill(lepton1.Phi(), mcEventWeight_Zee);
      // 2nd leading lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"+sysName]->Fill(lepton2.Perp() * 0.001, mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_eta"+sysName]->Fill(lepton2.Eta(), mcEventWeight_Zee);
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_phi"+sysName]->Fill(lepton2.Phi(), mcEventWeight_Zee);
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"+sysName]->Fill(mll * 0.001, mcEventWeight_Zee);
    }
  }

  ////////////////////////////////////////
  // Plot using Overlap-subtracted Jets //
  ////////////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodOSJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_OS"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n_OS"+sysName]->Fill(m_goodOSJet->size(), mcEventWeight_Zee);
      // Leading jet pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt_OS"+sysName]->Fill(m_goodOSJet->at(0)->pt() * 0.001, mcEventWeight_Zee);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodOSJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_OS"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee); // For publication binngin
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n_OS"+sysName]->Fill(m_goodOSJet->size(), mcEventWeight_Zee);
      // Leading jet pT distribution
      for (const auto &jet : *m_goodOSJet) {
        hMap1D["SM_study_"+channel+hist_prefix+"jet_pt_OS"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Zee);
      }
    }
  }



  // Test Statistical uncertainty for ratio (using various leading jet cuts)
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passExclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passExclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passExclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passExclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passExclusiveRecoJet(m_goodJet, 180000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt180"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveRecoJet(m_goodJet, 100000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt100"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 110000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt110"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 120000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt120"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 130000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt130"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 140000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt140"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 150000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt150"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 160000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt160"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
      if (passInclusiveRecoJet(m_goodJet, 170000., MET_phi)) hMap1D["SM_study_"+channel+hist_prefix+"met_LeadjetPt170"+sysName]->Fill(MET * 0.001, mcEventWeight_Zee);
    }
  }





}



void smZInvAnalysis::doWmunuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  std::string channel = "wmunu";


  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;


  //===========================
  // For rebuild the real MET
  //===========================


  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectron
  ConstDataVector<xAOD::ElectronContainer> m_MetElectron(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

  // iterate over our shallow copy
  for (const auto& electron : *m_goodElectron) { // C++11 shortcut
    // For MET rebuilding
    m_MetElectron.push_back( electron );
  } // end for loop over shallow copied electrons
  //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectron.asDataVector();

  // For real MET
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                                      //filling this met container
      m_MetElectron.asDataVector(),              //using these metElectrons that accepted our cuts
      metMap);                                  //and this association map

  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    // For MET rebuilding
    m_MetMuons.push_back( muon );
  } // end for loop over shallow copied muons
  // For real MET
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                                  //filling this met container
      m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
      metMap);                              //and this association map



  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For real MET
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term (For real MET)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For real MET for Znunu
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  /////////////////////////////
  // Fill real MET for Znunu //
  /////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());


  float real_met = ((*m_met)["Final"]->met());
  float real_mpx = ((*m_met)["Final"]->mpx());
  float real_mpy = ((*m_met)["Final"]->mpy());
  
  //std::cout << "Real MET = " << MET * 0.001 << " , sqrt(mpx^2+mpy^2) = " << std::sqrt(real_mpx*real_mpx+real_mpy*real_mpy) * 0.001 << std::endl;



  //===================================================================
  // For rebuild the emulated MET for Wmunu (by marking Muon invisible)
  //===================================================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  ConstDataVector<xAOD::ElectronContainer> m_MetElectrons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

  // iterate over our shallow copy
  for (const auto& electron : *m_goodElectron) { // C++11 shortcut
    // For MET rebuilding
    m_MetElectrons.push_back( electron );
  } // end for loop over shallow copied electrons
  //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectrons.asDataVector();

  // For real MET
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                                      //filling this met container
      m_MetElectrons.asDataVector(),              //using these metElectrons that accepted our cuts
      metMap);                                  //and this association map



  // Muon
  //-----------------
  // Make a container for invisible muons
  ConstDataVector<xAOD::MuonContainer> m_invisibleMuons(SG::VIEW_ELEMENTS);
  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    m_invisibleMuons.push_back( muon );
  } // end for loop over shallow copied muons

  // Mark muons invisible
  m_metMaker->markInvisible(m_invisibleMuons.asDataVector(), metMap, m_met);


  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Wmunu (For emulated MET marking muons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Wmunu marking muons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  //////////////////////////////////////////////////////////////
  // Fill emulated MET for Wmunu (by marking muons invisible) //
  //////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());





  //-----------------
  // Exact one muon
  //-----------------
  if (m_goodMuon->size() != 1) return;


  //----------------------
  // Define W Selection
  //----------------------
  float lepton_pt = m_goodMuon->at(0)->pt();
  float lepton_eta = m_goodMuon->at(0)->eta();
  float lepton_phi = m_goodMuon->at(0)->phi();

  // Transverse Mass
  float mT = TMath::Sqrt( 2. * lepton_pt * met * ( 1. - TMath::Cos(lepton_phi - metPhi) ) ); // where met and metPhi are from real MET


  //---------------
  // muon pt cut
  //---------------
  if ( lepton_pt < sm_lep1PtCut ) return;


  //-------------------------
  // mT cut (Transverse Mass)
  //-------------------------
  if ( mT < m_mTCut ) return; // SM Analysis
  //if ( mT < m_mTMin || mT > m_mTMax ) return; // Exotic Analysis



  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Electron veto
  if ( m_goodElectron->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because STDM4 derivation does not contain a aux data "trackLinks" in Tau container, I could not use tau selection tool
  //if ( m_goodTau->size() > 0  ) return;





  /////////////////////////////
  // Trigger Efficiency plot //
  /////////////////////////////
  if (sysName=="") { // No systematic
    // Pass single muon triggers to avoid bias
    if ( m_mu_trig_fire ) {
      // Exclusive
      if ( hist_prefix.find("exclusive")!=std::string::npos ) {
        // Pass exclusive jet cut
        if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
          // Efficiency plot
          // For 2015 Data
          if (m_dataYear == "2015") { // for HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // Pass MET triggers
          if (m_met_trig_fire) {
            // For 2015 Data
            if (m_dataYear == "2015") { // HLT_xe70_mht
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
            // For 2016 Data Period A ~ D3 (297730~302872)
            if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
            // For 2016 Data Period D4 ~ L  (302919~311481)
            if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
        }
      }
      // Inclusive
      if ( hist_prefix.find("inclusive")!=std::string::npos ) {
        // Pass inclusive jet cut
        if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
          // Efficiency plot
          // For 2015 Data
          if (m_dataYear == "2015") { // for HLT_xe70_mht
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period A ~ D3 (297730~302872)
          if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // for HLT_xe90_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // For 2016 Data Period D4 ~ L  (302919~311481)
          if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // for HLT_xe110_mht_L1XE50
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_for_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
          }
          // Pass MET triggers
          if (m_met_trig_fire) {
            // For 2015 Data
            if (m_dataYear == "2015") { // HLT_xe70_mht
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe70_mht"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
            // For 2016 Data Period A ~ D3 (297730~302872)
            if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe90_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
            // For 2016 Data Period D4 ~ L  (302919~311481)
            if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"met_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
              hMap1D["SM_study_"+channel+"_trig_eff"+hist_prefix+"MET_mono_pass_HLT_xe110_mht_L1XE50"+sysName]->Fill(MET * 0.001, mcEventWeight);
            }
          }
        }
      }
    } // muon trigger
  } // No systematic





  /////////////////
  // Emulate WpT //
  /////////////////

  float lepton_px = lepton_pt * TMath::Sin(lepton_phi);
  float lepton_py = lepton_pt * TMath::Cos(lepton_phi);
  float emul_WpT = TMath::Sqrt((real_mpx+lepton_px)*(real_mpx+lepton_px)+(real_mpy+lepton_py)*(real_mpy+lepton_py));


  ////////////////
  // pT(W) test //
  ////////////////
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      // Pass No jet cut
      hMap1D["SM_study_"+channel+"_reco_noJetCut_real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+"_reco_noJetCut_emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+"_reco_noJetCut_emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      // Pass exclusive jet cut
      if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
        hMap1D["SM_study_"+channel+hist_prefix+"real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      // Pass inclusive jet cut
      if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
        hMap1D["SM_study_"+channel+hist_prefix+"real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      }
    }
  } // No systematic




  //----------
  // MET cut
  //----------
  if ( MET < sm_metCut ) return;




  /////////////////////////////////
  // Calculate muon SF for Wmunu //
  /////////////////////////////////
  float mcEventWeight_Wmunu = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Wmunu original mcEventWeight = %.3f ", mcEventWeight);
      mcEventWeight_Wmunu = mcEventWeight_Wmunu * GetTotalMuonSF(*m_goodMuon, m_recoSF, m_isoMuonSF, m_ttvaSF, m_muonTrigSFforSM);
    //Info("execute()", " Wmunu mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Wmunu);
  }


  //---------------------
  // Single muon trigger
  //---------------------
  //if (!m_mu_trig_fire) return;


  //------------------
  // Pass MET Trigger
  //------------------
  if (!m_met_trig_fire) return;


  ////////////////////////////////////////
  // Calculate MET Trigger SF for Wmunu //
  ////////////////////////////////////////
  if (!m_isData && m_metTrigSF) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      mcEventWeight_Wmunu = mcEventWeight_Wmunu * GetMetTrigSF(MET,"exclusive","wmunu");
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      mcEventWeight_Wmunu = mcEventWeight_Wmunu * GetMetTrigSF(MET,"inclusive","wmunu");
    }
  }





  ///////////////////
  // Analysis Plot //
  ///////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Wmunu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Wmunu);
      // Leading jet distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"+sysName]->Fill(m_goodJet->at(0)->pt() * 0.001, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_eta"+sysName]->Fill(m_goodJet->at(0)->eta(), mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_phi"+sysName]->Fill(m_goodJet->at(0)->phi(), mcEventWeight_Wmunu);
      // Lepton distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"+sysName]->Fill(lepton_pt * 0.001, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_eta"+sysName]->Fill(lepton_eta, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_phi"+sysName]->Fill(lepton_phi, mcEventWeight_Wmunu);
      // mT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mT"+sysName]->Fill(mT * 0.001, mcEventWeight_Wmunu);
      // dPhi(jet,MET)
      float dPhijetmet = deltaPhi(m_goodJet->at(0)->phi(),MET_phi);
      hMap1D["SM_study_"+channel+hist_prefix+"dPhiJetMet"+sysName]->Fill(dPhijetmet, mcEventWeight_Wmunu);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Wmunu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Wmunu);
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMet"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Wmunu);
        if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet){
          // 1st Leading jet distribution
          if (m_goodJet->at(0) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wmunu);
          }
          // 2nd leading jet distribution
          if (m_goodJet->size() > 1 && m_goodJet->at(1) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wmunu);
          }
          // 3rd leading jet distribution
          if (m_goodJet->size() > 2 && m_goodJet->at(2) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wmunu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wmunu);
          }
        }
      }

      // Lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"+sysName]->Fill(lepton_pt * 0.001, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_eta"+sysName]->Fill(lepton_eta, mcEventWeight_Wmunu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_phi"+sysName]->Fill(lepton_phi, mcEventWeight_Wmunu);
      // mT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mT"+sysName]->Fill(mT * 0.001, mcEventWeight_Wmunu);
    }
  }




}



void smZInvAnalysis::doWenuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& met, const float& metPhi, const xAOD::ElectronContainer* elecSC, xAOD::ElectronContainer* goodElectron ,const float& mcEventWeight, std::string hist_prefix, std::string sysName){

  std::string channel = "wenu";


  //==============//
  // MET building //
  //==============//

  std::string softTerm = "PVSoftTrk";

  // MET
  float MET = -9e9;
  float MET_phi = -9e9;



  //===========================
  // For rebuild the real MET
  //===========================


  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();


  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectron
  ConstDataVector<xAOD::ElectronContainer> m_MetElectron(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Electron>

  // iterate over our shallow copy
  for (const auto& electron : *goodElectron) { // C++11 shortcut
    // For MET rebuilding
    m_MetElectron.push_back( electron );
  } // end for loop over shallow copied electrons
  //const xAOD::ElectronContainer* p_MetElectrons = m_MetElectron.asDataVector();

  // For real MET
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                                      //filling this met container
      m_MetElectron.asDataVector(),              //using these metElectrons that accepted our cuts
      metMap);                                  //and this association map

  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuons
  ConstDataVector<xAOD::MuonContainer> m_MetMuons(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    // For MET rebuilding
    m_MetMuons.push_back( muon );
  } // end for loop over shallow copied muons
  // For real MET
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                                  //filling this met container
      m_MetMuons.asDataVector(),              //using these metMuons that accepted our cuts
      metMap);                              //and this association map



  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For real MET
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term (For real MET)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For real MET for Znunu
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  /////////////////////////////
  // Fill real MET for Znunu //
  /////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());


  float real_met = ((*m_met)["Final"]->met());
  float real_mpx = ((*m_met)["Final"]->mpx());
  float real_mpy = ((*m_met)["Final"]->mpy());
  
  //std::cout << "Real MET = " << MET * 0.001 << " , sqrt(mpx^2+mpy^2) = " << std::sqrt(real_mpx*real_mpx+real_mpy*real_mpy) * 0.001 << std::endl;



  //=======================================
  // For rebuild the emulated MET for Wenu
  //=======================================

  // It is necessary to reset the selected objects before every MET calculation
  m_met->clear();
  metMap->resetObjSelectionFlags();

  // Electron
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Electron container m_electrons, placing selected electrons into m_MetElectrons
  //
  // For emulated MET (No electrons)
  // Make a empty container for invisible electrons
  ConstDataVector<xAOD::ElectronContainer> m_EmptyElectrons(SG::VIEW_ELEMENTS);
  m_EmptyElectrons.clear();
  m_metMaker->rebuildMET("RefElectron",           //name of metElectrons in metContainer
      xAOD::Type::Electron,                       //telling the rebuilder that this is electron met
      m_met,                         //filling this met container
      m_EmptyElectrons.asDataVector(),            //using these metElectrons that accepted our cuts
      metMap);                     //and this association map

  // Make a container for invisible electrons
  ConstDataVector<xAOD::ElectronContainer> m_invisibleElectrons(SG::VIEW_ELEMENTS);
  for (const auto& electron : *elecSC) { // C++11 shortcut
    for (const auto& goodelectron : *goodElectron) { // C++11 shortcut
      // Check if electrons are matched to good electrons
      if (electron->pt() == goodelectron->pt() && electron->eta() == goodelectron->eta() && electron->phi() == goodelectron->phi()){
        // Put good electrons in
        m_invisibleElectrons.push_back( electron );
      }
    }
  }
  // Mark electrons invisible (No electrons)
  m_metMaker->markInvisible(m_invisibleElectrons.asDataVector(), metMap, m_met);


  // Muon
  //-----------------
  /// Creat New Hard Object Containers
  // [For MET building] filter the Muon container m_muons, placing selected muons into m_MetMuon
  ConstDataVector<xAOD::MuonContainer> m_MetMuon(SG::VIEW_ELEMENTS); // This is really a DataVector<xAOD::Muon>

  // iterate over our shallow copy
  for (const auto& muon : *m_goodMuon) { // C++11 shortcut
    // For MET rebuilding
    m_MetMuon.push_back( muon );
  } // end for loop over shallow copied muons
  // For real MET
  m_metMaker->rebuildMET("RefMuon",           //name of metMuons in metContainer
      xAOD::Type::Muon,                       //telling the rebuilder that this is muon met
      m_met,                                  //filling this met container
      m_MetMuon.asDataVector(),              //using these metMuons that accepted our cuts
      metMap);                              //and this association map

  // JET
  //-----------------
  //Now time to rebuild jetMet and get the soft term
  //This adds the necessary soft term for both CST and TST
  //these functions create an xAODMissingET object with the given names inside the container

  // For emulated MET marking muons invisible
  m_metMaker->rebuildJetMET("RefJet",          //name of jet met
      "SoftClus",           //name of soft cluster term met
      "PVSoftTrk",          //name of soft track term met
      m_met,       //adding to this new met container
      m_allJet,                //using this jet collection to calculate jet met
      metCore,   //core met container
      metMap,    //with this association map
      true);                //apply jet jvt cut

  /////////////////////////////
  // Soft term uncertainties //
  /////////////////////////////
  if (!m_isData) {
    // Get the track soft term for Wenu (For emulated MET marking electrons invisible)
    xAOD::MissingET* softTrkmet = (*m_met)[softTerm];
    if (m_metSystTool->applyCorrection(*softTrkmet) != CP::CorrectionCode::Ok) {
      Error("execute()", "METSystematicsTool returns Error CorrectionCode");
    }
  }

  ///////////////
  // MET Build //
  ///////////////
  // For emulated MET for Wenu marking electrons invisible
  m_metMaker->buildMETSum("Final", m_met, (*m_met)[softTerm]->source());

  /////////////////////////////////////////////////////////////////
  // Fill emulated MET for Wenu (by marking electrons invisible) //
  /////////////////////////////////////////////////////////////////
  MET = ((*m_met)["Final"]->met());
  MET_phi = ((*m_met)["Final"]->phi());






  //--------------------
  // Exact one electron
  //--------------------
  if (goodElectron->size() != 1) return;


  //----------------------
  // Define W Selection
  //----------------------
  float lepton_pt = goodElectron->at(0)->pt();
  float lepton_eta = goodElectron->at(0)->eta();
  float lepton_phi = goodElectron->at(0)->phi();

  // Transverse Mass
  float mT = TMath::Sqrt( 2. * lepton_pt * met * ( 1. - TMath::Cos(lepton_phi - metPhi) ) ); // where met and metPhi are from real MET


  //---------------
  // electron pt cut
  //---------------
  if ( lepton_pt < sm_lep1PtCut ) return;


  //-------------------------
  // mT cut (Transverse Mass)
  //-------------------------
  //if ( mT < m_mTCut ) return; // SM Analysis
  //if ( mT < m_mTMin || mT > m_mTMax ) return; // Exotic Analysis
  if ( mT < 50000. || mT > 110000. ) return; // Our SM Analysis



  //------------------------
  // Additional lepton Veto 
  //------------------------
  // Muon veto
  if ( m_goodMuon->size() > 0  ) return;
  // Tau veto "ONLY" available in EXOT5 derivation
  // because STDM4 derivation does not contain a aux data "trackLinks" in Tau container, I could not use tau selection tool
  //if ( m_goodTau->size() > 0  ) return;


  /////////////////
  // Emulate WpT //
  /////////////////

  float lepton_px = lepton_pt * TMath::Sin(lepton_phi);
  float lepton_py = lepton_pt * TMath::Cos(lepton_phi);
  float emul_WpT = TMath::Sqrt((real_mpx+lepton_px)*(real_mpx+lepton_px)+(real_mpy+lepton_py)*(real_mpy+lepton_py));
  //std::cout << "Wenu MET = " << MET * 0.001 << " , emul_WpT = " << emul_WpT * 0.001 << std::endl;


  ////////////////
  // pT(W) test //
  ////////////////
  if (sysName=="") { // No systematic
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      // Pass No jet cut
      hMap1D["SM_study_"+channel+"_reco_noJetCut_real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+"_reco_noJetCut_emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+"_reco_noJetCut_emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      // Pass exclusive jet cut
      if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
        hMap1D["SM_study_"+channel+hist_prefix+"real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      // Pass inclusive jet cut
      if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
        hMap1D["SM_study_"+channel+hist_prefix+"real_MET"+sysName]->Fill(real_met * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_MET"+sysName]->Fill(MET * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"emul_Wpt"+sysName]->Fill(emul_WpT * 0.001, mcEventWeight);
      }
    }
  } // No systematic



  //-------------------------------
  // Pass Single Electron Triggers
  //-------------------------------
  if (!m_ele_trig_fire) return;

  //--------------
  // Real MET cut
  //--------------
  if (met < 70000. ) return;


  //----------
  // MET cut
  //----------
  if ( MET < sm_metCut ) return;


  /*
  std::cout << "MET trigger fired : " << m_met_trig_fire << std::endl;
  std::cout << "Electron trigger fired : " << m_ele_trig_fire << std::endl;
  std::cout << "Muon trigger fired : " << m_mu_trig_fire << std::endl;
  */



  ////////////////////////////////////
  // Calculate electron SF for Wenu //
  ////////////////////////////////////
  float mcEventWeight_Wenu = mcEventWeight;
  if (!m_isData) {
    //Info("execute()", " Wenu original mcEventWeight = %.3f ", mcEventWeight);
    mcEventWeight_Wenu = mcEventWeight_Wenu * GetTotalElectronSF(*goodElectron, m_recoSF, m_idSF, m_isoElectronSF, m_elecTrigSF);
    //Info("execute()", " Wenu mcEventWeight * TotalMuonSF = %.3f ", mcEventWeight_Wenu);
  }


  ///////////////////
  // Analysis Plot //
  ///////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Wenu);
      // Leading jet pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"+sysName]->Fill(m_goodJet->at(0)->pt() * 0.001, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_eta"+sysName]->Fill(m_goodJet->at(0)->eta(), mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"jet_phi"+sysName]->Fill(m_goodJet->at(0)->phi(), mcEventWeight_Wenu);
      // Lepton distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"+sysName]->Fill(lepton_pt * 0.001, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_eta"+sysName]->Fill(lepton_eta, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_phi"+sysName]->Fill(lepton_phi, mcEventWeight_Wenu);
      // mT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mT"+sysName]->Fill(mT * 0.001, mcEventWeight_Wenu);
      // dPhi(jet,MET)
      float dPhijetmet = deltaPhi(m_goodJet->at(0)->phi(),MET_phi);
      hMap1D["SM_study_"+channel+hist_prefix+"dPhiJetMet"+sysName]->Fill(dPhijetmet, mcEventWeight_Wenu);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"+sysName]->Fill(m_goodJet->size(), mcEventWeight_Wenu);
      float dPhiMinjetmet = 10.; // initialize with 10. to obtain minimum value of deltaPhi(Jet_i,MET)
      for (const auto &jet : *m_goodJet) {
        // dPhi(jet,MET)
        float dPhijetmet = deltaPhi(jet->phi(),MET_phi);
        dPhiMinjetmet = std::min(dPhiMinjetmet, dPhijetmet);
        hMap1D["SM_study_"+channel+hist_prefix+"dPhiMinJetMet"+sysName]->Fill(dPhiMinjetmet, mcEventWeight_Wenu);
        if (m_goodJet->at(0) == jet || m_goodJet->at(1) == jet || m_goodJet->at(2) == jet){
          // 1st Leading jet distribution
          if (m_goodJet->at(0) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet1_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wenu);
          }
          // 2nd leading jet distribution
          if (m_goodJet->size() > 1 && m_goodJet->at(1) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet2_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wenu);
          }
          // 3rd leading jet distribution
          if (m_goodJet->size() > 2 && m_goodJet->at(2) == jet) {
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_pt"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_eta"+sysName]->Fill(jet->eta(), mcEventWeight_Wenu);
            hMap1D["SM_study_"+channel+hist_prefix+"jet3_phi"+sysName]->Fill(jet->phi(), mcEventWeight_Wenu);
          }
        }
      }

      // Lepton pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"+sysName]->Fill(lepton_pt * 0.001, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_eta"+sysName]->Fill(lepton_eta, mcEventWeight_Wenu);
      hMap1D["SM_study_"+channel+hist_prefix+"lep_phi"+sysName]->Fill(lepton_phi, mcEventWeight_Wenu);
      // mT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mT"+sysName]->Fill(mT * 0.001, mcEventWeight_Wenu);
    }
  }


  /////////////////////////////////////////////////
  // Analysis Plot using Overlap-subtracted jets //
  /////////////////////////////////////////////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Common plots
    if (passExclusiveRecoJet(m_goodOSJet, sm_exclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_OS"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n_OS"+sysName]->Fill(m_goodOSJet->size(), mcEventWeight_Wenu);
      // Leading jet pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt_OS"+sysName]->Fill(m_goodOSJet->at(0)->pt() * 0.001, mcEventWeight_Wenu);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Common plots
    if (passInclusiveRecoJet(m_goodOSJet, sm_inclusiveJetPtCut, MET_phi)) {
      // MET distribution
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono_OS"+sysName]->Fill(MET * 0.001, mcEventWeight_Wenu); // For publication binning
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n_OS"+sysName]->Fill(m_goodOSJet->size(), mcEventWeight_Wenu);
      // Leading jet pT distribution
      for (const auto &jet : *m_goodOSJet) {
        hMap1D["SM_study_"+channel+hist_prefix+"jet_pt_OS"+sysName]->Fill(jet->pt() * 0.001, mcEventWeight_Wenu);
      }
    }
  }





}





void smZInvAnalysis::doZllEmulTruth(const xAOD::TruthParticleContainer* truthLepton, const xAOD::JetContainer* truthJet, const float& lep1Pt, const float& lep2Pt, const float& lepEta, const float& mcEventWeight, std::string channel, std::string hist_prefix ){

  //------------------------------
  // Define Truth Zll Selection
  //------------------------------

  //--------------------
  // At least 2 leptons
  //--------------------
  if (truthLepton->size() < 2) return;


  //----------------------
  // Define Zll Selection
  //----------------------
  TLorentzVector lepton1 = truthLepton->at(0)->p4();
  TLorentzVector lepton2 = truthLepton->at(1)->p4();
  auto Zll = lepton1 + lepton2;
  float mll = Zll.M();
  float ZPt = Zll.Pt();
  float ZPhi = Zll.Phi();



  //----------
  // MET cut
  //----------
  if ( ZPt < sm_metCut ) return;


  ///////////////////
  //  Lepton Test  //
  // No lepton cut //
  ///////////////////
  if (!is_customDerivation) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
        // All leptons pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(lepton1.Perp() * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(lepton2.Perp() * 0.001, mcEventWeight);
        // Leading lepton pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(lepton1.Perp() * 0.001, mcEventWeight);
        // 2nd leading lepton pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(lepton2.Perp() * 0.001, mcEventWeight);
        // mll distribution
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll"]->Fill(mll * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
        // All leptons pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(lepton1.Perp() * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(lepton2.Perp() * 0.001, mcEventWeight);
        // Leading lepton pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(lepton1.Perp() * 0.001, mcEventWeight);
        // 2nd leading lepton pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(lepton2.Perp() * 0.001, mcEventWeight);
        // mll distribution
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll"]->Fill(mll * 0.001, mcEventWeight);
      }
    }
  }


  //---------------
  // Dilepton cut
  //---------------
  // pT cut
  if ( lepton1.Perp() < lep1Pt || lepton2.Perp() < lep2Pt ) return;
  // Eta cut
  if ( std::fabs(lepton1.Eta()) > lepEta || std::fabs(lepton2.Eta()) > lepEta ) return;
  /*
  /////// Apply crack
  // Electron
  if (channel == "zee") {
    // Fiducial
    if ( hist_prefix.find("fid")!=std::string::npos ) {
      if ( std::fabs(lepton1.Eta()) > lepEta || (std::fabs(lepton1.Eta()) > 1.37 && std::fabs(lepton1.Eta()) < 1.52)) return;
      if ( std::fabs(lepton2.Eta()) > lepEta || (std::fabs(lepton2.Eta()) > 1.37 && std::fabs(lepton2.Eta()) < 1.52)) return;
      // Non-fiducial
    } else {
      if ( std::fabs(lepton1.Eta()) > lepEta || std::fabs(lepton2.Eta()) > lepEta ) return;
    }
  }
  // Muon
  if (channel == "zmumu") {
    if ( std::fabs(lepton1.Eta()) > lepEta || std::fabs(lepton2.Eta()) > lepEta ) return;
  }
  */


  //////////////////////////////
  // Mll Test                 //
  // Full Mll and Mll > 66GeV //
  //////////////////////////////
  if (!is_customDerivation) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
        // ZPt distribution
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"]->Fill(ZPt * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
        // ZPt distribution
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"]->Fill(ZPt * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      }
    }
    // Mll cut ( mll > 66GeV )
    if ( mll > m_mllMin ) {
      // Exclusive
      if ( hist_prefix.find("exclusive")!=std::string::npos ) {
        if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
          // ZPt distribution
          hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"]->Fill(ZPt * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
        }
      }
      // Inclusive
      if ( hist_prefix.find("inclusive")!=std::string::npos ) {
        if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
          // ZPt distribution
          hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"]->Fill(ZPt * 0.001, mcEventWeight);
          hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
        }
      }
    } // Mll cut
  }





  //-------------------------------
  // Mll cut (66Gev < mll < 116GeV)
  //-------------------------------
  if ( mll < m_mllMin || mll > m_mllMax ) return;






  //////////
  // Plot //
  //////////

  // Exclusive (66 < mll < 116)
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Test plots (using various exclusive jet pt cut)
    if (!is_customDerivation) {
      if ( hist_prefix.find("bare")!=std::string::npos || hist_prefix.find("born")!=std::string::npos ) {
        if (passExclusiveTruthJet(truthJet, 130000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(0.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 140000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(1.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 150000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(2.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 160000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(3.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 170000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(4.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 180000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(5.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 190000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(6.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 200000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(7.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 210000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(8.5, ZPt * 0.001, mcEventWeight);
        if (passExclusiveTruthJet(truthJet, 220000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(9.5, ZPt * 0.001, mcEventWeight);
      }
    }
    // Common plots
    if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"]->Fill(truthJet->size(), mcEventWeight);
      // Leading jet pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"]->Fill(truthJet->at(0)->pt() * 0.001, mcEventWeight);
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"]->Fill(mll * 0.001, mcEventWeight);
    }
  }

  // Inclusive (66 < mll < 116)
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Test plots (using various inclusive jet pt cut)
    if (!is_customDerivation) {
      if ( hist_prefix.find("bare")!=std::string::npos || hist_prefix.find("born")!=std::string::npos ) {
        if (passInclusiveTruthJet(truthJet, 100000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(0.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 110000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(1.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 120000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(2.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 130000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(3.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 140000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(4.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 150000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(5.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 160000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(6.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 170000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(7.5, ZPt * 0.001, mcEventWeight);
        if (passInclusiveTruthJet(truthJet, 180000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(8.5, ZPt * 0.001, mcEventWeight);
      }
    }
    // Common plots
    if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"]->Fill(truthJet->size(), mcEventWeight);
      // Leading jet pT distribution
      for (const auto &jet : *truthJet) {
        hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"]->Fill(jet->pt() * 0.001, mcEventWeight);
      }
      // mll distribution
      hMap1D["SM_study_"+channel+hist_prefix+"mll"]->Fill(mll * 0.001, mcEventWeight);
    }
  }



}



void smZInvAnalysis::doZnunuEmulTruth(const xAOD::TruthParticleContainer* truthNu, const xAOD::JetContainer* truthJet, const float& mcEventWeight, std::string channel, std::string hist_prefix ){

  //------------------------------
  // Define Truth Znunu Selection
  //------------------------------

  //----------------------
  // At least 2 neutrinos
  //----------------------
  if (truthNu->size() < 2) return;


  //----------------------
  // Define Znunu Selection
  //----------------------
  TLorentzVector neutrino1 = truthNu->at(0)->p4();
  TLorentzVector neutrino2 = truthNu->at(1)->p4();
  auto Znunu = neutrino1 + neutrino2;
  float Zmass = Znunu.M();
  float ZPt = Znunu.Pt();
  float ZPhi = Znunu.Phi();



  //----------
  // MET cut
  //----------
  if ( ZPt < sm_metCut ) return;



  ////////////////////
  // Lepton pT Test //
  ////////////////////
  if (!is_customDerivation) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
        // All neutrinos pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
        // Leading neutrino pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
        // 2nd leading neutrino pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
        // All neutrinos pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"lep_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
        // Leading neutrino pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
        // 2nd leading neutrino pT distribution
        hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
      }
    }
  }



  //////////////////////////////
  // Mll Test                 //
  // Full Mll and Mll > 66GeV //
  //////////////////////////////
  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
    }
  }
  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"fullmll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
    }
  }
  // Mll cut ( mll > 66GeV )
  if ( Zmass > m_mllMin ) {
    // Exclusive
    if ( hist_prefix.find("exclusive")!=std::string::npos ) {
      if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
        // ZPt distribution
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"]->Fill(ZPt * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      }
    }
    // Inclusive
    if ( hist_prefix.find("inclusive")!=std::string::npos ) {
      if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
        // ZPt distribution
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_met"]->Fill(ZPt * 0.001, mcEventWeight);
        hMap1D["SM_study_"+channel+hist_prefix+"only66mll_MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      }
    }
  } // Mll cut





  //-------------------------------
  // Mll cut (66Gev < mll < 116GeV)
  //-------------------------------
  if ( Zmass < m_mllMin || Zmass > m_mllMax ) return;





  //////////
  // Plot //
  //////////

  // Exclusive
  if ( hist_prefix.find("exclusive")!=std::string::npos ) {
    // Test plots (using various exclusive jet pt cut)
    if (passExclusiveTruthJet(truthJet, 130000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(0.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 140000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(1.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 150000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(2.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 160000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(3.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 170000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(4.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 180000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(5.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 190000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(6.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 200000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(7.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 210000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(8.5, ZPt * 0.001, mcEventWeight);
    if (passExclusiveTruthJet(truthJet, 220000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(9.5, ZPt * 0.001, mcEventWeight);
    // Common plots
    if (passExclusiveTruthJet(truthJet, sm_exclusiveJetPtCut, ZPhi)) {
      hMap1D["SM_study_"+channel+hist_prefix+"mll"]->Fill(Zmass * 0.001, mcEventWeight);
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"]->Fill(truthJet->size(), mcEventWeight);
      // Leading jet pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"]->Fill(truthJet->at(0)->pt() * 0.001, mcEventWeight);
      // Leading neutrino pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
      // 2nd leading neutrino pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
    }
  }

  // Inclusive
  if ( hist_prefix.find("inclusive")!=std::string::npos ) {
    // Test plots (using various inclusive jet pt cut)
    if (passInclusiveTruthJet(truthJet, 100000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(0.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 110000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(1.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 120000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(2.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 130000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(3.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 140000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(4.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 150000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(5.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 160000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(6.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 170000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(7.5, ZPt * 0.001, mcEventWeight);
    if (passInclusiveTruthJet(truthJet, 180000., ZPhi)) hMap2D["SM_study_"+channel+hist_prefix+"leadpt_vs_met"]->Fill(8.5, ZPt * 0.001, mcEventWeight);
    // Common plots
    if (passInclusiveTruthJet(truthJet, sm_inclusiveJetPtCut, ZPhi)) {
      hMap1D["SM_study_"+channel+hist_prefix+"mll"]->Fill(Zmass * 0.001, mcEventWeight);
      // ZPt distribution
      hMap1D["SM_study_"+channel+hist_prefix+"met"]->Fill(ZPt * 0.001, mcEventWeight);
      hMap1D["SM_study_"+channel+hist_prefix+"MET_mono"]->Fill(ZPt * 0.001, mcEventWeight);
      // Leading jet # distribution
      hMap1D["SM_study_"+channel+hist_prefix+"jet_n"]->Fill(truthJet->size(), mcEventWeight);
      // Leading jet pT distribution
      for (const auto &jet : *truthJet) {
        hMap1D["SM_study_"+channel+hist_prefix+"jet_pt"]->Fill(jet->pt() * 0.001, mcEventWeight);
      }
      // Leading neutrino pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep1_pt"]->Fill(neutrino1.Perp() * 0.001, mcEventWeight);
      // 2nd leading neutrino pT distribution
      hMap1D["SM_study_"+channel+hist_prefix+"lep2_pt"]->Fill(neutrino2.Perp() * 0.001, mcEventWeight);
    }
  }



}




bool smZInvAnalysis::passExclusiveTruthJet(const xAOD::JetContainer* truthJet, const float& leadJetPt, const float& metPhi){

   if (truthJet->size() != 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = truthJet->at(0)->pt();
  float monojet_eta = truthJet->at(0)->eta();

  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_exclusiveJetEtaCut ) return false;


  /*
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *truthJet) {
    float good_jet_pt = jet->pt();
    float good_jet_phi = jet->phi();

    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    if (truthJet->at(0) == jet || truthJet->at(1) == jet || truthJet->at(2) == jet || truthJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
      float dPhijetmet = deltaPhi(good_jet_phi,metPhi);
      if ( good_jet_pt > 30000. && dPhijetmet < 0.4 ) pass_dPhijetmet = false;
    }

  }

  if ( !pass_dPhijetmet ) return false;

  */

  // Pass Monojet phasespace
  return true;

}



bool smZInvAnalysis::passInclusiveTruthJet(const xAOD::JetContainer* truthJet, const float& leadJetPt, const float& metPhi){

   if (truthJet->size() < 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = truthJet->at(0)->pt();
  float monojet_eta = truthJet->at(0)->eta();

  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_inclusiveJetEtaCut ) return false;


  /*
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *truthJet) {
    float good_jet_pt = jet->pt();
    float good_jet_phi = jet->phi();

    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    if (truthJet->at(0) == jet || truthJet->at(1) == jet || truthJet->at(2) == jet || truthJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
      float dPhijetmet = deltaPhi(good_jet_phi,metPhi);
      if ( good_jet_pt > 30000. && dPhijetmet < 0.4 ) pass_dPhijetmet = false;
    }

  }

  if ( !pass_dPhijetmet ) return false;

  */

  // Pass Monojet phasespace
  return true;

}


bool smZInvAnalysis::passExclusiveRecoJet(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi){

  if (recoJet->size() != 1) return false;

  // Monojet Selection
  float monojet_pt = recoJet->at(0)->pt();
  float monojet_eta = recoJet->at(0)->eta();
  float monojet_phi = recoJet->at(0)->phi();

  // Define Monojet
  if ( recoJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *recoJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_exclusiveJetEtaCut ) return false;

  // Define dPhi(jet,MET) for leading jet
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
  float dPhijetmet = deltaPhi(monojet_phi,metPhi);
  if ( dPhijetmet < sm_dPhiJetMetCut ) pass_dPhijetmet = false;

  // Multijet Suppression
  if ( !pass_dPhijetmet ) return false;

  // Pass exclusive jet phasespace
  return true;

}


bool smZInvAnalysis::passInclusiveRecoJet(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi){

   if (recoJet->size() < 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = recoJet->at(0)->pt();
  float monojet_eta = recoJet->at(0)->eta();

  // Define Monojet
  if ( recoJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *recoJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_inclusiveJetEtaCut ) return false;
  
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *recoJet) {
    float reco_jet_pt = jet->pt();
    float reco_jet_phi = jet->phi();

    /*
    // Remove jet1, jet2, jet3 and jet4 if Phi(Jet_i,MET) < 0.4
    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    if (recoJet->at(0) == jet || recoJet->at(1) == jet || recoJet->at(2) == jet || recoJet->at(3) == jet){ // apply cut only to leading jet1, jet2, jet3 and jet4
      float dPhijetmet = deltaPhi(reco_jet_phi,metPhi);
      if ( dPhijetmet < 0.4 ) {
        pass_dPhijetmet = false;
        //std::cout << "fake jet : " << "jet pT = " << reco_jet_pt * 0.001 << " GeV " << " dPhi(jet,MET) = " << dPhijetmet << std::endl;
      }
    }
    */

    // Remove any jet if Phi(Jet_i,MET) < 0.4
    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    float dPhijetmet = deltaPhi(reco_jet_phi,metPhi);
    if ( dPhijetmet < sm_dPhiJetMetCut ) pass_dPhijetmet = false;
  }

  // Multijet suppression
  if ( !pass_dPhijetmet ) return false;

  // Pass Monojet phasespace
  return true;

}


bool smZInvAnalysis::passInclusiveRecoJetNoDPhiJetMET(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi){

   if (recoJet->size() < 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = recoJet->at(0)->pt();
  float monojet_eta = recoJet->at(0)->eta();

  // Define Monojet
  if ( recoJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *recoJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_inclusiveJetEtaCut ) return false;
  
  /*
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *recoJet) {
    float reco_jet_pt = jet->pt();
    float reco_jet_phi = jet->phi();

  // Remove any jet if Phi(Jet_i,MET) < 0.4
    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    float dPhijetmet = deltaPhi(reco_jet_phi,metPhi);
    if ( dPhijetmet < sm_dPhiJetMetCut ) pass_dPhijetmet = false;
  }

  // Multijet suppression
  if ( !pass_dPhijetmet ) return false;
  */

  // Pass Monojet phasespace
  return true;

}




bool smZInvAnalysis::passExclusiveMultijetCR(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi){

  if (recoJet->size() != 2) return false;

  // Monojet Selection
  float monojet_pt = recoJet->at(0)->pt();
  float monojet_eta = recoJet->at(0)->eta();
  float monojet_phi = recoJet->at(0)->phi();
  // Second-leading jet
  float secondjet_phi = recoJet->at(1)->phi();

  // Define Monojet
  if ( recoJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *recoJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_exclusiveJetEtaCut ) return false;

  // Define dPhi(jet,MET) for leading jet
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
  float dPhijetmet = deltaPhi(secondjet_phi,metPhi);
  if ( dPhijetmet < sm_dPhiJetMetCut ) pass_dPhijetmet = false;

  // Reverse cut (require second-leading jet pointing towards the MET)
  if ( pass_dPhijetmet ) return false;

  // Pass Multijet enrich Control region
  return true;

}



bool smZInvAnalysis::passInclusiveMultijetCR(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi){

   if (recoJet->size() < 1) return false;

  //---------------------------
  // Define Monojet Properties
  //---------------------------

  // Monojet Selection
  float monojet_pt = recoJet->at(0)->pt();
  float monojet_eta = recoJet->at(0)->eta();

  // Define Monojet
  if ( recoJet->at(0)->auxdata<bool>("RecoJet") && !m_jetCleaningTightBad->accept( *recoJet->at(0) ) ) return false; // Apply TightBad only to "Reco" jet
  if ( monojet_pt < leadJetPt || fabs(monojet_eta) > sm_inclusiveJetEtaCut ) return false;
  
  // Define dPhi(jet,MET) for leading jet1, jet2, jet3 and jet4
  bool pass_dPhijetmet = true; // deltaPhi(Jet_i,MET)
  for (const auto& jet : *recoJet) {
    float reco_jet_pt = jet->pt();
    float reco_jet_phi = jet->phi();
  
    // Remove any jet if Phi(Jet_i,MET) < 0.5
    // Calculate dPhi(Jet_i,MET) and dPhi_min(Jet_i,MET)
    float dPhijetmet = deltaPhi(reco_jet_phi,metPhi);
    if ( dPhijetmet < sm_dPhiJetMetCut ) pass_dPhijetmet = false;
  }

  // Reverse cut (require at least one jet pointing towards the MET)
  if ( pass_dPhijetmet ) return false;

  // Pass Multijet enrich Control region
  return true;

}



float smZInvAnalysis::GetGoodMuonSF(xAOD::Muon& mu,
    const bool recoSF, const bool isoSF, const bool ttvaSF, const bool muonTrigSF) {

  float sf(1.);

  if (recoSF) {
    float sf_reco(1.);
    if (m_muonEfficiencySFTool->getEfficiencyScaleFactor( mu, sf_reco ) == CP::CorrectionCode::OutOfValidityRange) {
      Error("execute()", " GetGoodMuonSF: Reco getEfficiencyScaleFactor out of validity range");
    }
    //Info("execute()", "  GetGoodMuonSF: sf_reco = %.5f ", sf_reco );
    sf *= sf_reco;
  }

  if (isoSF) {
    float sf_iso(1.);
    if (m_muonIsolationSFTool->getEfficiencyScaleFactor( mu, sf_iso ) == CP::CorrectionCode::OutOfValidityRange) {
      Error("execute()", " GetGoodMuonSF: Iso getEfficiencyScaleFactor out of validity range");
    }
    //Info("execute()", "  GetGoodMuonSF: sf_iso = %.5f ", sf_iso );
    sf *= sf_iso;
  }

  if (ttvaSF) {
    float sf_TTVA(1.);
    if (m_muonTTVAEfficiencySFTool->getEfficiencyScaleFactor( mu, sf_TTVA ) == CP::CorrectionCode::OutOfValidityRange) {
      Error("execute()", " GetGoodMuonSF: TTVA getEfficiencyScaleFactor out of validity range");
    }
    //Info("execute()", "  GetGoodMuonSF: sf_TTVA = %.5f ", sf_TTVA );
    sf *= sf_TTVA;
  }


  if (muonTrigSF) {
    double sf_trig(1.);

    ConstDataVector<xAOD::MuonContainer> trigger_SF_muon(SG::VIEW_ELEMENTS);
    trigger_SF_muon.push_back( &mu );

    // For 2015 data
    if (m_dataYear == "2015"){
      if (m_muonTriggerSFTool->getTriggerScaleFactor( *trigger_SF_muon.asDataVector(), sf_trig, "HLT_mu20_iloose_L1MU15_OR_HLT_mu50" ) == CP::CorrectionCode::OutOfValidityRange) {
        Error("execute()", " GetGoodMuonSF: Trigger (Loose) getEfficiencyScaleFactor out of validity range");
      }
    }

    // For 2016 data
    if (m_dataYear == "2016"){
      if (m_muonTriggerSFTool->getTriggerScaleFactor( *trigger_SF_muon.asDataVector(), sf_trig, "HLT_mu26_ivarmedium_OR_HLT_mu50" ) == CP::CorrectionCode::OutOfValidityRange) {
        Error("execute()", " GetGoodMuonSF: Trigger (Loose) getEfficiencyScaleFactor out of validity range");
      }
    }

    //Info("execute()", "  GetGoodMuonSF: muon pT = %.5f GeV", mu.pt() * 0.001 );
    //Info("execute()", "  GetGoodMuonSF: sf_trig = %.5f ", sf_trig );
    sf *= sf_trig;
  }


  //Info("execute()", "  GetGoodMuonSF: Good Muon SF = %.5f ", sf );
  return sf;

}


double smZInvAnalysis::GetTotalMuonSF(xAOD::MuonContainer& muons,
    bool recoSF, bool isoSF, bool ttvaSF, bool muonTrigSF) {

  double sf(1.);

  int muonCount = 0;
  bool trigSF = muonTrigSF;

  for (const auto& muon : muons) {
    muonCount++;
    // Since we use single muon trigger, I apply muon trigger scale factor "ONLY" to leading muon.
    // Do not use trigger scale factor for second muon!
    if (muonCount != 1) trigSF = false;
    //Info("execute()", "-------------------------------------------------");
    //Info("execute()", " muon # : %i, Good muon pt = %.2f GeV", muonCount, (muon->pt() * 0.001));
    sf *= GetGoodMuonSF(*muon, recoSF, isoSF, ttvaSF, trigSF);
  }

  //Info("execute()", "  GetTotalMuonSF: Total Muon SF = %.5f ", sf );
  return sf;

}


float smZInvAnalysis::GetGoodElectronSF(xAOD::Electron& elec,
    const bool recoSF, const bool idSF, const bool isoSF, const bool elecTrigSF) {

  float sf(1.);

  if (recoSF) {
    double sf_reco(1.);
    if (m_elecEfficiencySFTool_reco->getEfficiencyScaleFactor( elec, sf_reco ) == CP::CorrectionCode::Ok) {
      sf *= sf_reco;
      //Info("execute()", "  GetGoodElectronSF: sf_reco = %.5f ", sf_reco );
    }
    else {
      Error("execute()", " GetGoodElectronSF: Reco getEfficiencyScaleFactor returns Error CorrectionCode");
    }
  }

  if (idSF) {
    double sf_id(1.);
    if (m_elecEfficiencySFTool_id->getEfficiencyScaleFactor( elec, sf_id ) == CP::CorrectionCode::Ok) {
      sf *= sf_id;
      //Info("execute()", "  GetGoodElectronSF: sf_id = %.5f ", sf_id );
    }
    else {
      Error("execute()", " GetGoodElectronSF: Id getEfficiencyScaleFactor returns Error CorrectionCode");
    }
  }

  if (isoSF) {
    double sf_iso(1.);
    if (m_elecEfficiencySFTool_iso->getEfficiencyScaleFactor( elec, sf_iso ) == CP::CorrectionCode::Ok) {
      sf *= sf_iso;
      //Info("execute()", "  GetGoodElectronSF: sf_iso = %.5f ", sf_iso );
    }
    else {
      Error("execute()", " GetGoodElectronSF: Iso getEfficiencyScaleFactor returns Error CorrectionCode");
    }
  }

  if (elecTrigSF) {
    double sf_trig(1.);
    if (m_elecEfficiencySFTool_trigSF->getEfficiencyScaleFactor( elec, sf_trig ) == CP::CorrectionCode::Ok) {
      sf *= sf_trig;
      //Info("execute()", "  GetGoodElectronSF: sf_trig = %.5f ", sf_trig );
    }
    else {
      Error("execute()", " GetGoodElectronSF: Trigger getEfficiencyScaleFactor returns Error CorrectionCode");
    }
  }


  //Info("execute()", "  GetGoodElectronSF: Good Electron SF = %.5f ", sf );
  return sf;

}


float smZInvAnalysis::GetTotalElectronSF(xAOD::ElectronContainer& electrons,
    bool recoSF, bool idSF, bool isoSF, bool elecTrigSF) {

  float sf(1.);

  int elecCount = 0;
  bool trigSF = elecTrigSF;

  for (const auto& electron : electrons) {
    elecCount++;
    // Since we use single electron trigger, I apply electron trigger scale factor tool to "ONLY" leading electron.
    // Do not use trigger scale factor for second electron!
    if (elecCount != 1) trigSF = false;
    //Info("execute()", "-------------------------------------------------");
    //Info("execute()", " electron # : %i, Good electron pt = %.2f GeV", elecCount, (electron->pt() * 0.001));
    sf *= GetGoodElectronSF(*electron, recoSF, idSF, isoSF, trigSF);
    //Info("execute()", "  Good electron pt = %.2f GeV, trig_SF = %d, Electron SF = %.2f", (electron->pt() * 0.001), trig_SF, sf);
  }

  //Info("execute()", "  GetTotalElectronSF: Total Electron SF = %.5f ", sf );
  return sf;

}


float smZInvAnalysis::GetMetTrigSF(const float& met, std::string jetCut, std::string channel) {

  float sf(1.);

  TF1* m_SF = new TF1("SFfunc", "TMath::Erf((x-[0])/[1]) / 2 + 0.5", 100, 300);

  std::string metTrigger = "";

  // Apply MET trigger SF in MET (130GeV ~ 200GeV)
  if (met > 130000. && met < 200000.) {
    // Inclusive
    if (jetCut == "inclusive") {

      if (m_dataYear == "2015") { // HLT_xe70_mht
        metTrigger = "HLT_xe70_mht";
        // For Wmunu and Znunu
        if (channel == "wmunu" || channel == "znunu") {
          m_SF->FixParameter(0,3.87840e+01);
          m_SF->FixParameter(1,5.44155e+01);
        }
        // For Zmumu
        if (channel == "zmumu") {
          m_SF->FixParameter(0,-7.77707e+00);
          m_SF->FixParameter(1,8.46828e+01);
        }
      } else if (m_dataYear == "2016" && m_run2016Period == "AtoD3") { // HLT_xe90_mht_L1XE50
        metTrigger = "HLT_xe90_mht_L1XE50";
        // For Wmunu and Znunu
        if (channel == "wmunu" || channel == "znunu") {
          m_SF->FixParameter(0,4.64496e+01);
          m_SF->FixParameter(1,4.76320e+01);
        }
        // For Zmumu
        if (channel == "zmumu") {
          m_SF->FixParameter(0,7.29544e+01);
          m_SF->FixParameter(1,2.63631e+01);
        }
      } else if (m_dataYear == "2016" && m_run2016Period == "D4toL") { // HLT_xe110_mht_L1XE50
        metTrigger = "HLT_xe110_mht_L1XE50";
        // For Wmunu and Znunu
        if (channel == "wmunu" || channel == "znunu") {
          m_SF->FixParameter(0,7.11684e+01);
          m_SF->FixParameter(1,3.86691e+01);
        }
        // For Zmumu
        if (channel == "zmumu") {
          m_SF->FixParameter(0,8.01843e+01);
          m_SF->FixParameter(1,3.27954e+01);
        }
      }

      sf = m_SF->Eval(met*0.001);

      //std::cout << " GetMetTrigSF : Used Trigger = " << metTrigger << " in " << channel << " , MET = " << met*0.001 << "GeV , SF = " << sf << std::endl;

    }
    // Exclusive
    else if (jetCut == "exclusive") {
      sf = 1.;
    }

  } else {
    sf = 1.;
  } // Set Met range

  return sf;

}


