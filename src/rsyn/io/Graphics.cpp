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
 
#include "Graphics.h"

#include "rsyn/engine/Engine.h"

// Services
#include "rsyn/phy/PhysicalService.h"
#include "rsyn/model/timing/Timer.h"
#include "rsyn/util/Colorize.h"
#include "rsyn/util/Environment.h"

namespace Rsyn {

void Graphics::start(Engine engine, const Json &params) {
	clsEngine = engine;

	// Jucemar - 2017/03/25 -> Physical variable are initialized only when physical service was started.
	// It avoids crashes when a design without physical data is loaded. 
	if (clsEngine.isServiceRunning("rsyn.physical")) {
		clsPhysical = engine.getService("rsyn.physical");
		clsPhysicalDesign = clsPhysical->getPhysicalDesign();
	} // end if 
	
	clsTimer = engine.getService("rsyn.timer", Rsyn::SERVICE_OPTIONAL);
	clsDesign = engine.getDesign();
	clsModule = clsDesign.getTopModule();	
	

	clsColorCells = clsDesign.createAttribute();
	
	clsDesign.registerObserver(this);
	
	
	{ // colorInstances
		ScriptParsing::CommandDescriptor dscp;
		dscp.setName("colorInstancesBySlack");
		dscp.setDescription("Colors the instance on the canvas based on their slack.");
		
		dscp.addPositionalParam( "timingMode", 
			ScriptParsing::PARAM_TYPE_STRING,
			ScriptParsing::PARAM_SPEC_MANDATORY,
			"Name of the timing mode (Currently supported: \"early\", \"late\")"
		);
		
		clsEngine.registerCommand(dscp, [&](Engine engine, const ScriptParsing::Command &command) {
			const std::string timingMode = command.getParam("timingMode");
			
			if (timingMode == "early")
				coloringSlack(Rsyn::EARLY);
			else if(timingMode == "late")
				coloringSlack(Rsyn::LATE);
			else
				std::cout << "[ERROR] Invalid parameter " << timingMode << "\n";
		});
	} // end block
	
	{ // setInstanceColor
		ScriptParsing::CommandDescriptor dscp;
		dscp.setName("setInstanceColor");
		dscp.setDescription("Changes the color of an instance on the canvas.");
		
		dscp.addPositionalParam( "instance", 
			ScriptParsing::PARAM_TYPE_STRING,
			ScriptParsing::PARAM_SPEC_MANDATORY,
			"Name of the target instance."
		);
		
		dscp.addPositionalParam( "red", 
			ScriptParsing::PARAM_TYPE_INTEGER,
			ScriptParsing::PARAM_SPEC_MANDATORY,
			"Red component of the color [0,255]."
		);

		dscp.addPositionalParam( "green", 
			ScriptParsing::PARAM_TYPE_INTEGER,
			ScriptParsing::PARAM_SPEC_MANDATORY,
			"Green component of the color [0,255]."
		);
		
		dscp.addPositionalParam( "blue", 
			ScriptParsing::PARAM_TYPE_INTEGER,
			ScriptParsing::PARAM_SPEC_MANDATORY,
			"Blue component of the color [0,255]."
		);
		
		clsEngine.registerCommand(dscp, [&](Engine engine, const ScriptParsing::Command &command) {
			const std::string instanceName = command.getParam("instance");
			const int red = command.getParam("red");
			const int green = command.getParam("green");
			const int blue = command.getParam("blue");
			
			Rsyn::Instance instance = clsDesign.findCellByName(instanceName);
			
			if (!instance) {
				std::cout << "Instance \"" << instanceName << "\" not found.\n";
				return;
			} // end if
			
			Color& c = getCellColor(instance);
			c.r = std::max(std::min(red, 255), 0);
			c.g = std::max(std::min(green, 255), 0);
			c.b = std::max(std::min(blue, 255), 0);
		});
	} // end block
} // end method

// -----------------------------------------------------------------------------

void Graphics::stop() {
} // end method

// -----------------------------------------------------------------------------

void Graphics::onPostInstanceCreate(Rsyn::Instance instance) {
	Color& color = clsColorCells[instance.asCell()]; // TODO: hack, assuming that the instance is a cell
	color.r = 0;
	color.g = 0;
	color.b = (unsigned) (127 + 128 * (rand() / float(RAND_MAX)));
} // end method

// -----------------------------------------------------------------------------

Rsyn::Instance Graphics::searchCellAt(const DBU x, const DBU y) {
	Rsyn::Instance c = nullptr;

	// Jucemar - 2017/03/25 -> Physical variable are initialized only when physical service was started.
	// It avoids crashes when a design without physical data is loaded. 
	if(!clsPhysical)
		return c;
	
	for (Rsyn::Instance cell : clsModule.allInstances()) {
		if(cell.getType() != Rsyn::CELL ) {
			std::cout<<"TODO "<<__func__<<" "<<cell.getName()<<" is not Rsyn::CELL type\n";
			continue;
		}
		PhysicalCell ph = clsPhysicalDesign.getPhysicalCell(cell.asCell());
		if (!ph.hasLayerBounds()) {
			const Bounds &bound = ph.getBounds();
			if (bound.inside(x, y))
				c = cell;
		} else {
			Rsyn::PhysicalLibraryCell phLibCell = clsPhysicalDesign.getPhysicalLibraryCell(cell.asCell());
			for (const Bounds & obs : phLibCell.allLayerObstacles()) {
				Bounds bounds = obs;
				DBUxy lower = obs.getCoordinate(LOWER);
				bounds.moveTo(ph.getPosition() + lower);
				if (bounds.inside(x, y))
					c = cell;
			} // end for 
		}// end if 
	} // end for

	return c;
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringRandomBlue() {
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Color & color = clsColorCells[instance];
		if (instance.isSequential()){
			color.r = 255;
			color.g = 20;
			color.b = (unsigned) (127 + 128 * (rand() / float(RAND_MAX)));
		} else if (instance.isLCB()) {
			color.r = 3;
			color.g = 192;
			color.b = 60;
		} else if (instance.isFixed()) {
			color.r = 0;
			color.g = 0;
			color.b = 0;
		} else {
			color.r = 0;
			color.g = 0;
			color.b = (unsigned) (127 + 128 * (rand() / float(RAND_MAX)));
		} // end else
	} // end for
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringByCellType() {	
	const bool isScenarioRunning = clsEngine.isServiceRunning("rsyn.scenario");
	
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		Color & color = clsColorCells[cell];

		if (isScenarioRunning && instance.isLCB()) {
			color.r = 3;
			color.g = 192;
			color.b = 60;
		} else if (isScenarioRunning && cell.isSequential()) {
			color.r = 255;
			color.g = 20;
			color.b = (unsigned) (127 + 128 * (rand() / float(RAND_MAX)));
		} else if (instance.isFixed()) {
			color.r = 0;
			color.g = 0;
			color.b = 0;
		} else {
			color.r = 0;
			color.g = 0;
			color.b = (unsigned) (127 + 128 * (rand() / float(RAND_MAX)));
		} // end else
	} // end for

} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringColorful() {
	static const int N = 4;
	static const Color RAINBOW_COLORS[N] = {
		{255, 0, 0}, // red
		{ 0, 255, 0}, // green
		{ 0, 0, 255}, // blue
		{255, 255, 0} // yellow
	};

	const bool isScenarioRunning = clsEngine.isServiceRunning("rsyn.scenario");

	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		Color &color = clsColorCells[cell];
		Rsyn::PhysicalCell phCell = clsPhysicalDesign.getPhysicalCell(cell);	
		if (isScenarioRunning && cell.isSequential()) {
			color.setRGB(255, 20, 147);
		} else if (instance.isFixed()) {
			color.setRGB(0, 0, 0);
		} else {
			const int index = (int) (floor(0.5 + (N - 1)*(rand() / float(RAND_MAX))));
			color = RAINBOW_COLORS[index];
		} // end else
	} // end for 
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringGray() {
	const static Color gray(230, 230, 230);
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		Color & color = clsColorCells[cell];
		color = gray;
	} // end for
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringSlack(TimingMode mode) {
	Color noSlackColor(230, 230, 230);
	//TODO use getCriticaly 
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		Color & color = clsColorCells[cell];
		color.setRGB(noSlackColor);
	} // end for 

