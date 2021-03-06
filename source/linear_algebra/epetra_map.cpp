//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2016  by the igatools authors (see authors.txt).
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

#include <igatools/linear_algebra/epetra_map.h>

IGA_NAMESPACE_OPEN

#ifdef IGATOOLS_USES_TRILINOS

namespace EpetraTools
{

MapPtr
create_map(const std::set<Index> &dofs,
           const Comm &comm)
{
  SafeSTLVector<Index> dofs_vec(dofs.begin(), dofs.end());
  return std::make_shared<Map>(-1, dofs_vec.size(), dofs_vec.data(), 0, comm);
}

}

#endif //IGATOOLS_USES_TRILINOS

IGA_NAMESPACE_CLOSE
