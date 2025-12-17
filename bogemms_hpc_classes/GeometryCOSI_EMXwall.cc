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

#include "GeometryCOSI_EMXwall.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Polyhedra.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4Trap.hh"
#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SubtractionSolid.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "BoGEMMSGlobalMemory.hh"
#include "globals.hh"
#include "G4NistManager.hh"
#include "G4PVReplica.hh"
#include "MaterialsDefinition.hh"
#include "G4PVParameterised.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "World.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UserLimits.hh"

// Regions
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"

#include <vector>

//GDML
//#include "G4Writer/G4GDMLWriter.h"

// CADMesh
#include "CADMesh.hh"

GeometryCOSI_EMXwall::GeometryCOSI_EMXwall()
: G4VUserDetectorConstruction(), target_limit(NULL)
{

	World_phys = 0;
	World_log = 0;
	materials = new MaterialsDefinition;

}

GeometryCOSI_EMXwall::~GeometryCOSI_EMXwall() {

    delete target_limit;

}

DerivedRegister<GeometryCOSI_EMXwall> GeometryCOSI_EMXwall::reg("GeometryCOSI_EMXwall");


G4VPhysicalVolume* GeometryCOSI_EMXwall::Construct() {

    //DefineMaterials();
    World_phys = gm.ConstructWorld();
    World_log = gm.World_log;
    World_phys = ConstructGeometry(World_phys);
    return World_phys;
	
}

