/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Vincent Rouvreau
 *
 *    Copyright (C) 2014  INRIA
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

#include <boost/program_options.hpp>
#include <boost/variant.hpp>

#include <gudhi/Simplex_tree.h>
#include <gudhi/Persistent_cohomology.h>
#include <gudhi/Points_3D_off_io.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_traits_3.h>
#include <CGAL/Periodic_3_Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>
#include <CGAL/iterator.h>

#include <fstream>
#include <cmath>
#include <string>
#include <tuple>
#include <map>
#include <utility>
#include <list>
#include <vector>
#include <cstdlib>

#include "alpha_complex_3d_helper.h"

// Traits
using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using PK = CGAL::Periodic_3_Delaunay_triangulation_traits_3<K>;
// Vertex type
using DsVb = CGAL::Periodic_3_triangulation_ds_vertex_base_3<>;
using Vb = CGAL::Triangulation_vertex_base_3<PK, DsVb>;
using AsVb = CGAL::Alpha_shape_vertex_base_3<PK, Vb>;
// Cell type
using DsCb = CGAL::Periodic_3_triangulation_ds_cell_base_3<>;
using Cb = CGAL::Triangulation_cell_base_3<PK, DsCb>;
using AsCb = CGAL::Alpha_shape_cell_base_3<PK, Cb>;
using Tds = CGAL::Triangulation_data_structure_3<AsVb, AsCb>;
using P3DT3 = CGAL::Periodic_3_Delaunay_triangulation_3<PK, Tds>;
using Alpha_shape_3 = CGAL::Alpha_shape_3<P3DT3>;
using Point_3 = PK::Point_3;

// filtration with alpha values needed type definition
using Alpha_value_type = Alpha_shape_3::FT;
using Object = CGAL::Object;
using Dispatch =
    CGAL::Dispatch_output_iterator<CGAL::cpp11::tuple<Object, Alpha_value_type>,
                                   CGAL::cpp11::tuple<std::back_insert_iterator<std::vector<Object> >,
                                                      std::back_insert_iterator<std::vector<Alpha_value_type> > > >;
using Cell_handle = Alpha_shape_3::Cell_handle;
using Facet = Alpha_shape_3::Facet;
using Edge_3 = Alpha_shape_3::Edge;
using Vertex_handle = Alpha_shape_3::Vertex_handle;
using Vertex_list = std::list<Alpha_shape_3::Vertex_handle>;

// gudhi type definition
using ST = Gudhi::Simplex_tree<Gudhi::Simplex_tree_options_fast_persistence>;
using Filtration_value = ST::Filtration_value;
using Simplex_tree_vertex = ST::Vertex_handle;
using Alpha_shape_simplex_tree_map = std::map<Alpha_shape_3::Vertex_handle, Simplex_tree_vertex>;
using Simplex_tree_vector_vertex = std::vector<Simplex_tree_vertex>;
using Persistent_cohomology =
    Gudhi::persistent_cohomology::Persistent_cohomology<ST, Gudhi::persistent_cohomology::Field_Zp>;

void program_options(int argc, char *argv[], std::string &off_file_points, std::string &cuboid_file,
                     std::string &output_file_diag, int &coeff_field_characteristic, Filtration_value &min_persistence);

