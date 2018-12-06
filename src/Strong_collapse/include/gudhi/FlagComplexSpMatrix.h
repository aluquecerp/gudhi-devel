/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Siddharth Pritam 
 *
 *    Copyright (C) 2017 INRIA Sophia Antipolis (France)
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once

// #include "gudhi/Fake_simplex_tree.h"
// #include "gudhi/Simplex_tree.h"
#include <gudhi/Rips_edge_list.h>
// #include <boost/graph/adjacency_list.hpp>
// #include <boost/graph/bron_kerbosch_all_cliques.hpp>

#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_map>
#include <tuple>
#include <list>
#include <algorithm>
#include <chrono>

#include <ctime>
#include <fstream>

#include <Eigen/Sparse>

//using Fake_simplex_tree 	= Gudhi::Fake_simplex_tree ;
//using Simplex_tree   		= Gudhi::Simplex_tree<>;
//using Vertex 	            = std::size_t;
//using Simplex             	= Fake_simplex_tree::Simplex;

typedef std::size_t Vertex;
using Edge 					= std::pair<Vertex,Vertex>;
using Edge_list 			= std::vector<Edge>;

using MapVertexToIndex 	  	= std::unordered_map<Vertex,int>;
using Map 				  	= std::unordered_map<Vertex,Vertex>;

using sparseRowMatrix  	 	= Eigen::SparseMatrix<double, Eigen::RowMajor> ;
using rowInnerIterator 		= sparseRowMatrix::InnerIterator;

using intVector 	   = std::vector<int>;
using doubleVector 	   = std::vector<double>;
using vertexVector     = std::vector<Vertex>;
using boolVector       = std::vector<bool>;

using doubleQueue 	   = std::queue<double>;
// using matixTuple	   = std::tuple<sparseMatrix, sparseRowMatrix>;

typedef std::vector< std::tuple< double, Vertex, Vertex > > Filtered_sorted_edge_list;

//!  Class SparseMsMatrix 
/*!
  The class for storing the Vertices v/s MaxSimplices Sparse Matrix and performing collapses operations using the N^2() Algorithm.
*/
class FlagComplexSpMatrix
{
	private:

  	std::unordered_map<int,Vertex> rowToVertex;

  	// Vertices strored as an unordered_set
  	std::unordered_set<Vertex> vertices; 

  
  //! Stores the 1-simplices9edges) of the original Simplicial Complex.
    /*!
      \code
      simplexVector = std::vector< Simplex >
      \endcode
      This is a vector that stores all the maximal simplices of the Original Simplicial Complex. <br>
      \endverbatim
    */
 	Edge_list one_simplices;
 
	//! Stores the Map between vertices<B>rowToVertex  and row indices <B>rowToVertex -> row-index</B>.
    /*!
      \code
      MapVertexToIndex = std::unordered_map<Vertex,int>
      \endcode
      So, if the original simplex tree had vertices 0,1,4,5 <br>
      <B>rowToVertex</B> would store : <br>
      \verbatim
      Values =  | 0 | 1 | 4 | 5 | 
      Indices =   0   1   2   3
      \endverbatim
      And <B>vertexToRow</B> would be a map like the following : <br>
      \verbatim
      0 -> 0
      1 -> 1
      4 -> 2
      5 -> 3
      \endverbatim
    */
	MapVertexToIndex vertexToRow;

	//! Stores the number of vertices in the original Simplicial Complex.
    /*!
      This stores the count of vertices (which is also the number of rows in the Matrix).
    */
	std::size_t rows;

	std::size_t numOneSimplices;

	//! Stores the Sparse matrix of double values representing the Original Simplicial Complex.
    /*!
      \code
      sparseMatrix   = Eigen::SparseMatrix<double> ;
      \endcode
      So after counting the number of rows and num of Maximal simplices, this is initialised as : <br>
      \code
      sparseMxSimplices =  sparseMatrix(rows,numOneSimplices);
      \endcode
      And filled with columns by the Constructor with a Fake Simplex tree as an argument.
    ;
	sparseMatrix* Sparse*/


	sparseRowMatrix sparse_colpsd_adj_Matrix;       // Stores the collapsed sparse matrix representaion.
	sparseRowMatrix sparseRowAdjMatrix; 		// This is row-major version of the same sparse-matrix, to facilitate easy access to elements when traversing the matrix row-wise.