	for(Rsyn::Net net : clsModule.allNets()){
		Rsyn::Pin driver = net.getAnyDriver();
		
		if( !driver ) {
			std::cout << "Net: " << net.getName() << " without a driver...\n";
			continue;
		}
		
		Rsyn::Cell cell  = driver.getInstance().asCell(); 
		
		Number critically = clsTimer->getCellCriticality(cell, mode);
		
		if(critically == 0) 
			continue;
		
		Color & color = clsColorCells[cell];
			
		int r, g, b;
		Colorize::colorTemperature(critically, r, g, b);
		color.setRGB(r, g, b);
	} // end for 
		
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringCriticality(const TimingMode mode, const double threshold) {
	Color nonCritical(230, 230, 230);
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		const double criticality = clsTimer->getCellCriticality(cell, mode);

		Color &color = clsColorCells[cell];
		if (criticality > 0 && criticality >= threshold) {
			int r, g, b;
			Colorize::colorTemperature(criticality, r, g, b);
			color.setRGB(r, g, b);
		} else {
			color = nonCritical;
		} // end else
	} // end for
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringRelativity(const TimingMode mode) {
	double minRelativity = +std::numeric_limits<double>::infinity();
	double maxRelativity = -std::numeric_limits<double>::infinity();
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		const double relativity = clsTimer->getCellRelativity(cell, mode);
		minRelativity = std::min(minRelativity, relativity);
		maxRelativity = std::max(maxRelativity, relativity);
	} // end for
	
