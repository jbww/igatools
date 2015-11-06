//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
//
// This file is part of the igatools library.
//
// The igatools library is free software: you can use it, redistribute
// it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-+--------------------------------------------------------------------


/*
 *  Test for the ig mapping class iterator, geometrical quantities using a BSpline mapping
 *  author: antolin
 *  date: 2014-04-23
 *
 */

#include "../tests.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/functions/ig_grid_function.h>
#include <igatools/basis_functions/bspline_element.h>
#include <igatools/io/reader.h>

template <int dim>
void run_test(std::string &file_name)
{
  OUTSTART

  // Reading input file.
  auto map = dynamic_pointer_cast<IgGridFunction<dim,dim> >(get_mapping_from_file<dim,0>(file_name));
  out.begin_item("IgGridFunction infos:");
  map->print_info(out);
  out << endl;

  auto quad = QTrapez<dim>::create();
  const auto quad_pts = quad->get_points();
  out.begin_item("Quad pts.:");
  quad_pts.print_info(out);
  out.end_item();
  out << endl;


  const auto basis = map->get_basis();

  //------------------------------------------------------
  out.begin_item("Loop using the BSplineElement");
  auto sp_elem_handler = basis->create_cache_handler();
  sp_elem_handler->set_element_flags(space_element::Flags::value);

  auto sp_elem     = basis->begin();
  auto sp_elem_end = basis->end();

  sp_elem_handler->init_element_cache(sp_elem,quad);
  int elem_id = 0;
  for (; sp_elem != sp_elem_end; ++sp_elem,++elem_id)
  {
    sp_elem_handler->fill_element_cache(sp_elem);
    out << "Element: " << elem_id << endl;

    out << "Element id: " << sp_elem->get_index() << endl;

    const auto &values = sp_elem->get_element_values();
    out << "Values = ";
    values.print_info(out);
    out<< endl;
  }
  out.end_item();
  out << endl;
  //------------------------------------------------------


  //------------------------------------------------------
  out.begin_item("Loop using the GridFunctionElement");
  auto map_handler = map->create_cache_handler();
  map_handler->set_element_flags(grid_function_element::Flags::D0);

  auto map_elem     = map->begin();
  auto map_elem_end = map->end();

  map_handler->init_element_cache(map_elem,quad);

  elem_id = 0;
  for (; map_elem != map_elem_end; ++map_elem, ++elem_id)
  {
    map_handler->fill_element_cache(map_elem);

    out << "Element: " << elem_id << endl;
    out << "Element id: " << map_elem->get_index() << endl;

    const auto &points = map_elem->get_element_values_D0();
    int qp = 0;
    for (auto p : points)
      out << "    Point " << ++qp << ": " << p << endl;
  }
  out.end_item();
  out << endl;
  //------------------------------------------------------
  OUTEND
}


int main()
{

  std::string file_name = "cube_1D.xml";
  run_test<1>(file_name);

  file_name = "cube_2D.xml";
  run_test<2>(file_name);
  file_name = "cube_3D.xml";
  run_test<3>(file_name);
//*/
  return (0) ;
}
