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
 
#ifndef ROUTING_EXTRACTOR_H
#define ROUTING_EXTRACTOR_H

#include <iostream>

#include "rsyn/core/Rsyn.h"
#include "rsyn/engine/Service.h"
#include "rsyn/phy/PhysicalDesign.h"
#include "rsyn/model/routing/RCTree.h"
#include "rsyn/model/routing/RoutingTopology.h"
#include "rsyn/model/routing/RoutingEstimationModel.h"
#include "rsyn/model/scenario/Scenario.h"
#include "rsyn/util/Stopwatch.h"

namespace Rsyn {

class RoutingExtractionModel {
private:

	Rsyn::Design design;
	Rsyn::Module module; // top module

	Scenario * clsScenario = nullptr;

public:

	virtual void extract(const Rsyn::RoutingTopologyDescriptor<int> &topology, Rsyn::RCTree &tree) = 0;

	// TODO: Remove these.
	virtual Number getLocalWireResPerUnitLength() const = 0;
	virtual Number getLocalWireCapPerUnitLength() const = 0;

}; // end class

} // end namespace

#endif