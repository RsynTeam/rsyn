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
 
#ifndef MATRIX_H
#define	MATRIX_H

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <fstream>
#include <algorithm>

template< class T >
class DenseMatrix {
	private:
		vector<T> clsElements;
		
		int clsNumCols;
		int clsNumRows;
		
	public:

		DenseMatrix() {
			clsNumCols = 0;
			clsNumRows = 0;
		} // end constructor

		void initialize( const int numCols, const int numRows ) {
			clsNumCols = numCols;
			clsNumRows = numRows;
			clsElements.resize(clsNumCols * clsNumRows);
		} // end method

		void initialize( const int numCols, const int numRows, const T value ) {
			clsNumCols = numCols;
			clsNumRows = numRows;
			clsElements.clear();
			clsElements.resize(clsNumCols * clsNumRows, value);
		} // end method		
		
		void initialize( const int dimension ) {
			initialize( dimension, dimension );
		} // end method
		
		void assign(const T value) {
			clsElements.assign(clsElements.size(), value);
		} // end method			

		      T & operator()( const int col, const int row )       { return clsElements[ computeLinearIndex(col,row) ]; }
		const T & operator()( const int col, const int row ) const { return clsElements[ computeLinearIndex(col,row) ]; }

		      T & operator()( const int linearIndex )       { return clsElements[ linearIndex ]; }
		const T & operator()( const int linearIndex ) const { return clsElements[ linearIndex ]; }
		
		int getNumElements() const { return clsElements.size(); }
		int getNumCols() const { return clsNumCols; }
		int getNumRows() const { return clsNumRows; }
		
		int getCol( const int linearIndex ) const { return linearIndex % clsNumCols; }
		int getRow( const int linearIndex ) const { return linearIndex / clsNumCols; }

		int clampCol(const int col) const { return std::max(0, std::min(col, clsNumCols - 1)); }
		int clampRow(const int row) const { return std::max(0, std::min(row, clsNumRows - 1)); }

		bool isValidIndex(const int col, const int row) const {
			return (col>=0 && col<clsNumCols) && (row>=0 && row<clsNumRows);
		} // end method
		
        int computeLinearIndex( const int col, const int row ) const {
            return col + (row*clsNumCols);
        } // end method
    
		void gnuplotColorMap(const string &filename) const {
			std::ofstream plotfile(filename.c_str());
			if (!plotfile)
				return;

			plotfile << "set term png\n";
			plotfile << "set output \"" << filename << ".png\"\n";
			plotfile << "set autoscale fix\n";
			plotfile << "\n";
			plotfile << 
				"set palette defined ( 0 '#000090',\\\n"
				"                      1 '#000fff',\\\n"
				"                      2 '#0090ff',\\\n"
				"                      3 '#0fffee',\\\n"
				"                      4 '#90ff70',\\\n"
				"                      5 '#ffee00',\\\n"
				"                      6 '#ff7000',\\\n"
				"                      7 '#ee0000',\\\n"
				"                      8 '#7f0000')\n\n"
			;

			plotfile << "plot '-' matrix with image t ''\n";

			for ( int row = 0; row < clsNumRows; row++ ) {
				for ( int col = 0; col < clsNumCols; col++ ) {
					plotfile << (*this)(col,row) << " ";
				} // end for
				plotfile << "\n";
			} // end for

			plotfile.close();				
		} // end method
		
		void gnuplot3DMap(const string &filename) const {
			std::ofstream plotfile(filename.c_str());
			
			if (!plotfile)
				return;	
			
			plotfile << "set style data lines													\n"
						"set pm3d																\n"
						"set view 15, 30, 1, 1													\n"
						"set xlabel \"Bin x\"													\n" 
						"set ylabel \"Bin Y\"													\n"
						"set zlabel \"Utilization\" offset -4,0,0								\n"
						"set terminal epscairo enhanced color size 75in,50in font \"Arial,100\"	\n"
						"set output \"" << filename << ".eps									\n"
						"splot \"-\" using 2:1:3 notitle										\n\n";
			
			for ( int row = 0; row < clsNumRows; row++ ) {
				for ( int col = 0; col < clsNumCols; col++ ) {
					plotfile << row << " " << col << " " << (*this)(col,row) << "\n";
				} // end for
				
				plotfile << "\n";
			} // end for
		}			
		
}; // end class

#endif