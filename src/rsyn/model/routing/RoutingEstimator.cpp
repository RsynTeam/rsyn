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
 
#include "rsyn/model/routing/RoutingEstimator.h"
#include "rsyn/model/routing/DefaultRoutingEstimationModel.h"
#include "rsyn/model/routing/DefaultRoutingExtractionModel.h"
#include "rsyn/engine/Engine.h"
#include "rsyn/phy/PhysicalService.h"

namespace Rsyn {

// -----------------------------------------------------------------------------

void RoutingEstimator::start(Engine engine, const Json &params) {
	design = engine.getDesign();
	module = design.getTopModule();

	clsScenario = engine.getService("rsyn.scenario");;

	clsTotalWirelength = 0.0;
	clsRoutingNets = design.createAttribute();

	clsFullUpdateAlreadyPerformed = false;

	// TODO: Maybe we should not do this here as this create a soft dependency
	// to physical layer
	Rsyn::PhysicalService *physical =
			engine.getService("rsyn.physical", Rsyn::SERVICE_OPTIONAL);
	if (physical) {
		Rsyn::PhysicalDesign phDesign;
		phDesign = physical->getPhysicalDesign();
		phDesign.addPostInstanceMovedCallback(0, [&](Rsyn::PhysicalInstance instance) {
			dirtyInstance(instance.getInstance());
		});
	} // end if

	// Observe changes in the netlist.
	design.registerObserver(this);
	
	{ // updateRoutingEstimation
		ScriptParsing::CommandDescriptor dscp;
		dscp.setName("updateRoutingEstimation");
		dscp.setDescription("Performs the routing estimation of a given net or the entire netlist.");
		
		dscp.addNamedParam("full",
			ScriptParsing::PARAM_TYPE_BOOLEAN,
			ScriptParsing::PARAM_SPEC_OPTIONAL,
			"Determines whether a full routing estimation is performed.",
			"false");
		
		dscp.addNamedParam("net",
			ScriptParsing::PARAM_TYPE_STRING,
			ScriptParsing::PARAM_SPEC_OPTIONAL,
			"Name o of the target net.",
			"");
		
		engine.registerCommand(dscp, [&](Rsyn::Engine engine, const ScriptParsing::Command &command) {
			const bool full = command.getParam("full");
			const std::string netName = command.getParam("net");
			
			if (full) {
				updateRoutingFull();
			} else if (netName != "") {
				const Rsyn::Net net = design.findNetByName(netName);
				if (!net) {
					std::cout << "[ERROR] Net \"" << netName << "\" not found.\n";
					return;
				} // end if
				updateRoutingOfNet(net);
			} else {
				updateRouting();
			} // end if
		});
	} // end block
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::stop() {
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::onPostNetCreate(Rsyn::Net net) {
	std::cout << "INFO: RoutingEstimator was notified about a new net.\n";
	clsDirtyNets.insert(net);
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::onPreNetRemove(Rsyn::Net net) {
	std::cout << "INFO: RoutingEstimator was notified about a net removal.\n";
	clsTotalWirelength -= getNetWirelength(net);
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::onPostCellRemap(Rsyn::Cell cell, Rsyn::LibraryCell oldLibraryCell) {
	dirtyInstance(cell);
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::updateRoutingOfNet(Rsyn::Net net) {
	if (net.getNumPins() < 2 || net == clsScenario->getClockNet())
		return;

	RoutingNet &timingNet = clsRoutingNets[net];

	// Incrementally update the Steiner wirelength;
	clsTotalWirelength -= timingNet.wirelength;

	DBU netSteinerWirelength = 0;
	if (routingEstimationModel) {
		Rsyn::RoutingTopologyDescriptor<int> topology;
		routingEstimationModel->updateRoutingEstimation(net, topology, netSteinerWirelength);
		if (routingExtractionModel) {
			routingExtractionModel->extract(topology, timingNet.rctree);
		} // end if
	} // end if

	// Incrementally update the Steiner wirelength;
	timingNet.wirelength = netSteinerWirelength;
	clsTotalWirelength += netSteinerWirelength;
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::updateRoutingFull() { 
	StopwatchGuard guard(clsStopwatchUpdateSteinerTrees);
	
	// Update steiner trees.
#if !MULTI_THREADED_STEINER_TREE_UPDATE
	// Single-Thread
	for (Rsyn::Net net : module.allNets()) {
		updateRoutingOfNet(net);
	} // end for
#else
	// Multi-Thread
	// Why didn't it work? Looks straightforward, but even with 4 threads it
	// takes the same amount of time compared to a single thread. Looking at
	// processor usage, it looks only one is fully working. But I checked using
	// debug messages and the threads seems actually working in parallel. I did
	// not see the system using the swap memory.

	// Answer: it was because a single net, the clock net, which is huge, was
	// taking the most of runtime (~70% for leon3mp). So splitting the nets
	// without taking into account their degree is not a good idea. But even
	// doing a nice load balance, the clock tree would make the whole runtime
	// not much faster. We found that the bulk of runtime (~75%) for large trees 
	// in FLUTE is due to the sorting of node which is done using selection sort
	// in the version provided in the ICCAD 2014 contest.

	// For toy benchmarks in fact the number of cores may be greater than the 
	// nets, so we need to cast to double.
	const int numCores = max(2u, std::thread::hardware_concurrency());
	const int numWorkPerThread = (int) ceil(
		double(clsNumNets) / numCores
		);

	vector<std::thread> threads;

	cout << "#nets: " << clsNumNets << "\n";

	int n0 = 0;
	int n1 = 0;
	for (int i = 0; i < numCores; i++) {
		n1 = min(clsNumNets, n0 + numWorkPerThread);
		threads.push_back(std::thread([this, n0, n1]() { // don't use &, we need a copy here!
			for (int n = n0; n < n1; n++)
				updateRoutingOfNet(n);
		}));
		cout << "\tthread: " << n0 << " " << n1 << "\n";
		n0 = n1;
	} // end for

	for (auto &thread : threads) {
		thread.join();
	} // end for

	// Need to recompute Steiner WL due to race conditions in incremental 
	// steiner wirelength update.
	clsTotalWirelength = 0;
	for (DBU wl : clsNetSteinerWirelengths)
		clsTotalWirelength += wl;
#endif
	
	// Clear dirty routing cells.
	clsDirtyNets.clear();
	
	// Mark that a full update was already performed (required during 
	// initialization)
	clsFullUpdateAlreadyPerformed = true;
} // end method

// -----------------------------------------------------------------------------

void RoutingEstimator::updateRouting() {
	if (!clsFullUpdateAlreadyPerformed) {
		updateRoutingFull();
	} else {
		clsStopwatchUpdateSteinerTrees.start();
		for (auto& net : clsDirtyNets)
			updateRoutingOfNet(net);
		clsStopwatchUpdateSteinerTrees.stop();

		// Clear dirty routing cells.
		clsDirtyNets.clear();
	} // end if
} // end method

// -----------------------------------------------------------------------------

DBUxy RoutingEstimator::getSteinerPointer(Rsyn::Net net, Rsyn::Pin pin) const {
	const RCTree &rcTree = getRCTree(net);	
	const int index = getSteinerPointerIndex(rcTree, pin);
	if (index != -1) {
		DBU x = rcTree.getNodeTag(index).x;
		DBU y = rcTree.getNodeTag(index).y;
		return DBUxy(x,y);	
	} else {
		return DBUxy(
				std::numeric_limits<DBU>::quiet_NaN(),
				std::numeric_limits<DBU>::quiet_NaN());
	} // end else
} // end method 

// -----------------------------------------------------------------------------

int RoutingEstimator::getSteinerPointerIndex(const RCTree &rcTree, Rsyn::Pin pin) const {
	int pinIndex = getRCTreeConnectingNodeIndex(rcTree, pin);
	
	if (pinIndex != -1) {
		const RCTree::Node &node = rcTree.getNode(pinIndex);
		if (node.propEndpoint) {
			pinIndex = node.propParent;
		} // end if
	} // end if
	
	return pinIndex;
} // end method

// -----------------------------------------------------------------------------

int RoutingEstimator::getRCTreeConnectingNodeIndex(const RCTree &rcTree, Rsyn::Pin pin) const {
	int pinIndex = -1;
	for(int i = 0; i < rcTree.getNumNodes(); i++){
		Rsyn::Pin p1 = rcTree.getNodeTag(i).getPin();
		if(p1 == pin){
			pinIndex = i;
			break;
		} // end if 
	} // end for 	
	
	if( pinIndex < 0 ) {
		std::cout << "\n[BUG] Connecting node index not found.\n";
		std::cout << " pin = " << pin.getInstanceName() << ":" << pin.getName() << "\n";
	} // end if
	
	return pinIndex;
} // end method

} // end namespace