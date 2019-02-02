#ifndef smZInvAnalysis_smZInvAnalysis_H
#define smZInvAnalysis_smZInvAnalysis_H

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// Root inclues
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Event Bookkeepers
#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

// include files for using the trigger tools
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigConfInterfaces/ITrigConfigTool.h"

// GRL tools
#include "GoodRunsLists/GoodRunsListSelectionTool.h"
// PileupReweighting tool
#include "PileupReweighting/PileupReweightingTool.h"

// EDM
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

// Jet tools
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"
#include "JetResolution/JERTool.h"
#include "JetResolution/JERSmearingTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "JetMomentTools/JetForwardJvtTool.h"
#include "JetJvtEfficiency/JetJvtEfficiency.h"

// b-jet tools
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"

// Muon tools
#include "MuonMomentumCorrections/MuonCalibrationAndSmearingTool.h"
#include "MuonSelectorTools/MuonSelectionTool.h"
#include "MuonEfficiencyCorrections/MuonTriggerScaleFactors.h"
#include "MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h"

// Electron tools
#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronPhotonSelectorTools/AsgElectronLikelihoodTool.h"
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "ElectronPhotonSelectorTools/EGammaAmbiguityTool.h"

// Photon tools
#include "xAODEgamma/EgammaDefs.h"
#include "ElectronPhotonShowerShapeFudgeTool/ElectronPhotonShowerShapeFudgeTool.h"
#include "ElectronPhotonSelectorTools/egammaPIDdefs.h"
#include "ElectronPhotonSelectorTools/AsgPhotonIsEMSelector.h"

// Tau tools
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauOverlappingElectronLLHDecorator.h"
#include "TauAnalysisTools/TauSelectionTool.h"

// Isolation Selection tool
#include "IsolationSelection/IsolationSelectionTool.h"

// Overlap Removal tool
#include "AssociationUtils/ToolBox.h"
#include "AssociationUtils/OverlapRemovalInit.h"
#include "AssociationUtils/OverlapRemovalTool.h"
#include "AssociationUtils/EleJetOverlapTool.h"
#include "AssociationUtils/MuJetOverlapTool.h"
#include "AssociationUtils/DeltaROverlapTool.h"
#include "AssociationUtils/EleMuSharedTrkOverlapTool.h"
#include "AssociationUtils/TauLooseEleOverlapTool.h"
#include "AssociationUtils/TauLooseMuOverlapTool.h"

// MET builder
#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSystematicsTool.h"
#include "METUtilities/METMaker.h"
#include "METUtilities/CutsMETMaker.h"
#include "METUtilities/METHelpers.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETComposition.h"

// PathResolver
#include "PathResolver/PathResolver.h"

// Efficiency and Scale Factor
#include "METUtilities/METSystematicsTool.h"

// header for systematics:
#include "PATInterfaces/SystematicRegistry.h"

// Cutflow
#include <smZInvAnalysis/BitsetCutflow.h>