int main(int argc, char **argv) {
  std::string off_file_points;
  std::string cuboid_file;
  std::string output_file_diag;
  int coeff_field_characteristic;
  Filtration_value min_persistence;

  program_options(argc, argv, off_file_points, cuboid_file, output_file_diag, coeff_field_characteristic,
                  min_persistence);

  // Read the OFF file (input file name given as parameter) and triangulate points
  Gudhi::Points_3D_off_reader<Point_3> off_reader(off_file_points);
  // Check the read operation was correct
  if (!off_reader.is_valid()) {
    std::cerr << "Unable to read OFF file " << off_file_points << std::endl;
    exit(-1);
  }

  // Read iso_cuboid_3 information from file
  std::ifstream iso_cuboid_str(cuboid_file);
  double x_min, y_min, z_min, x_max, y_max, z_max;
  if (iso_cuboid_str.good()) {
    iso_cuboid_str >> x_min >> y_min >> z_min >> x_max >> y_max >> z_max;
  } else {
    std::cerr << "Unable to read file " << cuboid_file << std::endl;
    exit(-1);
  }

  // Retrieve the triangulation
  std::vector<Point_3> lp = off_reader.get_point_cloud();

  // Define the periodic cube
  P3DT3 pdt(PK::Iso_cuboid_3(x_min, y_min, z_min, x_max, y_max, z_max));
  // Heuristic for inserting large point sets (if pts is reasonably large)
  pdt.insert(lp.begin(), lp.end(), true);
  // As pdt won't be modified anymore switch to 1-sheeted cover if possible
  if (pdt.is_triangulation_in_1_sheet()) pdt.convert_to_1_sheeted_covering();
  std::cout << "Periodic Delaunay computed." << std::endl;

  // alpha shape construction from points. CGAL has a strange behavior in REGULARIZED mode. This is the default mode
  // Maybe need to set it to GENERAL mode
  Alpha_shape_3 as(pdt, 0, Alpha_shape_3::GENERAL);

  // filtration with alpha values from alpha shape
  std::vector<Object> the_objects;
  std::vector<Alpha_value_type> the_alpha_values;

  Dispatch disp = CGAL::dispatch_output<Object, Alpha_value_type>(std::back_inserter(the_objects),
                                                                  std::back_inserter(the_alpha_values));

  as.filtration_with_alpha_values(disp);
#ifdef DEBUG_TRACES
  std::cout << "filtration_with_alpha_values returns : " << the_objects.size() << " objects" << std::endl;
#endif  // DEBUG_TRACES

  Alpha_shape_3::size_type count_vertices = 0;
  Alpha_shape_3::size_type count_edges = 0;
  Alpha_shape_3::size_type count_facets = 0;
  Alpha_shape_3::size_type count_cells = 0;

  // Loop on objects vector
  Vertex_list vertex_list;
  ST simplex_tree;
  Alpha_shape_simplex_tree_map map_cgal_simplex_tree;
  std::vector<Alpha_value_type>::iterator the_alpha_value_iterator = the_alpha_values.begin();
  Filtration_value filtration_max = 0.0;
  for (auto object_iterator : the_objects) {
    // Retrieve Alpha shape vertex list from object
    if (const Cell_handle* cell = CGAL::object_cast<Cell_handle>(&object_iterator)) {
      vertex_list = from_cell<Vertex_list, Cell_handle>(*cell);
      count_cells++;
    } else if (const Facet* facet = CGAL::object_cast<Facet>(&object_iterator)) {
      vertex_list = from_facet<Vertex_list, Facet>(*facet);
      count_facets++;
    } else if (const Edge_3* edge = CGAL::object_cast<Edge_3>(&object_iterator)) {
      vertex_list = from_edge<Vertex_list, Edge_3>(*edge);
      count_edges++;
    } else if (const Vertex_handle* vertex = CGAL::object_cast<Vertex_handle>(&object_iterator)) {
      count_vertices++;
      vertex_list = from_vertex<Vertex_list, Vertex_handle>(*vertex);
    }
    // Construction of the vector of simplex_tree vertex from list of alpha_shapes vertex
    Simplex_tree_vector_vertex the_simplex_tree;
    for (auto the_alpha_shape_vertex : vertex_list) {
      Alpha_shape_simplex_tree_map::iterator the_map_iterator = map_cgal_simplex_tree.find(the_alpha_shape_vertex);
      if (the_map_iterator == map_cgal_simplex_tree.end()) {
        // alpha shape not found
        Simplex_tree_vertex vertex = map_cgal_simplex_tree.size();
#ifdef DEBUG_TRACES
        std::cout << "vertex [" << the_alpha_shape_vertex->point() << "] not found - insert " << vertex << std::endl;
#endif  // DEBUG_TRACES
        the_simplex_tree.push_back(vertex);
        map_cgal_simplex_tree.emplace(the_alpha_shape_vertex, vertex);
      } else {
        // alpha shape found
        Simplex_tree_vertex vertex = the_map_iterator->second;
#ifdef DEBUG_TRACES
        std::cout << "vertex [" << the_alpha_shape_vertex->point() << "] found in " << vertex << std::endl;
#endif  // DEBUG_TRACES
        the_simplex_tree.push_back(vertex);
      }
    }
    // Construction of the simplex_tree
    Filtration_value filtr = /*std::sqrt*/ (*the_alpha_value_iterator);
#ifdef DEBUG_TRACES
    std::cout << "filtration = " << filtr << std::endl;
#endif  // DEBUG_TRACES
    if (filtr > filtration_max) {
      filtration_max = filtr;
    }
    simplex_tree.insert_simplex(the_simplex_tree, filtr);
    if (the_alpha_value_iterator != the_alpha_values.end())
      ++the_alpha_value_iterator;
    else
      std::cout << "This shall not happen" << std::endl;
  }

#ifdef DEBUG_TRACES
  std::cout << "vertices \t\t" << count_vertices << std::endl;
  std::cout << "edges \t\t" << count_edges << std::endl;
  std::cout << "facets \t\t" << count_facets << std::endl;
  std::cout << "cells \t\t" << count_cells << std::endl;

  std::cout << "Information of the Simplex Tree: " << std::endl;
  std::cout << "  Number of vertices = " << simplex_tree.num_vertices() << " ";
  std::cout << "  Number of simplices = " << simplex_tree.num_simplices() << std::endl << std::endl;
  std::cout << "  Dimension = " << simplex_tree.dimension() << " ";
#endif  // DEBUG_TRACES

#ifdef DEBUG_TRACES
  std::cout << "Iterator on vertices: " << std::endl;
  for (auto vertex : simplex_tree.complex_vertex_range()) {
    std::cout << vertex << " ";
  }
#endif  // DEBUG_TRACES

  // Sort the simplices in the order of the filtration
  simplex_tree.initialize_filtration();

  std::cout << "Simplex_tree dim: " << simplex_tree.dimension() << std::endl;
  // Compute the persistence diagram of the complex
  Persistent_cohomology pcoh(simplex_tree, true);
  // initializes the coefficient field for homology
  pcoh.init_coefficients(coeff_field_characteristic);

  pcoh.compute_persistent_cohomology(min_persistence);

  // Output the diagram in filediag
  if (output_file_diag.empty()) {
    pcoh.output_diagram();
  } else {
    std::cout << "Result in file: " << output_file_diag << std::endl;
    std::ofstream out(output_file_diag);
    pcoh.output_diagram(out);
    out.close();
  }

  return 0;
}