	//Fake_simplex_tree collapsed_fake_simplex_tree;
	//Simplex_tree collapsed_simplex_tree;

	//! Stores <I>true</I> for dominated rows and  <I>false</I> for undominated rows. 
    /*!
      Initialised to a vector of length equal to the value of the variable <B>rows</B> with all <I>false</I> values.
      Subsequent removal of dominated vertices is reflected by concerned entries changing to <I>true</I> in this vector.
    */
  	boolVector vertDomnIndicator;  //(domination indicator)
	//! Stores <I>true</I> for maximal simplex(dominated) columns and  <I>false</I> for a non-maximal(non-dominated) columns. 
    /*!
      Initialised to an vector of length equal to the value of the variable <B>cols</B> with all <I>false</I> values.
      Subsequent removal of Maximal Simplices (caused by removal of vertices) is reflected by concerned entries changing to <I>false</I> in this array.
    */
  	//boolVector simpDomnIndicator; //(domination indicator)

	//! Stores the indices of the rows to-be checked for domination in the current iteration. 
    /*!
      Initialised to a queue with all row-indices inserted.
      Subsequently once the row is checked for dominated the row-index is poped out from the queue. A row-index is inserted once again if it is a non-zero element of a dominated column.
    */
	boolVector activeIndicator; 	// active indicator
	boolVector contractionIndicator; //(contraction indicator)

  	doubleQueue rowIterator;
  	//! Stores the indices of the colums to-be checked for domination in the current iteration. 
    /*!
      Initialised to an empty queue.
      Subsequently once a dominated row is found, its non-zero column indices are inserted.
    */
	//doubleQueue columnIterator;

	//! Stores <I>true</I> if the current row is inserted in the queue <B>rowIterator<B> otherwise its value is <I>false<I>. 
    /*!
      Initialised to a boolean vector of length equal to the value of the variable <B>rows</B> with all <I>true</I> values.
      Subsequent removal/addition of a row from <B>rowIterator<B> is reflected by concerned entries changing to <I>false</I>/<I>true</I> in this vector.
    */
	boolVector rowInsertIndicator;  //(current iteration row insertion indicator)

  //! Stores <I>true</I> if the current column is inserted in the queue <B>columnIterator<B> otherwise its value is <I>false<I>. 
    /*!
      Initialised to a boolean vector of length equal to the value of the variable <B>cols</B> with all <I>false</I> values.
      Subsequent addition/removal of a column in  <B>columnIterator<B> is reflected by concerned entries changing to <I>true</I>/<I>false</I> in this vector.
    */
	//boolVector colInsertIndicator; //(current iteration column insertion indicator)
	
  //! Map that stores the Reduction / Collapse of vertices.
    /*!
      \code
      Map = std::unordered_map<Vertex,Vertex>
      \endcode
      This is empty to begin with. As and when collapses are done (let's say from dominated vertex <I>v</I> to dominating vertex <I>v'</I>) : <br>
      <B>ReductionMap</B>[<I>v</I>] = <I>v'</I> is entered into the map. <br>
      <I>This does not store uncollapsed vertices. What it means is that say vertex <I>x</I> was never collapsed onto any other vertex. Then, this map <B>WILL NOT</B> have any entry like <I>x</I> -> <I>x</I>.
      Basically, it will have no entry corresponding to vertex <I>x</I> at all. </I> 
    */
	Map ReductionMap;

	bool already_collapsed;
	int expansion_limit;

	void init()
	{
		rowToVertex.clear();
    	vertexToRow.clear();
    	one_simplices.clear();
    	ReductionMap.clear();
    	
		vertDomnIndicator.clear();
		rowInsertIndicator.clear();
		rowIterator.push(0);
		rowIterator.pop();
		
		rows = 0;
		
		numOneSimplices = 0;
		expansion_limit = 3;

  		already_collapsed = false;
	}
	
