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

namespace Rsyn {

inline std::string PhysicalLayer::getName() const {
	return data->clsName;
} // end method 

// -----------------------------------------------------------------------------

inline Rsyn::PhysicalLayerType PhysicalLayer::getType() const {
	return data->clsType;
} // end method 

// -----------------------------------------------------------------------------

inline Rsyn::PhysicalLayerDirection PhysicalLayer::getDirection() const {
	return data->clsDirection;
} // end method 

// -----------------------------------------------------------------------------

inline DBU PhysicalLayer::getWidth() const {
	return data->clsWidth;
} // end method 

// -----------------------------------------------------------------------------

inline DBU PhysicalLayer::getMinWidth() const {
	return data->clsMinWidth;
} // end method 

// -----------------------------------------------------------------------------

inline DBU PhysicalLayer::getMinArea() const {
	return data->clsArea;
} // end method 

// -----------------------------------------------------------------------------

inline DBU PhysicalLayer::getPitch(const Dimension dim) const {
	return data->clsPitch[dim];
} // end method 

// -----------------------------------------------------------------------------

inline DBUxy PhysicalLayer::getPitch() const {
	return data->clsPitch;
} // end method 

// -----------------------------------------------------------------------------

inline const std::vector<Rsyn::PhysicalSpacingRule> & PhysicalLayer::allSpacingRule() const {
	return data->clsSpacing;
} // end method 

// -----------------------------------------------------------------------------

inline int PhysicalLayer::getIndex() const {
	return data->id;
} // end method 

// -----------------------------------------------------------------------------

inline int PhysicalLayer::getRelativeIndex() const {
	return data->clsRelativeIndex;
} // end method 

} // end namespace 