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
 * File:   DesignPositionReader.cpp
 * Author: jucemar
 * 
 * Created on 20 de Fevereiro de 2017, 19:12
 */

#include "DesignPositionReader.h"

#include "rsyn/io/parser/lef_def/DEFControlParser.h"

#include "rsyn/io/parser/bookshelf/BookshelfParser.h"

#include "rsyn/phy/util/BookshelfDscp.h"
#include "rsyn/phy/util/BookshelfMapper.h"

#include "rsyn/phy/PhysicalService.h"
#include "rsyn/phy/PhysicalDesign.h"

#include "rsyn/util/Stepwatch.h"

namespace Rsyn {

void DesignPositionReader::load(Rsyn::Engine engine, const Json & config ) {
	
	this->engine = engine;
	std::string path = config.value("path", "");
	clsDesign = engine.getDesign();
	clsModule = clsDesign.getTopModule();
	
	Rsyn::PhysicalService * phService = engine.getService("rsyn.physical");
	clsPhysicalDesign = phService->getPhysicalDesign();
	
	std::string ext = boost::filesystem::extension(path);
	
	// checking the extension file
	if (ext.compare(".pl") == 0) {
		openBookshelf(path);
	} else if (ext.compare(".def") == 0) {
		openDef(path);
	} else {
		throw Exception("Invalid extension file in the path " + path);
	} // end if-else 
} // end method 

// -----------------------------------------------------------------------------

void DesignPositionReader::openDef(std::string & path) {
	DEFControlParser defParser;
	DefDscp defDscp;
	defParser.parseDEF(path, defDscp);
	Rsyn::PhysicalService * phService = engine.getService("rsyn.physical");
	Rsyn::PhysicalDesign clsPhysicalDesign = phService->getPhysicalDesign();

	for (const DefComponentDscp &component : defDscp.clsComps) {
		Rsyn::Cell cell = clsDesign.findCellByName(component.clsName);

		if (!cell) {
			throw Exception("Library cell '" + component.clsName + "' not found.\n");
		} // end if

		if (cell.isFixed())
			continue;
		PhysicalCell physicalCell = clsPhysicalDesign.getPhysicalCell(cell);
		
		clsPhysicalDesign.placeCell(physicalCell, component.clsPos);
	} // end for 
} // end method 

// -----------------------------------------------------------------------------

void DesignPositionReader::openBookshelf(std::string & path) {
	BookshelfParser parser;
	BookshelfDscp dscp;

	parser.parsePlaced(path, dscp);
	Stepwatch watchParsing("Parsing Bookshelf Placed Design");
	watchParsing.finish();
	DBU scale = clsPhysicalDesign.getDatabaseUnits(Rsyn::DESIGN_DBU);
	for (const BookshelfNode & node : dscp.clsNodes) {
		Rsyn::Cell cell = clsDesign.findCellByName(node.clsName);
		if (!cell) {
			throw Exception("Library cell '" + node.clsName + "' not found.\n");
		} // end if

		if (cell.isFixed())
			continue;
		PhysicalCell physicalCell = clsPhysicalDesign.getPhysicalCell(cell);
		DBUxy pos = node.clsPos.convertToDbu();
		pos.scale(scale);
		clsPhysicalDesign.placeCell(physicalCell, pos);
	} // end for 

	clsPhysicalDesign.updateAllNetBounds(false);
} // end method 

// -----------------------------------------------------------------------------

} // end namespace 