	//!	Function for computing the Fake Simplex_tree corresponding to the core of the complex.
    /*!
      First calls strong_collapse(), and then computes the Fake Simplex_tree of the core using the Sparse matrix that we have.
      How does it compute the Fake simplex tree ? <br>
      Goes over all the columns (remaining MaximalSimplices) and for each of them, inserts that simplex <br>
      ['that simplex' means the maximal simplex with all the (remaining) vertices] with all subfaces using the <br>
      <I>insert_new_edges()</I> function from Gudhi's Fake_simplex_tree.
    */
 	void after_collapse()
	{
     	sparse_colpsd_adj_Matrix   =  sparseRowMatrix(rows,rows); // Just for debugging purpose.
    	one_simplices.clear();
		for(int rw = 0 ; rw < rows ; ++rw)
		{
			if(not vertDomnIndicator[rw]) 				//If the current column is not dominated
			{
				auto nbhrs_to_insert = read_row_index(rw); // returns row indices of the non-dominated vertices.
        		for(auto & v: nbhrs_to_insert){
          			sparse_colpsd_adj_Matrix.insert(rw, v) = 1;
          			if(rw <= v)
          				one_simplices.push_back({rowToVertex[rw],rowToVertex[v]});
        		}
			}			
		}
    	// std::cout << sparse_colpsd_adj_Matrix << std::endl;
		return ;
	}
	//! Function to fully compact a particular vertex of the ReductionMap.
    /*!
      It takes as argument the iterator corresponding to a particular vertex pair (key-value) stored in the ReductionMap. <br>
	  It then checks if the second element of this particular vertex pair is present as a first element of some other key-value pair in the map.
	  If no, then the first element of the vertex pair in consideration is fully compact. 
	  If yes, then recursively call fully_compact_this_vertex() on the second element of the original pair in consideration and assign its resultant image as the image of the first element of the original pair in consideration as well. 
    */
	void fully_compact_this_vertex(Map::iterator iter)
	{
		Map::iterator found = ReductionMap.find(iter->second);
		if ( found == ReductionMap.end() )
			return;

		fully_compact_this_vertex(found);
		iter->second = ReductionMap[iter->second];
	}

	//! Function to fully compact the Reduction Map.
    /*!
      While doing strong collapses, we store only the immediate collapse of a vertex. Which means that in one round, vertex <I>x</I> may collapse to vertex <I>y</I>.
      And in some later round it may be possible that vertex <I>y</I> collapses to <I>z</I>. In which case our map stores : <br>
      <I>x</I> -> <I>y</I> and also <I>y</I> -> <I>z</I>. But it really should store :
      <I>x</I> -> <I>z</I> and <I>y</I> -> <I>z</I>. This function achieves the same. <br>
      It basically calls fully_compact_this_vertex() for each entry in the map.
    */
	void fully_compact()
	{
		Map::iterator it = ReductionMap.begin();
		while(it != ReductionMap.end())
		{
			fully_compact_this_vertex(it);
			it++;
		}
	}

	void sparse_strong_collapse()
	{
 		complete_domination_check(rowIterator, rowInsertIndicator, vertDomnIndicator); 		// Complete check for rows in rowIterator, rowInsertIndicator is a list of boolean indicator if a vertex is already inserted in the working row_queue (rowIterator)
  		if( not rowIterator.empty())
			sparse_strong_collapse();
		else
			return ;
  	}

	void complete_domination_check (doubleQueue& iterator, boolVector& insertIndicator, boolVector& domnIndicator)
	{
	  	double k;
	  	doubleVector nonZeroInnerIdcs;
	    while(not iterator.empty())       // "iterator" contains list(FIFO) of rows to be considered for domination check 
	    { 
	      	k = iterator.front();
	      	iterator.pop();
	      	insertIndicator[k] = false;
	    	if( not domnIndicator[k]) 				// Check if is  already dominated
	    	{ 
		        nonZeroInnerIdcs  = read_row_index(k); 					     
		        for (doubleVector::iterator it = nonZeroInnerIdcs.begin(); it!=nonZeroInnerIdcs.end(); it++) 
		        {
		       		int checkDom = pair_domination_check(k, *it);   	// "true" for row domination comparison
		        	if( checkDom == 1)                                  	// row k is dominated by *it, k <= *it;
			        {
			            setZero(k, *it);
			            break ;
			        }
		          	else if(checkDom == -1)                 				// row *it is dominated by k, *it <= k;
		            	setZero(*it, k);
			    }         
	    	}
	    }
	}

	int pair_domination_check( double i, double j) // True for row comparison, false for column comparison
	{
		if(i != j)
		{
			doubleVector Listi = read_row_index(i);
			doubleVector Listj = read_row_index(j);
			if(Listj.size() <= Listi.size())
			{
      			if(std::includes(Listi.begin(), Listi.end(), Listj.begin(), Listj.end())) // Listj is a subset of Listi
					return -1;
			}
			
			else 
				if(std::includes(Listj.begin(), Listj.end(), Listi.begin(), Listi.end())) // Listi is a subset of Listj
					return 1;
		}
		return 0;	
	}
	
