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
 * File:   BookshelfReader.cpp
 * Author: jucemar
 * 
 * Created on 20 de Fevereiro de 2017, 19:03
 */

#include "BookshelfReader.h"


#include "rsyn/io/parser/bookshelf/BookshelfParser.h"

#include "rsyn/phy/util/BookshelfDscp.h"
#include "rsyn/phy/util/BookshelfMapper.h"

#include "rsyn/phy/PhysicalService.h"
#include "rsyn/phy/PhysicalDesign.h"
#include "rsyn/io/Graphics.h"

#include "rsyn/util/Stepwatch.h"

namespace  Rsyn {

void BookshelfReader::load(Engine engine, const Json &params) {
	path = params.value("path", "");
	optionTargetUtilization = params.value("targetUtilization", 0.9);
	optionBenchmark = params.value("config", "");
	
	this->engine = engine;
	BookshelfDscp dscp;
	BookshelfParser parser;
	LefDscp lefDscp;
	DefDscp defDscp;
	BookshelfMapper mapper;

	const std::string relativePath = path + 
		boost::filesystem::path::preferred_separator + optionBenchmark;

	const bool checkAbsolutePath = boost::filesystem::exists(optionBenchmark);
	const bool checkRelativePath = boost::filesystem::exists(relativePath);

	if (!checkAbsolutePath && !checkRelativePath) {
		std::cout << "ERROR: Could not open bookshelf file '" << optionBenchmark << "'.\n";
		std::exit(1);
	} // end if

	// Prefer relative over absolute path.
	const std::string filename =  checkRelativePath?
		relativePath : optionBenchmark;
	
	Stepwatch watchParsing("Parsing Design");
	try {
		parser.parse(filename, dscp);
	} catch (const std::exception &e) {
		std::cout << "EXCEPTION: " << e.what() << "\n";
		std::exit(1);
	} // end catch
	
	mapper.mapLefDef(dscp, lefDscp, defDscp);
	clsDesign = engine.getDesign();
	watchParsing.finish();
	clsModule = clsDesign.getTopModule();

	Stepwatch watchRsyn("Populating Rsyn");
	mapper.populateRsyn(dscp, lefDscp, defDscp, clsDesign);
	watchRsyn.finish();	

	Stepwatch watchPopulateLayers("Initializing Physical Layer");
	Json phDesignJason;
	phDesignJason["clsEnableNetPinBoundaries"] = true;
	phDesignJason["clsEnableRowSegments"] = true;
	engine.startService("rsyn.physical", phDesignJason);		
	Rsyn::PhysicalService * phService = engine.getService("rsyn.physical");
	Rsyn::PhysicalDesign clsPhysicalDesign = phService->getPhysicalDesign();
	clsPhysicalDesign.loadLibrary(lefDscp);
	clsPhysicalDesign.loadDesign(defDscp);
	clsPhysicalDesign.updateAllNetBounds(false);
	watchPopulateLayers.finish();

	engine.startService("rsyn.graphics", {});
	Graphics * graphics = engine.getService("rsyn.graphics");
	graphics->coloringByCellType();

	engine.startService("rsyn.writer", {});
} // end method 

} // end namespace 