	int r, g, b;
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		const double relativity = clsTimer->getCellRelativity(cell, mode);

		Color &color = clsColorCells[cell];
		if (relativity >= 1) {
			// Weight in the range: [0.5, 1]
			const double weight = ((relativity - 1)/(maxRelativity - 1) + 1.0)/2.0; 
			Colorize::colorTemperatureWarmCold(weight, r, g, b);
			color.setRGB(r, g, b);
		} else {
			// Weight in the range: [0, 0.5]
			const double weight = ((relativity - minRelativity)/(1 - minRelativity)) / 2.0;
			Colorize::colorTemperatureWarmCold(weight, r, g, b);
			color.setRGB(r, g, b);
		} // end else
	} // end for
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringCentrality(const TimingMode mode, const double threshold) {
	Color nonCritical(230, 230, 230);
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		const double centrality = clsTimer->getCellCentrality(cell, mode);

		Color &color = clsColorCells[cell];
		if (!FloatingPoint::approximatelyZero(centrality) && centrality >= threshold) {
			int r, g, b;
			Colorize::colorTemperature(centrality, r, g, b);
			color.setRGB(r, g, b);
		} else {
			color = nonCritical;
		} // end else
	} // end for
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringCriticalPath(const TimingMode mode, const bool showNeighbors) {
	Color colorCritical(255, 0, 0);
	Color colorNeighbor(0, 0, 255);

	// Clean-up
	coloringGray();
	
	// Get top critical path.
	std::vector<Timer::PathHop> criticalPath;
	clsTimer->queryTopCriticalPath(mode, criticalPath);

	if (criticalPath.empty())
		return;

	std::set<Rsyn::Cell> alreadyColored; // a cell can be in the critical path and also be a neighbor cell
	for (auto & hop : criticalPath) {
		Rsyn::Pin pin = hop.getPin();
		Rsyn::Cell cell  = pin.getInstance().asCell();
		
		if (alreadyColored.find(cell) == alreadyColored.end()) {
			Color &color = clsColorCells[cell];
			color = colorCritical;
			alreadyColored.insert(cell);
		} // end if
		
		if (showNeighbors) {
			for (Rsyn::Pin cellPin : cell.allPins()) {
				Rsyn::Net net = cellPin.getNet();
				if (net) {
					for (Rsyn::Pin netPin : net.allPins()) {
						Rsyn::Cell netCell  = netPin.getInstance().asCell();

						if (alreadyColored.find(netCell) == alreadyColored.end()) {
							Color &color = clsColorCells[netCell];
							color = colorNeighbor;
							// Don't insert the net cell in the already colored
							// cells as the critical color must have precedence.
						} // end if					
					} // end for
				} // end if
			} // end for
		} // end if
	} // end for	
} // end method

// -----------------------------------------------------------------------------

void Graphics::coloringQDPMoved() {
	Color noColor(200, 200, 200);

	// Jucemar - 2017/03/25 -> Physical variable are initialized only when physical service was started.
	// It avoids crashes when a design without physical data is loaded. 
	if(!clsPhysical)
		return;
	
	for (Rsyn::Instance instance : clsModule.allInstances()) {
		Rsyn::Cell cell = instance.asCell(); // TODO: hack, assuming that the instance is a cell
		Rsyn::PhysicalCell phCell = clsPhysicalDesign.getPhysicalCell(cell);
		const DBUxy initPos = phCell.getInitialPosition();//clsPhysicalDesign.getCellInitialPosition(cell);
		const Bounds & phBounds = phCell.getBounds();//clsPhysicalDesign.getCellBounds(cell);

		double hpwl = std::abs(phBounds[LOWER][X] - initPos[X]) + std::abs(phBounds[LOWER][Y] - initPos[Y]);
		
		Number wns = 0.0;
		for( auto pin : cell.allPins() )
			wns = std::min( wns, clsTimer->getPinWorstNegativeSlack( pin, LATE ) );
		
		double criticality = wns / clsTimer->getWns( LATE );
		
		if (hpwl == 0) {
			Color & color = clsColorCells[cell];
			color.setRGB(noColor);
		} else {
			int r, g, b;
			Colorize::colorTemperature(criticality, r, g, b);
			Color & color = clsColorCells[cell];
			color.setRGB(r, g, b);
		} // end if-else
	} // end for
} // end method  

} // end namespace