	doubleVector read_row_index(double indx) 	// Returns list of non-zero columns of the particular indx. 
	{													
	  	doubleVector nonZeroIndices;     
	  	if(not vertDomnIndicator[indx])
	      	for (rowInnerIterator it(sparseRowAdjMatrix, indx); it; ++it) {             // Iterate over the non-zero columns
	        	if(not vertDomnIndicator[it.index()]) {
	            	nonZeroIndices.push_back(it.index());  // inner index, here it is equal to it.columns()
	        	}
	    	}
      	return nonZeroIndices;
	}

	void setZero(double dominated, double dominating)
	{
  		vertDomnIndicator[dominated] = true;
  		ReductionMap[rowToVertex[dominated]]    = rowToVertex[dominating];
  		
  		vertexToRow.erase(rowToVertex[dominated]);
  		vertices.erase(rowToVertex[dominated]);
  		rowToVertex.erase(dominated);

  		for (rowInnerIterator it(sparseRowAdjMatrix,dominated); it; ++it)  // Iterate over the non-zero rows
	      if(not vertDomnIndicator[it.index()] && not rowInsertIndicator[it.index()]) // Checking if the row is already dominated(set zero) or inserted	
	      {  
	        rowIterator.push(it.index()); 
	        rowInsertIndicator[it.index()] = true;
	      }
	}
 	
	vertexVector readRow(double rowIndx) // Returns list of non-zero "vertices" of the particular colIndx. the difference is in the return type
	{
		vertexVector colmns ; 
  		for (rowInnerIterator itCol(sparseRowAdjMatrix,rowIndx); itCol; ++itCol)  // Iterate over the non-zero columns
     		if(not vertDomnIndicator[itCol.index()])  					          // Check if the row corresponds to a dominated vertex
      			colmns.push_back(rowToVertex[itCol.index()]); 			          // inner index, here it is equal to it.col()
      	std::sort(colmns.begin(), colmns.end());
  		return colmns;
	}

	vertexVector readActiveRow(double rowIndx) // Returns list of all non-zero "vertices" of the particular colIndx which are currently active. the difference is in the return type.
	{
		vertexVector colmns ; 
  		for (rowInnerIterator itCol(sparseRowAdjMatrix,rowIndx); itCol; ++itCol)  // Iterate over the non-zero columns
     		if( not contractionIndicator[itCol.index()])  					      // Check if the row corresponds to a contracted vertex
      			colmns.push_back(rowToVertex[itCol.index()]); 			          // inner index, here it is equal to it.col()

      	std::sort(colmns.begin(), colmns.end());	
  		return colmns;
	}
	
	vertexVector readAllRow(double rowIndx) // Returns list of all non-zero "vertices" of the particular colIndx whether dominated or not. the difference is in the return type.
	{
		vertexVector colmns ; 
  		for (rowInnerIterator itCol(sparseRowAdjMatrix,rowIndx); itCol; ++itCol)  // Iterate over the non-zero columns
     		colmns.push_back(rowToVertex[itCol.index()]); 			// inner index, here it is equal to it.row()
     	std::sort(colmns.begin(), colmns.end());
  		return colmns;
	}

	void swap_rows(const Vertex & v, const Vertex & w) {	// swap the rows of v and w. Both should be members of the skeleton
		if(membership(v) && membership(w)){
			auto rw_v = vertexToRow[v];
			auto rw_w = vertexToRow[w];
			vertexToRow[v] = rw_w;
			vertexToRow[w] = rw_v;
			rowToVertex[rw_v] = w;
			rowToVertex[rw_w] = v;
		}
	}
	// template<typename type, typename matrix> // To update the working list might possible to include at one place
 //  	void setZero(const matrix& m, boolVector& domnIndicator, boolVector& insertIndicator, doubleQueue& iterator, double indx) // Prepares the queue for the next iteration.
	// {
	//     for (type it(m,indx); it; ++it)  // Iterate over the non-zero rows
	//       if(not domnIndicator[it.index()] && not insertIndicator[it.index()]) // Checking if the row is already dominated(set zero) or inserted	
	//       {  
	//         iterator.push(it.index()); 
	//         insertIndicator[it.index()] = true;
	//       }
	// } 
	// template <typename type, typename matrix>
	// doubleVector read_row_index(const matrix& m, const boolVector& domnIndicator, bool firstEntrOnly, double indx)
	// {
	// 	doubleVector nonZeroIndices; 
 //      	for (type it(m, indx); it; ++it)             // Iterate over the non-zero rows/columns
 //        	if(not domnIndicator[it.index()])
 //        	{
 //            	nonZeroIndices.push_back(it.index());  // inner index, here it is equal to it.row()/it.columns()
 //        		if(firstEntrOnly)
 //        			break;
 //        	}
    
