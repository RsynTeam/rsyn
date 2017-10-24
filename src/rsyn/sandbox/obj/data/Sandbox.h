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
 
namespace Rsyn {

struct SandboxData {
	Module module;
	std::string name;

	List<SandboxPinData, RSYN_SANDBOX_LIST_CHUNCK_SIZE> pins;
	List<SandboxArcData, RSYN_SANDBOX_LIST_CHUNCK_SIZE> arcs;
	List<SandboxNetData, RSYN_SANDBOX_LIST_CHUNCK_SIZE> nets;
	List<SandboxInstanceData, RSYN_SANDBOX_LIST_CHUNCK_SIZE> instances;

	std::vector<std::string> instanceNames;
	std::vector<std::string> netNames;

	int anonymousInstanceId;
	int anonymousNetId;

	bool dirty;
	bool initialized;

	// Used for some netlist traversing (e.g. update topological ordering)...
	int sign;
	
	std::unordered_map<std::string, SandboxInstance> instanceNameMapping;
	std::unordered_map<std::string, SandboxNet> netMapping;
	std::map<Instance, SandboxInstance> mappingInstance;
	std::map<Net, SandboxNet> mappingNet;

	List<SandboxPort, RSYN_SANDBOX_LIST_CHUNCK_SIZE> ports;
	std::set<SandboxPort> portsByDirection[Rsyn::NUM_SIGNAL_DIRECTIONS];

	SandboxData() :
		initialized(false),
		dirty(false),
		anonymousInstanceId(0),
		anonymousNetId(0),
		sign(0) {
	} // end constructor

}; // end struct

} // end namespace
