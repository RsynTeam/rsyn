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
 * File:   PhysicalLayerData.h
 * Author: jucemar
 *
 * Created on 13 de Setembro de 2016, 19:09
 */

#ifndef PHYSICALDESIGN_PHYSICALLAYERDATA_H
#define PHYSICALDESIGN_PHYSICALLAYERDATA_H

namespace Rsyn {

class PhysicalLayerData  : public PhysicalObject  {
public:
	DBU clsPitch;
	DBU clsWidth;
	DBU clsSpacing;
	Rsyn::PhysicalLayerType clsType;
	Rsyn::PhysicalLayerDirection clsDirection;
	std::string clsName;
	PhysicalLayerData() {
		clsPitch = 0;
		clsWidth = 0;
		clsSpacing = 0;
		clsType = INVALID_PHY_LAYER_TYPE;
		clsDirection = INVALID_PHY_LAYER_DIRECTION;
		clsName = Rsyn::getPhysicalInvalidName();
	}
}; // end class 

} // end namespace 


#endif /* PHYSICALDESIGN_PHYSICALLAYERDATA_H */