 //      return nonZeroIndices;
	// }

public:

	//! Default Constructor
    /*!
      Only initialises all Data Members of the class to empty/Null values as appropriate.
      One <I>WILL</I> have to create the matrix using the Constructor that has an object of the Simplex_tree class as argument.
    */
	
	FlagComplexSpMatrix()
	{
		init();
	}

	FlagComplexSpMatrix(std::size_t expRows)
	{
		init();
		sparseRowAdjMatrix  = sparseRowMatrix(expansion_limit*expRows, expansion_limit*expRows);    	// Initializing sparseRowAdjMatrix, This is a row-major sparse matrix.  
	}

	//! Main Constructor
    /*!
      Argument is an instance of Fake_simplex_tree. <br>
      This is THE function that initialises all data members to appropriate values. <br>
      <B>rowToVertex</B>, <B>vertexToRow</B>, <B>rows</B>, <B>cols</B>, <B>sparseMxSimplices</B> are initialised here.
      <B>vertDomnIndicator</B>, <B>rowInsertIndicator</B> ,<B>rowIterator<B>,<B>simpDomnIndicator<B>,<B>colInsertIndicator<B> and <B>columnIterator<B> are initialised by init_lists() function which is called at the end of this. <br>
      What this does:
      	1. Populate <B>rowToVertex</B> and <B>vertexToRow</B> by going over through the vertices of the Fake_simplex_tree and assign the variable <B>rows</B> = no. of vertices
      	2. Initialise the variable <B>cols</B> to zero and allocate memory from the heap to <B>MxSimplices</B> by doing <br>
      	        <I>MxSimplices = new boolVector[rows];</I>
      	3. Iterate over all maximal simplices of the Fake_simplex_tree and populates the column of the sparseMatrix.
      	4. Initialise <B>active_rows</B> to an array of length equal to the value of the variable <B>rows</B> and all values assigned true. [All vertices are there in the simplex to begin with]
      	5. Initialise <B>active_cols</B> to an array of length equal to the value of the variable <B>cols</B> and all values assigned true. [All maximal simplices are maximal to begin with]
      	6. Calls the private function init_lists().
    */
    //Filtered_sorted_edge_list * edge_t = new Filtered_sorted_edge_list();
	FlagComplexSpMatrix(const size_t & num_vertices, const Filtered_sorted_edge_list  &edge_t) {
		init();

		// auto begin = std::chrono::high_resolution_clock::now();
 	  	
 		sparseRowAdjMatrix  = sparseRowMatrix(expansion_limit*num_vertices, expansion_limit*num_vertices);    	// Initializing sparseRowAdjMatrix, This is a row-major sparse matrix.  
		
		for(size_t bgn_idx = 0; bgn_idx < edge_t.size(); bgn_idx++) {
	  		std::vector<size_t>  s = {std::get<1>(edge_t.at(bgn_idx)), std::get<2>(edge_t.at(bgn_idx))};
	  		insert_new_edges(std::get<1>(edge_t.at(bgn_idx)), std::get<2>(edge_t.at(bgn_idx)), 1);
	  	}	

        // auto end = std::chrono::high_resolution_clock::now();
		 // std::cout << "Sparse Adjacency Matrix Created  " << std::endl;
		// std::cout << "Formation time : " <<  std::chrono::duration<double, std::milli>(end- begin).count()
	              // << " ms\n" << std::endl;
		// std::cout << "Total number of Initial maximal simplices are: " << cols << std::endl;		
		
	  	sparseRowAdjMatrix.makeCompressed(); 
       	
 	  	// std::cout << sparseRowAdjMatrix << std::endl;
	}

	//!	Destructor.
    /*!
      Frees up memory locations on the heap.
    */
	~FlagComplexSpMatrix()
	{
	}

