# Set the name of the package:
atlas_subdir( smZInvSkim )

# Set up which packages this package depends on:
atlas_depends_on_subdirs(
   PUBLIC
   Control/AthToolSupport/AsgTools
   PhysicsAnalysis/AnalysisCommon/PATCore
   PhysicsAnalysis/D3PDTools/EventLoop
   PhysicsAnalysis/D3PDTools/EventLoopGrid
   PhysicsAnalysis/D3PDTools/EventLoopAlgs
   PhysicsAnalysis/Interfaces/AsgAnalysisInterfaces
   Control/AthContainers
   Control/CxxUtils
   Control/xAODRootAccess
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
   Event/xAOD/xAODBTaggingEfficiency
   Event/xAOD/xAODTrigger
   Event/xAOD/xAODTriggerCnv
   Event/xAOD/xAODCutFlow
   Event/xAOD/xAODMetaDataCnv
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools
   PhysicsAnalysis/TauID/TauAnalysisTools
   PhysicsAnalysis/ElectronPhotonID/ElectronEfficiencyCorrection
   Trigger/TrigAnalysis/TrigDecisionTool
   Trigger/TrigConfiguration/TrigConfxAOD
   Trigger/TrigAnalysis/TriggerMatchingTool
   Tools/PathResolver
   PhysicsAnalysis/AnalysisCommon/IsolationSelection
   PhysicsAnalysis/AnalysisCommon/PileupReweighting
   DataQuality/GoodRunsLists
   Reconstruction/Jet/JetCalibTools
)

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Physics REQUIRED )

# Generate a dictionary for the library:
atlas_add_root_dictionary( smZInvSkimLib smZInvSkimLibCintDict
   ROOT_HEADERS smZInvSkim/*.h Root/LinkDef.h
   EXTERNAL_PACKAGES ROOT
)


# Build the shared library of the package:
atlas_add_library( smZInvSkimLib
   smZInvSkim/*.h Root/*.h Root/*.cxx ${smZInvSkimLibCintDict}
   PUBLIC_HEADERS smZInvSkim
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   AsgTools PATCoreLib EventLoop AsgAnalysisInterfaces AnaAlgorithmLib AthContainers CxxUtils
   xAODRootAccess xAODBase xAODCore xAODEgamma xAODTau xAODJet xAODMuon xAODMissingET
   xAODEventInfo xAODTracking xAODCaloEvent xAODTruth xAODCutFlow xAODTrigger
   GoodRunsListsLib PileupReweightingLib PathResolver TrigConfxAODLib TrigDecisionToolLib TriggerMatchingToolLib
   MuonMomentumCorrectionsLib FourMomUtils IsolationSelectionLib IsolationCorrectionsLib
   ElectronPhotonSelectorToolsLib TauAnalysisToolsLib ElectronEfficiencyCorrectionLib
   JetCalibToolsLib xAODTriggerCnvLib xAODMetaDataCnv xAODMetaData
)


# Add the dictionary:
atlas_add_dictionary (smZInvSkimDict
   smZInvSkim/smZInvSkimDict.h
   smZInvSkim/selection.xml
   LINK_LIBRARIES smZInvSkimLib
)


# Executable(s) in the package:
atlas_add_executable( testRun util/testRun.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES}
   smZInvSkimLib PATInterfaces xAODRootAccess RootCoreUtils SampleHandler EventLoop
)


# Install files from the package:
atlas_install_joboptions( share/*.py )
