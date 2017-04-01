/* Copyright 2014-2017 Rsyn
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PhysicalDescriptors.h
 * Author: jucemar
 *
 * Created on 6 de Julho de 2016, 15:29
 */


#ifndef LEFDESCRIPTORS_H
#define LEFDESCRIPTORS_H

#include <map>

#include "rsyn/util/DoubleRectangle.h"
#include "rsyn/util/double2.h"

// NOTE: Guilherme Flach - 2016/11/04
// I did not change here from double to dbu because the data is load from
// lef as is and therefore it's in micron. The conversion to dbu is currently
// done in the physical layer initialization.

static const std::string INVALID_LEF_NAME = "*<INVALID_LEF_NAME>*";

// -----------------------------------------------------------------------------

//! Descriptor for LEF Polygon
class LefPolygonDscp {
public:
	std::vector<double2> clsPolygonPoints;
	LefPolygonDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Port
class LefPortDscp {
public:
	std::string clsMetalName = INVALID_LEF_NAME;
	std::vector<DoubleRectangle> clsBounds;
	std::vector<LefPolygonDscp> clsLefPolygonDscp;
	LefPortDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Pin
class LefPinDscp {
public:
	bool clsHasPort = false;
	std::string clsPinName = INVALID_LEF_NAME;
	std::string clsPinDirection = INVALID_LEF_NAME;
	DoubleRectangle clsBounds;
	std::vector<LefPortDscp> clsPorts;
	LefPinDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Obstruction
class LefObsDscp {
public:
	std::string clsMetalLayer = INVALID_LEF_NAME;
	std::vector<DoubleRectangle> clsBounds;
	LefObsDscp()=default;
};

// -----------------------------------------------------------------------------

//! Descriptor for LEF Macro
class LefMacroDscp {
public:
	std::string clsMacroName = INVALID_LEF_NAME;
	std::string clsMacroClass = INVALID_LEF_NAME;
	std::string clsSite = INVALID_LEF_NAME;
	double2 clsOrigin;
	double2 clsSize;
	std::vector<std::string> clsSymmetry;
	std::vector<LefPinDscp> clsPins;
	std::vector<LefObsDscp> clsObs;
	LefMacroDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Layer
class LefLayerDscp {
public:
	std::string clsName = INVALID_LEF_NAME;
	std::string clsType = INVALID_LEF_NAME;
	std::string clsDirection = INVALID_LEF_NAME;
	double clsPitch = 0.0;
	double clsOffset = 0.0;
	double clsWidth = 0.0;
	double clsSpacing = 0.0;
	LefLayerDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Site
class LefSiteDscp {
public:
	std::string clsName = INVALID_LEF_NAME;
	double2 clsSize;
	bool clsHasClass = false;
	std::string clsSiteClass = INVALID_LEF_NAME;
	LefSiteDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Spacing
class LefSpacingDscp {
public:
	std::string clsLayer1 = INVALID_LEF_NAME;
	std::string clsLayer2 = INVALID_LEF_NAME;
	double clsDistance = 0.0;
	LefSpacingDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF Units
class LefUnitsDscp {
public:
	bool clsHasTime = false;
	bool clsHasCapacitance = false;
	bool clsHasResitance = false;
	bool clsHasPower = false;
	bool clsHasCurrent = false;
	bool clsHasVoltage = false;
	bool clsHasDatabase = false;
	bool clsHasFrequency = false;
	
	int clsTime = 0;
	int clsCapacitance = 0;
	int clsResitance = 0;
	int clsPower = 0;
	int clsCurrent = 0;
	int clsVoltage = 0;
	int clsDatabase = 100; // default value at LEF/DEF reference
	int clsFrequency = 0;

	LefUnitsDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for LEF. The LEF elements are kept in the units defined at LefUnitsDscp.
class LefDscp {
public:
	int clsMajorVersion = 0;
	int clsMinorVersion = 0;
	std::string clsCaseSensitive = INVALID_LEF_NAME;
	std::string clsBusBitChars = INVALID_LEF_NAME;
	std::string clsDivideChar = INVALID_LEF_NAME;
	double clsManufactGrid = 0.0;
	LefUnitsDscp clsLefUnitsDscp;
	std::vector<LefSiteDscp> clsLefSiteDscps;
	std::vector<LefLayerDscp> clsLefLayerDscps;
	std::vector<LefMacroDscp> clsLefMacroDscps;
	std::vector<LefSpacingDscp> clsLefSpacingDscps;
	LefDscp()=default;
}; // end class 

// -----------------------------------------------------------------------------

#endif /* PHYSICALDESCRIPTORS_H */