	//!	Function for performing strong collapse.
    /*!
      calls sparse_strong_collapse(), and
      Then, it compacts the ReductionMap by calling the function fully_compact().
    */
	double strong_collapse() {
			auto begin_collapse  = std::chrono::high_resolution_clock::now();
			sparse_strong_collapse();
			already_collapsed = true;
			auto end_collapse  = std::chrono::high_resolution_clock::now();
		
			auto collapseTime = std::chrono::duration<double, std::milli>(end_collapse- begin_collapse).count();
			// std::cout << "Time of Collapse : " << collapseTime << " ms\n" << std::endl;
			
			// Now we complete the Reduction Map
			fully_compact();
			//Post processing...
			after_collapse();
			return collapseTime;
	}

	bool membership(const Vertex & v) {
		auto rw = vertexToRow.find(v);
		if(rw != vertexToRow.end())	
			return true;
		else
			return false;
	}
	 
	bool membership(const Edge & e) {  
    	auto u = std::get<0>(e);
        auto v = std::get<1>(e);
	    if(membership(u) && membership(v)) {
	    	auto rw_u = vertexToRow[u];
	    	auto rw_v = vertexToRow[v];
	    	if(rw_u <= rw_v)
	    		for( auto x : read_row_index(rw_v)){ // Taking advantage of sorted lists.
	    			if(rw_u == x)
	    				return true;	
	    			else if(rw_u < x)
	    			 	return false;
	    		}	 
	    	else
	    		for( auto x : read_row_index(rw_u)){ // Taking advantage of sorted lists.
	    			if(rw_v == x)
	    				return true;	
	    			else if(rw_v < x)
	    			 	return false;		
	    		}	 
	    }
		return false;

	}	 
	void insert_vertex(const Vertex & vertex, double filt_val)
	{
		auto rw = vertexToRow.find(vertex);
		if(rw == vertexToRow.end()) {	 
			sparseRowAdjMatrix.insert(rows,rows) = filt_val; 		// Initializing the diagonal element of the adjency matrix corresponding to rw_b.
			vertDomnIndicator.push_back(false);
			rowInsertIndicator.push_back(true);
			contractionIndicator.push_back(false);
			rowIterator.push(rows);
			vertexToRow.insert(std::make_pair(vertex, rows));
			rowToVertex.insert(std::make_pair(rows, vertex)); 
			vertices.emplace(vertex);
			rows++;
		}
    } 

	void insert_new_edges(const Vertex & u, const Vertex & v, double filt_val)
	{	
		insert_vertex(u, filt_val);
		if( u != v) {
			insert_vertex(v, filt_val);

			auto rw_u = vertexToRow.find(u);
			auto rw_v = vertexToRow.find(v);

			sparseRowAdjMatrix.insert(rw_u->second,rw_v->second) 	  = filt_val;
			sparseRowAdjMatrix.insert(rw_v->second,rw_u->second) 	  = filt_val;
			one_simplices.emplace_back(u, v);
			numOneSimplices++;
		}	
	    // else
		// 	std::cout << "Already a member simplex,  skipping..." << std::endl;
       		
	}

			

    std::size_t num_vertices() const {
       return vertices.size();
	}


	//!	Function for returning the ReductionMap.
    /*!
      This is the (stl's unordered) map that stores all the collapses of vertices. <br>
      It is simply returned.
    */
	
	Map reduction_map() const {
		return ReductionMap;
	}
    std::unordered_set<Vertex> vertex_set() const {
    	return vertices;
    }
    sparseRowMatrix collapsed_matrix() const {
    	return sparse_colpsd_adj_Matrix;
    }

    sparseRowMatrix uncollapsed_matrix() const {
    	return sparseRowAdjMatrix;
    }
    
    Edge_list all_edges() const {
    	return one_simplices;
    }

    vertexVector active_neighbors(const Vertex & v) {  
    	vertexVector nb;
    	auto rw_v = vertexToRow.find(v);
    	if(rw_v != vertexToRow.end())  		
    		nb = readActiveRow(rw_v->second);
		
		// std::cout << "Active neighbors of " << v << "  are :"; 
 	// 	for(auto & x: nb){
		// 	std::cout << x << ", " ;
		// }
		// std::cout << std::endl;
    	return nb;
    }
   //  vertexVector non_active_neighbors(const Vertex & v) {
   //  	vertexVector non_active;
   //  	for(auto & x : all_neighbors(v) )
   //  		if(contractionIndicator[vertexToRow[x]])
   //  			non_active.push_back(x);	
 		// return non_active;
   //  }

