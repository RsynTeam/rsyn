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

#include "ISPD2018Reader.h"
#include "rsyn/util/Stepwatch.h"
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/phy/PhysicalDesign.h"
#include "rsyn/ispd18/Guide.h"
#include "rsyn/ispd18/RoutingGuide.h"
#include "rsyn/io/parser/guide-ispd18/GuideParser.h"
#include "rsyn/io/parser/lef_def/LEFControlParser.h"
#include "rsyn/io/parser/lef_def/DEFControlParser.h"
#include "rsyn/io/Graphics.h"

namespace Rsyn {
	
void ISPD2018Reader::load(const Json& params) {
	Rsyn::Session session;

	std::string path = params.value("path", "");
				
	if (!params.count("lefFile")) {
		std::cout << "[ERROR] LEF file not specified...\n";
		return;
	} // end if
	lefFile = session.findFile(params.value("lefFile", ""), path);
	
	if (!params.count("defFile")) {
		std::cout << "[ERROR] DEF file not specified...\n";
		return;
	} // end if
	defFile = session.findFile(params.value("defFile", ""), path);
	
	if (!params.count("guideFile")) {
		std::cout << "[ERROR] Guide file not specified...\n";
		return;
	} // end if
	guideFile = session.findFile(params.value("guideFile", ""), path);
	
	parsingFlow();
} // end method

// -----------------------------------------------------------------------------

void ISPD2018Reader::parsingFlow() {
	parseLEFFile();
	parseDEFFile();
	populateDesign();
	parseGuideFile();
	initializeAuxiliarInfrastructure();
} // end method

// -----------------------------------------------------------------------------

void ISPD2018Reader::parseLEFFile() {
	Stepwatch watch("Parsing LEF file");
	LEFControlParser lefParser;
	lefParser.parseLEF(lefFile, lefDescriptor);
} // end method

// -----------------------------------------------------------------------------

void ISPD2018Reader::parseDEFFile() {
	Stepwatch watch("Parsing DEF file");
	DEFControlParser defParser;
	defParser.parseDEF(defFile, defDescriptor);
} // end method

// -----------------------------------------------------------------------------

void ISPD2018Reader::parseGuideFile() {
	Stepwatch watch("Parsing guide file");
	GuideDscp guideDescriptor;
	GuideParser guideParser;
	guideParser.parse(guideFile, guideDescriptor);
	session.startService("rsyn.routingGuide");
	routingGuide = (RoutingGuide*) session.getService("rsyn.routingGuide");
	routingGuide->loadGuides(guideDescriptor);
} // end method

void ISPD2018Reader::populateDesign() {
	Stepwatch watch("Populating the design");

	Rsyn::Design design = session.getDesign();

	Reader::populateRsyn(lefDescriptor, defDescriptor, design);

	Json physicalDesignConfiguration;
	physicalDesignConfiguration["clsEnableMergeRectangles"] = true;
	physicalDesignConfiguration["clsEnableNetPinBoundaries"] = true;
	physicalDesignConfiguration["clsEnableRowSegments"] = true;
	session.startService("rsyn.physical", physicalDesignConfiguration);
	Rsyn::PhysicalService* phService = session.getService("rsyn.physical");
	Rsyn::PhysicalDesign physicalDesign = phService->getPhysicalDesign();
	physicalDesign.loadLibrary(lefDescriptor);
	physicalDesign.loadDesign(defDescriptor);
	physicalDesign.updateAllNetBounds(false);
} // end method

void ISPD2018Reader::initializeAuxiliarInfrastructure() {
	// Start graphics service...
	session.startService("rsyn.graphics",{});
	Graphics *graphics = session.getService("rsyn.graphics");
	graphics->coloringByCellType();
	//graphics->coloringRandomGray();

	// Start writer service...
	session.startService("rsyn.writer",{});
} // end method

} // end namespace