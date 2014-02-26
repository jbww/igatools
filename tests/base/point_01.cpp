//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2014  by the igatools authors (see authors.txt).
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

/**
 * This file contains the test for the instantiations of the Point type.
 * \author Massimiliano Martinelli (massimiliano.martinelli@gmail.com)
 * \date 08/04/2013
 */

#include "../tests.h"

#include "igatools/base/tensor.h"

using std::endl ;

template< int dim >
void do_test()
{
    Point< dim > p0 ;

    out << p0 ;
    out << endl ;
}



int main(int argc, char *argv[])
{

    do_test<0>() ;
    do_test<1>() ;
    do_test<2>() ;
    do_test<3>() ;

    return (0) ;
}