    vertexVector neighbors(const Vertex & v) {  
    	vertexVector nb;
    	auto rw_v = vertexToRow.find(v);
    	if(rw_v != vertexToRow.end())  		
    		nb = readRow(rw_v->second);
    	
    	return nb;
    }
    // vertexVector all_neighbors(const Vertex & v) {  
    // 	vertexVector nb;
    // 	auto rw_v = vertexToRow.find(v);
    // 	if(rw_v != vertexToRow.end())  		
    // 		nb = readAllRow(rw_v->second);
    	
    // 	return nb;
    // }

   //  vertexVector all_common_neigbhour(const Vertex &v, const Vertex & w){
   //  	auto  nbhrs_v = all_neighbors(v);
   //  	auto  nbhrs_w = all_neighbors(w);
   //  	std::vector<Vertex> common; // neighbors of v intersection w
 		// std::set_intersection(nbhrs_v.begin(), nbhrs_v.end(), nbhrs_w.begin(), nbhrs_w.end(), std::back_inserter(common));
 		// return common;
   //  }
    
   //  vertexVector all_active_common_neigbhour(const Vertex &v, const Vertex & w){
   //  	auto  nbhrs_v = active_neighbors(v);
   //  	auto  nbhrs_w = active_neighbors(w);
   //  	std::vector<Vertex> common_active; // neighbors of v intersection w
 		// std::set_intersection(nbhrs_v.begin(), nbhrs_v.end(), nbhrs_w.begin(), nbhrs_w.end(), std::back_inserter(common_active));
 		// return common_active;
   //  }

    vertexVector active_relative_neighbors(const Vertex & v, const Vertex & w){
    	std::vector<Vertex> diff; 
    	if(membership(v) && membership(w)){
    		auto nbhrs_v = active_neighbors(v);
    		auto nbhrs_w = active_neighbors(w);
 			std::set_difference(nbhrs_v.begin(), nbhrs_v.end(), nbhrs_w.begin(), nbhrs_w.end(), std::inserter(diff, diff.begin()));	
 			// std::cout << "Active neighbors of " << v << "relative to " << w << " are :"; 
 			// for(auto & x: diff){
 			// 	std::cout << x << ", " ;
 			// }
 			// std::cout << std::endl;
        }	
        return diff;
    }

 //    bool domination_check((const Vertex &v, const Vertex & w){ // checks if v is dominated by w // assumes both are active
 // 		if(v != w){
 // 			auto active_v = active_neighbors(v);
 // 			auto active_w = active_neighbors(w);
	// 		if(active_v.size() <= active_w.size())
	// 			if(std::includes(active_v.begin(), active_v.end(), active_w.begin(), active_w.end())) // Listj is a subset of Listi i.e. w is dominated by v.
	// 				return true;
	// 	}
	// 	return false;
	// }	

	// void update_active_indicator(const Vertex &v, const Vertex & w){ // Assumption [v,w] is an edge in the graph // updates the active/inactive flag in the neighbourhood of the edge [v,w].
		
	// 	bool init_active_v = activeIndicator[vertexToRow[v]];
	// 	bool init_active_w = activeIndicator[vertexToRow[v]];
	// 	if(init_active_v && init_active_w)


	// 	activeIndicator[vertexToRow[w]] = true; // forcefully making v,w active. So that it appears in v's neighbour list
	// 	activeIndicator[vertexToRow[v]] = true;
	// 	update_active_indicator(v);  
	// 	if(not init_active_v && not activeIndicator[vertexToRow[v]])
	// 		return;
		
	// 	if (activeIndicator[vertexToRow[v]]){
	// 		update_active_indicator(w);
	// 	}
	// 	else
	// 		activeIndicator[vertexToRow[w]] = init_active_w;

	// 	if( not init_active_v &&  activeIndicator[vertexToRow[v]])
	// 		deep_update_active_indicator(v);
	// 	if(not init_active_w &&  activeIndicator[vertexToRow[v]])
	// 		deep_update_active_indicator(w);
		
	// 	for( auto & x: all_active_common_neigbhour(v, w)){ 	// {v,w} may_not be present unless they are active. if they are active then can only be dominated by  {w,v} respectively.
	// 		if(domination_check (x,w) || domination_check(x,v))
	// 			activeIndicator[vertexToRow[x]] = false;
	// 	}
	// }
	
