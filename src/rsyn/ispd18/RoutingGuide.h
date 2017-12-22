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
 * File:   RoutingGuide.h
 * Author: jucemar
 *
 * Created on 21 de Dezembro de 2016, 17:47
 */

#ifndef ISPD18_ROUTINGGUIDE
#define ISPD18_ROUTINGGUIDE

#include "rsyn/session/Service.h"
#include "rsyn/session/Session.h"
#include "rsyn/ispd18/Guide.h"
#include "rsyn/io/parser/guide-ispd18/GuideDescriptor.h"

namespace Rsyn {

class RoutingGuide : public Rsyn::Service {
protected:
	Rsyn::Session clsSession;
	Rsyn::Design clsDesign;
	Rsyn::Module clsModule;
	Rsyn::PhysicalDesign clsPhDesign;
	Rsyn::Attribute<Rsyn::Net, Rsyn::NetGuide> clsGuides;
	bool clsInitialized  = false;
public:
	RoutingGuide() = default;
	void start(const Rsyn::Json &params);
	void stop();
	
	void loadGuides(const GuideDscp & dscp);
	
	const NetGuide & getGuide(Rsyn::Net net) const {
		return clsGuides[net];
	}
}; // end class 

} // end namespace 


#endif /* ISPD18_ROUTINGGUIDE */