smZinvAnalysis package
=========================

Description of the package
------------------------------

This package contains the slim/skim code to create a mini-xAOD on (D)xAOD for Z->ll and Z->vv analysis.

It is based on https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/SoftwareTutorialxAODAnalysisInROOT
and https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/SoftwareTutorialAnalysisInGitReleases


Instruction for running in AnalysisBase 21.2 with cmake
------------------------------



## setup

#### 1. create working directory and subdirectories

	mkdir WorkDir
	cd WorkDir
	mkdir build run

#### 2. Setup shell environment for git

**Git tutorial**: https://atlassoftwaredocs.web.cern.ch/gittutorial

    setupATLAS
    lsetup git

**Configure git**: If you are using git for the first time, it is important to set your git name and email address which will be used by git as author for your commits (if you do not specify someone else explicitly).

You can check that your name and email address are properly set by running:

    git config --list
    
Look for user.name and user.email. If it is not yet set, please run the following commands

    git config --global user.name "Your Name"
    git config --global user.email "your.name@cern.ch"


#### 3. clone main analysis package

	git clone https://gitlab.cern.ch/tufts-atlas/smZinvAnalysis.git

#### 4. setup the Analysis Release

    cd build/
    asetup 21.2,AnalysisBase,latest
	
#### 5. compile it

    cmake ../smZinvAnalysis/
    make

#### 6. setup environment

**Very Important**: Since you created a new package you also must call

    source */setup.sh

**Very Important**: Every time you start a new session, don't forget to do

	cd build/
	setupATLAS
	asetup --restore
	source */setup.sh
	
before trying to run the code (assuming you have already compiled it before and you did not edit the source meanwhile)



## run skim package

### make sure you are in run/ directory

    cd ../run/
	# load submission script testSkimRun.cxx
	testSkimRun submitDir

Each time you edit the RunZllg.cxx code, don't forget to do once again

    cd ../build/
    make
    
Or alternatively you can also use the --build option of cmake:

    cmake --build ../build/

After compiling, and every time you want to run your code :

	source ../build/*/setup.sh

**Very Important**: Note that submitDir is the directory where the output of your job is stored. If you want to run again, you either have to remove that directory or pass a different name into ATestRun.cxx.


the main function in the smZinvAnalysis/smZInvSkim/util/testSkimRun.cxx script is the following
    	
    // Rel. 21
    // MC16c (EXOT5)
    const char* inputFilePath = gSystem->ExpandPathName ("/cluster/tufts/atlas16/hson02/Dataset/Rel21/MC");
    //SH::ScanDir().filePattern("").scan(sh,inputFilePath); // 364109.Zmumu_MAXHTPTV280_500_CVetoBVeto
    SH::ScanDir().filePattern("DAOD_EXOT5.11892347._000020.pool.root.1").scan(sh,inputFilePath); // 364123.Zee_MAXHTPTV280_500_CVetoBVeto
    
    
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
    //job.options()->setDouble (EL::Job::optMaxEvents, 500); // for testing


For a test run, comment out the last line in order to limit to run over the first 500 events only.

    job.options()->setDouble (EL::Job::optMaxEvents, 500);


## Submit to the grid

#### 1. setup
    lsetup rucio
    voms-proxy-init -voms atlas
    lsetup panda

#### 2. run
    cd ../run/
	# load submission script gridSkimRun.cxx
	gridSkimRun submitDir


