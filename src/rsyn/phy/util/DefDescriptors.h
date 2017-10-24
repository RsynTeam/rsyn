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
 * File:   DefDescriptors.h
 * Author: jucemar
 *
 * Created on 6 de Julho de 2016, 18:24
 */

#ifndef DEFDESCRIPTORS_H
#define DEFDESCRIPTORS_H

#include <vector>

#include "rsyn/util/Bounds.h"
#include "rsyn/util/dbu.h"

static const std::string INVALID_DEF_NAME = "*<INVALID_DEF_NAME>*";

//! Descriptor for DEF rows.

class DefRowDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::string clsSite = INVALID_DEF_NAME;
	DBUxy clsOrigin;
	std::string clsOrientation = INVALID_DEF_NAME;
	int clsNumX = 0;
	int clsNumY = 0;
	int clsStepX = 0;
	int clsStepY = 0;
	DefRowDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF components

class DefComponentDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::string clsMacroName = INVALID_DEF_NAME;
	std::string clsLocationType = INVALID_DEF_NAME; // {FIXED | COVER | PLACED | UNPLACED }
	DBUxy clsPos;
	std::string clsOrientation = INVALID_DEF_NAME;
	bool clsIsFixed = false;
	bool clsIsPlaced = false;
	DefComponentDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF ports

class DefPortDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::string clsNetName = INVALID_DEF_NAME;
	std::string clsDirection = INVALID_DEF_NAME; // {INPUT | OUTPUT | INOUT | FEEDTHRU}
	std::string clsLocationType = INVALID_DEF_NAME; // {FIXED | COVER | PLACED }
	DBUxy clsPos;
	DBUxy clsICCADPos;
	std::string clsOrientation = INVALID_DEF_NAME;
	std::string clsLayerName = INVALID_DEF_NAME;
	Bounds clsLayerBounds;
	DefPortDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF Net connections

class DefNetConnection {
public:
	std::string clsPinName = INVALID_DEF_NAME;
	std::string clsComponentName = INVALID_DEF_NAME;
	DefNetConnection() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for wire path associated to wires 

class DefWireSegmentDscp {
public:
	std::string clsLayerName = INVALID_DEF_NAME;
	std::string clsViaName = INVALID_DEF_NAME;
	int clsExtensionBegin =  -1;
	int clsExtensionEnd = -1;
	int clsMask = -1;
	int clsWidth = 0;
	bool clsNew : 1;
	bool clsHasVia : 1;
	bool clsHasRectangle : 1;
	/*
	 * "RECT ( deltax1 deltay1 deltax2 deltay2 )
	 * Indicates that a rectangle is created from the previous ( x y ) 
	 * routing point using the delta values. The RECT values leave the 
	 * current point and layer unchanged." Source: LEf/DEf Reference Manual 5.8
	 */
	Bounds clsRect;
	std::vector<DBUxy> clsPoints;

	DefWireSegmentDscp() {
		clsNew = false;
		clsHasVia = false;
		clsHasRectangle = false;
	} // end constructor 
}; // end class 

//! Descriptor for routed wires 

class DefWireDscp {
public:
	std::vector<DefWireSegmentDscp> clsWireSegments;
	std::string clsWireType = INVALID_DEF_NAME;
	DefWireDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF Nets

class DefNetDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::vector<DefNetConnection> clsConnections;
	std::vector<DefWireDscp> clsWires;
	DefNetDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF Regions

class DefRegionDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::string clsType = INVALID_DEF_NAME; // FENCE or GUIDE
	std::vector<Bounds> clsBounds;
	DefRegionDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for Def Groups 

class DefGroupDscp {
public:
	std::string clsName = INVALID_DEF_NAME;
	std::vector<std::string> clsPatterns; // It may be a pattern, e.g. "name/*"
	std::string clsRegion = INVALID_DEF_NAME;
	DefGroupDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

//! Descriptor for DEF Design

class DefDscp {
public:
	double clsVersion = 0.0;
	std::string clsDeviderChar = INVALID_DEF_NAME;
	std::string clsBusBitChars = INVALID_DEF_NAME;
	std::string clsDesignName = INVALID_DEF_NAME;
	Bounds clsDieBounds;
	int clsDatabaseUnits = 0;
	std::vector<DefRowDscp> clsRows;
	std::vector<DefComponentDscp> clsComps;
	std::vector<DefPortDscp> clsPorts;
	std::vector<DefNetDscp> clsNets;
	std::vector<DefRegionDscp> clsRegions;
	std::vector<DefGroupDscp> clsGroups;
	DefDscp() = default;
}; // end class 

// -----------------------------------------------------------------------------

#endif /* DEFDESCRIPTORS_H */

