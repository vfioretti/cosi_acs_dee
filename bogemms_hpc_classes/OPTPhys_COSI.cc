// Copyright 2025 Valentina Fioretti, Alex Ciabattoni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// **********************************************************************

#if defined (GEANT4_11_1)

#include "OPTPhys_COSI.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UnitsTable.hh"
#include "G4LossTableManager.hh"
#include "BoGEMMSGlobalMemory.hh"

#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmLivermorePolarizedPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"


// Regions
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"

OPTPhys_COSI::OPTPhys_COSI()
{
    G4LossTableManager::Instance();
    verboseLevel    = 1;
    
    //cut off
    G4double inputCutValue = 1.*mm;
    gm.config->readInto(inputCutValue, "PHYS.DEFAULT.CUT");
    G4cout << "PHYS.DEFAULT.CUT: " << inputCutValue << G4endl;
    
    G4double defaultCutValue = inputCutValue*mm;
    
    
    // EM Physics
    RegisterPhysics( new G4EmLivermorePolarizedPhysics());
   
    // Synchroton Radiation & GN Physics
    RegisterPhysics( new G4EmExtraPhysics() );
   
    // Decays
    RegisterPhysics( new G4DecayPhysics() );
    RegisterPhysics( new G4RadioactiveDecayPhysics() );

    // Hadron Elastic scattering
    RegisterPhysics( new G4HadronElasticPhysics() );
   
    // Hadron Physics
    RegisterPhysics(  new G4HadronPhysicsFTFP_BERT());

    // Stopping Physics
    RegisterPhysics( new G4StoppingPhysics() );

    // Ion Physics
    RegisterPhysics( new G4IonPhysics());

    // Neutron tracking cut
    RegisterPhysics( new G4NeutronTrackingCut());

    /* optical physics */
    G4int setOptPhys = 1;
    gm.config->readInto(setOptPhys, "PHYS.COSI.OPT.ACTIVATE");
    G4cout << "PHYS.COSI.OPT.ACTIVATE: " << setOptPhys << G4endl;

    if (setOptPhys == 1) {
        G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
        auto opticalParams               = G4OpticalParameters::Instance();

        //opticalParams->SetScintEnhancedTimeConstants(true);
        //opticalParams->SetScintFiniteRiseTime(true); // for the rise time
        opticalParams->SetScintTrackSecondariesFirst(true); // default
        
        opticalParams->SetWLSTimeProfile("delta"); // default

        opticalParams->SetCerenkovMaxPhotonsPerStep(100);
        opticalParams->SetCerenkovMaxBetaChange(10.0);
        opticalParams->SetCerenkovTrackSecondariesFirst(true);
        
        RegisterPhysics(opticalPhysics);
    }
    
    /*
    ///COSI (2)
    G4VModularPhysicsList* physicsList = new G4VModularPhysicsList();
    physicsList->RegisterPhysics(new G4EmStandardPhysics());
    physicsList->RegisterPhysics(new G4OpticalPhysics());
    runManager->SetUserInitialization(physicsList);
     
    // COSI (3)
    G4VModularPhysicsList* physicsList = new FTFP_BERT;
    physicsList->ReplacePhysics(new G4EmLivermorePolarizedPhysics());
    runManager->SetUserInitialization(physicsList);
    */

}


OPTPhys_COSI::~OPTPhys_COSI()
{
    
}

PhysDerivedRegister<OPTPhys_COSI> OPTPhys_COSI::reg("OPTPhys_COSI");


void OPTPhys_COSI::SetCuts()
{

    SetDefaultCutValue(defaultCutValue);
    
    if (verboseLevel>0) DumpCutValuesTable();
}


#endif