class smZInvAnalysis : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  xAOD::TEvent *m_event; //!
  xAOD::TStore *m_store; //!

  int m_eventCounter; //!
  int m_numCleanEvents; //!
  float m_mcEventWeight; //!
  bool m_isData; //!
  bool is_customDerivation; //!
  std::string m_dataType; //!
  std::string m_nameDerivation; //!
  std::string m_nameDataset; //!
  std::string m_dataYear; //!
  std::string m_run2016Period; //!
  std::string m_ZtruthChannel; //!
  std::string m_generatorType; //!
  std::string m_input_filename; //!
  std::string m_fileType; //!

  TH1 *h_sumOfWeights; //!
  TH1 *h_dataType; //!

  // MC Campaigns
  std::string m_MC_campaign; //!

  // Event Channel
  bool m_isZnunu; //!
  bool m_isZmumu; //!
  bool m_isZee; //!
  bool m_isWmunu; //!
  bool m_isWenu; //!
  bool m_isZemu; //!

  std::string h_channel; //!
  std::string h_level; //!

  // Enable Reconstruction level analysis
  bool m_doReco; //!

  // Enable Truth level analysis
  bool m_doTruth; //!

  // Enable Systematics
  bool m_doSys; //!

  // Trigger Decision
  bool m_met_trig_fire; //!
  bool m_ele_trig_fire; //!
  bool m_mu_trig_fire; //!


  // Scale factor
  bool m_recoSF; //!
  bool m_idSF; //!
  bool m_ttvaSF; //!
  bool m_isoMuonSF; //!
  bool m_isoMuonSFforZ; //!
  bool m_elecTrigSF; //!
  bool m_muonTrigSFforExotic; //!
  bool m_muonTrigSFforSM; //!
  bool m_isoElectronSF; //!
  bool m_metTrigSF; //!


  // Cutflow
  bool m_useArrayCutflow; //!
  int m_eventCutflow[40]; //!
  // Custom classes (BitsetCutflow)
  BitsetCutflow* m_BitsetCutflow; //!
  bool m_useBitsetCutflow; //!

  // Cut values for SM study
  bool sm_doORMuon; //!
  float sm_metCut; //!
  bool sm_doPhoton_MET; //!
  bool sm_doTau_MET; //!
  float sm_ORJETdeltaR; //!
  // Jet pT
  float sm_goodJetPtCut; //!
  // No fiducial cut
  float sm_noLep1PtCut; //!
  float sm_noLep2PtCut; //!
  float sm_noLepEtaCut; //!
  // Exclusive
  float sm_exclusiveJetPtCut; //!
  float sm_exclusiveJetEtaCut; //!
  // Inclusive
  float sm_inclusiveJetPtCut; //!
  float sm_inclusiveJetEtaCut; //!
  // dPhi(Jet,MET) cut
  float sm_dPhiJetMetCut; //!
  // SM lepton cuts
  float sm_lep1PtCut; //!
  float sm_lep2PtCut; //!
  float sm_lepEtaCut; //!
  // BTagged jets (b-jet)
  bool sm_bJetVetoInclusive; //!
  int n_bJet; //!

  // Cut values
  float m_LeadLepPtCut; //!
  float m_SubLeadLepPtCut; //!
  float m_muonPtCut; //!
  float m_lepEtaCut; //!
  float m_elecPtCut; //!
  float m_elecEtaCut; //!
  float m_photPtCut; //!
  float m_photEtaCut; //!
  float m_mllMin; //!
  float m_mllMax; //!
  float m_mTCut; //!
  float m_mTMin; //!
  float m_mTMax; //!
  float m_monoJetPtCut; //!
  float m_monoJetEtaCut; //!
  float m_diJet1PtCut; //!
  float m_diJet2PtCut; //!
  float m_diJetRapCut; //!
  float m_CJVptCut; //!
  float m_metCut; //!
  float m_mjjCut; //!
  float m_ORJETdeltaR; //!

  // EXOT5 derivation Skim cut
  bool m_doSkimEXOT5; //!
  float m_skimUncalibMonoJetPt; //!
  float m_skimMonoJetPt; //!
  float m_skimLeadingJetPt; //!
  float m_skimSubleadingJetPt; //!
  float m_skimMjj; //!
  bool passUncalibMonojetCut; //!
  bool passRecoJetCuts; //!
  bool passTruthJetCuts; //!


  // OR tool test
  // Some object and event counters to help roughly
  // evaluate the effects of changes in the OR tool.
  unsigned int nInputElectrons = 0; //!
  unsigned int nInputMuons = 0; //!
  unsigned int nInputJets = 0; //!
  unsigned int nInputTaus = 0; //!
  unsigned int nInputPhotons = 0; //!
  unsigned int nOverlapElectrons = 0; //!
  unsigned int nOverlapMuons = 0; //!
  unsigned int nOverlapJets = 0; //!
  unsigned int nOverlapTaus = 0; //!
  unsigned int nOverlapPhotons = 0; //!



  // Histogram
  std::map<std::string, TH1*> hMap1D; //!
  std::map<std::string, TH2*> hMap2D; //!

  // trigger tools member variables
  Trig::TrigDecisionTool *m_trigDecisionTool; //!
  //TrigConf::xAODConfigTool *m_trigConfigTool; //!
  TrigConf::xAODConfigTool *m_trigConfigTool; //!

  // GRL
  GoodRunsListSelectionTool *m_grl; //!
  // PileupReweighting
  CP::PileupReweightingTool* m_prwTool; //!

  // Jet
  JetCalibrationTool* m_jetCalibration; //!
  JetUncertaintiesTool* m_jetUncertaintiesTool; //!
  JetCleaningTool *m_jetCleaningLooseBad; //! 
  JetCleaningTool *m_jetCleaningTightBad; //!
  ToolHandle<IJetUpdateJvt> m_hjvtagup; //!
  asg::AnaToolHandle<IJetModifier> m_fJvtTool; //!
  CP::JetJvtEfficiency* m_jvtefficiencyTool; //!
  CP::JetJvtEfficiency* m_fjvtefficiencyTool; //!

  // b-jet
  BTaggingSelectionTool *m_BJetSelectTool; //!
  BTaggingEfficiencyTool *m_BJetEfficiencyTool; //!

  // Muon
  CP::MuonCalibrationAndSmearingTool *m_muonCalibrationAndSmearingTool2016; //!
  CP::MuonCalibrationAndSmearingTool *m_muonCalibrationAndSmearingTool2017; //!
  CP::MuonSelectionTool *m_muonMediumSelection; //!
  CP::MuonSelectionTool *m_muonLooseSelection; //!
  CP::MuonTriggerScaleFactors* m_muonTriggerSFTool; //!
  CP::MuonEfficiencyScaleFactors* m_muonEfficiencySFTool; //!
  CP::MuonEfficiencyScaleFactors* m_muonIsolationSFTool; //!
  CP::MuonEfficiencyScaleFactors* m_muonTTVAEfficiencySFTool; //!

  // Electron
  CP::EgammaCalibrationAndSmearingTool *m_egammaCalibrationAndSmearingTool; //!
  AsgElectronLikelihoodTool* m_LHToolTight; //!
  AsgElectronLikelihoodTool* m_LHToolLoose; //!
  AsgElectronEfficiencyCorrectionTool* m_elecEfficiencySFTool_reco; //!
  AsgElectronEfficiencyCorrectionTool* m_elecEfficiencySFTool_id; //!
  AsgElectronEfficiencyCorrectionTool* m_elecEfficiencySFTool_iso; //!
  AsgElectronEfficiencyCorrectionTool* m_elecEfficiencySFTool_trigSF; //!
  IEGammaAmbiguityTool *m_egammaAmbiguityTool; //!


  // Photon
  ElectronPhotonShowerShapeFudgeTool* m_fudgeMCTool; //!
  AsgPhotonIsEMSelector* m_photonTightIsEMSelector; //!

  // Tau
  TauAnalysisTools::TauSmearingTool* m_tauSmearingTool; //!
  TauAnalysisTools::TauOverlappingElectronLLHDecorator* m_tauOverlappingElectronLLHDecorator; //!
  TauAnalysisTools::TauSelectionTool* m_tauSelTool; //!

  // Isolation
  CP::IsolationSelectionTool *m_isolationFixedCutTightSelectionTool; //!
  CP::IsolationSelectionTool *m_isolationLooseTrackOnlySelectionTool; //!

  // Overlap Removal Tool
  const bool outputPassValue = false; //!
  ORUtils::ToolBox *m_toolBox; //!

  // MET builder
  asg::AnaToolHandle<IMETMaker> m_metMaker; //!

  // Systematics Tools
  asg::AnaToolHandle<IMETSystematicsTool> m_metSystTool; //!

  // list of systematics
  std::vector<CP::SystematicSet> m_sysList; //!


  //------------------------------------
  // Global variable for Cz calulation
  //------------------------------------
  // Truth Zmumu
  bool m_passTruthNoMetZmumu; //!
  float m_truthPseudoMETZmumu; //!
  float m_truthPseudoMETPhiZmumu; //!


  //----------------------------------------------------------------------------------
  // Global deep copied new objects/containers to use my function, i.e. plotMonojet()
  //----------------------------------------------------------------------------------
  // Particle (truth) level
  xAOD::TruthParticleContainer* m_selectedTruthNeutrino; //!
  xAOD::AuxContainerBase* m_selectedTruthNeutrinoAux; //!
  xAOD::TruthParticleContainer* m_dressedTruthMuon; //!
  xAOD::AuxContainerBase* m_dressedTruthMuonAux; //!
  xAOD::TruthParticleContainer* m_dressedTruthElectron; //!
  xAOD::AuxContainerBase* m_dressedTruthElectronAux; //!
  xAOD::TruthParticleContainer* m_selectedTruthTau; //!
  xAOD::AuxContainerBase* m_selectedTruthTauAux; //!
  xAOD::JetContainer* m_selectedTruthJet; //!
  xAOD::AuxContainerBase* m_selectedTruthJetAux; //!
  xAOD::JetContainer* m_selectedTruthWZJet; //!
  xAOD::AuxContainerBase* m_selectedTruthWZJetAux; //!

  // Reco level
  xAOD::JetContainer* m_allJet; //!
  xAOD::AuxContainerBase* m_allJetAux; //!
  xAOD::JetContainer* m_goodJet; //!
  xAOD::AuxContainerBase* m_goodJetAux; //!
  xAOD::JetContainer* m_goodJetORTruthNuNu; //!
  xAOD::AuxContainerBase* m_goodJetORTruthNuNuAux; //!
  xAOD::JetContainer* m_goodJetORTruthMuMu; //!
  xAOD::AuxContainerBase* m_goodJetORTruthMuMuAux; //!
  xAOD::JetContainer* m_goodJetORTruthElEl; //!
  xAOD::AuxContainerBase* m_goodJetORTruthElElAux; //!
  xAOD::JetContainer* m_goodOSJet; //!
  xAOD::AuxContainerBase* m_goodOSJetAux; //!
  xAOD::MuonContainer* m_goodMuon; //!
  xAOD::AuxContainerBase* m_goodMuonAux; //!
  xAOD::MuonContainer* m_goodMuonForZ; //!
  xAOD::AuxContainerBase* m_goodMuonForZAux; //!
  xAOD::ElectronContainer* m_baselineElectron; //!
  xAOD::AuxContainerBase* m_baselineElectronAux; //!
  xAOD::ElectronContainer* m_goodElectron; //!
  xAOD::AuxContainerBase* m_goodElectronAux; //!
  xAOD::PhotonContainer* m_goodPhoton; //!
  xAOD::AuxContainerBase* m_goodPhotonAux; //!
  xAOD::TauJetContainer* m_goodTau; //!
  xAOD::AuxContainerBase* m_goodTauAux; //!
  xAOD::MissingETContainer* m_met; //!
  xAOD::MissingETAuxContainer* m_metAux; //!



  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  // this is a standard constructor
  smZInvAnalysis ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool /*firstFile*/);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // Custom made functions
  virtual EL::StatusCode addHist(std::map<std::string, TH1*> &hMap, std::string tag,
      int bins, double min, double max);

  virtual EL::StatusCode addHist(std::map<std::string, TH1*> &hMap, std::string tag,
      int bins, Float_t binArray[]);

  virtual EL::StatusCode addHist(std::map<std::string, TH2*> &hMap, std::string tag, 
      int binsX, double minX, double maxX,
      int binsY, double minY, double maxY);

  virtual EL::StatusCode addHist(std::map<std::string, TH2*> &hMap, std::string tag, 
      int binsX, Float_t binArrayX[],
      int binsY, Float_t binArrayY[]);


  float deltaPhi(float phi1, float phi2);

  float deltaR(float eta1, float eta2, float phi1, float phi2);

  bool passMonojet(const xAOD::JetContainer* goodJet, const float& metPhi);
  bool passDijet(const xAOD::JetContainer* goodJet, const float& metPhi);
  bool passVBF(const xAOD::JetContainer* goodJet, const float& metPhi);

  void plotMonojet(const xAOD::JetContainer* goodJet, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void plotVBF(const xAOD::JetContainer* goodJet, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName);

  void doZnunuExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& mcEventWeight, std::string sysName);
  void doZmumuExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::MuonContainer* muons, const xAOD::MuonContainer* muonSC, const float& mcEventWeight, std::string sysName);
  void doZeeExoticReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::ElectronContainer* elecSC, const float& mcEventWeight, std::string sysName);
  void doZnunuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doZmumuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::MuonContainer* muons, const xAOD::MuonContainer* muonSC, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doZeeSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const xAOD::ElectronContainer* elecSC, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doWmunuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& met, const float& metPhi, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doWenuSMReco(const xAOD::MissingETContainer* metCore, const xAOD::MissingETAssociationMap* metMap, const float& met, const float& metPhi, const xAOD::ElectronContainer* elecSC, xAOD::ElectronContainer* goodElectron, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doZmumuTruth(const xAOD::TruthParticleContainer* truthMuon, const float& mcEventWeight, std::string hist_prefix, std::string sysName);
  void doZllEmulTruth(const xAOD::TruthParticleContainer* truthLepton, const xAOD::JetContainer* truthJet, const float& lep1Pt, const float& lep2Pt, const float& lepEta, const float& mcEventWeight, std::string channel, std::string hist_prefix );
  void doZnunuEmulTruth(const xAOD::TruthParticleContainer* truthLepton, const xAOD::JetContainer* truthJet, const float& mcEventWeight, std::string channel, std::string hist_prefix );
  bool passExclusiveTruthJet(const xAOD::JetContainer* truthJet, const float& leadJetPt, const float& metPhi);
  bool passInclusiveTruthJet(const xAOD::JetContainer* truthJet, const float& leadJetPt, const float& metPhi);
  bool passExclusiveRecoJet(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi);
  bool passInclusiveRecoJet(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi);
  bool passInclusiveRecoJetNoDPhiJetMET(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi);
  bool passExclusiveMultijetCR(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi);
  bool passInclusiveMultijetCR(const xAOD::JetContainer* recoJet, const float& leadJetPt, const float& metPhi);

  float GetGoodMuonSF(xAOD::Muon& mu, const bool recoSF, const bool isoSF, const bool ttvaSF, const bool muonTrigSF);
  double GetTotalMuonSF(xAOD::MuonContainer& muons, bool recoSF, bool isoSF, bool ttvaSF, bool muonTrigSF);

  float GetGoodElectronSF(xAOD::Electron& elec, const bool recoSF, const bool idSF, const bool isoSF, const bool elecTrigSF);
  float GetTotalElectronSF(xAOD::ElectronContainer& electrons, bool recoSF, bool idSF, bool isoSF, bool elecTrigSF);

  float GetMetTrigSF(const float& met, std::string jetCut, std::string channel);


  // this is needed to distribute the algorithm to the workers
  ClassDef(smZInvAnalysis, 1);
};

#endif
