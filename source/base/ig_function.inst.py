#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
#
# This file is part of the igatools library.
#
# The igatools library is free software: you can use it, redistribute
# it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#-+--------------------------------------------------------------------

from init_instantiation_data import *
include_files = [
                 'basis_functions/reference_element.h',
                 'basis_functions/bspline_space.h',
                 'basis_functions/bspline_element.h',
                 'basis_functions/bspline_element_handler.h',
                 'basis_functions/nurbs_space.h',
                 'basis_functions/nurbs_element.h',
                 'basis_functions/nurbs_element_handler.h',
                 'basis_functions/physical_space.h',
                 'basis_functions/physical_space_element.h',
                 'basis_functions/space_element_handler.h']
# include_files = ['basis_functions/bspline_space.h',
#                  'basis_functions/bspline_element_accessor.h',
#                  'basis_functions/bspline_uniform_quad_cache.h',
#                  'basis_functions/nurbs_space.h',
#                  'basis_functions/nurbs_element_accessor.h',
#                  'basis_functions/nurbs_uniform_quad_cache.h',
#                  'basis_functions/physical_space.h',
#                  'geometry/cartesian_grid_element.h',
#                  'geometry/mapping_element_accessor.h',
#                  'geometry/push_forward_element_accessor.h',
#                  'geometry/push_forward_uniform_quad_cache.h',
#                  'basis_functions/physical_space_element_accessor.h',
#                  'basis_functions/space_uniform_quad_cache.h']
#include_files = ['../../source/base/function_element.cpp',
#                 '../../source/geometry/cartesian_grid_iterator.cpp']

data = Instantiation(include_files)

(f, inst) = (data.file_output, data.inst)
for x in  inst.AllRefSpaces + inst.AllPhysSpaces: 
    s = ( 'template class IgFunction<%s> ;\n'   %(x.name) )
    f.write(s)
    
  
