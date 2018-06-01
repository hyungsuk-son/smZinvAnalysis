# Set the name of the package:
atlas_subdir( smZInvAnalysis )

# Set up which packages this package depends on:
atlas_depends_on_subdirs(
   PUBLIC
   AsgExternal/Asg_MCUtils
   PhysicsAnalysis/TauID/TauAnalysisTools
   Control/AthToolSupport/AsgTools
   Control/xAODRootAccess
   Control/xAODRootAccessInterfaces
   Control/AthContainers
   Control/CxxUtils
   DataQuality/GoodRunsLists
   PhysicsAnalysis/AnalysisCommon/PATCore
   PhysicsAnalysis/AnalysisCommon/PATInterfaces
   PhysicsAnalysis/D3PDTools/RootCoreUtils
   PhysicsAnalysis/D3PDTools/EventLoop
   PhysicsAnalysis/D3PDTools/EventLoopGrid
   PhysicsAnalysis/D3PDTools/EventLoopAlgs
   PhysicsAnalysis/D3PDTools/SampleHandler
   PhysicsAnalysis/Interfaces/AsgAnalysisInterfaces
   PhysicsAnalysis/AnalysisCommon/AssociationUtils
   Event/xAOD/xAODBase
   Event/xAOD/xAODCore
   Event/xAOD/xAODEgamma
   Event/xAOD/xAODEventInfo
   Event/xAOD/xAODTracking
   Event/xAOD/xAODJet
   Event/xAOD/xAODMuon
   Event/xAOD/xAODTau
   Event/xAOD/xAODCaloEvent
   Event/xAOD/xAODTruth
   Event/xAOD/xAODMissingET
   Event/xAOD/xAODTrigger
   Event/xAOD/xAODTriggerCnv
   Event/xAOD/xAODCutFlow
   Event/xAOD/xAODMetaDataCnv
   Event/xAOD/xAODAssociations
   Event/EventPrimitives
   InnerDetector/InDetRecTools/InDetTrackSelectionTool
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools
   PhysicsAnalysis/ElectronPhotonID/ElectronEfficiencyCorrection
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonFourMomentumCorrection
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonShowerShapeFudgeTool
   PhysicsAnalysis/ElectronPhotonID/PhotonEfficiencyCorrection
   PhysicsAnalysis/ElectronPhotonID/IsolationCorrections
   PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonEfficiencyCorrections
   PhysicsAnalysis/MuonID/MuonIDAnalysis/MuonMomentumCorrections
   PhysicsAnalysis/MuonID/MuonSelectorTools
   PhysicsAnalysis/JetMissingEtID/JetSelectorTools
   Reconstruction/egamma/egammaLayerRecalibTool
   Reconstruction/egamma/egammaMVACalib
   PhysicsAnalysis/TauID/TauAnalysisTools
   Trigger/TrigAnalysis/TrigDecisionTool
   Trigger/TrigConfiguration/TrigConfxAOD
   Trigger/TrigAnalysis/TriggerMatchingTool
   Trigger/TrigAnalysis/TriggerAnalysisInterfaces
   Trigger/TrigAnalysis/TrigGlobalEfficiencyCorrection
   Trigger/TrigConfiguration/TrigConfxAOD
   Reconstruction/RecoTools/IsolationTool
   Reconstruction/RecoTools/RecoToolInterfaces
   Tools/PathResolver
   PhysicsAnalysis/AnalysisCommon/IsolationSelection
   PhysicsAnalysis/AnalysisCommon/PileupReweighting
   Reconstruction/Jet/JetCalibTools
   Reconstruction/Jet/JetJvtEfficiency
   Reconstruction/Jet/JetMomentTools
   Reconstruction/Jet/JetResolution
   Reconstruction/Jet/JetUncertainties
   Reconstruction/MET/METInterface
   Reconstruction/MET/METUtilities
   PhysicsAnalysis/TrackingID/InDetTrackSystematicsTools
)

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Physics REQUIRED )