	// void update_active_indicator(const Vertex & v){  // checks for domination in the active neighborhood of the vertex v.
	// 	for(auto & x: active_neighbors(v))
	// 		if(domination_check(v,x)){  // Check in v is dominated by x
	// 			activeIndicator[vertexToRow[v]] = false;
	// 			return;
	// 		}
	// 	activeIndicator[vertexToRow[v]] = true;
	// 	return;	
	// }
	// void deep_update_active_indicator(const Vertex & v){
	// 	for(auto & x: non_active_neighbors(v)){
	// 		for(auto & y : active_neighbors(x)){
	// 			if(domination_check(x,y)){
	// 				activeIndicator[vertexToRow[x]] = true;	
	// 				deep_update_active_indicator(x);
	// 			}
	// 		}
	// 	}
	// 	return;		
	// }
    
    void contraction(const Vertex & del, const Vertex & keep){	
		if(del != keep){
			bool del_mem  = membership (del);
			bool keep_mem = membership(keep);
			if( del_mem && keep_mem)
			{
				doubleVector  del_indcs, keep_indcs, diff;
				auto row_del = vertexToRow[del];
				auto row_keep = vertexToRow[keep];
				del_indcs = read_row_index(row_del);
				keep_indcs = read_row_index(row_keep);
				std::set_difference(del_indcs.begin(), del_indcs.end(), keep_indcs.begin(), keep_indcs.end(), std::inserter(diff, diff.begin()));
				for (auto & v : diff) {
					if( v != row_del){
	        			sparseRowAdjMatrix.insert(row_keep,v) = 1;
	        			sparseRowAdjMatrix.insert(v, row_keep) = 1;
	        		}		
				}	
				vertexToRow.erase(del);
				vertices.erase(del);
				rowToVertex.erase(row_del); 
				//setZero(row_del->second, row_keep->second);
			}
			else if(del_mem && not keep_mem)
			{
				vertexToRow.insert(std::make_pair(keep, vertexToRow.find(del)->second));
				rowToVertex[vertexToRow.find(del)->second] = keep;
				vertices.emplace(keep);
				vertices.erase(del);
				vertexToRow.erase(del);

			}
			else
			{
				std::cerr << "The first vertex entered in the method contraction() doesn't exist in the skeleton." <<std::endl;	
				exit(-1); 	
			}
		}	
	}

     void relable(const Vertex & v, const Vertex & w){ // relable v as w.
     	if(membership(v) and v != w){
     		auto rw_v = vertexToRow[v];
      		rowToVertex[rw_v] = w;
      		vertexToRow.insert(std::make_pair(w, rw_v));	 
			vertices.emplace(w);
			vertexToRow.erase(v);
      		vertices.erase(v);
      		// std::cout<< "Re-named the vertex " << v << " as " << w << std::endl;
     	}
     } 

    //Returns the contracted edge. along with the contracted vertex in the begining of the list at {u,u} or {v,v}
	
    void active_strong_expansion(const Vertex & v, const Vertex & w, double filt_val){
		if(membership(v) && membership(w) && v!= w){
			// std::cout << "Strong expansion of the vertex " << v << " and " << w << " begins. " << std::endl;
			auto active_list_v_w = active_relative_neighbors(v,w);
			auto active_list_w_v = active_relative_neighbors(w,v);
			if(active_list_w_v.size() < active_list_v_w.size()){ // simulate the contraction of w by expanding the star of v
				for (auto & x : active_list_w_v){
					active_edge_insertion(v,x, filt_val);
					// std::cout << "Inserted the edge " << v << " , " << x  << std::endl;
				}
				swap_rows(v,w);
				// std::cout << "A swap of the vertex " << v << " and " << w << "took place." << std::endl;
			}
			else {  					
				for (auto & y : active_list_v_w){
					active_edge_insertion(w,y,filt_val);
					// std::cout << "Inserted the edge " << w << ", " << y  << std::endl;
				}
			}
			auto rw_v = vertexToRow.find(v);
			contractionIndicator[rw_v->second] = true;
		}
		if(membership(v) && !membership(w)){
			relable(v,w);
		}
	}
	void active_edge_insertion(const Vertex & v, const Vertex & w, double filt_val){
		insert_new_edges(v,w, filt_val);
		//update_active_indicator(v,w);
	}	

	void print_sparse_skeleton(){
		std::cout << sparseRowAdjMatrix << std::endl;
	}	

};