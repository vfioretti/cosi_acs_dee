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


#if defined(GEANT4_11_1)


#ifndef OPTPhys_COSI_h
#define OPTPhys_COSI_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

#include "PhysClassFactory.hh"

class G4VPhysicsConstructor;

class OPTPhys_COSI: public G4VModularPhysicsList
{
public:
    
    explicit OPTPhys_COSI();
    ~OPTPhys_COSI() override;


    void SetCuts() override;
    
private:

    
    static PhysDerivedRegister<OPTPhys_COSI> reg;

    
};


#endif
#endif
