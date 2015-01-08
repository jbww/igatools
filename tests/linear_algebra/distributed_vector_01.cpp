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
 *  Test for Vector class
 *
 *  author: pauletti
 *  date: Apr 5, 2013
 *
 */

#include "../tests.h"

#include <igatools/linear_algebra/distributed_vector.h>


template <LAPack la_pack>
void run_test()
{
    using VectorType = Vector<la_pack>;

    VectorType v0(0);
    v0.print_info(out);
    out << endl;

    VectorType v1(10);
    v1.print_info(out);
    out << endl;

}

int main()
{
    out.depth_console(10);

#if defined(USE_TRILINOS)

    out.begin_item("Testing Trilinos/Tpetra vector:");
    run_test<LAPack::trilinos_tpetra>();
    out.end_item();

    out.begin_item("Testing Trilinos/Epetra vector:");
    run_test<LAPack::trilinos_epetra>();
    out.end_item();

#elif defined(USE_PETSC)

    out.begin_item("Testing Petsc vector:");
    run_test<LAPack::petsc>();
    out.end_item();

#endif

    return  0;
}
