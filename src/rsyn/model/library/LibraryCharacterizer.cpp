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
 
#include <iostream>
#include <iomanip>
#include <cassert>

#include "LibraryCharacterizer.h"
#include "rsyn/util/FloatingPoint.h"
#include "rsyn/session/Session.h"

namespace Rsyn {

void LibraryCharacterizer::start(const Rsyn::Json &params) {
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::stop() {
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::logicalEffort_FindReferenceLibraryTimingArc() {
	const bool report = false;
	
	// Look for the timing arc with the small delay when driven a zero load.
	// Usually this should be the smaller inverter.
	
	clsLogicalEffort_ReferenceLibraryArcPointer[EARLY] = nullptr;
	clsLogicalEffort_ReferenceLibraryArcPointer[LATE ] = nullptr;
	
	clsLogicalEffort_ReferenceLibraryCell[EARLY ] = nullptr;
	clsLogicalEffort_ReferenceLibraryCell[LATE  ] = nullptr;
	
	Number smallerDelay[NUM_TIMING_MODES] = {
		+std::numeric_limits<Number>::infinity(),
		+std::numeric_limits<Number>::infinity()
	};
	
	for (Rsyn::LibraryCell lcell : clsDesign.allLibraryCells()) {
		for (const TimingMode mode : allTimingModes()) {
			for (Rsyn::LibraryArc larc : lcell.allLibraryArcs()) {
				Number avgDelay = 0;
				Number delay;
				Number oslew;
				
				clsTimingModel->calculateLibraryArcTiming(larc, mode, RISE, 0, 0, delay, oslew);
				avgDelay += delay;
				clsTimingModel->calculateLibraryArcTiming(larc, mode, FALL, 0, 0, delay, oslew);
				avgDelay += delay;
				
				avgDelay /= 2;

				if (avgDelay < smallerDelay[mode]) {
					clsLogicalEffort_ReferenceLibraryArcPointer[mode] =	larc;
					clsLogicalEffort_ReferenceLibraryCell[mode] = lcell;
					smallerDelay[mode] = avgDelay;
					if (report) {
						std::cout << "New ref larc '" << larc.getFullName() << " " << avgDelay << "\n";
					} // end if
				} // end if
			} // end for
		} // end for
	} // end for
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::logicalEffort_ClaculateReferenceSlew() {
	const bool report = false;
	const int maxIterations = 100;
	const Number h = 1;

	for (const TimingMode mode : allTimingModes()) {
		Rsyn::LibraryCell lcell = clsLogicalEffort_ReferenceLibraryCell[mode];
		Rsyn::LibraryArc larc = clsLogicalEffort_ReferenceLibraryArcPointer[mode];
		Rsyn::LibraryPin lpin = larc.getFromLibraryPin();
		
		const LibraryArcCharacterization &timingLibraryArc = 
				getLibraryArcCharacterization(larc);
		
		for (const TimingTransition edge : allTimingTransitions()) {
			const Number Cin = clsTimingModel->getLibraryPinInputCapacitance(lpin);
			const Number load = h*Cin;
			if (report) {
				std::cout << "Computing reference slew for " << (edge==RISE?"RISE":"FALL") << " transition...\n";
				std::cout << "reference arc : " << larc.getFullName() << "\n";
				std::cout << "reference gain: " << h << "\n";
				std::cout << "reference load: " << load << "\n";
				std::cout << "edge: " << (edge==FALL? "FALL" : "RISE") << "\n";
			} // end if

			Number currSlew = 0;
			Number prevSlew = 0;
			
			for (int i = 0; i < maxIterations; i++) {
				Number dummy;
				clsTimingModel->calculateLibraryArcTiming(larc, mode, edge, prevSlew, load, dummy, currSlew);
				
				if (report) {
					std::cout << std::setw(2) << i << " " << (edge==FALL? "FALL" : "RISE")
							<< " " << prevSlew << " " << currSlew << " " << "\n";
				} // end if
				if (FloatingPoint::approximatelyEqual(prevSlew, currSlew))
					break;
				prevSlew = currSlew;
			} // end for

			clsLogicalEffort_ReferenceSlew[mode][edge] = currSlew;

			if (report) {
				std::cout << "Reference Slew: "
						<< (mode==EARLY? "EARLY" : "LATE") << " "
						<< (edge==FALL? "FALL" : "RISE") << " "
						<< clsLogicalEffort_ReferenceSlew[mode][edge] << "\n";
			} // end if		
			
		} // end for
	} // end for
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::runLibraryAnalysis(Rsyn::Design design, TimingModel * timingModel) {
	// Initialization...
	clsDesign = design;
	clsTimingModel = timingModel;
	clsLibraryMaxDriverResistance[EARLY] = -std::numeric_limits<Number>::max();
	clsLibraryMaxDriverResistance[LATE] = -std::numeric_limits<Number>::max();
	clsLibraryMinDriverResistance[EARLY] = std::numeric_limits<Number>::max();
	clsLibraryMinDriverResistance[LATE] = std::numeric_limits<Number>::max();
	
	// Creates an attribute to hold the characterization data.
	clsLibraryArcCharacterizations = clsDesign.createAttribute();
	
	logicalEffort_FindReferenceLibraryTimingArc();
	logicalEffort_ClaculateReferenceSlew();
	
	// Define gains where the delay will be computed.
	const int N = 32 + 1; // 0 ... 32
	clsLogicalEffort_Gains.resize(N);
	for (int i = 0; i < N; i++) {
		clsLogicalEffort_Gains[i] = i;
	} // end for
	
	// Compute the logical effort for each timing arc.
	for (Rsyn::LibraryCell lcell : clsDesign.allLibraryCells()) {
		for (Rsyn::LibraryArc larc : lcell.allLibraryArcs()) {
			Rsyn::LibraryPin lpin = larc.getFromLibraryPin();
			
			LibraryArcCharacterization &timingLibraryArc = getLibraryArcCharacterization(larc);
			timingLibraryArc.sense = clsTimingModel->getLibraryArcSense(larc);
			
			for (std::tuple<TimingMode, TimingTransition> element : allTimingModeAndTransitionPairs()) {
				const TimingMode mode = std::get<0>(element);
				const TimingTransition oedge = std::get<1>(element);

				LibraryArcCharacterization::LogicalEffort &arcLe = timingLibraryArc.le[mode];
				
				// [NOTE] Since the our infrastructure does not takes into
				// account different capacitance of each transition, there's
				// nothing to be done in this switch. But we let it here for 
				// future use.
				switch (timingLibraryArc.sense) {
					case POSITIVE_UNATE:
						arcLe.cin[RISE] = clsTimingModel->getLibraryPinInputCapacitance(lpin); // [RISE]
						arcLe.cin[FALL] = clsTimingModel->getLibraryPinInputCapacitance(lpin); // [FALL]
						
						arcLe.slew[RISE] = clsLogicalEffort_ReferenceSlew[mode][RISE];
						arcLe.slew[FALL] = clsLogicalEffort_ReferenceSlew[mode][FALL];
						break;
						
					case NEGATIVE_UNATE:
						arcLe.cin[RISE] = clsTimingModel->getLibraryPinInputCapacitance(lpin); // [FALL];
						arcLe.cin[FALL] = clsTimingModel->getLibraryPinInputCapacitance(lpin); // [RISE];
						
						arcLe.slew[RISE] = clsLogicalEffort_ReferenceSlew[mode][FALL];
						arcLe.slew[FALL] = clsLogicalEffort_ReferenceSlew[mode][RISE];				
						break;
						
					case NON_UNATE:
						arcLe.cin[RISE] = arcLe.cin[FALL] =	(
								clsTimingModel->getLibraryPinInputCapacitance(lpin)/*[RISE]*/ + 
								clsTimingModel->getLibraryPinInputCapacitance(lpin)/*[FALL]*/) / 2;
						
						arcLe.slew[RISE] = arcLe.slew[FALL] = (
								clsLogicalEffort_ReferenceSlew[mode][FALL] + 
								clsLogicalEffort_ReferenceSlew[mode][RISE]) / 2;
						break;
						
					default:
						assert(false);
				} // end switch					
								
				logicalEffort_TimingArc(larc, mode, oedge, 
						arcLe.cin[oedge], clsLogicalEffort_ReferenceSlew[mode][oedge], clsLogicalEffort_Gains,
						arcLe.g[oedge], arcLe.p[oedge], arcLe.residuum[oedge]);
				arcLe.valid[oedge] = 1;
			} // end for
			
			clsLibraryMaxDriverResistance[EARLY] = std::max(
					clsLibraryMaxDriverResistance[EARLY],
					getDriverResistance(larc, EARLY));
			clsLibraryMaxDriverResistance[LATE] = std::max(
					clsLibraryMaxDriverResistance[LATE],
					getDriverResistance(larc, LATE));
			if (getDriverResistance(larc, EARLY, RISE)) {
				clsLibraryMinDriverResistance[EARLY] = std::min(
						clsLibraryMinDriverResistance[EARLY],
						getDriverResistance(larc, EARLY, RISE));
			} // end if
			if (getDriverResistance(larc, EARLY, FALL)) {
				clsLibraryMinDriverResistance[EARLY] = std::min(
						clsLibraryMinDriverResistance[EARLY],
						getDriverResistance(larc, EARLY, FALL));
			} // end if
			if (getDriverResistance(larc, LATE, RISE)) {
				clsLibraryMinDriverResistance[LATE] = std::min(
						clsLibraryMinDriverResistance[LATE],
						getDriverResistance(larc, LATE, RISE));
			} // end if
			if (getDriverResistance(larc, LATE, FALL)) {
				clsLibraryMinDriverResistance[LATE] = std::min(
						clsLibraryMinDriverResistance[LATE],
						getDriverResistance(larc, LATE, FALL));
			} // end if
		} // end for
	} // end for cells
} // end method

// -----------------------------------------------------------------------------

// Adapted from https://people.sc.fsu.edu/~jburkardt/cpp_src/llsq/llsq.cpp
// y = a * x + b 

void LibraryCharacterizer::logicalEffort_LinearLeastSquares(
		const std::vector<Number> &x, 
		const std::vector<Number> &y, 
		Number &a, 
		Number &b
) {
	const int n = x.size();

	Number bot;
	int i;
	Number top;
	Number xbar;
	Number ybar;

	if (n == 1) {
		//  Special case.
		a = 0.0;
		b = y[0];
	} else {
		//  Average X and Y.
		xbar = 0.0;
		ybar = 0.0;
		for (i = 0; i < n; i++) {
			xbar = xbar + x[i];
			ybar = ybar + y[i];
		} // end for
		xbar = xbar / (Number) n;
		ybar = ybar / (Number) n;

		//  Compute Beta.
		top = 0.0;
		bot = 0.0;
		for (i = 0; i < n; i++) {
			top = top + (x[i] - xbar) * (y[i] - ybar);
			bot = bot + (x[i] - xbar) * (x[i] - xbar);
		} // end for
		a = top / bot;
		b = ybar - a * xbar;
	} // end else
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::logicalEffort_LinearLeastSquaresError(
		const std::vector<Number> &x, 
		const std::vector<Number> &y, 
		const Number a, 
		const Number b,
		Number &residuum
){
	const int n = x.size();
	
	residuum = 0;
	for (int i = 0; i < n; i++) {	
		const Number estimate = a*x[i] + b;
		const Number error = std::pow(y[i] - estimate, 2.0f);
		//cout << "h=" << x[i] << " delay=" << y[i] << " estimate=" << (estimate) << "\n";
		residuum += error;
	} // end for
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::logicalEffort_TimingArc(
		Rsyn::LibraryArc larc,
		const TimingMode mode,
		const TimingTransition oedge,
		const Number Cin, 
		const Number inputSlew, 
		const std::vector<Number> &gains,
		Number &g, 
		Number &p,
		Number &residuum
) {
	const int N = gains.size();

	std::vector<Number> delays(N);
	
	for (int i = 0; i < N; i++) {
		const Number h = gains[i];
		const Number load = h*Cin;
		
		Number delay;
		Number oslew;
		clsTimingModel->calculateLibraryArcTiming(
				larc, mode, oedge, inputSlew, load, delay, oslew);
		delays[i] = delay;
	} // end for
	
	logicalEffort_LinearLeastSquares(gains, delays, g, p);
	logicalEffort_LinearLeastSquaresError(gains, delays, g, p, residuum);
} // end method

// -----------------------------------------------------------------------------

void LibraryCharacterizer::logicalEffort_Report(std::ostream &out) {
	out << "--------------------------------------------------------------------------------\n";
	out << "Logical Effort Report\n";
	out << "--------------------------------------------------------------------------------\n";
	
	// Compute the logical effort for each timing arc.
	for (Rsyn::LibraryCell lcell : clsDesign.allLibraryCells()) {

		out << "Cell: "	<< lcell.getName() << "\n";
		
		for (Rsyn::LibraryArc larc : lcell.allLibraryArcs()) {
			const LibraryArcCharacterization &timingLibraryArc = getLibraryArcCharacterization(larc);
			
			for (std::tuple<TimingMode, TimingTransition> element : allTimingModeAndTransitionPairs()) {
				const TimingMode mode = std::get<0>(element);
				const TimingTransition oedge = std::get<1>(element);			

				const LibraryArcCharacterization::LogicalEffort &arcLe = timingLibraryArc.le[mode];
				
					out << "\t" 
							<< larc.getName() << ": "
							<< "mode=" << (mode==EARLY?"EARLY":"LATE") << " "
							<< "edge=" << (oedge==RISE?"RISE":"FALL") << " "
							<< "g=" << arcLe.g[oedge] << " "
							<< "p=" << arcLe.p[oedge] << " "
							<< "cin=" << arcLe.cin[oedge] << " "
							<< "slew=" << arcLe.slew[oedge] << " "							
							<< "error=" << logicalEffort_MeanMSE(arcLe.residuum[oedge]) << " "
							<< "valid=" << arcLe.valid[oedge] << "\n";
					
					out << "\t\t" << "Delay Estimation via Logical Effort\n";
					
					out << "\t\t";
					out << std::setw(4) << "gain";
					out << std::setw(9) << "load";
					out << std::setw(9) << "delay";
					out << std::setw(9) << "le_delay";
					out << "\n";					
					
					const LibraryArcCharacterization::LogicalEffort &le = timingLibraryArc.le[mode];
					
					for (unsigned i = 0; i < clsLogicalEffort_Gains.size(); i++) {
						const Number h = clsLogicalEffort_Gains[i];
						const Number load = h * le.cin[oedge]; // cin is timing sense aware
						const Number le_delay = le.p[oedge] + le.g[oedge]*h;

						Number libraryDelay;
						Number librarySlew;
						clsTimingModel->calculateLibraryArcTiming(
								larc, mode, oedge, le.slew[oedge], load, libraryDelay, librarySlew); // le.slew is timing sense aware
												
						
						out << "\t\t";
						out << std::setw(4) << h;
						out << std::setw(9) << load;
						out << std::setw(9) << libraryDelay;
						out << std::setw(9) << le_delay;
						out << "\n";
					} // end for
			} // end for each timing mode / timing transition
		} // end for each library arc
	} // end for
	
} // end method

} // end namespace