void program_options(int argc, char *argv[], std::string &off_file_points, std::string &cuboid_file,
                     std::string &output_file_diag, int &coeff_field_characteristic,
                     Filtration_value &min_persistence) {
  namespace po = boost::program_options;
  po::options_description hidden("Hidden options");
  hidden.add_options()("input-file", po::value<std::string>(&off_file_points),
                       "Name of file containing a point set. Format is one point per line:   X1 ... Xd ")(
      "cuboid-file", po::value<std::string>(&cuboid_file),
      "Name of file describing the periodic domain. Format is: min_hx min_hy min_hz\nmax_hx max_hy max_hz");

  po::options_description visible("Allowed options", 100);
  visible.add_options()("help,h", "produce help message")(
      "output-file,o", po::value<std::string>(&output_file_diag)->default_value(std::string()),
      "Name of file in which the persistence diagram is written. Default print in std::cout")(
      "field-charac,p", po::value<int>(&coeff_field_characteristic)->default_value(11),
      "Characteristic p of the coefficient field Z/pZ for computing homology.")(
      "min-persistence,m", po::value<Filtration_value>(&min_persistence),
      "Minimal lifetime of homology feature to be recorded. Default is 0. Enter a negative value to see zero length "
      "intervals");

  po::positional_options_description pos;
  pos.add("input-file", 1);
  pos.add("cuboid-file", 2);

  po::options_description all;
  all.add(visible).add(hidden);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(all).positional(pos).run(), vm);
  po::notify(vm);

  if (vm.count("help") || !vm.count("input-file") || !vm.count("cuboid-file")) {
    std::cout << std::endl;
    std::cout << "Compute the persistent homology with coefficient field Z/pZ \n";
    std::cout << "of a periodic 3D Alpha complex defined on a set of input points.\n \n";
    std::cout << "The output diagram contains one bar per line, written with the convention: \n";
    std::cout << "   p   dim b d \n";
    std::cout << "where dim is the dimension of the homological feature,\n";
    std::cout << "b and d are respectively the birth and death of the feature and \n";
    std::cout << "p is the characteristic of the field Z/pZ used for homology coefficients." << std::endl << std::endl;

    std::cout << "Usage: " << argv[0] << " [options] input-file cuboid-file" << std::endl << std::endl;
    std::cout << visible << std::endl;
    std::abort();
  }
}