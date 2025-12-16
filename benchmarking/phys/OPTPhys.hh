//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
// Copyright 2025 Valentina Fioretti
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
//
//
// ***************************************************************************
//                          OPTPhys.hh  -  description
//                             -------------------
//    Author		: Valentina Fioretti, Andrea Bulgarelli
//    creation date	: 10/02/2015
//    email		: fioretti@iasfbo.inaf.it, bulgarelli@iasfbo.inaf.it
// ***************************************************************************/



#if defined(GEANT4_11_1)


#ifndef OPTPhys_h
#define OPTPhys_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

#include "PhysClassFactory.hh"

class G4VPhysicsConstructor;

class OPTPhys: public G4VModularPhysicsList
{
public:
    
    explicit OPTPhys();
    ~OPTPhys() override;


    void SetCuts() override;
    
private:

    
    static PhysDerivedRegister<OPTPhys> reg;

    
};


#endif
#endif
