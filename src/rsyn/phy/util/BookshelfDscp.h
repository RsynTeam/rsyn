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
 * File:   BookshelfDscp.h
 * Author: jucemar
 *
 * Created on 2 de Novembro de 2016, 10:28
 */

#include "rsyn/util/double2.h"

#include <vector>
#include <unordered_map>

#ifndef BOOKSHELFDSCP_H
#define BOOKSHELFDSCP_H

  // -----------------------------------------------------------------------------

class BookshelfRow {
public:
	std::string clsDirection;
	int clsCoordinate;
	int clsHeight;
	int clsSiteWidth;
	int clsSiteSpacing;
	std::string clsSiteOrientation;
	std::string clsSiteSymmetry;
	int clsSubRowOrigin;
	int clsNumSites;
}; // end class 

// -----------------------------------------------------------------------------

class BookshelfPin {
public:
	std::string clsNodeName;
	std::string clsDirection;
	double2 clsDisplacement;
}; // end class 

// -----------------------------------------------------------------------------

class BookshelfNet {
public:
	bool clsHasName;
	std::string clsName;
	std::vector<BookshelfPin> clsPins;
	BookshelfNet() {
		clsHasName = false;
	} // end constructor 
}; // end class 

// -----------------------------------------------------------------------------

class BookshelfNode {
public:
	std::string clsName;
	std::string clsOrientation;
	double2 clsSize;
	double2 clsPos;
	bool clsIsTerminal : 1;
	bool clsIsFixed : 1;
	bool clsSetPosition : 1;
	BookshelfNode() {
		clsIsTerminal = false;
		clsIsFixed = false;
		clsSetPosition = false;
	} // end constructor 
}; // end class 

// -----------------------------------------------------------------------------

class BookshelfDscp {
public:
	int clsNumPins;
	std::string clsDesignName;
	std::vector<BookshelfRow> clsRows;
	std::vector<BookshelfNet> clsNets;
	std::vector<BookshelfNode> clsNodes;
	std::unordered_map<std::string, int> clsMapNets;
	std::unordered_map<std::string, int> clsMapNodes;
	BookshelfDscp() {
		clsNumPins = 0;
	} // end constructor 
}; // end class 

// -----------------------------------------------------------------------------

#endif /* BOOKSHELFDSCP_H */

