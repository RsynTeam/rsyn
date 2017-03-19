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
 * SPEFControlParser.cpp
 *
 *  Created on: Feb 9, 2015
 *      Author: jucemar
 */

#include "SPEFControlParser.h"

SPEFControlParser::SPEFControlParser() {
	std::setlocale(LC_ALL, "en_US.UTF-8");
}  // end constructor

// -----------------------------------------------------------------------------

void SPEFControlParser::parseSPEF(const string &filename, ISPD13::SPEFInfo &spefInfos) {
	ISPD13::SpefParser spefParse(filename);

	bool valid;
	do {
        ISPD13::SpefNet spefNet;
		valid = spefParse.read_net_data(spefNet);
		if (!valid)
			break;
		//spefInfos.SPEFNets.push_back(spefNet);
		spefInfos.addNet(spefNet);

		spefNet.clear();

	} while (valid);
    cout << "\tRead " << spefInfos.getSize() << " SPEF nets." << "\n";

} // end method

// -----------------------------------------------------------------------------

SPEFControlParser::~SPEFControlParser() {

} // end destructor

// -----------------------------------------------------------------------------

