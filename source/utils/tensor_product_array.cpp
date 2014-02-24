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

#include <igatools/utils/tensor_product_array.h>
#include <igatools/base/exceptions.h>


using std::array;
using std::vector;

IGA_NAMESPACE_OPEN


template<int rank>
TensorProductArray<rank>::
TensorProductArray(const TensorSize<rank> &size)
    :
    CartesianProductArray<Real,rank>(size)
{}

template<int rank>
TensorProductArray<rank>::
TensorProductArray(const Size size)
    :
    CartesianProductArray<Real,rank>(size)
{}



template<int rank>
void
TensorProductArray<rank>::
dilate_translate(const array<Real,rank> &dilate,
                 const Point<rank> &translate)
{
    this->dilate(dilate) ;
    this->translate(translate) ;
}

template<int rank>
void
TensorProductArray<rank>::
dilate(const array<Real,rank> &dilate)
{
    const TensorSize<rank> size = this->tensor_size();
    for (int i=0; i<rank; ++i)
        for (int j = 0 ; j < size(i) ; ++j)
            this->data_[i][j] *= dilate[i];
}


template<int rank>
void
TensorProductArray<rank>::
translate(const Point<rank> &translate)
{
    const TensorSize<rank> size = this->tensor_size();
    for (int i=0; i<rank; ++i)
        for (int j = 0 ; j < size(i) ; ++j)
            this->data_[i][j] += translate[i];
}


template<int rank>
std::vector< Real >
TensorProductArray<rank>::
get_flat_tensor_product() const
{
    const Size flat_size = this->flat_size();
    vector<Real> result(flat_size);
    for (Size i = 0; i < flat_size; ++i)
    {
        const auto tensor_id = this->flat_to_tensor(i);
        result[i] = this->tensor_product(tensor_id);
    }

    return result;
}



template<int rank>
Real
TensorProductArray<rank>::
tensor_product(const TensorIndex<rank> &tensor_id) const
{
    //TODO: maybe it is better to put an assertion if rank <= 0
    if (rank > 0)
    {
        Real result = this->data_[0][tensor_id[0]];
        for (int i = 1; i < rank; ++i)
            result *= this->data_[i][tensor_id[i]];
        return result;
    }
    else
        return 1.0;
}


template<int rank>
TensorProductArray<rank+1>
TensorProductArray<rank>::
insert(const int index, const std::vector<Real> &new_vector) const
{
    Assert(index<rank+1, ExcIndexRange(index,0,rank+1));

    TensorSize<rank+1> size;

    for (int i=0, j=0; i<rank+1; ++i)
    {
        if (i == index)
            size(i) = new_vector.size();
        else
        {
            size(i) = this->tensor_size()(j);
            ++j;
        }
    }

    TensorProductArray<rank+1> product(size);

    for (int i=0, j=0; i<rank+1; ++i)
    {
        if (i == index)
            product.copy_data_direction(i,new_vector);
        else
        {
            product.copy_data_direction(i,this->data_[j]);
            ++j;
        }
    }
    return product;
}




IGA_NAMESPACE_CLOSE

#ifndef NDEBUG
#include <igatools/utils/tensor_product_array-inline.h>
#endif


#include <igatools/utils/tensor_product_array.inst>

