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

#ifndef GeometryCOSI_EMXwall_H
#define GeometryCOSI_EMXwall_H
#include "globals.hh"


class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4Region;
class G4UserLimits;

#include "G4VisAttributes.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4Material.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4Element.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "MaterialsDefinition.hh"

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

//Factory
#include "GeoClassFactory.hh"


class GeometryCOSI_EMXwall: public G4VUserDetectorConstruction {
public:

    GeometryCOSI_EMXwall();
    ~GeometryCOSI_EMXwall();

    G4VPhysicalVolume* Construct();
    void SetPhysicalWorld(G4VPhysicalVolume* World_phys);
    void DefineMaterials();
    G4VPhysicalVolume* ConstructGeometry(G4VPhysicalVolume* World_phys);

protected:

    void DefineSensitiveDetector();
    MaterialsDefinition* materials;

    // World
    //Physical Volumes
    G4VPhysicalVolume* World_phys;
    G4LogicalVolume* World_log;

    //Materials

    G4Material* chamber_mat;
    G4Material* CsI_mat;
    G4Material* SiPad_mat;
    G4Material* boro_glass_mat;
    G4Material* bialkali_mat;        
    G4Material* Al_mat;
    G4Material* PMTInside_mat;
    G4Material* bgo_mat;
    G4Material* ej560_mat;
    G4Material* refl1_mat;
    G4Material* refl2_mat;
    G4Material* refl3_mat;
    G4Material* SiPM_mat;
    G4Material* sy184_mat;
    G4Material* tefl_mat;
    G4Material* plastic_mat;
    G4Material* wood_mat;
    G4Material* wood2_mat;
    G4Material* coll_mat;
    G4Material* glass_mat;
    G4Material* coat_mat;
    G4Material* foam_mat;
    G4Material* rubber_mat;
    G4Material* steel_mat;
    G4Material* peek_mat;

private:
    
    G4UserLimits* target_limit;            // pointer to user step limits
    static DerivedRegister<GeometryCOSI_EMXwall> reg;
};

#endif