# Generate a dictionary for the library:
atlas_add_root_dictionary( smZInvAnalysisLib smZInvAnalysisLibCintDict
   ROOT_HEADERS smZInvAnalysis/*.h Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT
)


# Build the shared library of the package:
atlas_add_library( smZInvAnalysisLib
   smZInvAnalysis/*.h Root/*.h Root/*.cxx ${smZInvAnalysisLibCintDict}
   PUBLIC_HEADERS smZInvAnalysis
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   AsgTools PATCoreLib EventLoop EventLoopAlgs EventLoopGrid SampleHandler AsgAnalysisInterfaces AnaAlgorithmLib AthContainers CxxUtils
   xAODRootAccess xAODRootAccessInterfaces xAODBase xAODCore xAODEgamma xAODTau xAODJet xAODMuon xAODMissingET
   xAODEventInfo xAODTracking xAODCaloEvent xAODTruth xAODCutFlow xAODTrigger xAODAssociations
   InDetTrackSelectionToolLib PhotonEfficiencyCorrectionLib EventPrimitives
   GoodRunsListsLib PileupReweightingLib PathResolver TrigConfxAODLib TrigDecisionToolLib TrigConfInterfaces TriggerMatchingToolLib TrigGlobalEfficiencyCorrectionLib
   MuonMomentumCorrectionsLib FourMomUtils IsolationSelectionLib IsolationCorrectionsLib
   ElectronPhotonSelectorToolsLib TauAnalysisToolsLib ElectronEfficiencyCorrectionLib
   JetCalibToolsLib xAODTriggerCnvLib xAODMetaDataCnv xAODMetaData IsolationToolLib RecoToolInterfaces
   JetResolutionLib JetUncertaintiesLib JetSelectorToolsLib JetMomentToolsLib MuonSelectorToolsLib
   MuonEfficiencyCorrectionsLib ElectronPhotonFourMomentumCorrectionLib ElectronPhotonShowerShapeFudgeToolLib
   AssociationUtilsLib METInterface METUtilitiesLib egammaLayerRecalibTool egammaMVACalibLib TrigConfxAODLib
   JetJvtEfficiencyLib InDetTrackSystematicsToolsLib
)


# Add the dictionary:
atlas_add_dictionary (smZInvAnalysisDict
   smZInvAnalysis/smZInvAnalysisDict.h
   smZInvAnalysis/selection.xml
   LINK_LIBRARIES smZInvAnalysisLib
)

# Add share directory (i.e. GRL xml files, prw root files.. )
atlas_install_data( share/* )

# Executable(s) in the package:
atlas_add_executable( testAnalRun util/testAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   smZInvAnalysisLib PATInterfaces xAODRootAccess RootCoreUtils SampleHandler EventLoop
)
atlas_add_executable( batchTestAnalRun util/batchTestAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   smZInvAnalysisLib PATInterfaces xAODRootAccess RootCoreUtils SampleHandler EventLoop
)
atlas_add_executable( submitBatchAnalRun util/submitBatchAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   smZInvAnalysisLib PATInterfaces xAODRootAccess RootCoreUtils SampleHandler EventLoop
)
atlas_add_executable( gridAnalRun util/gridAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} smZInvAnalysisLib PATInterfaces xAODRootAccess
   RootCoreUtils SampleHandler EventLoop EventLoopGrid EventLoopAlgs
)
atlas_add_executable( submitGridAnalRun util/submitGridAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} smZInvAnalysisLib PATInterfaces xAODRootAccess
   RootCoreUtils SampleHandler EventLoop EventLoopGrid EventLoopAlgs
)
atlas_add_executable( waitAnalRun util/waitAnalRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} smZInvAnalysisLib PATInterfaces xAODRootAccess
   RootCoreUtils SampleHandler EventLoop EventLoopGrid EventLoopAlgs
)


# Install files from the package:
atlas_install_joboptions( share/*.py )