G4VPhysicalVolume* GeometryCOSI_EMXwall::ConstructGeometry(G4VPhysicalVolume* World_phys) {

    /* experiment type */
    // 3: BGO + SiPM prototype
    G4int ACS_exp_type = 3;

    if (ACS_exp_type == 3) { /* SiPM model */

        /* --------------------- PARAMETERS ----------------------- */

        /* geometry case: 8 for NRL, 9 for SSL */
        G4int setGeomType = 0.;
        gm.config->readInto(setGeomType, "GEOM.COSI.ACS.TYPE");
        G4cout << "GEOM.COSI.ACS.TYPE: " << setGeomType << G4endl;

        /* set BGO absorption length case: 1: 10cm, 2: 30.3cm, 3: 200cm, 4: 300cm, 5: 1000cm, 6: 500cm */
        G4int BGO_absl_type = 0; // absorption length case
        gm.config->readInto(BGO_absl_type, "PHYS.COSI.BGO.ABSL.TYPE");
        G4cout << "PHYS.COSI.BGO.ABSL.TYPE: " << BGO_absl_type << G4endl;

        /* set BGO light yield */
        G4double BGO_light_yield = 8.2; // default value (in photons/keV)
        gm.config->readInto(BGO_light_yield, "PHYS.COSI.BGO.LIGHTYIELD");
        G4cout << "PHYS.COSI.BGO.LIGHTYIELD: " << BGO_light_yield << G4endl;

        /* set if sources are collimated */
        G4int is_collimated = 0;
        gm.config->readInto(is_collimated, "GEOM.COSI.COLLIMATED");
        G4cout << "GEOM.COSI.COLLIMATED: " << is_collimated << G4endl;

        /* set the source type */
        G4int source = 0;
        gm.config->readInto(source, "GEOM.COSI.SOURCE");
        G4cout << "GEOM.COSI.SOURCE: " << source << G4endl;      

        /* optical surface case */
        G4int setReflSurfaceType = 0;
        gm.config->readInto(setReflSurfaceType, "PHYS.ACS.OPTSURFACE.WRAPPER");
        G4cout << "PHYS.ACS.OPTSURFACE.WRAPPER: " << setReflSurfaceType << G4endl;
        /* optical surface case (SiPM face) */
        G4int setReflSurface2Type = 0;
        gm.config->readInto(setReflSurface2Type, "PHYS.ACS.OPTSURFACE2.WRAPPER");
        G4cout << "PHYS.ACS.OPTSURFACE2.WRAPPER: " << setReflSurface2Type << G4endl;

        /* set the x-y source coordinates (position of the collimator) */
        G4double beamX = 0.;
        G4double beamY = 0.;
        gm.config->readInto(beamX, "GEOM.COSI.BEAMX");
        G4cout << "GEOM.COSI.BEAMX: " << beamX << G4endl;
        gm.config->readInto(beamY, "GEOM.COSI.BEAMY");
        G4cout << "GEOM.COSI.BEAMY: " << beamY << G4endl;

        /* set volumes sensitive */
        G4int chamber_sens = 0;
        gm.config->readInto(chamber_sens, "SENSITIVE.CHAMBER");
        G4cout << "SENSITIVE.CHAMBER: " << chamber_sens << G4endl;
        G4int plastic_sens = 0;
        gm.config->readInto(plastic_sens, "SENSITIVE.PLASTIC");
        G4cout << "SENSITIVE.PLASTIC: " << plastic_sens << G4endl;
        G4int table_sens = 0;
        gm.config->readInto(table_sens, "SENSITIVE.TABLE");
        G4cout << "SENSITIVE.TABLE: " << table_sens << G4endl;
        G4int scint_sens = 0;
        gm.config->readInto(scint_sens, "SENSITIVE.SCINT");
        G4cout << "SENSITIVE.SCINT: " << scint_sens << G4endl;
        G4int ej560_sens = 0;
        gm.config->readInto(ej560_sens, "SENSITIVE.EJ560");
        G4cout << "SENSITIVE.EJ560: " << ej560_sens << G4endl;
        G4int SiPM_sens = 0;
        gm.config->readInto(SiPM_sens, "SENSITIVE.SIPM");
        G4cout << "SENSITIVE.SIPM: " << SiPM_sens << G4endl;
        G4int coll_sens = 0;
        gm.config->readInto(coll_sens, "SENSITIVE.COLL");
        G4cout << "SENSITIVE.COLL: " << coll_sens << G4endl;
        G4int coat_sens = 0;
        gm.config->readInto(coat_sens, "SENSITIVE.BLACK.COAT");
        G4cout << "SENSITIVE.BLACK.COAT: " << coat_sens << G4endl;        
        G4int foam_sens = 0;
        gm.config->readInto(foam_sens, "SENSITIVE.FOAM");
        G4cout << "SENSITIVE.FOAM: " << foam_sens << G4endl;   
        G4int housing_sens = 0;
        gm.config->readInto(housing_sens, "SENSITIVE.HOUSING");
        G4cout << "SENSITIVE.HOUSING: " << housing_sens << G4endl;   
        G4int cap_sens = 0;
        gm.config->readInto(cap_sens, "SENSITIVE.CAP");
        G4cout << "SENSITIVE.CAP: " << cap_sens << G4endl;  
        G4int mat_sens = 0;
        gm.config->readInto(mat_sens, "SENSITIVE.MAT");
        G4cout << "SENSITIVE.MAT: " << mat_sens << G4endl;         
        G4int pole_sens = 0;
        gm.config->readInto(pole_sens, "SENSITIVE.POLE");
        G4cout << "SENSITIVE.POLE: " << pole_sens << G4endl;
        G4int stake_sens = 0;
        gm.config->readInto(stake_sens, "SENSITIVE.STAKE");
        G4cout << "SENSITIVE.STAKE: " << stake_sens << G4endl; 
        G4int holder_sens = 0;
        gm.config->readInto(holder_sens, "SENSITIVE.HOLDER");
        G4cout << "SENSITIVE.HOLDER: " << holder_sens << G4endl; 
        G4int base_sens = 0;
        gm.config->readInto(base_sens, "SENSITIVE.BASE");
        G4cout << "SENSITIVE.BASE: " << base_sens << G4endl; 
        G4int screw_sens = 0;
        gm.config->readInto(screw_sens, "SENSITIVE.SCREW");
        G4cout << "SENSITIVE.SCREW: " << screw_sens << G4endl; 
        G4int block_sens = 0;
        gm.config->readInto(block_sens, "SENSITIVE.BLOCK");
        G4cout << "SENSITIVE.BLOCK: " << block_sens << G4endl; 
        G4int sourceCase_sens = 0;
        gm.config->readInto(sourceCase_sens, "SENSITIVE.SOURCE.CASE");
        G4cout << "SENSITIVE.SOURCE.CASE: " << sourceCase_sens << G4endl; 
        G4int frame_sens = 0;
        gm.config->readInto(frame_sens, "SENSITIVE.FRAME");
        G4cout << "SENSITIVE.FRAME: " << frame_sens << G4endl;  
        G4int aligner_sens = 0;
        gm.config->readInto(aligner_sens, "SENSITIVE.ALIGNER");
        G4cout << "SENSITIVE.ALIGNER: " << aligner_sens << G4endl;  
        G4int casing_sens = 0;
        gm.config->readInto(casing_sens, "SENSITIVE.CASING");
        G4cout << "SENSITIVE.CASING: " << casing_sens << G4endl;  
        G4int foam_source_sens = 0;
        gm.config->readInto(foam_source_sens, "SENSITIVE.FOAM.SOURCE");
        G4cout << "SENSITIVE.FOAM.SOURCE: " << foam_source_sens << G4endl;  
        G4int refl3_sens_BGO1 = 0;
        gm.config->readInto(refl3_sens_BGO1, "SENSITIVE.REFL3_BGO1");
        G4cout << "SENSITIVE.REFL3_BGO1: " << refl3_sens_BGO1 << G4endl;
        G4int refl2_sens_BGO1 = 0;
        gm.config->readInto(refl2_sens_BGO1, "SENSITIVE.REFL2");
        G4cout << "SENSITIVE.REFL2_BGO1: " << refl2_sens_BGO1 << G4endl;
        G4int refl22_sens_BGO1 = 0;
        gm.config->readInto(refl22_sens_BGO1, "SENSITIVE.REFL22_BGO1");
        G4cout << "SENSITIVE.REFL22_BGO1: " << refl22_sens_BGO1 << G4endl;
        G4int refl1_sens_BGO1 = 0;
        gm.config->readInto(refl1_sens_BGO1, "SENSITIVE.REFL1_BGO1");
        G4cout << "SENSITIVE.REFL1_BGO1: " << refl1_sens_BGO1 << G4endl;
        G4int refl11_sens_BGO1 = 0;
        gm.config->readInto(refl11_sens_BGO1, "SENSITIVE.REFL11_BGO1");
        G4cout << "SENSITIVE.REFL11_BGO1: " << refl11_sens_BGO1 << G4endl;
        G4int scint_sens_BGO1 = 0;
        gm.config->readInto(scint_sens_BGO1, "SENSITIVE.SCINT_BGO1");
        G4cout << "SENSITIVE.SCINT_BGO1: " << scint_sens_BGO1 << G4endl;
        G4int ej560_sens_BGO1 = 0;
        gm.config->readInto(ej560_sens_BGO1, "SENSITIVE.EJ560_BGO1");
        G4cout << "SENSITIVE.EJ560_BGO1: " << ej560_sens_BGO1 << G4endl;
        G4int SiPM_sens_BGO1 = 0;
        gm.config->readInto(SiPM_sens_BGO1, "SENSITIVE.SIPM_BGO1");
        G4cout << "SENSITIVE.SIPM_BGO1: " << SiPM_sens_BGO1 << G4endl;
        G4int refl3_sens_BGO2 = 0;
        gm.config->readInto(refl3_sens_BGO2, "SENSITIVE.REFL3_BGO2");
        G4cout << "SENSITIVE.REFL3_BGO2: " << refl3_sens_BGO2 << G4endl;
        G4int refl2_sens_BGO2 = 0;
        gm.config->readInto(refl2_sens_BGO2, "SENSITIVE.REFL2");
        G4cout << "SENSITIVE.REFL2_BGO2: " << refl2_sens_BGO2 << G4endl;
        G4int refl22_sens_BGO2 = 0;
        gm.config->readInto(refl22_sens_BGO2, "SENSITIVE.REFL22_BGO2");
        G4cout << "SENSITIVE.REFL22_BGO2: " << refl22_sens_BGO2 << G4endl;
        G4int refl1_sens_BGO2 = 0;
        gm.config->readInto(refl1_sens_BGO2, "SENSITIVE.REFL1_BGO2");
        G4cout << "SENSITIVE.REFL1_BGO2: " << refl1_sens_BGO2 << G4endl;
        G4int refl11_sens_BGO2 = 0;
        gm.config->readInto(refl11_sens_BGO2, "SENSITIVE.REFL11_BGO2");
        G4cout << "SENSITIVE.REFL11_BGO2: " << refl11_sens_BGO2 << G4endl;
        G4int scint_sens_BGO2 = 0;
        gm.config->readInto(scint_sens_BGO2, "SENSITIVE.SCINT_BGO2");
        G4cout << "SENSITIVE.SCINT_BGO2: " << scint_sens_BGO2 << G4endl;
        G4int ej560_sens_BGO2 = 0;
        gm.config->readInto(ej560_sens_BGO2, "SENSITIVE.EJ560_BGO2");
        G4cout << "SENSITIVE.EJ560_BGO2: " << ej560_sens_BGO2 << G4endl;
        G4int SiPM_sens_BGO2 = 0;
        gm.config->readInto(SiPM_sens_BGO2, "SENSITIVE.SIPM_BGO2");
        G4cout << "SENSITIVE.SIPM_BGO2: " << SiPM_sens_BGO2 << G4endl;
        G4int refl3_sens_BGO3 = 0;
        gm.config->readInto(refl3_sens_BGO3, "SENSITIVE.REFL3_BGO3");
        G4cout << "SENSITIVE.REFL3_BGO3: " << refl3_sens_BGO3 << G4endl;
        G4int refl2_sens_BGO3 = 0;
        gm.config->readInto(refl2_sens_BGO3, "SENSITIVE.REFL2");
        G4cout << "SENSITIVE.REFL2_BGO3: " << refl2_sens_BGO3 << G4endl;
        G4int refl22_sens_BGO3 = 0;
        gm.config->readInto(refl22_sens_BGO3, "SENSITIVE.REFL22_BGO3");
        G4cout << "SENSITIVE.REFL22_BGO3: " << refl22_sens_BGO3 << G4endl;
        G4int refl1_sens_BGO3 = 0;
        gm.config->readInto(refl1_sens_BGO3, "SENSITIVE.REFL1_BGO3");
        G4cout << "SENSITIVE.REFL1_BGO3: " << refl1_sens_BGO3 << G4endl;
        G4int refl11_sens_BGO3 = 0;
        gm.config->readInto(refl11_sens_BGO3, "SENSITIVE.REFL11_BGO3");
        G4cout << "SENSITIVE.REFL11_BGO3: " << refl11_sens_BGO3 << G4endl;
        G4int scint_sens_BGO3 = 0;
        gm.config->readInto(scint_sens_BGO3, "SENSITIVE.SCINT_BGO3");
        G4cout << "SENSITIVE.SCINT_BGO3: " << scint_sens_BGO3 << G4endl;
        G4int ej560_sens_BGO3 = 0;
        gm.config->readInto(ej560_sens_BGO3, "SENSITIVE.EJ560_BGO3");
        G4cout << "SENSITIVE.EJ560_BGO3: " << ej560_sens_BGO3 << G4endl;
        G4int SiPM_sens_BGO3 = 0;
        gm.config->readInto(SiPM_sens_BGO3, "SENSITIVE.SIPM_BGO3");
        G4cout << "SENSITIVE.SIPM_BGO3: " << SiPM_sens_BGO3 << G4endl;
        

        /* --------------------- LABORATORY CHAMBER ----------------------- */

        G4int vacuum_flag = 0;
        if (vacuum_flag){
            chamber_mat = materials->GetMaterial(12);
        }
        if (!vacuum_flag){
            chamber_mat = materials->GetMaterial(45);
        }
        
        /* chamber */
        G4double chamber_side = 3.0*m;
        G4Box* chamber = new G4Box("chamber",
                                   chamber_side/2.,
                                   chamber_side/2.,
                                   chamber_side/2.);
        
        G4LogicalVolume* log_chamber = new G4LogicalVolume(chamber, chamber_mat, "log_chamber");
        G4VPhysicalVolume* phys_chamber = new G4PVPlacement(0, G4ThreeVector(0.0,0.0,0.0), log_chamber, "phys_chamber", World_log, false, 0);

        if (chamber_sens == 1)
            gm.AddXYZDetector(log_chamber);
        
        /* set chamber invisible */
        //log_chamber->SetVisAttributes (G4VisAttributes::GetInvisible);
        G4VisAttributes* VisChamber = new G4VisAttributes();
        VisChamber->SetVisibility(false);
        log_chamber->SetVisAttributes(VisChamber);



        /* --------------------- MATERIALS ----------------------- */

        bgo_mat = materials->GetMaterial(21);
        Al_mat = materials->GetMaterial(67);
        plastic_mat = materials->GetMaterial(71);
        wood_mat = materials->GetMaterial(72);
        wood2_mat = materials->GetMaterial(76);
        ej560_mat = materials->GetMaterial(63); /* optical coupler */
        sy184_mat = materials->GetMaterial(63);
        glass_mat = materials->GetMaterial(65);
        refl1_mat = materials->GetMaterial(71); /* VM 2000 */
        refl2_mat = materials->GetMaterial(62); /* white Tetratex PTFE */
        refl3_mat = materials->GetMaterial(71); /* 3M Super 33+ electrical tape */
        tefl_mat = materials->GetMaterial(62);
        SiPM_mat = materials->GetMaterial(12);
        coll_mat = materials->GetMaterial(3);
        coat_mat = materials->GetMaterial(44);
        foam_mat = materials->GetMaterial(73);
        rubber_mat = materials->GetMaterial(74);
        steel_mat = materials->GetMaterial(34);
        peek_mat = materials->GetMaterial(75);


        /* --------------------- MATERIAL OPTICAL PROPERTIES ----------------------- */

        /** Here we define the optical properties of the materials **/

        /* Scintillator */

        // Optical light emission spectrum
        std::vector<G4double> BGO_energy_emission = {2.087824*eV,   2.10417557*eV, 2.13482837*eV, 2.16058022*eV, 2.18696093*eV, 2.20792891*eV,
            2.22930285*eV, 2.25109467*eV, 2.27011532*eV, 2.29271638*eV, 2.31577199*eV, 2.33929601*eV,
            2.35984319*eV, 2.37724359*eV, 2.40921966*eV, 2.43469094*eV, 2.46070656*eV, 2.47963216*eV,
            2.50273073*eV, 2.52626369*eV, 2.55428436*eV, 2.5829336*eV,  2.61223279*eV, 2.64654219*eV,
            2.6862337*eV,  2.74571426*eV, 2.78846019*eV, 2.81770465*eV, 2.86783282*eV, 2.90923808*eV,
            2.94108524*eV, 2.95727173*eV, 2.9846488*eV,  3.01253749*eV, 3.04669969*eV, 3.09347304*eV,
            3.14170493*eV, 3.19146465*eV, 3.23631558*eV, 3.26913159*eV, 3.31620815*eV, 3.36466035*eV,
           3.45109991*eV, 3.54989816*eV, 3.64625367*eV, 3.75672042*eV};

        std::vector<G4double> BGO_emission = {0.0666666666667, 0.0733333333333, 0.087619047619, 0.1, 0.115238095238, 0.126666666667, 0.139047619048, 
        	0.154285714286, 0.164761904762, 0.181904761905, 0.197142857143, 0.213333333333, 0.231428571429, 0.24380952381, 0.265714285714, 0.279047619048, 0.292380952381,
        	0.302857142857, 0.314285714286, 0.320952380952, 0.331428571429, 0.341904761905, 0.34380952381, 0.345714285714, 0.342857142857, 0.334285714286, 0.318095238095, 
        	0.299047619048, 0.269523809524, 0.248571428571, 0.222857142857, 0.202857142857, 0.180952380952, 0.157142857143, 0.130476190476, 0.106666666667, 0.0828571428571,
        	0.0628571428571, 0.0495238095238, 0.0409523809524, 0.0314285714286, 0.0219047619048, 0.0104761904762, 0.0047619047619, 0.00190476190476, 1.11022302463e-16};
        
        // BGO refractive index
        std::vector<G4double> BGO_energy_rindex = {1.23984198*eV, 1.24845633*eV, 1.25731871*eV, 1.2661785*eV,  1.27529519*eV, 1.28441105*eV,
         1.29379316*eV, 1.30317636*eV, 1.31283565*eV, 1.32249812*eV, 1.33244705*eV, 1.34240146*eV,
         1.35265327*eV, 1.36291303*eV, 1.37348176*eV, 1.38406116*eV, 1.39496173*eV, 1.40587593*eV,
         1.41712422*eV, 1.42838938*eV, 1.4400023*eV,  1.45163562*eV, 1.46363119*eV, 1.47565102*eV,
         1.48804847*eV, 1.50047439*eV, 1.51329426*eV, 1.5261472*eV,  1.53941145*eV, 1.55271382*eV,
         1.56644597*eV, 1.58022175*eV, 1.594447*eV,   1.60872192*eV, 1.62346731*eV, 1.63826901*eV,
         1.6535636*eV,  1.66892177*eV, 1.68479683*eV, 1.70074346*eV, 1.71723267*eV, 1.73380224*eV,
         1.75094194*eV, 1.76817168*eV, 1.78600113*eV, 1.8039313*eV,  1.82249299*eV, 1.84116719*eV,
         1.86050718*eV, 1.87997268*eV, 1.90014097*eV, 1.92044917*eV, 1.94150013*eV, 1.96270696*eV,
         1.98469983*eV, 2.00686627*eV, 2.02986572*eV, 2.05305843*eV, 2.07713517*eV, 2.10142709*eV,
         2.12665864*eV, 2.15212981*eV, 2.17860127*eV, 2.20533971*eV, 2.23314478*eV, 2.26124746*eV,
         2.29048953*eV, 2.32006359*eV, 2.350857*eV,   2.3820211*eV,  2.41449267*eV, 2.44737857*eV,
         2.4816693*eV,  2.51642376*eV, 2.55269093*eV, 2.58947783*eV, 2.62789738*eV, 2.66690037*eV,
         2.70766976*eV, 2.74909531*eV, 2.7924369*eV,  2.83651792*eV, 2.88268306*eV, 2.92968333*eV,
         2.97895719*eV, 3.0291766*eV,  3.08188413*eV, 3.13566511*eV, 3.19217813*eV, 3.24991346*eV,
         3.3106595*eV,  3.37280192*eV, 3.43827505*eV, 3.50534912*eV, 3.5761234*eV,  3.64874039*eV,
            3.72548673*eV, 3.80436325*eV, 3.88787076*eV, 3.97385251*eV, 4.06505569*eV};
        
        
        std::vector<G4double> BGO_rindex = {2.05601944, 2.05638898, 2.05677213, 2.05715818, 2.05755858, 2.05796215,
            2.05838086, 2.05880303, 2.0592412,  2.05968315, 2.06014202, 2.06060501,
            2.06108591, 2.06157133, 2.06207571, 2.06258503, 2.06311446, 2.0636493,
            2.0642055,  2.06476761, 2.06535242, 2.06594372, 2.06655917, 2.06718174,
            2.06783003, 2.06848614, 2.06916969, 2.06986182, 2.07058327, 2.07131414,
            2.07207638, 2.07284899, 2.0736552,  2.07447284, 2.07532652, 2.07619281,
            2.07709783, 2.07801676, 2.07897738, 2.0799534,  2.08097435, 2.08201235,
            2.08309888, 2.08420433, 2.08536229, 2.08654128, 2.08777719, 2.08903651,
            2.09035768, 2.09170496, 2.09311957, 2.09456336, 2.09608065, 2.09763062,
            2.09926098, 2.10092805, 2.10268329, 2.10447985, 2.10637338, 2.10831355,
            2.11036068, 2.11246061, 2.11467889, 2.11695711, 2.11936674, 2.12184464,
            2.12446894, 2.12717132, 2.13003743, 2.13299315, 2.13613274, 2.13937563,
            2.14282593, 2.14639584, 2.15020084, 2.15414504, 2.1583571,  2.16273204,
            2.16741392, 2.17228756, 2.17751512, 2.18296991, 2.18883564, 2.19497261,
            2.20159037, 2.20853456, 2.21604609, 2.22395404, 2.23253789, 2.24160817,
            2.2514924,  2.26198049, 2.27346095, 2.28570119, 2.29916857, 2.31360684,
            2.32958785, 2.34683227, 2.36605436, 2.38695646, 2.41045369};

        G4MaterialPropertiesTable* BGO_MPT = new G4MaterialPropertiesTable();
        
        // properties independent of energy
        BGO_MPT->AddConstProperty("SCINTILLATIONYIELD", BGO_light_yield/keV);
        
        // decay time
        BGO_MPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 300.*ns);
        // resolution scale
        BGO_MPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
        // rise time
        //BGO_MPT->AddConstProperty("SCINTILLATIONRISETIME1", 0.*ns);
        
        // properties that depend on energy
        BGO_MPT->AddProperty("SCINTILLATIONCOMPONENT1", BGO_energy_emission, BGO_emission);
        BGO_MPT->AddProperty("RINDEX", BGO_energy_rindex, BGO_rindex, false, false);

        G4double BGO_absl_length;
        if (BGO_absl_type == 1) {
            BGO_absl_length = 10*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        if (BGO_absl_type == 2) {
            BGO_absl_length = 30.3*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        if (BGO_absl_type == 3) {
            BGO_absl_length = 200*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        if (BGO_absl_type == 4) {
            BGO_absl_length = 300*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        if (BGO_absl_type == 5) {
            BGO_absl_length = 1000*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        if (BGO_absl_type == 6) {
            BGO_absl_length = 500*cm;
            std::vector<G4double> BGO_Energy_abs  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> BGO_abs_length  = { BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length, BGO_absl_length };
            BGO_MPT->AddProperty("ABSLENGTH", BGO_Energy_abs, BGO_abs_length, false, false);
        }
        

        BGO_MPT->DumpTable();
        bgo_mat->SetMaterialPropertiesTable(BGO_MPT);

        /* Aluminum */

        // Aluminum refractive index
        std::vector<G4double> Al_Energy  = {1.24000318*eV,1.25000452*eV,1.26000202*eV,1.27000459*eV,1.27999544*eV,1.28999707
                *eV,1.30000627*eV,1.3100059*eV,1.32000595*eV,1.33000288*eV,1.33999307*eV,1.35000216
                *eV,1.35999779*eV,1.37000628*eV,1.37999419*eV,1.39000413*eV,1.40000224*eV,1.41000089
                *eV,1.419997*eV,1.4300039*eV,1.4400023*eV,1.45000583*eV,1.45999456*eV,1.46999986
                *eV,1.48000189*eV,1.4899977*eV,1.5000024*eV,1.50999523*eV,1.51999164*eV,1.530008
                *eV,1.5400042*eV,1.54999623*eV,1.56000099*eV,1.56999656*eV,1.58000023*eV,1.59000985
                *eV,1.60000256*eV,1.60999621*eV,1.62000965*eV,1.6299984*eV,1.64000262*eV,1.64999865
                *eV,1.66000614*eV,1.67000079*eV,1.68000269*eV,1.69000993*eV,1.69999724*eV,1.71000894
                *eV,1.71999609*eV,1.73000402*eV,1.740007*eV,1.7500028*eV,1.75998919*eV,1.76998913
                *eV,1.78000113*eV,1.78999781*eV,1.80000288*eV,1.8099883*eV,1.82000497*eV,1.82999806
                *eV,1.83999226*eV,1.85001341*eV,1.86000478*eV,1.86999183*eV,1.88000119*eV,1.89000302
                *eV,1.89999538*eV,1.91000568*eV,1.92000307*eV,1.9299855*eV,1.94001155*eV,1.94998897
                *eV,1.96000756*eV,1.97000442*eV,1.98000892*eV,1.98998778*eV,2.0000032*eV,2.0099896
                *eV,2.0200104*eV,2.02999866*eV,2.0399855*eV,2.05000328*eV,2.05998303*eV,2.06999129
                *eV,2.07999259*eV,2.08998531*eV,2.10000336*eV,2.11001018*eV,2.12000408*eV,2.12998331
                *eV,2.13998306*eV,2.15000257*eV,2.16000346*eV,2.16998387*eV,2.18001861*eV,2.18999185
                *eV,2.20001772*eV,2.21001762*eV,2.21998959*eV,2.23001184*eV,2.24000359*eV,2.2500036
                *eV,2.26001091*eV,2.26998294*eV,2.28000144*eV,2.28998187*eV,2.30000739*eV,2.30999196
                *eV,2.32002018*eV,2.33000448*eV,2.33998676*eV,2.3500104*eV,2.3599855*eV,2.37000035
                *eV,2.38000918*eV,2.39001076*eV,2.40000384*eV,2.40998714*eV,2.42000661*eV,2.43001447
                *eV,2.44000942*eV,2.44999009*eV,2.46000394*eV,2.47000156*eV,2.47998157*eV,2.48999254
                *eV,2.49998384*eV,2.51000483*eV,2.52000403*eV,2.52997997*eV,2.53998317*eV,2.55001334
                *eV,2.56001731*eV,2.56999354*eV,2.57999414*eV,2.59001877*eV,2.60001255*eV,2.60997386
                *eV,2.6200118*eV,2.63001566*eV,2.63998378*eV,2.65002775*eV,2.65997722*eV,2.67000169
                *eV,2.67998613*eV,2.68998717*eV,2.70000432*eV,2.70997789*eV,2.72002541*eV,2.73002749
                *eV,2.73998229*eV,2.75000995*eV,2.75998839*eV,2.76997762*eV,2.77997709*eV,2.78998624
                *eV,2.80000448*eV,2.81003124*eV,2.82000178*eV,2.82997874*eV,2.84002654*eV,2.85001491
                *eV,2.86000781*eV,2.87000459*eV,2.88000461*eV,2.89000719*eV,2.90001166*eV,2.91001733
                *eV,2.92002351*eV,2.9300295*eV,2.94003458*eV,2.94996784*eV,2.95996845*eV,2.96996595
                *eV,2.98003121*eV,2.9900207*eV,3.0000048*eV,3.00998273*eV,3.02002724*eV,3.02999092
                *eV,3.04002056*eV,3.04996675*eV,3.05997824*eV,3.06997966*eV,3.07997015*eV,3.09002588
                *eV,3.09999246*eV,3.11002354*eV,3.11996272*eV,3.12996563*eV,3.14003288*eV,3.15000504
                *eV,3.1599602*eV,3.16997848*eV,3.17997893*eV,3.18996059*eV,3.20000512*eV,3.21002999
                *eV,3.22003424*eV,3.23001689*eV,3.23997696*eV,3.24999865*eV,3.2599968*eV,3.26997042
                *eV,3.28000525*eV,3.29001455*eV,3.2999973*eV,3.31004081*eV,3.31996783*eV,3.33004401
                *eV,3.34000157*eV,3.35001887*eV,3.36000538*eV,3.36996*eV,3.37997379*eV,3.38995457
                *eV,3.39999447*eV,3.41000023*eV,3.41997072*eV,3.42999968*eV,3.43999219*eV,3.45004309
                *eV,3.45995977*eV,3.47003074*eV,3.47996515*eV,3.48995661*eV,3.5000056*eV,3.51001326
                *eV,3.51997838*eV,3.53000024*eV,3.53997826*eV,3.55001284*eV,3.56000225*eV,3.57004804
                *eV,3.58004731*eV,3.5899988*eV,3.60000576*eV,3.60996356*eV,3.6199766*eV,3.63004534
                *eV,3.6399565*eV,3.65002939*eV,3.66005014*eV,3.67001742*eV,3.68003913*eV,3.69000591
                *eV,3.70002681*eV,3.70999128*eV,3.72000955*eV,3.72996987*eV,3.73998366*eV,3.75005137
                *eV,3.75994537*eV,3.77000634*eV,3.78000605*eV,3.7899431*eV,3.80004899*eV,3.80997475
                *eV,3.8199525*eV,3.82998265*eV,3.83994668*eV,3.84996269*eV,3.86003108*eV,3.87003148
                *eV,3.8799624*eV,3.88994442*eV,3.89997793*eV,3.90994003*eV,3.91995316*eV,3.9300177
                *eV,3.94000885*eV,3.95005092*eV,3.96001784*eV,3.97003517*eV,3.97997555*eV,3.98996584
                *eV,4.0000064*eV,4.00996793*eV,4.0199792*eV,4.03004058*eV,4.0400208*eV,4.05005058
                *eV,4.05999733*eV,4.06999305*eV,4.08003812*eV,4.08999797*eV,4.10000656*eV,4.11006426
                *eV,4.12003451*eV,4.13005325*eV,4.13998258*eV,4.14995978*eV,4.15998518*eV,4.17005914
                *eV,4.18004108*eV,4.18992932*eV,4.20000672*eV,4.20998976*eV,4.22002037*eV,4.22995457
                *eV,4.23993566*eV,4.24996395*eV,4.2600398*eV,4.27001648*eV,4.28003999*eV,4.28996223
                *eV,4.29993058*eV,4.30994537*eV,4.32000691*eV,4.32996432*eV,4.33996774*eV,4.35001749
                *eV,4.35996056*eV,4.36994919*eV,4.37998369*eV,4.39006439*eV,4.40003543*eV,4.41005188
                *eV,4.41995645*eV,4.4300639*eV,4.44005867*eV,4.44993893*eV,4.46002369*eV,4.46999309
                *eV,4.48000717*eV,4.49006622*eV,4.5000072*eV,4.5099923*eV,4.52002182*eV,4.52993052
                *eV,4.54004901*eV,4.55004582*eV,4.55991903*eV,4.57000363*eV,4.57996374*eV,4.58996736
                *eV,4.60001478*eV,4.60993487*eV,4.62007*eV,4.63007687*eV,4.63995354*eV,4.65004682
                *eV,4.66000896*eV,4.67001388*eV,4.68006185*eV,4.68997573*eV,4.69993171*eV,4.70993004
                *eV,4.71997101*eV,4.73005488*eV,4.7400007*eV,4.74998845*eV,4.76001837*eV,4.77009074
                *eV,4.78002153*eV,4.78999376*eV,4.80000768*eV,4.81006356*eV,4.81997428*eV,4.82992592
                *eV,4.83991874*eV,4.849953*eV,4.86002895*eV,4.86995555*eV,4.87992279*eV,4.88993092
                *eV,4.89998018*eV,4.91007083*eV,4.92000787*eV,4.92998523*eV,4.94000313*eV,4.95006182
                *eV,4.95996313*eV,4.96990413*eV,4.98008509*eV,4.98990616*eV,4.99996767*eV,5.01006984
                *eV,5.02000965*eV,5.02998898*eV,5.04000807*eV,5.05006714*eV,5.05995994*eV,5.0700989
                *eV,5.08007041*eV,5.09008122*eV,5.09992178*eV,5.11001106*eV,5.11992891*eV,5.13009758
                *eV,5.14009363*eV,5.14991478*eV,5.15998828*eV,5.17010126*eV,5.18003754*eV,5.19001207
                *eV,5.2000251*eV,5.21007683*eV,5.21994773*eV,5.23007671*eV,5.2400236*eV,5.2500084
                *eV,5.26003133*eV,5.2700926*eV,5.27996757*eV,5.29010532*eV,5.30005551*eV,5.31004319
                *eV,5.32006859*eV,5.32990278*eV,5.34000338*eV,5.34991148*eV,5.36008813*eV,5.37007096
                *eV,5.38009106*eV,5.38991429*eV,5.40000864*eV,5.40990481*eV,5.42007425*eV,5.43004417
                *eV,5.44005083*eV,5.45009444*eV,5.45993476*eV,5.47005199*eV,5.47996457*eV,5.48991314
                *eV,5.4998979*eV};
            
        std::vector<G4double> Al_RIND = { 9.45280e-01, 9.70930e-01, 9.96370e-01, 1.02147e+00, 1.04652e+00, 1.07150e+00
            , 1.09684e+00, 1.12312e+00, 1.15085e+00, 1.18042e+00, 1.21266e+00, 1.24832e+00
            , 1.28752e+00, 1.33076e+00, 1.37779e+00, 1.42868e+00, 1.48243e+00, 1.53824e+00
            , 1.59457e+00, 1.64985e+00, 1.70258e+00, 1.75087e+00, 1.79304e+00, 1.82839e+00
            , 1.85601e+00, 1.87539e+00, 1.88656e+00, 1.88979e+00, 1.88587e+00, 1.87531e+00
            , 1.85909e+00, 1.83807e+00, 1.81296e+00, 1.78488e+00, 1.75427e+00, 1.72192e+00
            , 1.68859e+00, 1.65413e+00, 1.61955e+00, 1.58489e+00, 1.55061e+00, 1.51655e+00
            , 1.48310e+00, 1.45052e+00, 1.41881e+00, 1.38753e+00, 1.35763e+00, 1.32829e+00
            , 1.30012e+00, 1.27267e+00, 1.24623e+00, 1.22072e+00, 1.19586e+00, 1.17210e+00
            , 1.14916e+00, 1.12692e+00, 1.10544e+00, 1.08469e+00, 1.06475e+00, 1.04542e+00
            , 1.02678e+00, 1.00885e+00, 9.91420e-01, 9.74560e-01, 9.58270e-01, 9.42440e-01
            , 9.27230e-01, 9.12500e-01, 8.98150e-01, 8.84370e-01, 8.70860e-01, 8.57830e-01
            , 8.45220e-01, 8.32900e-01, 8.20990e-01, 8.09400e-01, 7.98150e-01, 7.87190e-01
            , 7.76570e-01, 7.66180e-01, 7.56120e-01, 7.46350e-01, 7.36770e-01, 7.27510e-01
            , 7.18450e-01, 7.09570e-01, 7.00960e-01, 6.92560e-01, 6.84350e-01, 6.76330e-01
            , 6.68510e-01, 6.60860e-01, 6.53420e-01, 6.46100e-01, 6.38990e-01, 6.32010e-01
            , 6.25190e-01, 6.18510e-01, 6.11960e-01, 6.05560e-01, 5.99290e-01, 5.93190e-01
            , 5.87160e-01, 5.81280e-01, 5.75550e-01, 5.69880e-01, 5.64350e-01, 5.58890e-01
            , 5.53550e-01, 5.48320e-01, 5.43180e-01, 5.38160e-01, 5.33180e-01, 5.28340e-01
            , 5.23550e-01, 5.18860e-01, 5.14270e-01, 5.09760e-01, 5.05300e-01, 5.00910e-01
            , 4.96640e-01, 4.92410e-01, 4.88240e-01, 4.84170e-01, 4.80110e-01, 4.76180e-01
            , 4.72280e-01, 4.68430e-01, 4.64670e-01, 4.60960e-01, 4.57320e-01, 4.53700e-01
            , 4.50150e-01, 4.46640e-01, 4.43220e-01, 4.39820e-01, 4.36470e-01, 4.33180e-01
            , 4.29920e-01, 4.26720e-01, 4.23590e-01, 4.20470e-01, 4.17390e-01, 4.14370e-01
            , 4.11380e-01, 4.08440e-01, 4.05540e-01, 4.02660e-01, 3.99840e-01, 3.97040e-01
            , 3.94300e-01, 3.91580e-01, 3.88900e-01, 3.86260e-01, 3.83640e-01, 3.81050e-01
            , 3.78500e-01, 3.75970e-01, 3.73500e-01, 3.71040e-01, 3.68600e-01, 3.66200e-01
            , 3.63830e-01, 3.61490e-01, 3.59180e-01, 3.56890e-01, 3.54640e-01, 3.52390e-01
            , 3.50200e-01, 3.48020e-01, 3.45870e-01, 3.43720e-01, 3.41610e-01, 3.39540e-01
            , 3.37480e-01, 3.35430e-01, 3.33410e-01, 3.31420e-01, 3.29440e-01, 3.27500e-01
            , 3.25570e-01, 3.23650e-01, 3.21760e-01, 3.19910e-01, 3.18060e-01, 3.16220e-01
            , 3.14410e-01, 3.12620e-01, 3.10850e-01, 3.09100e-01, 3.07350e-01, 3.05640e-01
            , 3.03930e-01, 3.02250e-01, 3.00580e-01, 2.98930e-01, 2.97290e-01, 2.95680e-01
            , 2.94060e-01, 2.92470e-01, 2.90900e-01, 2.89350e-01, 2.87800e-01, 2.86280e-01
            , 2.84760e-01, 2.83260e-01, 2.81780e-01, 2.80310e-01, 2.78840e-01, 2.77390e-01
            , 2.75970e-01, 2.74550e-01, 2.73140e-01, 2.71750e-01, 2.70380e-01, 2.69010e-01
            , 2.67660e-01, 2.66320e-01, 2.64990e-01, 2.63660e-01, 2.62360e-01, 2.61070e-01
            , 2.59790e-01, 2.58500e-01, 2.57250e-01, 2.56000e-01, 2.54750e-01, 2.53510e-01
            , 2.52300e-01, 2.51090e-01, 2.49900e-01, 2.48710e-01, 2.47530e-01, 2.46360e-01
            , 2.45210e-01, 2.44050e-01, 2.42910e-01, 2.41780e-01, 2.40660e-01, 2.39560e-01
            , 2.38450e-01, 2.37350e-01, 2.36270e-01, 2.35200e-01, 2.34130e-01, 2.33060e-01
            , 2.32020e-01, 2.30970e-01, 2.29940e-01, 2.28930e-01, 2.27910e-01, 2.26890e-01
            , 2.25900e-01, 2.24900e-01, 2.23910e-01, 2.22930e-01, 2.21970e-01, 2.21000e-01
            , 2.20040e-01, 2.19080e-01, 2.18150e-01, 2.17220e-01, 2.16290e-01, 2.15370e-01
            , 2.14460e-01, 2.13550e-01, 2.12640e-01, 2.11750e-01, 2.10880e-01, 2.09990e-01
            , 2.09110e-01, 2.08250e-01, 2.07390e-01, 2.06530e-01, 2.05690e-01, 2.04850e-01
            , 2.04010e-01, 2.03170e-01, 2.02360e-01, 2.01540e-01, 2.00730e-01, 1.99910e-01
            , 1.99120e-01, 1.98320e-01, 1.97540e-01, 1.96750e-01, 1.95970e-01, 1.95200e-01
            , 1.94420e-01, 1.93680e-01, 1.92900e-01, 1.92150e-01, 1.91420e-01, 1.90660e-01
            , 1.89940e-01, 1.89210e-01, 1.88490e-01, 1.87770e-01, 1.87040e-01, 1.86340e-01
            , 1.85630e-01, 1.84920e-01, 1.84220e-01, 1.83530e-01, 1.82840e-01, 1.82160e-01
            , 1.81480e-01, 1.80810e-01, 1.80140e-01, 1.79470e-01, 1.78810e-01, 1.78160e-01
            , 1.77500e-01, 1.76860e-01, 1.76210e-01, 1.75570e-01, 1.74930e-01, 1.74300e-01
            , 1.73670e-01, 1.73050e-01, 1.72430e-01, 1.71820e-01, 1.71210e-01, 1.70600e-01
            , 1.70000e-01, 1.69390e-01, 1.68800e-01, 1.68210e-01, 1.67620e-01, 1.67030e-01
            , 1.66450e-01, 1.65880e-01, 1.65300e-01, 1.64730e-01, 1.64160e-01, 1.63600e-01
            , 1.63040e-01, 1.62480e-01, 1.61930e-01, 1.61380e-01, 1.60830e-01, 1.60290e-01
            , 1.59750e-01, 1.59220e-01, 1.58680e-01, 1.58160e-01, 1.57630e-01, 1.57100e-01
            , 1.56580e-01, 1.56060e-01, 1.55550e-01, 1.55040e-01, 1.54530e-01, 1.54030e-01
            , 1.53530e-01, 1.53030e-01, 1.52530e-01, 1.52040e-01, 1.51550e-01, 1.51060e-01
            , 1.50580e-01, 1.50100e-01, 1.49620e-01, 1.49140e-01, 1.48670e-01, 1.48200e-01
            , 1.47730e-01, 1.47260e-01, 1.46800e-01, 1.46340e-01, 1.45890e-01, 1.45440e-01
            , 1.44980e-01, 1.44530e-01, 1.44090e-01, 1.43640e-01, 1.43200e-01, 1.42760e-01
            , 1.42330e-01, 1.41890e-01, 1.41460e-01, 1.41030e-01, 1.40610e-01, 1.40180e-01
            , 1.39760e-01, 1.39340e-01, 1.38920e-01, 1.38510e-01, 1.38100e-01, 1.37690e-01
            , 1.37280e-01, 1.36870e-01, 1.36470e-01, 1.36070e-01, 1.35670e-01, 1.35270e-01
            , 1.34880e-01, 1.34480e-01, 1.34100e-01, 1.33710e-01, 1.33320e-01, 1.32940e-01
            , 1.32560e-01, 1.32180e-01, 1.31800e-01, 1.31430e-01, 1.31050e-01, 1.30680e-01
            , 1.30310e-01, 1.29950e-01, 1.29580e-01, 1.29220e-01, 1.28850e-01, 1.28490e-01
            , 1.28140e-01, 1.27780e-01, 1.27430e-01, 1.27080e-01, 1.26730e-01, 1.26380e-01
            , 1.26030e-01, 1.25690e-01, 1.25340e-01, 1.25000e-01, 1.24660e-01, 1.24310e-01
            , 1.23970e-01};
        
        std::vector<G4double> Al_KIND = {7.69295, 7.63287, 7.57296, 7.51368, 7.45468, 7.39616, 7.33792, 7.27925, 7.22077
            , 7.16271, 7.10504, 7.04828, 6.99369, 6.9419, 6.89437, 6.8518, 6.81583, 6.78728
            , 6.76703, 6.75544, 6.75239, 6.75748, 6.76979, 6.78793, 6.81056, 6.83616, 6.86302
            , 6.88965, 6.91538, 6.93857, 6.95904, 6.97607, 6.98941, 6.99877, 7.00432, 7.00614
            , 7.00438, 6.99915, 6.99085, 6.97968, 6.96595, 6.94967, 6.93142, 6.9113, 6.88956
            , 6.86617, 6.84172, 6.81589, 6.78935, 6.76178, 6.73351, 6.70464, 6.67511, 6.64536
            , 6.61525, 6.58475, 6.55393, 6.52308, 6.49213, 6.46103, 6.4298, 6.39889, 6.36765
            , 6.3367, 6.30565, 6.27465, 6.24386, 6.21318, 6.18269, 6.15241, 6.12212, 6.09212
            , 6.06229, 6.03255, 6.00308, 5.9737, 5.94467, 5.91571, 5.88711, 5.85859, 5.83024
            , 5.80242, 5.7745, 5.74694, 5.71954, 5.69232, 5.66536, 5.63867, 5.61207, 5.58582
            , 5.55976, 5.53378, 5.50817, 5.48265, 5.4575, 5.43235, 5.40757, 5.38289, 5.3584
            , 5.33411, 5.3101, 5.28629, 5.26268, 5.23917, 5.21606, 5.19295, 5.17015, 5.14736
            , 5.12479, 5.10251, 5.08036, 5.05842, 5.03659, 5.01489, 4.99351, 4.97214, 4.95111
            , 4.9302, 4.90941, 4.88875, 4.86833, 4.84814, 4.82787, 4.80795, 4.78806, 4.76841
            , 4.7489, 4.72953, 4.7103, 4.69121, 4.67238, 4.65348, 4.63494, 4.61633, 4.59798
            , 4.57979, 4.56163, 4.54364, 4.5258, 4.50811, 4.49059, 4.47312, 4.45569, 4.43866
            , 4.42156, 4.40452, 4.38776, 4.37094, 4.35441, 4.33793, 4.32163, 4.30538, 4.2892
            , 4.27319, 4.25725, 4.24149, 4.22579, 4.21015, 4.1947, 4.17931, 4.164, 4.14887
            , 4.13381, 4.11882, 4.1039, 4.08918, 4.07453, 4.05995, 4.04557, 4.03114, 4.01679
            , 4.00252, 3.98845, 3.97445, 3.96054, 3.94671, 3.93283, 3.91916, 3.90558, 3.89208
            , 3.87879, 3.86546, 3.85221, 3.83905, 3.82599, 3.813, 3.79998, 3.78718, 3.77447
            , 3.76185, 3.74919, 3.73663, 3.72416, 3.71179, 3.69951, 3.68732, 3.67524, 3.66312
            , 3.65109, 3.63917, 3.62721, 3.61535, 3.60372, 3.59193, 3.58038, 3.56879, 3.55744
            , 3.54592, 3.53465, 3.52334, 3.51214, 3.50105, 3.48993, 3.47891, 3.46787, 3.45693
            , 3.44611, 3.43539, 3.42465, 3.41402, 3.40336, 3.39281, 3.38238, 3.37192, 3.36157
            , 3.3512, 3.34094, 3.33081, 3.32064, 3.3106, 3.30052, 3.29042, 3.28044, 3.27058
            , 3.26069, 3.25093, 3.2413, 3.23163, 3.22194, 3.21238, 3.20279, 3.19333, 3.18384
            , 3.17448, 3.16511, 3.15581, 3.14656, 3.13736, 3.12822, 3.11914, 3.11005, 3.10101
            , 3.09206, 3.08311, 3.07419, 3.06532, 3.05655, 3.04778, 3.03905, 3.03036, 3.02172
            , 3.01312, 3.00456, 2.99606, 2.98757, 2.97916, 2.97076, 2.96242, 2.95411, 2.94584
            , 2.93762, 2.92943, 2.92127, 2.91317, 2.90508, 2.89704, 2.88902, 2.88106, 2.87311
            , 2.86521, 2.85732, 2.8495, 2.84173, 2.834, 2.82629, 2.81858, 2.81091, 2.80327
            , 2.79571, 2.78817, 2.78065, 2.77312, 2.76569, 2.75827, 2.75089, 2.74356, 2.73624
            , 2.72893, 2.72163, 2.71445, 2.70726, 2.70009, 2.69292, 2.68581, 2.67876, 2.67172
            , 2.66472, 2.65772, 2.65076, 2.64382, 2.63691, 2.63006, 2.62324, 2.61642, 2.60965
            , 2.60287, 2.59613, 2.5894, 2.58274, 2.57609, 2.56948, 2.56291, 2.55635, 2.5498
            , 2.54327, 2.53677, 2.53033, 2.5239, 2.5175, 2.5111, 2.50472, 2.4984, 2.49209
            , 2.48581, 2.47955, 2.47329, 2.46708, 2.46089, 2.45471, 2.44858, 2.44245, 2.43637
            , 2.43031, 2.42426, 2.41826, 2.41226, 2.40627, 2.40033, 2.39439, 2.38846, 2.38258
            , 2.37673, 2.37088, 2.36506, 2.35925, 2.35345, 2.34769, 2.34197, 2.33623, 2.33056
            , 2.32488, 2.31923, 2.31361, 2.30797, 2.30237, 2.29679, 2.29125, 2.28569, 2.28021
            , 2.27473, 2.26927, 2.26381, 2.25839, 2.25295, 2.24755, 2.24217, 2.23681, 2.23148
            , 2.22615, 2.22084, 2.21554, 2.21029, 2.20506, 2.19981, 2.19461, 2.18941, 2.18424
            , 2.17908, 2.17393, 2.16881, 2.16369, 2.15862, 2.15354, 2.14849, 2.14346, 2.13843
            , 2.13342, 2.12842, 2.12346, 2.11851, 2.11355, 2.10864, 2.10373, 2.09885, 2.09396
            , 2.08911, 2.08427, 2.07942, 2.0746, 2.06983, 2.06504, 2.06027, 2.05553, 2.05078
            , 2.04605, 2.04133, 2.03667, 2.03198, 2.02733, 2.02269, 2.01806, 2.01343, 2.00883
            , 2.00423, 1.99963, 1.99503, 1.99044};
        
        G4MaterialPropertiesTable* Al_MPT = new G4MaterialPropertiesTable();

        Al_MPT->AddProperty("REALRINDEX", Al_Energy, Al_RIND, false, true);
        Al_MPT->AddProperty("IMAGINARYRINDEX", Al_Energy, Al_KIND, false, true);

        Al_mat->SetMaterialPropertiesTable(Al_MPT);

        /* plastic */

        std::vector<G4double> plastic_Energy  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
        std::vector<G4double> plastic_RIND    = { 1.54, 1.54, 1.54, 1.54, 1.54, 1.54}; 

        G4MaterialPropertiesTable* plastic_MPT = new G4MaterialPropertiesTable();
        plastic_MPT->AddProperty("RINDEX", plastic_Energy, plastic_RIND, false, false);

        plastic_MPT->DumpTable();
        plastic_mat->SetMaterialPropertiesTable(plastic_MPT);

        /* refl1 (VM 2000) */

        std::vector<G4double> refl1_Energy  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
        std::vector<G4double> refl1_RIND    = { 1.49, 1.49, 1.49, 1.49, 1.49, 1.49}; 

        G4MaterialPropertiesTable* refl1_MPT = new G4MaterialPropertiesTable();
        refl1_MPT->AddProperty("RINDEX", refl1_Energy, refl1_RIND, false, false);

        refl1_MPT->DumpTable();
        refl1_mat->SetMaterialPropertiesTable(refl1_MPT);

        /* refl2 (white Tetratex PTFE) */

        std::vector<G4double> refl2_Energy  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
        std::vector<G4double> refl2_RIND    = { 1.38, 1.38, 1.38, 1.38, 1.38, 1.38};

        G4MaterialPropertiesTable* refl2_MPT = new G4MaterialPropertiesTable();
        refl2_MPT->AddProperty("RINDEX", refl2_Energy, refl2_RIND, false, false);

        refl2_MPT->DumpTable();
        refl2_mat->SetMaterialPropertiesTable(refl2_MPT);

        /* refl3 (electrical tape) */

        std::vector<G4double> refl3_Energy  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };

        // refractiveindex.info
        std::vector<G4double> refl3_RIND    = { 1.5425, 1.5425, 1.5425, 1.5425, 1.5425, 1.5425};
        std::vector<G4double> refl3_KIND    = { 0.0000023096, 0.0000023096, 0.0000023096, 0.0000023096, 0.0000023096, 0.0000023096};


        G4MaterialPropertiesTable* refl3_MPT = new G4MaterialPropertiesTable();

        refl3_MPT->AddProperty("REALRINDEX", refl3_Energy, refl3_RIND, false, false);
        refl3_MPT->AddProperty("IMAGINARYRINDEX", refl3_Energy, refl3_KIND, false, false);
        
        refl3_MPT->DumpTable();
        refl3_mat->SetMaterialPropertiesTable(refl3_MPT);

        /* Teflon PTFE */
        // https://www.mcmaster.com/8569K58/

        std::vector<G4double> tefl_Energy  = { 0.001 * eV, 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
        std::vector<G4double> tefl_RIND    = { 1.38, 1.38, 1.38, 1.38, 1.38, 1.38}; // https://ieeexplore.ieee.org/document/5411657

        G4MaterialPropertiesTable* tefl_MPT = new G4MaterialPropertiesTable();
        tefl_MPT->AddProperty("RINDEX", tefl_Energy, tefl_RIND, false, false);

        tefl_MPT->DumpTable();
        tefl_mat->SetMaterialPropertiesTable(tefl_MPT);

        /* EJ-560 */

        std::vector<G4double> ej560_Energy  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
        std::vector<G4double> ej560_RIND  = { 1.43, 1.43, 1.43, 1.43, 1.43 }; // material data sheet

        G4MaterialPropertiesTable* ej560_MPT = new G4MaterialPropertiesTable();
        ej560_MPT->AddProperty("RINDEX", ej560_Energy, ej560_RIND, false, false);

        ej560_MPT->DumpTable();
        ej560_mat->SetMaterialPropertiesTable(ej560_MPT);

        /* SYLGARD 184 */
        // datasheet

        std::vector<G4double> sy184_Energy = { 0.8 * eV, 0.94 * eV, 1.96 * eV, 2.1 * eV, 10 * eV };
        std::vector<G4double> sy184_RIND = { 1.3997, 1.4028, 1.4225, 1.4118, 1.4118 }; // material data sheet

        G4MaterialPropertiesTable* sy184_MPT = new G4MaterialPropertiesTable();
        sy184_MPT->AddProperty("RINDEX", sy184_Energy, sy184_RIND, false, false);

        sy184_MPT->DumpTable();
        sy184_mat->SetMaterialPropertiesTable(sy184_MPT);

        // Glass

        std::vector<G4double> glass_Energy = {1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV};
        std::vector<G4double> glass_RIND = {1.53, 1.53, 1.53, 1.53, 1.53 };


        G4MaterialPropertiesTable* glass_MPT = new G4MaterialPropertiesTable();
        glass_MPT->AddProperty("RINDEX", glass_Energy, glass_RIND, false, false);

        glass_MPT->DumpTable();
        glass_mat->SetMaterialPropertiesTable(glass_MPT);

        // SiPM
        std::vector<G4double> SiPM_Energy = { 1.*eV, 3.*eV, 5.*eV, 7.*eV, 10.*eV };
        std::vector<G4double> SiPM_RIND = { 1., 1., 1., 1., 1. };

        G4MaterialPropertiesTable* SiPM_MPT = new G4MaterialPropertiesTable();

        SiPM_MPT->AddProperty("RINDEX", SiPM_Energy, SiPM_RIND, false, false);
        SiPM_mat->SetMaterialPropertiesTable(SiPM_MPT);



        /* --------------------- GEOMETRY ----------------------- */
        // 8: 3x3 SiPM array, X-wall at NRL
        // 9: 3x3 SiPM array, X-wall at SSL

        if (setGeomType == 8){

            // Set up some dimensions (in mm)
            // BGO
            G4double scintX = 198.; // BGO x length 
            G4double scintY = 118.; // BGO y length
            G4double scintZ = 23.; // BGO z length
            // Reflective layers: two layers of VM2000, one layer of Tetratex, one layer of Teflon PTFE
            G4double refl1Thick = 0.065; // thickness of first VM200 reflective layer (Janecek 2012)
            G4double refl11Thick = 0.065; // thickness of second VM2000 reflective layer (Janecek 2012)
            G4double refl2Thick = 0.254; // thickness of Tetratex reflective layer (prototype description)
            G4double refl3Thick = 0.0762; // thickness of Teflon relfective layer (https://www.mcmaster.com/8569K58/)
            // Optical coupler
            G4double optX = 18.; // optical coupler x length
            G4double optY = 18.; // optical coupler y length
            G4double optZ = 1.; // optical coupler thickness

            // Half lengths of scintillator + reflective layers
            G4double xlen = scintX/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double ylen = scintY/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double zlen = scintZ/2. + refl1Thick*2 + refl2Thick + refl3Thick;

            G4double pos_sub = xlen;

            /*** Aluminum housing ***/
            G4int housing_copy = 30;

            auto rotation_housing = new G4RotationMatrix();
            rotation_housing->rotateX(-90*deg);
            rotation_housing->rotateY(90*deg);

            G4String cad_path = "";
            gm.config->readInto(cad_path, "GEOM.CAD.PATH");
            G4cout << "GEOM.CAD.PATH: " << cad_path << G4endl;

            auto mesh_housing = CADMesh::TessellatedMesh::FromPLY(cad_path+"/ACS-ST-0101revB_ACS_X-WALL_CHASSIS.stl");
            G4VSolid* solid_housing = mesh_housing->GetSolid();

            G4LogicalVolume* log_housing = new G4LogicalVolume(solid_housing, Al_mat, "log_housing");
            G4VPhysicalVolume* phys_housing = new G4PVPlacement(rotation_housing, G4ThreeVector(xlen+7., 0., zlen+3.5), log_housing, "housing", log_chamber, false, housing_copy, true);
    
            G4VisAttributes* VisHousing = new G4VisAttributes(G4Colour::Grey());
            log_housing->SetVisAttributes(VisHousing);

            if (housing_sens == 1)
                gm.AddXYZDetector(log_housing);

            /* End caps */
            G4int cap_front_copy = 31;
            G4int cap_back_copy = 32;

            G4double capX = 2.54; // from housing drawing
            G4double capY = 365.76; // from housing drawing
            G4double capZ = 26.035; // from housing drawing

            G4Box* solid_cap = new G4Box("solid_cap", capX/2., capY/2., capZ/2.);
            G4LogicalVolume* log_cap = new G4LogicalVolume(solid_cap, Al_mat, "log_cap");
            G4VPhysicalVolume* phys_cap_front = new G4PVPlacement(0, G4ThreeVector(102., 0., 0.), log_cap, "cap_front", log_chamber, false, cap_front_copy, true);
            G4VPhysicalVolume* phys_cap_back = new G4PVPlacement(0, G4ThreeVector(-102., 0., 0.), log_cap, "cap_back", log_chamber, false, cap_back_copy, true);

            G4VisAttributes* VisCap = new G4VisAttributes(G4Colour::Grey());
            log_cap->SetVisAttributes(VisCap);

            if (cap_sens == 1)
                gm.AddXYZDetector(log_cap);

            /* Plastic piece */
            G4int plastic_copy = 33;

            // prototype information
            G4double plasticX = 300.;
            G4double plasticY = 550;
            G4double plasticZ = 25.; 

            G4double plastic_posX = -60;
            G4double plastic_posY = 0.;
            G4double plastic_posZ = -39.1498 - plasticZ/2.;

            G4Box* solid_plastic = new G4Box("solid_plastic", plasticX/2., plasticY/2, plasticZ/2.);
            G4LogicalVolume* log_plastic = new G4LogicalVolume(solid_plastic, plastic_mat, "log_plastic");
            G4VPhysicalVolume* phys_plastic = new G4PVPlacement(0, G4ThreeVector(plastic_posX, plastic_posY, plastic_posZ), log_plastic, "plastic", log_chamber, false, plastic_copy, true);

            G4VisAttributes* VisPlastic = new G4VisAttributes(G4Colour::Cyan());
            log_plastic->SetVisAttributes(VisPlastic);

            if (plastic_sens == 1)
                gm.AddXYZDetector(log_plastic);

            /* Aluminum table */
            G4int table_copy = 34;

            G4double tableX = 431.8;
            G4double tableY = 2311.4;
            G4double tableZ = 63.5;

            G4double table_posX = 0.;
            G4double table_posY = 0.;
            G4double table_posZ = plastic_posZ - plasticZ/2. - tableZ/2.;

            G4Box* solid_table = new G4Box("solid_table", tableX/2., tableY/2., tableZ/2.);
            G4LogicalVolume* log_table = new G4LogicalVolume(solid_table, Al_mat, "log_table");
            G4VPhysicalVolume* phys_table = new G4PVPlacement(0, G4ThreeVector(table_posX, table_posY, table_posZ), log_table, "table", log_chamber, false, table_copy, true);

            G4VisAttributes* VisTable = new G4VisAttributes(G4Colour::Grey());
            log_table->SetVisAttributes(VisTable);

            if (table_sens == 1)
                gm.AddXYZDetector(log_table);

            /* Black coat */
            G4int coat_top_copy = 36;
            G4int coat_left_copy = 37;
            G4int coat_right_copy = 38;

            G4double coat_shiftX = -15.;

            G4double coat_topX = 200.;
            G4double coat_topY = 425;
            G4double coat_topZ = 1;
            G4double coat_sideX = coat_topX;
            G4double coat_sideY = coat_topZ;
            G4double coat_sideZ = 50. - coat_topZ;

            G4double coat_top_posX = 0. + coat_shiftX;
            G4double coat_top_posY = 0.;
            G4double coat_top_posZ = 15.4602 + coat_topZ/2.;
            G4double coat_side_posX = 0. + coat_shiftX;
            G4double coat_side_posY = coat_topY/2. - coat_sideY/2.;
            G4double coat_side_posZ = coat_top_posZ - coat_topZ/2. - coat_sideZ/2.;

            G4Box* solid_coat_top = new G4Box("solid_coat_top", coat_topX/2., coat_topY/2., coat_topZ/2.);
            G4LogicalVolume* log_coat_top = new G4LogicalVolume(solid_coat_top, coat_mat, "log_coat_top");
            G4VPhysicalVolume* phys_coat_top = new G4PVPlacement(0, G4ThreeVector(coat_top_posX, coat_top_posY, coat_top_posZ), log_coat_top, "coat_top", log_chamber, false, coat_top_copy, true);

            G4Box* solid_coat_side = new G4Box("solid_coat_side", coat_sideX/2., coat_sideY/2., coat_sideZ/2.);
            G4LogicalVolume* log_coat_side = new G4LogicalVolume(solid_coat_side, coat_mat, "log_coat_side");
            G4VPhysicalVolume* phys_coat_left = new G4PVPlacement(0, G4ThreeVector(coat_side_posX, -coat_side_posY, coat_side_posZ), log_coat_side, "coat_left", log_chamber, false, coat_left_copy, true);
            G4VPhysicalVolume* phys_coat_right = new G4PVPlacement(0, G4ThreeVector(coat_side_posX, coat_side_posY, coat_side_posZ), log_coat_side, "coat_right", log_chamber, false, coat_right_copy, true);

            G4VisAttributes* VisCoat = new G4VisAttributes(G4Colour::Black());
            log_coat_top->SetVisAttributes(VisCoat);
            log_coat_side->SetVisAttributes(VisCoat);

            if (coat_sens == 1) {
                gm.AddXYZDetector(log_coat_top);
                gm.AddXYZDetector(log_coat_side);
            }

            // Keyword to check if the source is Am241 or Cs137
            G4int Am241_or_Cs137 = 0;
            gm.config->readInto(Am241_or_Cs137, "GEOM.COSI.AM241.OR.CS137");
            G4cout << "GEOM.COSI.AM241.OR.CS137: " << Am241_or_Cs137 << G4endl;   
            // Case with uncollimated Am241 or Cs137 source
            if (Am241_or_Cs137 == 1 && is_collimated == 0){
                /* Aluminum frame */
                G4int frame_top_copy = 39;
                G4int frame_left_copy = 40;
                G4int frame_right_copy = 41;

                G4double frame_shiftX = -40.;

                G4double frame_topX = 250.;
                G4double frame_topY = 148.;
                G4double frame_topZ = 1.;
                G4double frame_sideX = frame_topX;
                G4double frame_sideY = frame_topZ;
                G4double frame_sideZ = 85. - frame_topZ;

                G4double frame_side_posX = 0. + frame_shiftX;
                G4double frame_side_posY = frame_topY/2. - frame_sideY/2.;
                G4double frame_side_posZ = coat_top_posZ + coat_topZ/2. + frame_sideZ/2.;
                G4double frame_top_posX = 0. + frame_shiftX;
                G4double frame_top_posY = 0.;
                G4double frame_top_posZ = frame_side_posZ + frame_sideZ/2. + frame_topZ/2.;

                G4Box* solid_frame_top = new G4Box("solid_frame_top", frame_topX/2., frame_topY/2., frame_topZ/2.);
                G4LogicalVolume* log_frame_top = new G4LogicalVolume(solid_frame_top, Al_mat, "log_frame_top");
                G4VPhysicalVolume* phys_frame_top = new G4PVPlacement(0, G4ThreeVector(frame_top_posX, frame_top_posY, frame_top_posZ), log_frame_top, "frame_top", log_chamber, false, frame_top_copy, true);

                G4Box* solid_frame_side = new G4Box("solid_frame_side", frame_sideX/2., frame_sideY/2., frame_sideZ/2.);
                G4LogicalVolume* log_frame_side = new G4LogicalVolume(solid_frame_side, Al_mat, "log_frame_side");
                G4VPhysicalVolume* phys_frame_left = new G4PVPlacement(0, G4ThreeVector(frame_side_posX, -frame_side_posY, frame_side_posZ), log_frame_side, "frame_left", log_chamber, false, frame_left_copy, true);
                G4VPhysicalVolume* phys_frame_right = new G4PVPlacement(0, G4ThreeVector(frame_side_posX, frame_side_posY, frame_side_posZ), log_frame_side, "frame_right", log_chamber, false, frame_right_copy, true);

                G4VisAttributes* VisFrame = new G4VisAttributes(G4Colour::Grey());
                log_frame_top->SetVisAttributes(VisFrame);
                log_frame_side->SetVisAttributes(VisFrame);

                if (frame_sens == 1) {
                    gm.AddXYZDetector(log_frame_top);
                    gm.AddXYZDetector(log_frame_side);
                }

                /* Foam block */
                G4int foam_copy = 42;

                G4double foamX = 240.;
                G4double foamY = 135.;
                G4double foamZ = 210.;

                G4double foam_posX = 0.;
                G4double foam_posY = 0.;
                G4double foam_posZ = frame_top_posZ + frame_topZ/2. + foamZ/2.;

                G4Box* solid_foam_notsub = new G4Box("solid_foam_notsub", foamX/2., foamY/2., foamZ/2.);
                G4Tubs* tube_sub = new G4Tubs("tube_sub", 0., 56, foamZ, 0., 360.);
                G4SubtractionSolid* solid_foam = new G4SubtractionSolid("solid_foam", solid_foam_notsub, tube_sub, 0, G4ThreeVector(0., -foamY/2., 0.));

                G4LogicalVolume* log_foam = new G4LogicalVolume(solid_foam, foam_mat, "log_foam");

                G4VPhysicalVolume* phys_foam = new G4PVPlacement(0, G4ThreeVector(foam_posX, foam_posY, foam_posZ), log_foam, "foam", log_chamber, false, foam_copy, true);

                G4VisAttributes* VisFoam = new G4VisAttributes(G4Colour::Brown());
                log_foam->SetVisAttributes(VisFoam);

                if (foam_sens == 1)
                    gm.AddXYZDetector(log_foam);
            }

            // Case with uncollimated Na22 source
            if (Am241_or_Cs137 == 0 && is_collimated == 0) {
                /* Plastic casing aroung source */
                G4int casing_copy = 45;

                G4double casingX = 10.;
                G4double casingY = 10.;
                G4double casingZ = 5.;
                G4double casingThick = 1.;

                G4double casing_posX = 0.;
                G4double casing_posY = 0.;
                G4double casing_posZ = 16.4602 + casingZ/2.;

                G4Box* solid_casing_notsub = new G4Box("solid_casing_notsub", casingX/2., casingY/2., casingZ/2.);
                G4Box* casing_cavity = new G4Box("casing_cavity", casingX/2.-casingThick, casingY/2.-casingThick, casingZ/2.-casingThick);
                G4SubtractionSolid* solid_casing = new G4SubtractionSolid("solid_casing", solid_casing_notsub, casing_cavity, 0, G4ThreeVector(0., 0., 0.));
                
                G4LogicalVolume* log_casing = new G4LogicalVolume(solid_casing, plastic_mat, "log_casing");

                G4VPhysicalVolume* phys_casing = new G4PVPlacement(0, G4ThreeVector(casing_posX, casing_posY, casing_posZ), log_casing, "casing", log_chamber, false, casing_copy, true);
                
                G4VisAttributes* VisCasing = new G4VisAttributes(G4Colour::White());
                log_casing->SetVisAttributes(VisCasing);

                if (casing_sens == 1)
                    gm.AddXYZDetector(log_casing);

                /* Foam aroung source */
                G4int foam_source_copy = 46;

                G4double foam_sourceX = casingX-casingThick*2.;
                G4double foam_sourceY = casingY-casingThick*2.;
                G4double foam_sourceZ = (casingZ-casingThick*2.)/2.;
                G4double foam_cavityThick = 1.;

                G4double foam_source_posX = 0.;
                G4double foam_source_posY = 0.;
                G4double foam_source_posZ = casing_posZ - (casingZ-casingThick*2.)/2. + foam_sourceZ/2.;

                G4Box* solid_foam_source_notsub = new G4Box("solid_foam_source_notsub", foam_sourceX/2., foam_sourceY/2., foam_sourceZ/2.);
                G4Tubs* foam_source_cavity = new G4Tubs("foam_cavity", 0., 2.5, foam_cavityThick/2., 0., 360.);
                G4SubtractionSolid* solid_foam_source = new G4SubtractionSolid("solid_foam_source", solid_foam_source_notsub, foam_source_cavity, 0, G4ThreeVector(0., 0., foam_sourceZ/2.));
                
                G4LogicalVolume* log_foam_source = new G4LogicalVolume(solid_foam_source, foam_mat, "log_foam_source");

                G4VPhysicalVolume* phys_foam_source = new G4PVPlacement(0, G4ThreeVector(foam_source_posX, foam_source_posY, foam_source_posZ), log_foam_source, "foam_source", log_chamber, false, foam_source_copy, true);
                
                G4VisAttributes* VisFoam_source = new G4VisAttributes(G4Colour::Brown());
                log_foam_source->SetVisAttributes(VisFoam_source);

                if (foam_source_sens == 1)
                    gm.AddXYZDetector(log_foam_source);                

            }

            // Case with collimated sources
            if (is_collimated == 1) {

                /* 3D printed aligner */
                G4int aligner_copy = 43;

                auto rotation_aligner = new G4RotationMatrix();
                rotation_aligner->rotateZ(180*deg);

                G4double aligner_posX = 17.5;
                G4double aligner_posY = 90.;
                G4double aligner_posZ = -53.19805;

                auto mesh_aligner = CADMesh::TessellatedMesh::FromPLY(cad_path+"/CollimatorSheet.stl");
                G4VSolid* solid_aligner = mesh_aligner->GetSolid();

                G4LogicalVolume* log_aligner = new G4LogicalVolume(solid_aligner, plastic_mat, "log_aligner");
                G4VPhysicalVolume* phys_aligner = new G4PVPlacement(rotation_aligner, G4ThreeVector(aligner_posX, aligner_posY, aligner_posZ), log_aligner, "aligner", log_chamber, false, aligner_copy, true);
        
                G4VisAttributes* VisAligner = new G4VisAttributes(G4Colour::White());
                log_aligner->SetVisAttributes(VisAligner);

                if (aligner_sens == 1)
                    gm.AddXYZDetector(log_aligner);

                /* Collimator */
                G4int coll_copy = 44;

                G4double innerR = 8.;
                G4double outerR = 50.;
                G4double collH = 85.;

                G4double coll_posX = beamX*cm;
                G4double coll_posY = beamY*cm;
                G4double coll_posZ = aligner_posZ + 82.5 + collH/2.;

                G4Tubs* solid_coll = new G4Tubs("solid_coll", innerR, outerR, collH/2., 0., 360.);
                G4LogicalVolume* log_coll = new G4LogicalVolume(solid_coll, coll_mat, "log_coll");
                G4VPhysicalVolume* phys_coll = new G4PVPlacement(0, G4ThreeVector(coll_posX, coll_posY, coll_posZ), log_coll, "collimator", log_chamber, false, coll_copy, true);

                G4VisAttributes* VisColl = new G4VisAttributes(G4Colour::Black());
                log_coll->SetVisAttributes(VisColl);

                if (coll_sens == 1)
                    gm.AddXYZDetector(log_coll);
            }

            /*** BGO crystals ***/
            G4double posY_BGO1 = -122.555; // from housing drawing
            G4double posY_BGO2 = 0.;
            G4double posY_BGO3 = 122.555; // from housing drawing

            /** BGO1 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO1 = 100; 

            G4Box* solid_refl3_notsub_BGO1 = new G4Box("solid_refl3_notsub_BGO1", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO1 = new G4Box("solid_sub_BGO1", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO1 = new G4SubtractionSolid("solid_refl3_BGO1", solid_refl3_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO1 = new G4LogicalVolume(solid_refl3_BGO1, tefl_mat, "log_refl3_BGO1");

            G4VPhysicalVolume* phys_refl3_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO1, 0.), log_refl3_BGO1, "refl3_BGO1", log_chamber, false, tefl_copy_BGO1, true);
            
            G4VisAttributes* Visrefl3_BGO1 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO1->SetVisAttributes(Visrefl3_BGO1);

            if (refl3_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl3_BGO1);

            /* Tetratex */
            G4int refl2_copy_BGO1 = 110; 

            G4double xlen2 = scintX/2. + refl1Thick*2 + refl2Thick;
            G4double ylen2 = scintY/2. + refl1Thick*2 + refl2Thick;
            G4double zlen2 = scintZ/2. + refl1Thick*2 + refl2Thick;

            G4Box* solid_refl2_notsub_BGO1 = new G4Box("solid_refl2_BGO1", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO1 = new G4SubtractionSolid("solid_refl2_BGO1", solid_refl2_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO1 = new G4LogicalVolume(solid_refl2_BGO1, refl2_mat, "log_refl2_BGO1");

            G4VPhysicalVolume* phys_refl2_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO1, "refl2_BGO1", log_refl3_BGO1, false, refl2_copy_BGO1, true);
            
            G4VisAttributes* Visrefl2_BGO1 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO1->SetVisAttributes(Visrefl2_BGO1);

            if (refl2_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl2_BGO1);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO1 = 120; 

            G4double xlen3 = scintX/2. + refl1Thick*2;
            G4double ylen3 = scintY/2. + refl1Thick*2;
            G4double zlen3 = scintZ/2. + refl1Thick*2;

            G4Box* solid_refl1_notsub_BGO1 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO1= new G4SubtractionSolid("solid_refl1_BGO1", solid_refl1_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO1 = new G4LogicalVolume(solid_refl1_BGO1, refl1_mat, "log_refl1_BGO1");

            G4VPhysicalVolume* phys_refl1_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO1, "refl1_BGO1", log_refl2_BGO1, false, refl1_copy_BGO1, true);
            
            G4VisAttributes* Visrefl1_BGO1 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO1->SetVisAttributes(Visrefl1_BGO1);

            if (refl1_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl1_BGO1);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO1 = 130; 

            G4double xlen4 = scintX/2. + refl1Thick;
            G4double ylen4 = scintY/2. + refl1Thick;
            G4double zlen4 = scintZ/2. + refl1Thick;

            G4Box* solid_refl11_notsub_BGO1 = new G4Box("solid_refl11_BGO1", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO1 = new G4SubtractionSolid("solid_refl11_BGO1", solid_refl11_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO1 = new G4LogicalVolume(solid_refl11_BGO1, refl1_mat, "log_refl11_BGO1");

            G4VPhysicalVolume* phys_refl11_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO1, "refl11_BGO1", log_refl1_BGO1, false, refl11_copy_BGO1, true);
            
            G4VisAttributes* Visrefl11_BGO1 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO1->SetVisAttributes(Visrefl11_BGO1);

            if (refl11_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl11_BGO1);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO1 = 140; 

            G4double xlen5 = refl2Thick; // I set directly two layers as one with the reflectivity of two (Janecek)
            G4double ylen5 = scintY/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double zlen5 = scintZ/2. + refl1Thick*2 + refl2Thick + refl3Thick;

            G4Box* solid_SiPMface_notsub_BGO1 = new G4Box("solid_SiPMface_BGO1", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO1 = new G4Box("solid_sub_BGO1", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO1 = new G4SubtractionSolid("solid_SiPMface_BGO1", solid_SiPMface_notsub_BGO1, SiPM_sub_BGO1, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO1 = new G4LogicalVolume(solid_SiPMface_BGO1, refl2_mat, "log_SiPMface_BGO1");

            G4VPhysicalVolume* phys_SiPMface_BGO1 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO1, 0.), log_SiPMface_BGO1, "SiPMface_BGO1", log_chamber, false, SiPMface_copy_BGO1, true);
            
            G4VisAttributes* VisSiPMface_BGO1 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO1->SetVisAttributes(VisSiPMface_BGO1);

            if (refl2_sens_BGO1 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO1);

            /* scintillator */
            G4int scint_copy_BGO1 = 150;

            G4Box* solid_scint_BGO1 = new G4Box("solid_scint_BGO1", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO1 = new G4LogicalVolume(solid_scint_BGO1, bgo_mat, "log_scint_BGO1");

            G4VPhysicalVolume* phys_scint_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO1, "scint_BGO1", log_refl11_BGO1, false, scint_copy_BGO1, true);

            G4VisAttributes* VisScint_BGO1 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO1->SetVisAttributes(VisScint_BGO1);

            if (scint_sens_BGO1 == 1)
                gm.AddXYZDetector(log_scint_BGO1);

            /* Optical coupler */
            
            G4int sy184_copy_BGO1 = 160;

            auto rotation_BGO1 = new G4RotationMatrix();
            rotation_BGO1->rotateY(90*deg);

            G4Box* solid_sy184_BGO1 = new G4Box("solid_sy184_BGO1", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO1 = new G4LogicalVolume(solid_sy184_BGO1, sy184_mat, "log_sy184_BGO1");

            G4VPhysicalVolume* phys_sy184_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ/2., posY_BGO1, 0.), log_sy184_BGO1, "Sylgard184_BGO1", log_chamber, false, sy184_copy_BGO1, true);

            G4VisAttributes* VisSy184_BGO1 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO1->SetVisAttributes(VisSy184_BGO1);

            if (ej560_sens_BGO1 == 1)
                gm.AddXYZDetector(log_sy184_BGO1);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO1 = 210;
            G4int SiPM12_copy_BGO1 = 220;
            G4int SiPM13_copy_BGO1 = 230;
            G4int SiPM21_copy_BGO1 = 240;
            G4int SiPM22_copy_BGO1 = 250;
            G4int SiPM23_copy_BGO1 = 260;
            G4int SiPM31_copy_BGO1 = 270;
            G4int SiPM32_copy_BGO1 = 280;
            G4int SiPM33_copy_BGO1 = 290;

            G4double SiPMX = 6.;
            G4double SiPMY = 6.;
            G4double SiPMZ = 0.1;

            G4Box* solid_SiPM_BGO1 = new G4Box("solid_SiPM_BGO1", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO1 = new G4LogicalVolume(solid_SiPM_BGO1, SiPM_mat, "log_SiPM_BGO1");

            G4VPhysicalVolume* phys_SiPM11_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM11_BGO1", log_chamber, false, SiPM11_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM12_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM12_BGO1", log_chamber, false, SiPM12_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM13_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM13_BGO1", log_chamber, false, SiPM13_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM21_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM21_BGO1", log_chamber, false, SiPM21_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM22_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM22_BGO1", log_chamber, false, SiPM22_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM23_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM23_BGO1", log_chamber, false, SiPM23_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM31_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM31_BGO1", log_chamber, false, SiPM31_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM32_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM32_BGO1", log_chamber, false, SiPM32_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM33_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM33_BGO1", log_chamber, false, SiPM33_copy_BGO1, true);

            G4VisAttributes* VisSiPM_BGO1 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO1->SetVisAttributes(VisSiPM_BGO1);

            if (SiPM_sens_BGO1 == 1)
                gm.AddXYZDetector(log_SiPM_BGO1);

            /** BGO2 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO2 = 10; 

            G4Box* solid_refl3_notsub_BGO2 = new G4Box("solid_refl3_notsub_BGO2", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO2 = new G4Box("solid_sub_BGO2", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO2 = new G4SubtractionSolid("solid_refl3_BGO2", solid_refl3_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO2 = new G4LogicalVolume(solid_refl3_BGO2, tefl_mat, "log_refl3_BGO2");

            G4VPhysicalVolume* phys_refl3_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO2, 0.), log_refl3_BGO2, "refl3_BGO2", log_chamber, false, tefl_copy_BGO2, true);
            
            G4VisAttributes* Visrefl3_BGO2 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO2->SetVisAttributes(Visrefl3_BGO2);

            if (refl3_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl3_BGO2);

            /* Tetratex */
            G4int refl2_copy_BGO2 = 11; 

            G4Box* solid_refl2_notsub_BGO2 = new G4Box("solid_refl2_BGO2", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO2 = new G4SubtractionSolid("solid_refl2_BGO2", solid_refl2_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO2 = new G4LogicalVolume(solid_refl2_BGO2, refl2_mat, "log_refl2_BGO2");

            G4VPhysicalVolume* phys_refl2_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO2, "refl2_BGO2", log_refl3_BGO2, false, refl2_copy_BGO2, true);
            
            G4VisAttributes* Visrefl2_BGO2 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO2->SetVisAttributes(Visrefl2_BGO2);

            if (refl2_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl2_BGO2);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO2 = 12; 

            G4Box* solid_refl1_notsub_BGO2 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO2= new G4SubtractionSolid("solid_refl1_BGO2", solid_refl1_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO2 = new G4LogicalVolume(solid_refl1_BGO2, refl1_mat, "log_refl1_BGO2");

            G4VPhysicalVolume* phys_refl1_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO2, "refl1_BGO2", log_refl2_BGO2, false, refl1_copy_BGO2, true);
            
            G4VisAttributes* Visrefl1_BGO2 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO2->SetVisAttributes(Visrefl1_BGO2);

            if (refl1_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl1_BGO2);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO2 = 13; 

            G4Box* solid_refl11_notsub_BGO2 = new G4Box("solid_refl11_BGO2", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO2 = new G4SubtractionSolid("solid_refl11_BGO2", solid_refl11_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO2 = new G4LogicalVolume(solid_refl11_BGO2, refl1_mat, "log_refl11_BGO2");

            G4VPhysicalVolume* phys_refl11_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO2, "refl11_BGO2", log_refl1_BGO2, false, refl11_copy_BGO2, true);
            
            G4VisAttributes* Visrefl11_BGO2 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO2->SetVisAttributes(Visrefl11_BGO2);

            if (refl11_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl11_BGO2);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO2 = 14; 

            G4Box* solid_SiPMface_notsub_BGO2 = new G4Box("solid_SiPMface_BGO2", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO2 = new G4Box("solid_sub_BGO2", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO2 = new G4SubtractionSolid("solid_SiPMface_BGO2", solid_SiPMface_notsub_BGO2, SiPM_sub_BGO2, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO2 = new G4LogicalVolume(solid_SiPMface_BGO2, refl2_mat, "log_SiPMface_BGO2");

            G4VPhysicalVolume* phys_SiPMface_BGO2 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO2, 0.), log_SiPMface_BGO2, "SiPMface_BGO2", log_chamber, false, SiPMface_copy_BGO2, true);
            
            G4VisAttributes* VisSiPMface_BGO2 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO2->SetVisAttributes(VisSiPMface_BGO2);

            if (refl2_sens_BGO2 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO2);

            /* scintillator */
            G4int scint_copy_BGO2 = 15;

            G4Box* solid_scint_BGO2 = new G4Box("solid_scint_BGO2", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO2 = new G4LogicalVolume(solid_scint_BGO2, bgo_mat, "log_scint_BGO2");

            G4VPhysicalVolume* phys_scint_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO2, "scint_BGO2", log_refl11_BGO2, false, scint_copy_BGO2, true);

            G4VisAttributes* VisScint_BGO2 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO2->SetVisAttributes(VisScint_BGO2);

            if (scint_sens_BGO2 == 1)
                gm.AddXYZDetector(log_scint_BGO2);

            /* Optical coupler */
            
            G4int sy184_copy_BGO2 = 16;

            auto rotation_BGO2 = new G4RotationMatrix();
            rotation_BGO2->rotateY(90*deg);

            G4Box* solid_sy184_BGO2 = new G4Box("solid_sy184_BGO2", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO2 = new G4LogicalVolume(solid_sy184_BGO2, sy184_mat, "log_sy184_BGO2");

            G4VPhysicalVolume* phys_sy184_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ/2., posY_BGO2, 0.), log_sy184_BGO2, "Sylgard184_BGO2", log_chamber, false, sy184_copy_BGO2, true);

            G4VisAttributes* VisSy184_BGO2 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO2->SetVisAttributes(VisSy184_BGO2);

            if (ej560_sens_BGO2 == 1)
                gm.AddXYZDetector(log_sy184_BGO2);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO2 = 21;
            G4int SiPM12_copy_BGO2 = 22;
            G4int SiPM13_copy_BGO2 = 23;
            G4int SiPM21_copy_BGO2 = 24;
            G4int SiPM22_copy_BGO2 = 25;
            G4int SiPM23_copy_BGO2 = 26;
            G4int SiPM31_copy_BGO2 = 27;
            G4int SiPM32_copy_BGO2 = 28;
            G4int SiPM33_copy_BGO2 = 29;

            G4Box* solid_SiPM_BGO2 = new G4Box("solid_SiPM_BGO2", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO2 = new G4LogicalVolume(solid_SiPM_BGO2, SiPM_mat, "log_SiPM_BGO2");

            G4VPhysicalVolume* phys_SiPM11_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM11_BGO2", log_chamber, false, SiPM11_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM12_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM12_BGO2", log_chamber, false, SiPM12_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM13_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM13_BGO2", log_chamber, false, SiPM13_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM21_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM21_BGO2", log_chamber, false, SiPM21_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM22_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM22_BGO2", log_chamber, false, SiPM22_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM23_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM23_BGO2", log_chamber, false, SiPM23_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM31_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM31_BGO2", log_chamber, false, SiPM31_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM32_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM32_BGO2", log_chamber, false, SiPM32_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM33_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM33_BGO2", log_chamber, false, SiPM33_copy_BGO2, true);

            G4VisAttributes* VisSiPM_BGO2 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO2->SetVisAttributes(VisSiPM_BGO2);

            if (SiPM_sens_BGO2 == 1)
                gm.AddXYZDetector(log_SiPM_BGO2);

            /** BGO3 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO3 = 1000; 

            G4Box* solid_refl3_notsub_BGO3 = new G4Box("solid_refl3_notsub_BGO3", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO3 = new G4Box("solid_sub_BGO3", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO3 = new G4SubtractionSolid("solid_refl3_BGO3", solid_refl3_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO3 = new G4LogicalVolume(solid_refl3_BGO3, tefl_mat, "log_refl3_BGO3");

            G4VPhysicalVolume* phys_refl3_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO3, 0.), log_refl3_BGO3, "refl3_BGO3", log_chamber, false, tefl_copy_BGO3, true);
            
            G4VisAttributes* Visrefl3_BGO3 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO3->SetVisAttributes(Visrefl3_BGO3);

            if (refl3_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl3_BGO3);

            /* Tetratex */
            G4int refl2_copy_BGO3 = 1100; 

            G4Box* solid_refl2_notsub_BGO3 = new G4Box("solid_refl2_BGO3", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO3 = new G4SubtractionSolid("solid_refl2_BGO3", solid_refl2_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO3 = new G4LogicalVolume(solid_refl2_BGO3, refl2_mat, "log_refl2_BGO3");

            G4VPhysicalVolume* phys_refl2_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO3, "refl2_BGO3", log_refl3_BGO3, false, refl2_copy_BGO3, true);
            
            G4VisAttributes* Visrefl2_BGO3 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO3->SetVisAttributes(Visrefl2_BGO3);

            if (refl2_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl2_BGO3);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO3 = 1200; 

            G4Box* solid_refl1_notsub_BGO3 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO3= new G4SubtractionSolid("solid_refl1_BGO3", solid_refl1_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO3 = new G4LogicalVolume(solid_refl1_BGO3, refl1_mat, "log_refl1_BGO3");

            G4VPhysicalVolume* phys_refl1_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO3, "refl1_BGO3", log_refl2_BGO3, false, refl1_copy_BGO3, true);
            
            G4VisAttributes* Visrefl1_BGO3 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO3->SetVisAttributes(Visrefl1_BGO3);

            if (refl1_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl1_BGO3);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO3 = 1300; 

            G4Box* solid_refl11_notsub_BGO3 = new G4Box("solid_refl11_BGO3", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO3 = new G4SubtractionSolid("solid_refl11_BGO3", solid_refl11_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO3 = new G4LogicalVolume(solid_refl11_BGO3, refl1_mat, "log_refl11_BGO3");

            G4VPhysicalVolume* phys_refl11_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO3, "refl11_BGO3", log_refl1_BGO3, false, refl11_copy_BGO3, true);
            
            G4VisAttributes* Visrefl11_BGO3 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO3->SetVisAttributes(Visrefl11_BGO3);

            if (refl11_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl11_BGO3);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO3 = 1400; 

            G4Box* solid_SiPMface_notsub_BGO3 = new G4Box("solid_SiPMface_BGO3", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO3 = new G4Box("solid_sub_BGO3", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO3 = new G4SubtractionSolid("solid_SiPMface_BGO3", solid_SiPMface_notsub_BGO3, SiPM_sub_BGO3, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO3 = new G4LogicalVolume(solid_SiPMface_BGO3, refl2_mat, "log_SiPMface_BGO3");

            G4VPhysicalVolume* phys_SiPMface_BGO3 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO3, 0.), log_SiPMface_BGO3, "SiPMface_BGO3", log_chamber, false, SiPMface_copy_BGO3, true);
            
            G4VisAttributes* VisSiPMface_BGO3 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO3->SetVisAttributes(VisSiPMface_BGO3);

            if (refl2_sens_BGO3 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO3);

            /* scintillator */
            G4int scint_copy_BGO3 = 1500;

            G4Box* solid_scint_BGO3 = new G4Box("solid_scint_BGO3", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO3 = new G4LogicalVolume(solid_scint_BGO3, bgo_mat, "log_scint_BGO3");

            G4VPhysicalVolume* phys_scint_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO3, "scint_BGO3", log_refl11_BGO3, false, scint_copy_BGO3, true);

            G4VisAttributes* VisScint_BGO3 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO3->SetVisAttributes(VisScint_BGO3);

            if (scint_sens_BGO3 == 1)
                gm.AddXYZDetector(log_scint_BGO3);

            /* Optical coupler */
            
            G4int sy184_copy_BGO3 = 1600;

            auto rotation_BGO3 = new G4RotationMatrix();
            rotation_BGO3->rotateY(90*deg);

            G4Box* solid_sy184_BGO3 = new G4Box("solid_sy184_BGO3", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO3 = new G4LogicalVolume(solid_sy184_BGO3, sy184_mat, "log_sy184_BGO3");

            G4VPhysicalVolume* phys_sy184_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ/2., posY_BGO3, 0.), log_sy184_BGO3, "Sylgard184_BGO3", log_chamber, false, sy184_copy_BGO3, true);

            G4VisAttributes* VisSy184_BGO3 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO3->SetVisAttributes(VisSy184_BGO3);

            if (ej560_sens_BGO3 == 1)
                gm.AddXYZDetector(log_sy184_BGO3);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO3 = 2100;
            G4int SiPM12_copy_BGO3 = 2200;
            G4int SiPM13_copy_BGO3 = 2300;
            G4int SiPM21_copy_BGO3 = 2400;
            G4int SiPM22_copy_BGO3 = 2500;
            G4int SiPM23_copy_BGO3 = 2600;
            G4int SiPM31_copy_BGO3 = 2700;
            G4int SiPM32_copy_BGO3 = 2800;
            G4int SiPM33_copy_BGO3 = 2900;

            G4Box* solid_SiPM_BGO3 = new G4Box("solid_SiPM_BGO3", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO3 = new G4LogicalVolume(solid_SiPM_BGO3, SiPM_mat, "log_SiPM_BGO3");

            G4VPhysicalVolume* phys_SiPM11_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM11_BGO3", log_chamber, false, SiPM11_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM12_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM12_BGO3", log_chamber, false, SiPM12_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM13_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM13_BGO3", log_chamber, false, SiPM13_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM21_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM21_BGO3", log_chamber, false, SiPM21_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM22_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM22_BGO3", log_chamber, false, SiPM22_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM23_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM23_BGO3", log_chamber, false, SiPM23_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM31_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM31_BGO3", log_chamber, false, SiPM31_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM32_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM32_BGO3", log_chamber, false, SiPM32_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM33_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM33_BGO3", log_chamber, false, SiPM33_copy_BGO3, true);

            G4VisAttributes* VisSiPM_BGO3 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO3->SetVisAttributes(VisSiPM_BGO3);

            if (SiPM_sens_BGO3 == 1)
                gm.AddXYZDetector(log_SiPM_BGO3);
/*
            G4int housing_copy = 50;

            G4double housingThick = 2.54;
            G4double housingGap = 1.905;
            G4double housingX = scintX + housingThick*2 + optZ + SiPMZ;
            G4double housingY = ylen*2*3 + housingGap*2 + housingThick*2;
            G4double housingZ = zlen*2 + housingThick*2;

            G4Box* solid_housing_notsub = new G4Box("solid_housing_notsub", housingX/2., housingY/2., housingZ/2.);
            G4Box* BGO_cavity = new G4Box("solid_BGO_cavity", (scintX + (xlen*2-scintX)/2. + optZ + SiPMZ)/2., ylen, zlen);
            G4SubtractionSolid* solid_housing1 = new G4SubtractionSolid("solid_housing1", solid_housing_notsub, BGO_cavity, 0, G4ThreeVector(0., 0., 0.));
            G4SubtractionSolid* solid_housing2 = new G4SubtractionSolid("solid_housing2", solid_housing1, BGO_cavity, 0, G4ThreeVector(0., ylen*2+housingGap, 0.));
            G4SubtractionSolid* solid_housing = new G4SubtractionSolid("solid_housing", solid_housing2, BGO_cavity, 0, G4ThreeVector(0., -ylen*2-housingGap, 0.));

            G4LogicalVolume* log_housing = new G4LogicalVolume(solid_housing, Al_mat, "log_housing");
            G4VPhysicalVolume* phys_housing = new G4PVPlacement(0, G4ThreeVector(scintX/4.-xlen/2.+optZ/2.+SiPMZ/2., 0., 0.), log_housing, "housing", log_chamber, false, housing_copy, true);

            G4VisAttributes* VisHousing = new G4VisAttributes(G4Colour::Grey());
            log_housing->SetVisAttributes(VisHousing);

            if (housing_sens == 1)
                gm.AddXYZDetector(log_housing);
*/  

            /* --------------------- OPTICAL SURFACES ----------------------- */

            /* We use the Unified Model.*/
            /* For each BGO, there are two optical surfaces between the BGO and the reflective layers:*/
            /* 1. BGO -> VM2000 (non-SiPM face) */
            /*    Finish: Polishedbackpainted, sigma = 1.3 deg (Janecek 2009), SS reflection and reflectivity to ~99% (Janecek 2012) (type1) */
            /* 2. BGO -> Tetratex (SiPM face)   */
            /*    Finish: Groundbackpainted, sigma = 1.3 deg (Janecek 2009), L reflection and reflectivity to ~96% (Janecek 2012) (type4) */

            /** BGO1 **/

            /* BGO1 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO1 = new G4OpticalSurface("panel_OPTSURFACE_BGO1");
            G4LogicalBorderSurface* panel_BORDER1_BGO1 = new G4LogicalBorderSurface("panel_BORDER1_BGO1", phys_scint_BGO1, phys_refl11_BGO1, panel_OPTSURFACE_BGO1);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO1->SetModel(unified);
                panel_OPTSURFACE_BGO1->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO1->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO1->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO1 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO1 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO1 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO1 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO1, OPTSURFACE_refl_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_SPECULARLOBECONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_SPECULARSPIKECONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_BACKSCATTERCONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("RINDEX", OPTSURFACE_Energy_BGO1, OPTSURFACE_RIND_BGO1);
                panel_OPTSURFACE_MPT_BGO1->DumpTable();
            }
            panel_OPTSURFACE_BGO1->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO1);

            /* BGO1 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO1 = new G4OpticalSurface("panel_OPTSURFACE2_BGO1");
            G4LogicalBorderSurface* panel_BORDER2_BGO1 = new G4LogicalBorderSurface("panel_BORDER2_BGO1", phys_scint_BGO1, phys_SiPMface_BGO1, panel_OPTSURFACE2_BGO1);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO1 = new G4MaterialPropertiesTable();

            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO1->SetModel(unified);
                panel_OPTSURFACE2_BGO1->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO1->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO1->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO1 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO1 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO1 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO1->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_SPECULARLOBECONSTANT_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_BACKSCATTERCONSTANT_BGO1);
                std::vector<G4double> OPTSURFACE2_RIND_BGO1 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_refl_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_RIND_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->DumpTable();
            }

            panel_OPTSURFACE2_BGO1->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO1);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO1 = new G4OpticalSurface("external_OPTSURFACE_BGO1");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO1 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO1", phys_sy184_BGO1, phys_SiPMface_BGO1, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO1", phys_SiPM11_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO1", phys_SiPM12_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO1", phys_SiPM13_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO1", phys_SiPM21_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO1", phys_SiPM22_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO1", phys_SiPM23_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO1", phys_SiPM31_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO1", phys_SiPM32_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO1", phys_SiPM33_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO1->SetModel(unified);
            external_OPTSURFACE_BGO1->SetType(dielectric_metal);
            external_OPTSURFACE_BGO1->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO1  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO1  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO1 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO1, OPTSURFACE_ext_refl_BGO1);
            external_OPTSURFACE_MPT_BGO1->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO1, OPTSURFACE_ext_eff_BGO1);

            external_OPTSURFACE_MPT_BGO1->DumpTable();

            external_OPTSURFACE_BGO1->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO1);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO1 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO1");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM11_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM12_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM13_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM21_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM22_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM23_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM31_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM32_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM33_BGO1, OptSiPM_OPTSURFACE_BGO1);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO1->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO1->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO1->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO1  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO1  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO1 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO1, OPTSURFACE_OptSiPM_refl_BGO1);
            OptSiPM_OPTSURFACE_MPT_BGO1->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO1, OPTSURFACE_OptSiPM_eff_BGO1);

            OptSiPM_OPTSURFACE_MPT_BGO1->DumpTable();

            OptSiPM_OPTSURFACE_BGO1->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO1);

            /** BGO2 **/

            /* BGO2 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO2 = new G4OpticalSurface("panel_OPTSURFACE_BGO2");
            G4LogicalBorderSurface* panel_BORDER1_BGO2 = new G4LogicalBorderSurface("panel_BORDER1_BGO2", phys_scint_BGO2, phys_refl11_BGO2, panel_OPTSURFACE_BGO2);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO2->SetModel(unified);
                panel_OPTSURFACE_BGO2->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO2->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO2->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO2 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO2 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO2 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO2, OPTSURFACE_refl_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_SPECULARLOBECONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_SPECULARSPIKECONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_BACKSCATTERCONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("RINDEX", OPTSURFACE_Energy_BGO2, OPTSURFACE_RIND_BGO2);
                panel_OPTSURFACE_MPT_BGO2->DumpTable();
            }
            panel_OPTSURFACE_BGO2->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO2);

            /* BGO2 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO2 = new G4OpticalSurface("panel_OPTSURFACE2_BGO2");
            G4LogicalBorderSurface* panel_BORDER2_BGO2 = new G4LogicalBorderSurface("panel_BORDER2_BGO2", phys_scint_BGO2, phys_SiPMface_BGO2, panel_OPTSURFACE2_BGO2);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO2 = new G4MaterialPropertiesTable();
            
            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO2->SetModel(unified);
                panel_OPTSURFACE2_BGO2->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO2->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO2->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO2 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO2 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO2->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_SPECULARLOBECONSTANT_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_BACKSCATTERCONSTANT_BGO2);
                std::vector<G4double> OPTSURFACE2_RIND_BGO2 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_refl_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_RIND_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->DumpTable();
            }
            panel_OPTSURFACE2_BGO2->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO2);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO2 = new G4OpticalSurface("external_OPTSURFACE_BGO2");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO2 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO2", phys_sy184_BGO2, phys_SiPMface_BGO2, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO2", phys_SiPM11_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO2", phys_SiPM12_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO2", phys_SiPM13_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO2", phys_SiPM21_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO2", phys_SiPM22_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO2", phys_SiPM23_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO2", phys_SiPM31_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO2", phys_SiPM32_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO2", phys_SiPM33_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO2->SetModel(unified);
            external_OPTSURFACE_BGO2->SetType(dielectric_metal);
            external_OPTSURFACE_BGO2->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO2  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO2  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO2 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO2, OPTSURFACE_ext_refl_BGO2);
            external_OPTSURFACE_MPT_BGO2->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO2, OPTSURFACE_ext_eff_BGO2);

            external_OPTSURFACE_MPT_BGO2->DumpTable();

            external_OPTSURFACE_BGO2->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO2);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO2 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO2");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM11_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM12_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM13_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM21_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM22_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM23_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM31_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM32_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM33_BGO2, OptSiPM_OPTSURFACE_BGO2);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO2->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO2->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO2->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO2  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO2  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO2 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO2, OPTSURFACE_OptSiPM_refl_BGO2);
            OptSiPM_OPTSURFACE_MPT_BGO2->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO2, OPTSURFACE_OptSiPM_eff_BGO2);

            OptSiPM_OPTSURFACE_MPT_BGO2->DumpTable();

            OptSiPM_OPTSURFACE_BGO2->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO2);

            /** BGO3 **/

            /* BGO3 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO3 = new G4OpticalSurface("panel_OPTSURFACE_BGO3");
            G4LogicalBorderSurface* panel_BORDER1_BGO3 = new G4LogicalBorderSurface("panel_BORDER1_BGO3", phys_scint_BGO3, phys_refl11_BGO3, panel_OPTSURFACE_BGO3);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO3->SetModel(unified);
                panel_OPTSURFACE_BGO3->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO3->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO3->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO3 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO3 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO3 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO3 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO3, OPTSURFACE_refl_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_SPECULARLOBECONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_SPECULARSPIKECONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_BACKSCATTERCONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("RINDEX", OPTSURFACE_Energy_BGO3, OPTSURFACE_RIND_BGO3);
                panel_OPTSURFACE_MPT_BGO3->DumpTable();
            }
            panel_OPTSURFACE_BGO3->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO3);

            /* BGO3 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO3 = new G4OpticalSurface("panel_OPTSURFACE2_BGO3");
            G4LogicalBorderSurface* panel_BORDER2_BGO3 = new G4LogicalBorderSurface("panel_BORDER2_BGO3", phys_scint_BGO3, phys_SiPMface_BGO3, panel_OPTSURFACE2_BGO3);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO3 = new G4MaterialPropertiesTable();
            
            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO3->SetModel(unified);
                panel_OPTSURFACE2_BGO3->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO3->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO3->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO3 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO3 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO3 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO3->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_SPECULARLOBECONSTANT_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_BACKSCATTERCONSTANT_BGO3);
                std::vector<G4double> OPTSURFACE2_RIND_BGO3 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_refl_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_RIND_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->DumpTable();
            }
            panel_OPTSURFACE2_BGO3->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO3);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO3 = new G4OpticalSurface("external_OPTSURFACE_BGO3");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO3 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO3", phys_sy184_BGO3, phys_SiPMface_BGO3, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO3", phys_SiPM11_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO3", phys_SiPM12_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO3", phys_SiPM13_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO3", phys_SiPM21_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO3", phys_SiPM22_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO3", phys_SiPM23_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO3", phys_SiPM31_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO3", phys_SiPM32_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO3", phys_SiPM33_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO3->SetModel(unified);
            external_OPTSURFACE_BGO3->SetType(dielectric_metal);
            external_OPTSURFACE_BGO3->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO3  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO3  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO3 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO3, OPTSURFACE_ext_refl_BGO3);
            external_OPTSURFACE_MPT_BGO3->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO3, OPTSURFACE_ext_eff_BGO3);

            external_OPTSURFACE_MPT_BGO3->DumpTable();

            external_OPTSURFACE_BGO3->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO3);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO3 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO3");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM11_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM12_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM13_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM21_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM22_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM23_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM31_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM32_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM33_BGO3, OptSiPM_OPTSURFACE_BGO3);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO3->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO3->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO3->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO3  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO3  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO3 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO3, OPTSURFACE_OptSiPM_refl_BGO3);
            OptSiPM_OPTSURFACE_MPT_BGO3->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO3, OPTSURFACE_OptSiPM_eff_BGO3);

            OptSiPM_OPTSURFACE_MPT_BGO3->DumpTable();

            OptSiPM_OPTSURFACE_BGO3->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO3);

        }

        if (setGeomType == 9){

            // Set up some dimensions (in mm)
            // BGO
            G4double scintX = 198.;
            G4double scintY = 118.;
            G4double scintZ = 23.;
            // Reflective layers: two layers of VM2000, one layer of Tetratex, one layer of Teflon PTFE
            G4double refl1Thick = 0.065; // thickness of first VM200 reflective layer (Janecek 2012)
            G4double refl11Thick = 0.065; // thickness of second VM2000 reflective layer (Janecek 2012)
            G4double refl2Thick = 0.254; // thickness of Tetratex reflective layer (prototype description)
            G4double refl3Thick = 0.0762; // thickness of Teflon relfective layer (https://www.mcmaster.com/8569K58/)
            // Optical coupler
            G4double optX = 18.; // optical coupler x length
            G4double optY = 18.; // optical coupler y length
            G4double optZ = 1.; // optical coupler thickness

            // Half lengths of scintillator + reflective layers
            G4double xlen = scintX/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double ylen = scintY/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double zlen = scintZ/2. + refl1Thick*2 + refl2Thick + refl3Thick;

            G4double pos_sub = xlen;

            /*** Aluminum housing ***/
            G4int housing_copy = 30;

            auto rotation_housing = new G4RotationMatrix();
            rotation_housing->rotateX(90*deg);
            rotation_housing->rotateY(90*deg);

            G4String cad_path = "";
            gm.config->readInto(cad_path, "GEOM.CAD.PATH");
            G4cout << "GEOM.CAD.PATH: " << cad_path << G4endl;

            auto mesh_housing = CADMesh::TessellatedMesh::FromPLY(cad_path+"/ACS-ST-0101revB_ACS_X-WALL_CHASSIS.stl");
            G4VSolid* solid_housing = mesh_housing->GetSolid();

            G4LogicalVolume* log_housing = new G4LogicalVolume(solid_housing, Al_mat, "log_housing");
            G4VPhysicalVolume* phys_housing = new G4PVPlacement(rotation_housing, G4ThreeVector(xlen+7., 0., zlen-27.5), log_housing, "housing", log_chamber, false, housing_copy, true);
    
            G4VisAttributes* VisHousing = new G4VisAttributes(G4Colour::Grey());
            log_housing->SetVisAttributes(VisHousing);

            if (housing_sens == 1)
                gm.AddXYZDetector(log_housing);

            /* End caps */
            G4int cap_front_copy = 31;
            G4int cap_back_copy = 32;

            G4double capX = 2.54; // from housing drawing
            G4double capY = 365.76; // from housing drawing
            G4double capZ = 26.035; // from housing drawing

            G4Box* solid_cap = new G4Box("solid_cap", capX/2., capY/2., capZ/2.);
            G4LogicalVolume* log_cap = new G4LogicalVolume(solid_cap, Al_mat, "log_cap");
            G4VPhysicalVolume* phys_cap_front = new G4PVPlacement(0, G4ThreeVector(102., 0., 0.), log_cap, "cap_front", log_chamber, false, cap_front_copy, true);
            G4VPhysicalVolume* phys_cap_back = new G4PVPlacement(0, G4ThreeVector(-102., 0., 0.), log_cap, "cap_back", log_chamber, false, cap_back_copy, true);

            G4VisAttributes* VisCap = new G4VisAttributes(G4Colour::Grey());
            log_cap->SetVisAttributes(VisCap);

            if (cap_sens == 1)
                gm.AddXYZDetector(log_cap);

            /* Anti-static mat */
            G4int mat_copy = 33;

            G4double matX = 1524.;
            G4double matY = 1828.8;
            G4double matZ = 2.;

            G4double mat_posX = -matX/4.;
            G4double mat_posY = 0.;
            G4double mat_posZ = - 19.38155 - matZ/2.;

            G4Box* solid_mat = new G4Box("solid_mat", matX/2., matY/2., matZ/2.);
            G4LogicalVolume* log_mat = new G4LogicalVolume(solid_mat, rubber_mat, "log_mat");
            G4VPhysicalVolume* phys_mat = new G4PVPlacement(0, G4ThreeVector(mat_posX, mat_posY, mat_posZ), log_mat, "mat", log_chamber, false, mat_copy, true);

            G4VisAttributes* VisMat = new G4VisAttributes(G4Colour::Cyan());
            log_mat->SetVisAttributes(VisMat);

            if (mat_sens == 1)
                gm.AddXYZDetector(log_mat);

            /* Wooden table */
            G4int table_copy = 34;

            G4double tableX = matX;
            G4double tableY = matY;
            G4double tableZ = 38.1;

            G4double table_posX = mat_posX;
            G4double table_posY = mat_posY;
            G4double table_posZ = mat_posZ - matZ/2. - tableZ/2.;

            G4Box* solid_table = new G4Box("solid_table", tableX/2., tableY/2, tableZ/2.);
            G4LogicalVolume* log_table = new G4LogicalVolume(solid_table, wood2_mat, "log_table");
            G4VPhysicalVolume* phys_table = new G4PVPlacement(0, G4ThreeVector(table_posX, table_posY, table_posZ), log_table, "table", log_chamber, false, table_copy, true);

            G4VisAttributes* VisTable = new G4VisAttributes(G4Colour::Brown());
            log_table->SetVisAttributes(VisTable);

            if (table_sens == 1)
                gm.AddXYZDetector(log_table);

            /* Black coat */
            G4int coat_copy = 35;

            G4double coatX = 225.;
            G4double coatY = 450.;
            G4double coatZ = 60.;
            G4double coatThick = 2.;

            G4double coat_posX = 0.;
            G4double coat_posY = 0.;
            G4double coat_posZ = 12.;

            G4Box* coat_notsub = new G4Box("coat_notsub", coatX/2., coatY/2., coatZ/2.);
            G4Box* cavity = new G4Box("cavity", coatX/2-coatThick, coatY/2-coatThick, coatZ/2-coatThick);
            G4SubtractionSolid* coat_withcavity = new G4SubtractionSolid("coat_withcavity", coat_notsub, cavity, 0, G4ThreeVector(0., 0., 0.));
            G4Box* bottom_panel = new G4Box("bottom_panel", coatX/2-coatThick, coatY/2-coatThick, coatThick/2.);
            G4SubtractionSolid* solid_coat = new G4SubtractionSolid("solid_coat", coat_withcavity, bottom_panel, 0, G4ThreeVector(0., 0., -coatZ/2+coatThick/2.));
            G4LogicalVolume* log_coat = new G4LogicalVolume(solid_coat, coat_mat, "log_coat");
            G4VPhysicalVolume* phys_coat = new G4PVPlacement(0, G4ThreeVector(coat_posX, coat_posY, coat_posZ), log_coat, "coat", log_chamber, false, coat_copy, true);

            G4VisAttributes* VisCoat = new G4VisAttributes(G4Colour::Black());
            log_coat->SetVisAttributes(VisCoat);

            /*** Source holder ***/
            G4double poleH = 647.7;
            G4double poleR = 6.35;

            G4double stakeH = 178;
            G4double stakeR = 3.175;

            G4double holderX = 63.5;
            G4double holderY = 25.4;
            G4double holderZ = 12.7;
            G4double holderThick = 1.;

            G4double screwR = 2.38;
            G4double screwH = holderY - holderThick*2.;
            G4double screwendR = screwR;
            G4double screwendH = 15.875;

            /* Pole */
            G4int pole_copy = 36;

            G4double pole_posX = - coatX/2. - 76.2;
            G4double pole_posY = -stakeR - holderY/2. - poleR;
            G4double pole_posZ = mat_posZ + matZ/2. + poleH/2.;

            G4Tubs* solid_pole = new G4Tubs("solid_pole", 0., poleR, poleH/2., 0., 360.);
            G4LogicalVolume* log_pole = new G4LogicalVolume(solid_pole, steel_mat, "log_pole");
            G4VPhysicalVolume* phys_pole = new G4PVPlacement(0, G4ThreeVector(pole_posX, pole_posY, pole_posZ), log_pole, "pole", log_chamber, false, pole_copy, true);

            G4VisAttributes* VisPole = new G4VisAttributes(G4Colour::Grey());
            log_pole->SetVisAttributes(VisPole);

            if (pole_sens == 1)
                gm.AddXYZDetector(log_pole);

            /* Base */
            G4int base_copy = 37;

            G4double baseR = 19.;
            G4double baseH = 38.1;
            G4double footX = 76.2;
            G4double footY = baseR;
            G4double footZ = baseH;
        
            G4double base_posX = pole_posX;
            G4double base_posY = pole_posY;
            G4double base_posZ = pole_posZ - poleH/2. + baseH/2.;

            auto rotation_foot1 = new G4RotationMatrix();
            rotation_foot1->rotateZ(-60*deg);
            auto rotation_foot2 = new G4RotationMatrix();
            rotation_foot2->rotateZ(60*deg);

            G4Tubs* solid_tube = new G4Tubs("solid_tube", poleR, baseR, baseH/2., 0., 360.);
            G4Box* solid_foot = new G4Box("solid_foot", footX/2., footY/2., footZ/2.);
            G4UnionSolid* solid_base1 = new G4UnionSolid("solid_base1", solid_tube, solid_foot, rotation_foot1, G4ThreeVector(baseR*2-10., baseR+footX/2.-10., 0.));
            G4UnionSolid* solid_base2 = new G4UnionSolid("solid_base2", solid_base1, solid_foot, rotation_foot2, G4ThreeVector(baseR*2-10., -(baseR+footX/2.-10.), 0.));
            G4UnionSolid* solid_base = new G4UnionSolid("solid_base", solid_base2, solid_foot, 0, G4ThreeVector(-baseR-footX/2.+2., 0., 0.));
            G4LogicalVolume* log_base = new G4LogicalVolume(solid_base, steel_mat, "log_base");
            G4VPhysicalVolume* phys_base = new G4PVPlacement(0, G4ThreeVector(base_posX, base_posY, base_posZ), log_base, "base", log_chamber, false, base_copy, true);

            G4VisAttributes* VisBase = new G4VisAttributes(G4Colour::Black());
            log_base->SetVisAttributes(VisBase);

            if (base_sens == 1)
                gm.AddXYZDetector(log_base);

            /* Stake */
            G4int stake_copy = 38;

            G4double stake_posX = pole_posX + 14.;
            G4double stake_posY = -holderY/2.;
            G4double stake_posZ = 585;

            auto rotation_stake = new G4RotationMatrix();
            rotation_stake->rotateY(90*deg);

            G4Tubs* solid_stake = new G4Tubs("solid_stake", 0., stakeR, stakeH/2., 0., 360.);
            G4LogicalVolume* log_stake = new G4LogicalVolume(solid_stake, steel_mat, "log_stake");
            G4VPhysicalVolume* phys_stake = new G4PVPlacement(rotation_stake, G4ThreeVector(stake_posX, stake_posY, stake_posZ), log_stake, "stake", log_chamber, false, stake_copy, true);

            G4VisAttributes* VisStake = new G4VisAttributes(G4Colour::Grey());
            log_stake->SetVisAttributes(VisStake);

            if (stake_sens == 1)
                gm.AddXYZDetector(log_stake);

            /* Source holder */
            G4int holder_copy = 39;

            G4double holder_posX = stake_posX + stakeH/2. + holderX/2.;
            G4double holder_posY = 0.;
            G4double holder_posZ = stake_posZ;

            G4Box* holder_notsub = new G4Box("holder_notsub", holderX/2., holderY/2., holderZ/2.);
            G4Box* cavity_holder = new G4Box("cavity_holder", holderX, holderY/2.-holderThick, holderZ/2.-holderThick);
            G4SubtractionSolid* holder_sub = new G4SubtractionSolid("holder_sub", holder_notsub, cavity_holder, 0, G4ThreeVector(0., 0., 0.));
            G4Box* top_bottom_face = new G4Box("top_bottom_face", holderX/2, 15.875/2., 20.);
            G4SubtractionSolid* solid_holder = new G4SubtractionSolid("solid_holder", holder_sub, top_bottom_face, 0, G4ThreeVector(12.7, 0., 0.));
            G4LogicalVolume* log_holder = new G4LogicalVolume(solid_holder, steel_mat, "log_holder");
            G4VPhysicalVolume* phys_holder = new G4PVPlacement(0, G4ThreeVector(holder_posX, holder_posY, holder_posZ), log_holder, "holder", log_chamber, false, holder_copy, true);

            G4VisAttributes* VisHolder = new G4VisAttributes(G4Colour::Grey());
            log_holder->SetVisAttributes(VisHolder);

            if (holder_sens == 1)
                gm.AddXYZDetector(log_holder);       

            /* Screw */
            G4int screw_copy = 40;

            G4double screw_posX = holder_posX + 6.35;
            G4double screw_posY = holder_posY;
            G4double screw_posZ = holder_posZ;

            auto rotation_screw = new G4RotationMatrix();
            rotation_screw->rotateX(90*deg);

            G4Tubs* solid_screw = new G4Tubs("solid_screw", 0., screwR, screwH/2., 0., 360.);
            G4LogicalVolume* log_screw = new G4LogicalVolume(solid_screw, steel_mat, "log_screw");
            G4VPhysicalVolume* phys_screw = new G4PVPlacement(rotation_screw, G4ThreeVector(screw_posX, screw_posY, screw_posZ), log_screw, "screw", log_chamber, false, screw_copy, true);

            G4int screwend_copy = 41;

            G4double screwend_posX = screw_posX ;
            G4double screwend_posY = holder_posY - screwH/2. - holderThick - screwendH/2.;
            G4double screwend_posZ = holder_posZ;

            G4Tubs* solid_screwend = new G4Tubs("solid_screwend", 0., screwendR, screwendH/2., 0., 360.);
            G4LogicalVolume* log_screwend = new G4LogicalVolume(solid_screwend, steel_mat, "log_screwend");
            G4VPhysicalVolume* phys_screwend = new G4PVPlacement(rotation_screw, G4ThreeVector(screwend_posX, screwend_posY, screwend_posZ), log_screwend, "screwend", log_chamber, false, screwend_copy, true);

            G4int screwcap_copy = 42;

            G4double rectX = 8.;
            G4double rectY = 12.7;
            G4double rectZ = 1.;
            G4double circR = 12.7;
            G4double circH = 1.;

            G4double screwcap_posX = screw_posX ;
            G4double screwcap_posY = holder_posY + screwH/2. + holderThick + rectY/2.;
            G4double screwcap_posZ = holder_posZ;

            G4Box* solid_rect = new G4Box("solid_rect", rectX/2., rectY/2., rectZ/2.);
            G4Tubs* solid_circ = new G4Tubs("solid_circ", 0., circR, circH/2., 0., 360.);
            G4UnionSolid* solid_screwcap = new G4UnionSolid("solid_screwcap", solid_rect, solid_circ, 0, G4ThreeVector(0., rectY/2.+circR-2., 0.));
            G4LogicalVolume* log_screwcap = new G4LogicalVolume(solid_screwcap, steel_mat, "log_screwcap");
            G4VPhysicalVolume* phys_screwcap = new G4PVPlacement(0, G4ThreeVector(screwcap_posX, screwcap_posY, screwcap_posZ), log_screwcap, "screwcap", log_chamber, false, screwcap_copy, true);
     
            G4VisAttributes* VisScrew = new G4VisAttributes(G4Colour::Grey());
            log_screw->SetVisAttributes(VisScrew);
            log_screwend->SetVisAttributes(VisScrew);
            log_screwcap->SetVisAttributes(VisScrew);

            if (screw_sens == 1){
                gm.AddXYZDetector(log_screw);
                gm.AddXYZDetector(log_screwend);
                gm.AddXYZDetector(log_screwcap);
            }
               
            /* Foam */
            G4int foam_copy = 43;

            G4double foamX = 38.1;
            G4double foamY = 63.5;
            G4double foamZ = 12.7;

            G4double foam_posX = screw_posX;
            G4double foam_posY = holder_posY;
            G4double foam_posZ = holder_posZ + holderZ/2. + foamZ/2.;

            G4Box* solid_foam = new G4Box("solid_foam", foamX/2., foamY/2., foamZ/2.);
            G4LogicalVolume* log_foam = new G4LogicalVolume(solid_foam, foam_mat, "log_foam");
            G4VPhysicalVolume* phys_foam = new G4PVPlacement(0, G4ThreeVector(foam_posX, foam_posY, foam_posZ), log_foam, "foam", log_chamber, false, foam_copy, true);

            G4VisAttributes* VisFoam = new G4VisAttributes(G4Colour::Black());
            log_foam->SetVisAttributes(VisFoam);

            if (foam_sens == 1)
                gm.AddXYZDetector(log_foam);

            /* Lead block */
            G4int block_copy = 44;

            G4double blockX = 80.;
            G4double blockY = 40.;
            G4double blockZ = 160.;

            G4double block_posX = scintX/2. - blockX/2.;
            G4double block_posY = scintY/2. + scintY + 300.;
            G4double block_posZ = mat_posZ + matZ/2. + blockZ/2.;

            G4Box* solid_block = new G4Box("solid_block", blockX/2., blockY/2., blockZ/2.);
            G4LogicalVolume* log_block = new G4LogicalVolume(solid_block, coll_mat, "log_block");
            G4VPhysicalVolume* phys_block = new G4PVPlacement(0, G4ThreeVector(block_posX, block_posY, block_posZ), log_block, "block", log_chamber, false, block_copy, true);

            G4VisAttributes* VisBlock = new G4VisAttributes(G4Colour::Black());
            log_block->SetVisAttributes(VisBlock);

            if (block_sens == 1)
                gm.AddXYZDetector(log_block);

            // Keyword for sources with casing
            G4int is_casing = 0;
            gm.config->readInto(is_casing, "GEOM.COSI.IS.CASING");
            G4cout << "GEOM.COSI.IS.CASING: " << is_casing << G4endl;
            if (is_casing == 1) {
                /* Plastic casing aroung source */
                G4int casing_copy = 45;

                G4double casingX = 27.4;
                G4double casingY = 27.4;
                G4double casingZ = 17.875;
                G4double casingThick = 1.;

                G4double casing_posX = foam_posX;
                G4double casing_posY = foam_posY;
                G4double casing_posZ = foam_posZ + foamZ/2. + casingZ/2.;

                G4Box* solid_casing_notsub = new G4Box("solid_casing_notsub", casingX/2., casingY/2., casingZ/2.);
                G4Box* casing_cavity = new G4Box("casing_cavity", casingX/2.-casingThick, casingY/2.-casingThick, casingZ/2.-casingThick);
                G4SubtractionSolid* solid_casing = new G4SubtractionSolid("solid_casing", solid_casing_notsub, casing_cavity, 0, G4ThreeVector(0., 0., 0.));
                
                G4LogicalVolume* log_casing = new G4LogicalVolume(solid_casing, plastic_mat, "log_casing");

                G4VPhysicalVolume* phys_casing = new G4PVPlacement(0, G4ThreeVector(casing_posX, casing_posY, casing_posZ), log_casing, "casing", log_chamber, false, casing_copy, true);
                
                G4VisAttributes* VisCasing = new G4VisAttributes(G4Colour::White());
                log_casing->SetVisAttributes(VisCasing);

                if (casing_sens == 1)
                    gm.AddXYZDetector(log_casing);

                /* Foam aroung source */
                G4int foam_source_copy = 46;

                G4double foam_sourceX = casingX-casingThick*2.;
                G4double foam_sourceY = casingY-casingThick*2.;
                G4double foam_sourceZ = 9.525;

                G4double foam_source_posX = casing_posX;
                G4double foam_source_posY = casing_posY;
                G4double foam_source_posZ = casing_posZ - (casingZ-casingThick*2.)/2. + foam_sourceZ/2.;

                G4Box* solid_foam_source = new G4Box("solid_foam_source", foam_sourceX/2., foam_sourceY/2., foam_sourceZ/2.);
                G4LogicalVolume* log_foam_source = new G4LogicalVolume(solid_foam_source, foam_mat, "log_foam_source");
                G4VPhysicalVolume* phys_foam_source = new G4PVPlacement(0, G4ThreeVector(foam_source_posX, foam_source_posY, foam_source_posZ), log_foam_source, "foam_source", log_chamber, false, foam_source_copy, true);
                
                G4VisAttributes* VisFoam_source = new G4VisAttributes(G4Colour::Black());
                log_foam_source->SetVisAttributes(VisFoam_source);

                if (foam_source_sens == 1)
                    gm.AddXYZDetector(log_foam_source);   

                // Source case
                G4int sourceCase_copy = 47;

                G4double sourceCaseR = 12.7;
                G4double sourceCaseH = 6.35;
                G4double sourceR = 2.5;
                G4double sourceH = 3.18;

                G4double sourceCase_posX = -47.6;
                G4double sourceCase_posY = 0.;
                G4double sourceCase_posZ = casing_posZ - casingZ/2. + casingThick + foam_sourceZ + sourceCaseH/2.; // 617.75 mm

                G4Tubs* solid_source_notsub = new G4Tubs("solid_source_notsub", 0., sourceCaseR, sourceCaseH/2., 0., 360.);
                G4Tubs* source_cavity = new G4Tubs("source_cavity", 0., sourceR, sourceH/2. ,0., 360.);
                G4SubtractionSolid* solid_sourceCase = new G4SubtractionSolid("solid_sourceCase", solid_source_notsub, source_cavity, 0, G4ThreeVector(0., 0., 0.));
                G4LogicalVolume* log_sourceCase = new G4LogicalVolume(solid_sourceCase, peek_mat, "log_sourceCase");
                G4VPhysicalVolume* phys_sourceCase = new G4PVPlacement(0, G4ThreeVector(sourceCase_posX, sourceCase_posY, sourceCase_posZ), log_sourceCase, "source_case", log_chamber, false, sourceCase_copy, true);
                
                G4VisAttributes* VisSource = new G4VisAttributes(G4Colour::Red());
                log_sourceCase->SetVisAttributes(VisSource);

                if (sourceCase_sens == 1)
                    gm.AddXYZDetector(log_sourceCase);

            }

            else {
                G4int sourceCase_copy = 47;

                G4double sourceCaseR = 12.7;
                G4double sourceCaseH = 6.35;
                G4double sourceR = 2.5;
                G4double sourceH = 3.18;

                G4double sourceCase_posX = -47.6;
                G4double sourceCase_posY = 0.;
                G4double sourceCase_posZ = foam_posZ + foamZ/2. + sourceCaseH/2.; // 607.225 mm

                G4Tubs* solid_source_notsub = new G4Tubs("solid_source_notsub", 0., sourceCaseR, sourceCaseH/2., 0., 360.);
                G4Tubs* source_cavity = new G4Tubs("source_cavity", 0., sourceR, sourceH/2. ,0., 360.);
                G4SubtractionSolid* solid_sourceCase = new G4SubtractionSolid("solid_sourceCase", solid_source_notsub, source_cavity, 0, G4ThreeVector(0., 0., 0.));
                G4LogicalVolume* log_sourceCase = new G4LogicalVolume(solid_sourceCase, peek_mat, "log_sourceCase");
                G4VPhysicalVolume* phys_sourceCase = new G4PVPlacement(0, G4ThreeVector(sourceCase_posX, sourceCase_posY, sourceCase_posZ), log_sourceCase, "source_case", log_chamber, false, sourceCase_copy, true);
                
                G4VisAttributes* VisSource = new G4VisAttributes(G4Colour::Red());
                log_sourceCase->SetVisAttributes(VisSource);

                if (sourceCase_sens == 1)
                    gm.AddXYZDetector(log_sourceCase);
            }


            /*** BGO crystals ***/
            G4double posY_BGO1 = -122.555; // from housing drawing
            G4double posY_BGO2 = 0.;
            G4double posY_BGO3 = 122.555; // from housing drawing

            /** BGO1 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO1 = 100; 

            G4Box* solid_refl3_notsub_BGO1 = new G4Box("solid_refl3_notsub_BGO1", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO1 = new G4Box("solid_sub_BGO1", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO1 = new G4SubtractionSolid("solid_refl3_BGO1", solid_refl3_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO1 = new G4LogicalVolume(solid_refl3_BGO1, tefl_mat, "log_refl3_BGO1");

            G4VPhysicalVolume* phys_refl3_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO1, 0.), log_refl3_BGO1, "refl3_BGO1", log_chamber, false, tefl_copy_BGO1, true);
            
            G4VisAttributes* Visrefl3_BGO1 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO1->SetVisAttributes(Visrefl3_BGO1);

            if (refl3_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl3_BGO1);

            /* Tetratex */
            G4int refl2_copy_BGO1 = 110; 

            G4double xlen2 = scintX/2. + refl1Thick*2 + refl2Thick;
            G4double ylen2 = scintY/2. + refl1Thick*2 + refl2Thick;
            G4double zlen2 = scintZ/2. + refl1Thick*2 + refl2Thick;

            G4Box* solid_refl2_notsub_BGO1 = new G4Box("solid_refl2_BGO1", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO1 = new G4SubtractionSolid("solid_refl2_BGO1", solid_refl2_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO1 = new G4LogicalVolume(solid_refl2_BGO1, refl2_mat, "log_refl2_BGO1");

            G4VPhysicalVolume* phys_refl2_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO1, "refl2_BGO1", log_refl3_BGO1, false, refl2_copy_BGO1, true);
            
            G4VisAttributes* Visrefl2_BGO1 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO1->SetVisAttributes(Visrefl2_BGO1);

            if (refl2_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl2_BGO1);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO1 = 120; 

            G4double xlen3 = scintX/2. + refl1Thick*2;
            G4double ylen3 = scintY/2. + refl1Thick*2;
            G4double zlen3 = scintZ/2. + refl1Thick*2;

            G4Box* solid_refl1_notsub_BGO1 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO1= new G4SubtractionSolid("solid_refl1_BGO1", solid_refl1_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO1 = new G4LogicalVolume(solid_refl1_BGO1, refl1_mat, "log_refl1_BGO1");

            G4VPhysicalVolume* phys_refl1_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO1, "refl1_BGO1", log_refl2_BGO1, false, refl1_copy_BGO1, true);
            
            G4VisAttributes* Visrefl1_BGO1 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO1->SetVisAttributes(Visrefl1_BGO1);

            if (refl1_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl1_BGO1);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO1 = 130; 

            G4double xlen4 = scintX/2. + refl1Thick;
            G4double ylen4 = scintY/2. + refl1Thick;
            G4double zlen4 = scintZ/2. + refl1Thick;

            G4Box* solid_refl11_notsub_BGO1 = new G4Box("solid_refl11_BGO1", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO1 = new G4SubtractionSolid("solid_refl11_BGO1", solid_refl11_notsub_BGO1, SiPMface_sub_BGO1, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO1 = new G4LogicalVolume(solid_refl11_BGO1, refl1_mat, "log_refl11_BGO1");

            G4VPhysicalVolume* phys_refl11_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO1, "refl11_BGO1", log_refl1_BGO1, false, refl11_copy_BGO1, true);
            
            G4VisAttributes* Visrefl11_BGO1 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO1->SetVisAttributes(Visrefl11_BGO1);

            if (refl11_sens_BGO1 == 1)
                gm.AddXYZDetector(log_refl11_BGO1);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO1 = 140; 

            G4double xlen5 = refl2Thick; // I set directly two layers as one with the reflectivity of two (Janecek)
            G4double ylen5 = scintY/2. + refl1Thick*2 + refl2Thick + refl3Thick;
            G4double zlen5 = scintZ/2. + refl1Thick*2 + refl2Thick + refl3Thick;

            G4Box* solid_SiPMface_notsub_BGO1 = new G4Box("solid_SiPMface_BGO1", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO1 = new G4Box("solid_sub_BGO1", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO1 = new G4SubtractionSolid("solid_SiPMface_BGO1", solid_SiPMface_notsub_BGO1, SiPM_sub_BGO1, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO1 = new G4LogicalVolume(solid_SiPMface_BGO1, refl2_mat, "log_SiPMface_BGO1");

            G4VPhysicalVolume* phys_SiPMface_BGO1 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO1, 0.), log_SiPMface_BGO1, "SiPMface_BGO1", log_chamber, false, SiPMface_copy_BGO1, true);
            
            G4VisAttributes* VisSiPMface_BGO1 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO1->SetVisAttributes(VisSiPMface_BGO1);

            if (refl2_sens_BGO1 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO1);

            /* scintillator */
            G4int scint_copy_BGO1 = 150;

            G4Box* solid_scint_BGO1 = new G4Box("solid_scint_BGO1", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO1 = new G4LogicalVolume(solid_scint_BGO1, bgo_mat, "log_scint_BGO1");

            G4VPhysicalVolume* phys_scint_BGO1 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO1, "scint_BGO1", log_refl11_BGO1, false, scint_copy_BGO1, true);

            G4VisAttributes* VisScint_BGO1 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO1->SetVisAttributes(VisScint_BGO1);

            if (scint_sens_BGO1 == 1)
                gm.AddXYZDetector(log_scint_BGO1);

            /* Optical coupler */
            
            G4int sy184_copy_BGO1 = 160;

            auto rotation_BGO1 = new G4RotationMatrix();
            rotation_BGO1->rotateY(90*deg);

            G4Box* solid_sy184_BGO1 = new G4Box("solid_sy184_BGO1", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO1 = new G4LogicalVolume(solid_sy184_BGO1, sy184_mat, "log_sy184_BGO1");

            G4VPhysicalVolume* phys_sy184_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ/2., posY_BGO1, 0.), log_sy184_BGO1, "Sylgard184_BGO1", log_chamber, false, sy184_copy_BGO1, true);

            G4VisAttributes* VisSy184_BGO1 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO1->SetVisAttributes(VisSy184_BGO1);

            if (ej560_sens_BGO1 == 1)
                gm.AddXYZDetector(log_sy184_BGO1);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO1 = 210;
            G4int SiPM12_copy_BGO1 = 220;
            G4int SiPM13_copy_BGO1 = 230;
            G4int SiPM21_copy_BGO1 = 240;
            G4int SiPM22_copy_BGO1 = 250;
            G4int SiPM23_copy_BGO1 = 260;
            G4int SiPM31_copy_BGO1 = 270;
            G4int SiPM32_copy_BGO1 = 280;
            G4int SiPM33_copy_BGO1 = 290;

            G4double SiPMX = 6.;
            G4double SiPMY = 6.;
            G4double SiPMZ = 0.1;

            G4Box* solid_SiPM_BGO1 = new G4Box("solid_SiPM_BGO1", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO1 = new G4LogicalVolume(solid_SiPM_BGO1, SiPM_mat, "log_SiPM_BGO1");

            G4VPhysicalVolume* phys_SiPM11_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM11_BGO1", log_chamber, false, SiPM11_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM12_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM12_BGO1", log_chamber, false, SiPM12_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM13_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, SiPMX), log_SiPM_BGO1, "SiPM13_BGO1", log_chamber, false, SiPM13_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM21_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM21_BGO1", log_chamber, false, SiPM21_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM22_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM22_BGO1", log_chamber, false, SiPM22_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM23_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, 0.), log_SiPM_BGO1, "SiPM23_BGO1", log_chamber, false, SiPM23_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM31_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM31_BGO1", log_chamber, false, SiPM31_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM32_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM32_BGO1", log_chamber, false, SiPM32_copy_BGO1, true);
            G4VPhysicalVolume* phys_SiPM33_BGO1 = new G4PVPlacement(rotation_BGO1, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO1, -SiPMX), log_SiPM_BGO1, "SiPM33_BGO1", log_chamber, false, SiPM33_copy_BGO1, true);

            G4VisAttributes* VisSiPM_BGO1 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO1->SetVisAttributes(VisSiPM_BGO1);

            if (SiPM_sens_BGO1 == 1)
                gm.AddXYZDetector(log_SiPM_BGO1);

            /** BGO2 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO2 = 10; 

            G4Box* solid_refl3_notsub_BGO2 = new G4Box("solid_refl3_notsub_BGO2", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO2 = new G4Box("solid_sub_BGO2", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO2 = new G4SubtractionSolid("solid_refl3_BGO2", solid_refl3_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO2 = new G4LogicalVolume(solid_refl3_BGO2, tefl_mat, "log_refl3_BGO2");

            G4VPhysicalVolume* phys_refl3_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO2, 0.), log_refl3_BGO2, "refl3_BGO2", log_chamber, false, tefl_copy_BGO2, true);
            
            G4VisAttributes* Visrefl3_BGO2 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO2->SetVisAttributes(Visrefl3_BGO2);

            if (refl3_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl3_BGO2);

            /* Tetratex */
            G4int refl2_copy_BGO2 = 11; 

            G4Box* solid_refl2_notsub_BGO2 = new G4Box("solid_refl2_BGO2", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO2 = new G4SubtractionSolid("solid_refl2_BGO2", solid_refl2_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO2 = new G4LogicalVolume(solid_refl2_BGO2, refl2_mat, "log_refl2_BGO2");

            G4VPhysicalVolume* phys_refl2_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO2, "refl2_BGO2", log_refl3_BGO2, false, refl2_copy_BGO2, true);
            
            G4VisAttributes* Visrefl2_BGO2 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO2->SetVisAttributes(Visrefl2_BGO2);

            if (refl2_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl2_BGO2);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO2 = 12; 

            G4Box* solid_refl1_notsub_BGO2 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO2= new G4SubtractionSolid("solid_refl1_BGO2", solid_refl1_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO2 = new G4LogicalVolume(solid_refl1_BGO2, refl1_mat, "log_refl1_BGO2");

            G4VPhysicalVolume* phys_refl1_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO2, "refl1_BGO2", log_refl2_BGO2, false, refl1_copy_BGO2, true);
            
            G4VisAttributes* Visrefl1_BGO2 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO2->SetVisAttributes(Visrefl1_BGO2);

            if (refl1_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl1_BGO2);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO2 = 13; 

            G4Box* solid_refl11_notsub_BGO2 = new G4Box("solid_refl11_BGO2", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO2 = new G4SubtractionSolid("solid_refl11_BGO2", solid_refl11_notsub_BGO2, SiPMface_sub_BGO2, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO2 = new G4LogicalVolume(solid_refl11_BGO2, refl1_mat, "log_refl11_BGO2");

            G4VPhysicalVolume* phys_refl11_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO2, "refl11_BGO2", log_refl1_BGO2, false, refl11_copy_BGO2, true);
            
            G4VisAttributes* Visrefl11_BGO2 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO2->SetVisAttributes(Visrefl11_BGO2);

            if (refl11_sens_BGO2 == 1)
                gm.AddXYZDetector(log_refl11_BGO2);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO2 = 14; 

            G4Box* solid_SiPMface_notsub_BGO2 = new G4Box("solid_SiPMface_BGO2", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO2 = new G4Box("solid_sub_BGO2", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO2 = new G4SubtractionSolid("solid_SiPMface_BGO2", solid_SiPMface_notsub_BGO2, SiPM_sub_BGO2, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO2 = new G4LogicalVolume(solid_SiPMface_BGO2, refl2_mat, "log_SiPMface_BGO2");

            G4VPhysicalVolume* phys_SiPMface_BGO2 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO2, 0.), log_SiPMface_BGO2, "SiPMface_BGO2", log_chamber, false, SiPMface_copy_BGO2, true);
            
            G4VisAttributes* VisSiPMface_BGO2 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO2->SetVisAttributes(VisSiPMface_BGO2);

            if (refl2_sens_BGO2 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO2);

            /* scintillator */
            G4int scint_copy_BGO2 = 15;

            G4Box* solid_scint_BGO2 = new G4Box("solid_scint_BGO2", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO2 = new G4LogicalVolume(solid_scint_BGO2, bgo_mat, "log_scint_BGO2");

            G4VPhysicalVolume* phys_scint_BGO2 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO2, "scint_BGO2", log_refl11_BGO2, false, scint_copy_BGO2, true);

            G4VisAttributes* VisScint_BGO2 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO2->SetVisAttributes(VisScint_BGO2);

            if (scint_sens_BGO2 == 1)
                gm.AddXYZDetector(log_scint_BGO2);

            /* Optical coupler */
            
            G4int sy184_copy_BGO2 = 16;

            auto rotation_BGO2 = new G4RotationMatrix();
            rotation_BGO2->rotateY(90*deg);

            G4Box* solid_sy184_BGO2 = new G4Box("solid_sy184_BGO2", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO2 = new G4LogicalVolume(solid_sy184_BGO2, sy184_mat, "log_sy184_BGO2");

            G4VPhysicalVolume* phys_sy184_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ/2., posY_BGO2, 0.), log_sy184_BGO2, "Sylgard184_BGO2", log_chamber, false, sy184_copy_BGO2, true);

            G4VisAttributes* VisSy184_BGO2 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO2->SetVisAttributes(VisSy184_BGO2);

            if (ej560_sens_BGO2 == 1)
                gm.AddXYZDetector(log_sy184_BGO2);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO2 = 21;
            G4int SiPM12_copy_BGO2 = 22;
            G4int SiPM13_copy_BGO2 = 23;
            G4int SiPM21_copy_BGO2 = 24;
            G4int SiPM22_copy_BGO2 = 25;
            G4int SiPM23_copy_BGO2 = 26;
            G4int SiPM31_copy_BGO2 = 27;
            G4int SiPM32_copy_BGO2 = 28;
            G4int SiPM33_copy_BGO2 = 29;

            G4Box* solid_SiPM_BGO2 = new G4Box("solid_SiPM_BGO2", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO2 = new G4LogicalVolume(solid_SiPM_BGO2, SiPM_mat, "log_SiPM_BGO2");

            G4VPhysicalVolume* phys_SiPM11_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM11_BGO2", log_chamber, false, SiPM11_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM12_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM12_BGO2", log_chamber, false, SiPM12_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM13_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, SiPMX), log_SiPM_BGO2, "SiPM13_BGO2", log_chamber, false, SiPM13_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM21_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM21_BGO2", log_chamber, false, SiPM21_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM22_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM22_BGO2", log_chamber, false, SiPM22_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM23_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, 0.), log_SiPM_BGO2, "SiPM23_BGO2", log_chamber, false, SiPM23_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM31_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM31_BGO2", log_chamber, false, SiPM31_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM32_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM32_BGO2", log_chamber, false, SiPM32_copy_BGO2, true);
            G4VPhysicalVolume* phys_SiPM33_BGO2 = new G4PVPlacement(rotation_BGO2, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO2, -SiPMX), log_SiPM_BGO2, "SiPM33_BGO2", log_chamber, false, SiPM33_copy_BGO2, true);

            G4VisAttributes* VisSiPM_BGO2 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO2->SetVisAttributes(VisSiPM_BGO2);

            if (SiPM_sens_BGO2 == 1)
                gm.AddXYZDetector(log_SiPM_BGO2);

            /** BGO3 **/

            /* Teflon PTFE */
            G4int tefl_copy_BGO3 = 1000; 

            G4Box* solid_refl3_notsub_BGO3 = new G4Box("solid_refl3_notsub_BGO3", xlen, ylen, zlen);
            G4Box* SiPMface_sub_BGO3 = new G4Box("solid_sub_BGO3", refl1Thick*2+refl2Thick+refl3Thick, ylen, zlen);
            G4SubtractionSolid* solid_refl3_BGO3 = new G4SubtractionSolid("solid_refl3_BGO3", solid_refl3_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl3_BGO3 = new G4LogicalVolume(solid_refl3_BGO3, tefl_mat, "log_refl3_BGO3");

            G4VPhysicalVolume* phys_refl3_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., posY_BGO3, 0.), log_refl3_BGO3, "refl3_BGO3", log_chamber, false, tefl_copy_BGO3, true);
            
            G4VisAttributes* Visrefl3_BGO3 = new G4VisAttributes(G4Colour::Black());
            log_refl3_BGO3->SetVisAttributes(Visrefl3_BGO3);

            if (refl3_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl3_BGO3);

            /* Tetratex */
            G4int refl2_copy_BGO3 = 1100; 

            G4Box* solid_refl2_notsub_BGO3 = new G4Box("solid_refl2_BGO3", xlen2, ylen2, zlen2);
            G4SubtractionSolid* solid_refl2_BGO3 = new G4SubtractionSolid("solid_refl2_BGO3", solid_refl2_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl2_BGO3 = new G4LogicalVolume(solid_refl2_BGO3, refl2_mat, "log_refl2_BGO3");

            G4VPhysicalVolume* phys_refl2_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl2_BGO3, "refl2_BGO3", log_refl3_BGO3, false, refl2_copy_BGO3, true);
            
            G4VisAttributes* Visrefl2_BGO3 = new G4VisAttributes(G4Colour::Grey());
            log_refl2_BGO3->SetVisAttributes(Visrefl2_BGO3);

            if (refl2_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl2_BGO3);

            /* refl1 (external layer) */
            G4int refl1_copy_BGO3 = 1200; 

            G4Box* solid_refl1_notsub_BGO3 = new G4Box("solid_refl1", xlen3, ylen3, zlen3);
            G4SubtractionSolid* solid_refl1_BGO3= new G4SubtractionSolid("solid_refl1_BGO3", solid_refl1_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl1_BGO3 = new G4LogicalVolume(solid_refl1_BGO3, refl1_mat, "log_refl1_BGO3");

            G4VPhysicalVolume* phys_refl1_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl1_BGO3, "refl1_BGO3", log_refl2_BGO3, false, refl1_copy_BGO3, true);
            
            G4VisAttributes* Visrefl1_BGO3 = new G4VisAttributes(G4Colour::White());
            log_refl1_BGO3->SetVisAttributes(Visrefl1_BGO3);

            if (refl1_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl1_BGO3);

            /* refl11 (internal layer) */
            G4int refl11_copy_BGO3 = 1300; 

            G4Box* solid_refl11_notsub_BGO3 = new G4Box("solid_refl11_BGO3", xlen4, ylen4, zlen4);
            G4SubtractionSolid* solid_refl11_BGO3 = new G4SubtractionSolid("solid_refl11_BGO3", solid_refl11_notsub_BGO3, SiPMface_sub_BGO3, 0, G4ThreeVector(pos_sub, 0., 0.));

            G4LogicalVolume* log_refl11_BGO3 = new G4LogicalVolume(solid_refl11_BGO3, refl1_mat, "log_refl11_BGO3");

            G4VPhysicalVolume* phys_refl11_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_refl11_BGO3, "refl11_BGO3", log_refl1_BGO3, false, refl11_copy_BGO3, true);
            
            G4VisAttributes* Visrefl11_BGO3 = new G4VisAttributes(G4Colour::White());
            log_refl11_BGO3->SetVisAttributes(Visrefl11_BGO3);

            if (refl11_sens_BGO3 == 1)
                gm.AddXYZDetector(log_refl11_BGO3);

            /* Tetratex (SiPM face) */
            G4int SiPMface_copy_BGO3 = 1400; 

            G4Box* solid_SiPMface_notsub_BGO3 = new G4Box("solid_SiPMface_BGO3", xlen5, ylen5, zlen5);
            G4Box* SiPM_sub_BGO3 = new G4Box("solid_sub_BGO3", refl1Thick*2+refl2Thick+refl3Thick, optX/2., optY/2.);
            G4SubtractionSolid* solid_SiPMface_BGO3 = new G4SubtractionSolid("solid_SiPMface_BGO3", solid_SiPMface_notsub_BGO3, SiPM_sub_BGO3, 0, G4ThreeVector(0., 0., 0.));

            G4LogicalVolume* log_SiPMface_BGO3 = new G4LogicalVolume(solid_SiPMface_BGO3, refl2_mat, "log_SiPMface_BGO3");

            G4VPhysicalVolume* phys_SiPMface_BGO3 = new G4PVPlacement(0, G4ThreeVector(scintX/2. + refl2Thick, posY_BGO3, 0.), log_SiPMface_BGO3, "SiPMface_BGO3", log_chamber, false, SiPMface_copy_BGO3, true);
            
            G4VisAttributes* VisSiPMface_BGO3 = new G4VisAttributes(G4Colour::Grey());
            log_SiPMface_BGO3->SetVisAttributes(VisSiPMface_BGO3);

            if (refl2_sens_BGO3 == 1)
                gm.AddXYZDetector(log_SiPMface_BGO3);

            /* scintillator */
            G4int scint_copy_BGO3 = 1500;

            G4Box* solid_scint_BGO3 = new G4Box("solid_scint_BGO3", scintX/2., scintY/2., scintZ/2.);

            G4LogicalVolume* log_scint_BGO3 = new G4LogicalVolume(solid_scint_BGO3, bgo_mat, "log_scint_BGO3");

            G4VPhysicalVolume* phys_scint_BGO3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), log_scint_BGO3, "scint_BGO3", log_refl11_BGO3, false, scint_copy_BGO3, true);

            G4VisAttributes* VisScint_BGO3 = new G4VisAttributes(G4Colour::Yellow());
            log_scint_BGO3->SetVisAttributes(VisScint_BGO3);

            if (scint_sens_BGO3 == 1)
                gm.AddXYZDetector(log_scint_BGO3);

            /* Optical coupler */
            
            G4int sy184_copy_BGO3 = 1600;

            auto rotation_BGO3 = new G4RotationMatrix();
            rotation_BGO3->rotateY(90*deg);

            G4Box* solid_sy184_BGO3 = new G4Box("solid_sy184_BGO3", optX/2., optY/2., optZ/2.);

            G4LogicalVolume* log_sy184_BGO3 = new G4LogicalVolume(solid_sy184_BGO3, sy184_mat, "log_sy184_BGO3");

            G4VPhysicalVolume* phys_sy184_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ/2., posY_BGO3, 0.), log_sy184_BGO3, "Sylgard184_BGO3", log_chamber, false, sy184_copy_BGO3, true);

            G4VisAttributes* VisSy184_BGO3 = new G4VisAttributes(G4Colour::Cyan());
            log_sy184_BGO3->SetVisAttributes(VisSy184_BGO3);

            if (ej560_sens_BGO3 == 1)
                gm.AddXYZDetector(log_sy184_BGO3);
            
            /* ----- SiPMs ----- */

            /* PMT housing */
            G4int SiPM11_copy_BGO3 = 2100;
            G4int SiPM12_copy_BGO3 = 2200;
            G4int SiPM13_copy_BGO3 = 2300;
            G4int SiPM21_copy_BGO3 = 2400;
            G4int SiPM22_copy_BGO3 = 2500;
            G4int SiPM23_copy_BGO3 = 2600;
            G4int SiPM31_copy_BGO3 = 2700;
            G4int SiPM32_copy_BGO3 = 2800;
            G4int SiPM33_copy_BGO3 = 2900;

            G4Box* solid_SiPM_BGO3 = new G4Box("solid_SiPM_BGO3", SiPMX/2., SiPMY/2., SiPMZ/2.);

            G4LogicalVolume* log_SiPM_BGO3 = new G4LogicalVolume(solid_SiPM_BGO3, SiPM_mat, "log_SiPM_BGO3");

            G4VPhysicalVolume* phys_SiPM11_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM11_BGO3", log_chamber, false, SiPM11_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM12_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM12_BGO3", log_chamber, false, SiPM12_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM13_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, SiPMX), log_SiPM_BGO3, "SiPM13_BGO3", log_chamber, false, SiPM13_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM21_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM21_BGO3", log_chamber, false, SiPM21_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM22_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM22_BGO3", log_chamber, false, SiPM22_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM23_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, 0.), log_SiPM_BGO3, "SiPM23_BGO3", log_chamber, false, SiPM23_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM31_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., -SiPMX+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM31_BGO3", log_chamber, false, SiPM31_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM32_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., 0.+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM32_BGO3", log_chamber, false, SiPM32_copy_BGO3, true);
            G4VPhysicalVolume* phys_SiPM33_BGO3 = new G4PVPlacement(rotation_BGO3, G4ThreeVector(scintX/2.+optZ+SiPMZ/2., SiPMX+posY_BGO3, -SiPMX), log_SiPM_BGO3, "SiPM33_BGO3", log_chamber, false, SiPM33_copy_BGO3, true);

            G4VisAttributes* VisSiPM_BGO3 = new G4VisAttributes(G4Colour::Black());
            log_SiPM_BGO3->SetVisAttributes(VisSiPM_BGO3);

            if (SiPM_sens_BGO3 == 1)
                gm.AddXYZDetector(log_SiPM_BGO3);
/*
            G4int housing_copy = 50;

            G4double housingThick = 2.54;
            G4double housingGap = 1.905;
            G4double housingX = scintX + housingThick*2 + optZ + SiPMZ;
            G4double housingY = ylen*2*3 + housingGap*2 + housingThick*2;
            G4double housingZ = zlen*2 + housingThick*2;

            G4Box* solid_housing_notsub = new G4Box("solid_housing_notsub", housingX/2., housingY/2., housingZ/2.);
            G4Box* BGO_cavity = new G4Box("solid_BGO_cavity", (scintX + (xlen*2-scintX)/2. + optZ + SiPMZ)/2., ylen, zlen);
            G4SubtractionSolid* solid_housing1 = new G4SubtractionSolid("solid_housing1", solid_housing_notsub, BGO_cavity, 0, G4ThreeVector(0., 0., 0.));
            G4SubtractionSolid* solid_housing2 = new G4SubtractionSolid("solid_housing2", solid_housing1, BGO_cavity, 0, G4ThreeVector(0., ylen*2+housingGap, 0.));
            G4SubtractionSolid* solid_housing = new G4SubtractionSolid("solid_housing", solid_housing2, BGO_cavity, 0, G4ThreeVector(0., -ylen*2-housingGap, 0.));

            G4LogicalVolume* log_housing = new G4LogicalVolume(solid_housing, Al_mat, "log_housing");
            G4VPhysicalVolume* phys_housing = new G4PVPlacement(0, G4ThreeVector(scintX/4.-xlen/2.+optZ/2.+SiPMZ/2., 0., 0.), log_housing, "housing", log_chamber, false, housing_copy, true);

            G4VisAttributes* VisHousing = new G4VisAttributes(G4Colour::Grey());
            log_housing->SetVisAttributes(VisHousing);

            if (housing_sens == 1)
                gm.AddXYZDetector(log_housing);
*/  

            /* --------------------- OPTICAL SURFACES ----------------------- */

            /* We use the Unified Model.*/
            /* For each BGO, there are two optical surfaces between the BGO and the reflective layers:*/
            /* 1. BGO -> VM2000 (non-SiPM face) */
            /*    Finish: Polishedbackpainted, sigma = 1.3 deg (Janecek 2009), SS reflection and reflectivity to ~99% (Janecek 2012) (type1) */
            /* 2. BGO -> Tetratex (SiPM face)   */
            /*    Finish: Groundbackpainted, sigma = 1.3 deg (Janecek 2009), L reflection and reflectivity to ~96% (Janecek 2012) (type4) */

            /** BGO1 **/

            /* BGO1 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO1 = new G4OpticalSurface("panel_OPTSURFACE_BGO1");
            G4LogicalBorderSurface* panel_BORDER1_BGO1 = new G4LogicalBorderSurface("panel_BORDER1_BGO1", phys_scint_BGO1, phys_refl11_BGO1, panel_OPTSURFACE_BGO1);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO1->SetModel(unified);
                panel_OPTSURFACE_BGO1->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO1->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO1->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO1 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO1 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO1 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO1 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO1, OPTSURFACE_refl_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_SPECULARLOBECONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_SPECULARSPIKECONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO1, OPTSURFACE_BACKSCATTERCONSTANT_BGO1);
                panel_OPTSURFACE_MPT_BGO1->AddProperty("RINDEX", OPTSURFACE_Energy_BGO1, OPTSURFACE_RIND_BGO1);
                panel_OPTSURFACE_MPT_BGO1->DumpTable();
            }
            panel_OPTSURFACE_BGO1->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO1);

            /* BGO1 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO1 = new G4OpticalSurface("panel_OPTSURFACE2_BGO1");
            G4LogicalBorderSurface* panel_BORDER2_BGO1 = new G4LogicalBorderSurface("panel_BORDER2_BGO1", phys_scint_BGO1, phys_SiPMface_BGO1, panel_OPTSURFACE2_BGO1);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO1 = new G4MaterialPropertiesTable();
            
            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO1->SetModel(unified);
                panel_OPTSURFACE2_BGO1->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO1->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO1->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO1 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO1 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO1 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO1->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_SPECULARLOBECONSTANT_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_BACKSCATTERCONSTANT_BGO1);
                std::vector<G4double> OPTSURFACE2_RIND_BGO1 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_refl_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO1, OPTSURFACE2_RIND_BGO1);
                panel_OPTSURFACE2_MPT_BGO1->DumpTable();
            }
            panel_OPTSURFACE2_BGO1->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO1);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO1 = new G4OpticalSurface("external_OPTSURFACE_BGO1");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO1 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO1", phys_sy184_BGO1, phys_SiPMface_BGO1, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO1", phys_SiPM11_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO1", phys_SiPM12_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO1", phys_SiPM13_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO1", phys_SiPM21_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO1", phys_SiPM22_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO1", phys_SiPM23_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO1", phys_SiPM31_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO1", phys_SiPM32_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO1 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO1", phys_SiPM33_BGO1, phys_chamber, external_OPTSURFACE_BGO1);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO1->SetModel(unified);
            external_OPTSURFACE_BGO1->SetType(dielectric_metal);
            external_OPTSURFACE_BGO1->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO1  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO1  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO1 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO1, OPTSURFACE_ext_refl_BGO1);
            external_OPTSURFACE_MPT_BGO1->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO1, OPTSURFACE_ext_eff_BGO1);

            external_OPTSURFACE_MPT_BGO1->DumpTable();

            external_OPTSURFACE_BGO1->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO1);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO1 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO1");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM11_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM12_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM13_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM21_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM22_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM23_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM31_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM32_BGO1, OptSiPM_OPTSURFACE_BGO1);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO1 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO1", phys_sy184_BGO1, phys_SiPM33_BGO1, OptSiPM_OPTSURFACE_BGO1);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO1 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO1->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO1->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO1->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO1  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO1  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO1 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO1->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO1, OPTSURFACE_OptSiPM_refl_BGO1);
            OptSiPM_OPTSURFACE_MPT_BGO1->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO1, OPTSURFACE_OptSiPM_eff_BGO1);

            OptSiPM_OPTSURFACE_MPT_BGO1->DumpTable();

            OptSiPM_OPTSURFACE_BGO1->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO1);

            /** BGO2 **/

            /* BGO2 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO2 = new G4OpticalSurface("panel_OPTSURFACE_BGO2");
            G4LogicalBorderSurface* panel_BORDER1_BGO2 = new G4LogicalBorderSurface("panel_BORDER1_BGO2", phys_scint_BGO2, phys_refl11_BGO2, panel_OPTSURFACE_BGO2);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO2->SetModel(unified);
                panel_OPTSURFACE_BGO2->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO2->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO2->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO2 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO2 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO2 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO2, OPTSURFACE_refl_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_SPECULARLOBECONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_SPECULARSPIKECONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO2, OPTSURFACE_BACKSCATTERCONSTANT_BGO2);
                panel_OPTSURFACE_MPT_BGO2->AddProperty("RINDEX", OPTSURFACE_Energy_BGO2, OPTSURFACE_RIND_BGO2);
                panel_OPTSURFACE_MPT_BGO2->DumpTable();
            }
            panel_OPTSURFACE_BGO2->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO2);

            /* BGO2 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO2 = new G4OpticalSurface("panel_OPTSURFACE2_BGO2");
            G4LogicalBorderSurface* panel_BORDER2_BGO2 = new G4LogicalBorderSurface("panel_BORDER2_BGO2", phys_scint_BGO2, phys_SiPMface_BGO2, panel_OPTSURFACE2_BGO2);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO2 = new G4MaterialPropertiesTable();
            
            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO2->SetModel(unified);
                panel_OPTSURFACE2_BGO2->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO2->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO2->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO2 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO2 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO2->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_SPECULARLOBECONSTANT_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_BACKSCATTERCONSTANT_BGO2);
                std::vector<G4double> OPTSURFACE2_RIND_BGO2 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_refl_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO2, OPTSURFACE2_RIND_BGO2);
                panel_OPTSURFACE2_MPT_BGO2->DumpTable();
            }
            panel_OPTSURFACE2_BGO2->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO2);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO2 = new G4OpticalSurface("external_OPTSURFACE_BGO2");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO2 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO2", phys_sy184_BGO2, phys_SiPMface_BGO2, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO2", phys_SiPM11_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO2", phys_SiPM12_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO2", phys_SiPM13_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO2", phys_SiPM21_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO2", phys_SiPM22_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO2", phys_SiPM23_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO2", phys_SiPM31_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO2", phys_SiPM32_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO2 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO2", phys_SiPM33_BGO2, phys_chamber, external_OPTSURFACE_BGO2);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO2->SetModel(unified);
            external_OPTSURFACE_BGO2->SetType(dielectric_metal);
            external_OPTSURFACE_BGO2->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO2  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO2  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO2 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO2, OPTSURFACE_ext_refl_BGO2);
            external_OPTSURFACE_MPT_BGO2->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO2, OPTSURFACE_ext_eff_BGO2);

            external_OPTSURFACE_MPT_BGO2->DumpTable();

            external_OPTSURFACE_BGO2->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO2);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO2 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO2");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM11_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM12_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM13_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM21_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM22_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM23_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM31_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM32_BGO2, OptSiPM_OPTSURFACE_BGO2);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO2 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO2", phys_sy184_BGO2, phys_SiPM33_BGO2, OptSiPM_OPTSURFACE_BGO2);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO2 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO2->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO2->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO2->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO2  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO2  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO2 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO2->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO2, OPTSURFACE_OptSiPM_refl_BGO2);
            OptSiPM_OPTSURFACE_MPT_BGO2->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO2, OPTSURFACE_OptSiPM_eff_BGO2);

            OptSiPM_OPTSURFACE_MPT_BGO2->DumpTable();

            OptSiPM_OPTSURFACE_BGO2->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO2);

            /** BGO3 **/

            /* BGO3 -> VM2000 */

            G4OpticalSurface* panel_OPTSURFACE_BGO3 = new G4OpticalSurface("panel_OPTSURFACE_BGO3");
            G4LogicalBorderSurface* panel_BORDER1_BGO3 = new G4LogicalBorderSurface("panel_BORDER1_BGO3", phys_scint_BGO3, phys_refl11_BGO3, panel_OPTSURFACE_BGO3);

            G4MaterialPropertiesTable* panel_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();
            
            if (setReflSurfaceType == 1) {
                panel_OPTSURFACE_BGO3->SetModel(unified);
                panel_OPTSURFACE_BGO3->SetType(dielectric_dielectric);

                panel_OPTSURFACE_BGO3->SetFinish(polishedbackpainted);
                panel_OPTSURFACE_BGO3->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE_Energy_BGO3 = {1.547868 *eV, 1.569420 *eV, 1.593627 *eV, 1.618593 *eV, 1.631371 *eV, 1.657543 *eV, 1.696090 *eV, 1.748719 *eV, 1.789094 *eV, 1.850510 *eV, 1.919260 *eV, 1.990116 *eV, 2.066403 *eV, 2.137659 *eV, 2.206125 *eV, 2.270773 *eV, 2.352641 *eV, 2.455133 *eV, 2.561657 *eV, 2.637962 *eV, 2.736958 *eV, 2.843674 *eV, 3.002039 *eV, 3.107373 *eV, 3.162862 *eV, 3.195469 *eV, 3.212026 *eV, 3.220369 *eV, 3.245660 *eV, 3.254178 *eV, 3.280005 *eV, 3.315086 *eV, 3.415543 *eV, 3.492513 *eV, 3.573032 *eV, 3.657351 *eV, 3.745746 *eV, 3.948541 *eV, 4.146629 *eV, 4.381067 *eV, 4.558243 *eV, 4.714228 *eV, 4.959368 *eV};
                std::vector<G4double> OPTSURFACE_refl_BGO3 = {0.999, 0.999, 0.996, 0.994, 0.985, 0.988, 0.990, 0.985, 0.987, 0.978, 0.970, 0.973, 0.975, 0.976, 0.982, 0.987, 0.990, 0.993, 0.997, 0.996, 0.984, 0.990, 0.985, 0.953, 0.870, 0.762, 0.657, 0.533, 0.392, 0.274, 0.157, 0.124, 0.124, 0.102, 0.0991, 0.0902, 0.0873, 0.0710, 0.0385, 0.0592, 0.0843, 0.123, 0.149}; // Janecek 2012
                std::vector<G4double> OPTSURFACE_SPECULARLOBECONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_SPECULARSPIKECONSTANT_BGO3 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE_BACKSCATTERCONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE_RIND_BGO3 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_Energy_BGO3, OPTSURFACE_refl_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_SPECULARLOBECONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_SPECULARSPIKECONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE_Energy_BGO3, OPTSURFACE_BACKSCATTERCONSTANT_BGO3);
                panel_OPTSURFACE_MPT_BGO3->AddProperty("RINDEX", OPTSURFACE_Energy_BGO3, OPTSURFACE_RIND_BGO3);
                panel_OPTSURFACE_MPT_BGO3->DumpTable();
            }
            panel_OPTSURFACE_BGO3->SetMaterialPropertiesTable(panel_OPTSURFACE_MPT_BGO3);

            /* BGO3 -> Tetratex (SiPM face) */

            G4OpticalSurface* panel_OPTSURFACE2_BGO3 = new G4OpticalSurface("panel_OPTSURFACE2_BGO3");
            G4LogicalBorderSurface* panel_BORDER2_BGO3 = new G4LogicalBorderSurface("panel_BORDER2_BGO3", phys_scint_BGO3, phys_SiPMface_BGO3, panel_OPTSURFACE2_BGO3);

            G4MaterialPropertiesTable* panel_OPTSURFACE2_MPT_BGO3 = new G4MaterialPropertiesTable();
            
            if (setReflSurface2Type == 4) {
                panel_OPTSURFACE2_BGO3->SetModel(unified);
                panel_OPTSURFACE2_BGO3->SetType(dielectric_dielectric);

                panel_OPTSURFACE2_BGO3->SetFinish(groundbackpainted);
                panel_OPTSURFACE2_BGO3->SetSigmaAlpha(1.3*degree);
                
                std::vector<G4double> OPTSURFACE2_Energy_BGO3 = {1.573404*eV, 1.637836*eV, 1.736473*eV, 1.881399*eV, 2.029201*eV, 2.270773*eV, 2.610194*eV, 2.917275*eV, 3.195469*eV, 3.350924*eV, 3.562764*eV, 3.838520*eV, 4.146629*eV, 4.320007*eV, 4.524971*eV, 4.696371*eV, 4.939609*eV};
                std::vector<G4double> OPTSURFACE2_refl_BGO3 = {0.870, 0.881, 0.891, 0.906, 0.916, 0.928, 0.937, 0.946, 0.954, 0.957, 0.958, 0.963, 0.964, 0.969, 0.969, 0.963, 0.972}; // Janecek 2012
                std::vector<G4double> OPTSURFACE2_SPECULARLOBECONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                std::vector<G4double> OPTSURFACE2_SPECULARSPIKECONSTANT_BGO3 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                std::vector<G4double> OPTSURFACE2_BACKSCATTERCONSTANT_BGO3 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

                panel_OPTSURFACE2_MPT_BGO3->AddProperty("SPECULARLOBECONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_SPECULARLOBECONSTANT_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("SPECULARSPIKECONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_SPECULARSPIKECONSTANT_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("BACKSCATTERCONSTANT", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_BACKSCATTERCONSTANT_BGO3);
                std::vector<G4double> OPTSURFACE2_RIND_BGO3 = { 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293, 1.00293};
            
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_refl_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->AddProperty("RINDEX", OPTSURFACE2_Energy_BGO3, OPTSURFACE2_RIND_BGO3);
                panel_OPTSURFACE2_MPT_BGO3->DumpTable();
            }
            panel_OPTSURFACE2_BGO3->SetMaterialPropertiesTable(panel_OPTSURFACE2_MPT_BGO3);

            /* Optical coupler -> Tetratex, chamber */
            /* SiPMs -> chamber */
            /* total absorption, to avoid strange behaviors */

            G4OpticalSurface* external_OPTSURFACE_BGO3 = new G4OpticalSurface("external_OPTSURFACE_BGO3");
            
            G4LogicalBorderSurface* extOpt_BORDER_BGO3 = new G4LogicalBorderSurface("extOpt_BORDER1_BGO3", phys_sy184_BGO3, phys_SiPMface_BGO3, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM11_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM11_BORDER_BGO3", phys_SiPM11_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM12_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM12_BORDER_BGO3", phys_SiPM12_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM13_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM13_BORDER_BGO3", phys_SiPM13_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM21_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM21_BORDER_BGO3", phys_SiPM21_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM22_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM22_BORDER_BGO3", phys_SiPM22_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM23_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM23_BORDER_BGO3", phys_SiPM23_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM31_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM31_BORDER_BGO3", phys_SiPM31_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM32_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM32_BORDER_BGO3", phys_SiPM32_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* extSiPM33_BORDER_BGO3 = new G4LogicalBorderSurface("extSiPM33_BORDER_BGO3", phys_SiPM33_BGO3, phys_chamber, external_OPTSURFACE_BGO3);
    
            G4MaterialPropertiesTable* external_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();

            external_OPTSURFACE_BGO3->SetModel(unified);
            external_OPTSURFACE_BGO3->SetType(dielectric_metal);
            external_OPTSURFACE_BGO3->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_ext_Energy_BGO3  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_ext_refl_BGO3  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_ext_eff_BGO3 = { 1, 1, 1, 1, 1 };

            external_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_ext_Energy_BGO3, OPTSURFACE_ext_refl_BGO3);
            external_OPTSURFACE_MPT_BGO3->AddProperty("EFFICIENCY", OPTSURFACE_ext_Energy_BGO3, OPTSURFACE_ext_eff_BGO3);

            external_OPTSURFACE_MPT_BGO3->DumpTable();

            external_OPTSURFACE_BGO3->SetMaterialPropertiesTable(external_OPTSURFACE_MPT_BGO3);

            /* Optical coupler -> SiPMs (total absorption, then one should apply quantum efficiency) */

            G4OpticalSurface* OptSiPM_OPTSURFACE_BGO3 = new G4OpticalSurface("extPMT_OPTSURFACE_BGO3");
            
            G4LogicalBorderSurface* OptSiPM11_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM11_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM11_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM12_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM12_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM12_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM13_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM13_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM13_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM21_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM21_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM21_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM22_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM22_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM22_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM23_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM23_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM23_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM31_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM31_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM31_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM32_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM32_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM32_BGO3, OptSiPM_OPTSURFACE_BGO3);
            G4LogicalBorderSurface* OptSiPM33_BORDER_BGO3 = new G4LogicalBorderSurface("OptSiPM33_BORDER_BGO3", phys_sy184_BGO3, phys_SiPM33_BGO3, OptSiPM_OPTSURFACE_BGO3);

            G4MaterialPropertiesTable* OptSiPM_OPTSURFACE_MPT_BGO3 = new G4MaterialPropertiesTable();

            OptSiPM_OPTSURFACE_BGO3->SetModel(unified);
            OptSiPM_OPTSURFACE_BGO3->SetType(dielectric_metal);
            OptSiPM_OPTSURFACE_BGO3->SetFinish(polished);
            
            std::vector<G4double> OPTSURFACE_OptSiPM_Energy_BGO3  = { 1.0 * eV, 3.0 * eV, 5.0 * eV, 7.0 * eV, 10 * eV };
            std::vector<G4double> OPTSURFACE_OptSiPM_refl_BGO3  = { 0, 0, 0, 0, 0 };
            std::vector<G4double> OPTSURFACE_OptSiPM_eff_BGO3 = { 1, 1, 1, 1, 1 };

            OptSiPM_OPTSURFACE_MPT_BGO3->AddProperty("REFLECTIVITY", OPTSURFACE_OptSiPM_Energy_BGO3, OPTSURFACE_OptSiPM_refl_BGO3);
            OptSiPM_OPTSURFACE_MPT_BGO3->AddProperty("EFFICIENCY", OPTSURFACE_OptSiPM_Energy_BGO3, OPTSURFACE_OptSiPM_eff_BGO3);

            OptSiPM_OPTSURFACE_MPT_BGO3->DumpTable();

            OptSiPM_OPTSURFACE_BGO3->SetMaterialPropertiesTable(OptSiPM_OPTSURFACE_MPT_BGO3);

        }

    }

    return World_phys;
}
