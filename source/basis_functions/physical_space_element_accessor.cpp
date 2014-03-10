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

#include <igatools/basis_functions/physical_space_element_accessor.h>
#include <igatools/base/exceptions.h>

using std::array;
using std::vector;
using std::shared_ptr;

IGA_NAMESPACE_OPEN

template< class PhysSpace >
PhysicalSpaceElementAccessor<PhysSpace>::
PhysicalSpaceElementAccessor(const PhysSpace &phys_space, const Index index)
    :
    RefElemAccessor(*phys_space.get_reference_space(), index),
    PfElemAccessor(*phys_space.push_forward_, index),
    phys_space_(&phys_space)
{
    Assert(phys_space_ != nullptr, ExcNullPtr());
}



template< class PhysSpace >
int
PhysicalSpaceElementAccessor<PhysSpace>::
get_flat_index() const
{
    return RefElemAccessor::get_flat_index();
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
ValuesCache::
reset(const int n_basis_per_element,
      const QuadratureType &quad,
      const ValueFlags fill_flag)
{
    n_points_ = quad.get_num_points();

    if (contains(fill_flag , ValueFlags::value))
    {
        if (D0phi_.get_num_points() != n_points_ ||
            D0phi_.get_num_functions() != n_basis_per_element)
            D0phi_.resize(n_basis_per_element,n_points_);

        D0phi_.zero();
        fill_values_ = true;
    }
    else
    {
        D0phi_.clear();
        fill_values_ = false;
    }

    if (contains(fill_flag , ValueFlags::gradient))
    {
        if (D1phi_.get_num_points() != n_points_ ||
            D1phi_.get_num_functions() != n_basis_per_element)
            D1phi_.resize(n_basis_per_element,n_points_);

        D1phi_.zero();
        fill_gradients_ = true;
    }
    else
    {
        D1phi_.clear();
        fill_gradients_ = false;
    }

    if (contains(fill_flag , ValueFlags::hessian))
    {
        if (D2phi_.get_num_points() != n_points_ ||
            D2phi_.get_num_functions() != n_basis_per_element)
            D2phi_.resize(n_basis_per_element,n_points_);

        D2phi_.zero();
        fill_hessians_ = true;
    }
    else
    {
        D2phi_.clear();
        fill_hessians_ = false;
    }

    this->set_initialized(true);
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
ElementValuesCache::
reset(const int n_basis_per_element,
      const QuadratureType &quad,
      const ValueFlags fill_flag)
{
    ValuesCache::reset(n_basis_per_element, quad, fill_flag);
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
FaceValuesCache::
reset(const Index face_id,
      const int n_basis_per_element,
      const QuadratureType &quad_elem,
      const ValueFlags fill_flag)
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    const auto quad_face = quad_elem.collapse_to_face(face_id);
    ValuesCache::reset(n_basis_per_element, quad_face, fill_flag);
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
FaceValuesCache::
reset(const Index face_id,
      const int n_basis_per_element,
      const QuadratureFaceType &quad,
      const ValueFlags fill_flag)
{
    AssertThrow(false,ExcNotImplemented());
}



template< class PhysSpace >
ValueFlags
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_flags(const ValueFlags fill_flag) const
{
    ValueFlags face_fill_flag = ValueFlags::none ;

    if (contains(fill_flag , ValueFlags::face_value))
        face_fill_flag |= ValueFlags::value ;

    if (contains(fill_flag , ValueFlags::face_gradient))
        face_fill_flag |= ValueFlags::gradient ;

    if (contains(fill_flag , ValueFlags::face_hessian))
        face_fill_flag |= ValueFlags::hessian ;

    return face_fill_flag ;
}



template< class PhysSpace >
ValueFlags
PhysicalSpaceElementAccessor<PhysSpace>::
get_reference_space_accessor_fill_flags(const ValueFlags fill_flag) const
{
    bool fill_values = false;
    bool fill_gradients = false;
    bool fill_hessians = false;
    bool fill_face_values = false;
    bool fill_face_gradients = false;
    bool fill_face_hessians = false;

    if (contains(fill_flag , ValueFlags::value))
        fill_values = true;

    if (contains(fill_flag , ValueFlags::gradient))
        fill_gradients = true;

    if (contains(fill_flag , ValueFlags::hessian))
        fill_hessians = true;

    if (contains(fill_flag , ValueFlags::face_value))
        fill_face_values = true;

    if (contains(fill_flag , ValueFlags::face_gradient))
        fill_face_gradients = true;

    if (contains(fill_flag , ValueFlags::face_hessian))
        fill_face_hessians = true;


    bool fill_D0_phi_hat = false;
    bool fill_D1_phi_hat = false;
    bool fill_D2_phi_hat = false;
    bool fill_face_D0_phi_hat = false;
    bool fill_face_D1_phi_hat = false;
    bool fill_face_D2_phi_hat = false;
    if (transformation_type == Transformation::h_grad)
    {
        fill_D0_phi_hat = fill_values;
        fill_D1_phi_hat = fill_gradients || fill_hessians;
        fill_D2_phi_hat = fill_hessians;
        fill_face_D0_phi_hat = fill_face_values;
        fill_face_D1_phi_hat = fill_face_gradients || fill_face_hessians;
        fill_face_D2_phi_hat = fill_face_hessians;
    }
    else if (transformation_type == Transformation::h_div  ||
             transformation_type == Transformation::h_curl ||
             transformation_type == Transformation::l_2)
    {
        fill_D0_phi_hat = fill_values || fill_gradients || fill_hessians;
        fill_D1_phi_hat = fill_gradients || fill_hessians;
        fill_D2_phi_hat = fill_hessians;
        fill_face_D0_phi_hat = fill_face_values || fill_face_gradients || fill_face_hessians;
        fill_face_D1_phi_hat = fill_face_gradients || fill_face_hessians;
        fill_face_D2_phi_hat = fill_face_hessians;
    }


    ValueFlags reference_space_accessor_fill_flags = ValueFlags::none;
    if (fill_D0_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::value;

    if (fill_D1_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::gradient;

    if (fill_D2_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::hessian;

    if (fill_face_D0_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::face_value;

    if (fill_face_D1_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::face_gradient;

    if (fill_face_D2_phi_hat)
        reference_space_accessor_fill_flags |= ValueFlags::face_hessian;


    return reference_space_accessor_fill_flags;
}



template< class PhysSpace >
ValueFlags
PhysicalSpaceElementAccessor<PhysSpace>::
get_push_forward_accessor_fill_flags(const ValueFlags fill_flag) const
{
    const ValueFlags common_flag =
        ValueFlags::point|ValueFlags::map_value|ValueFlags::map_gradient|ValueFlags::map_hessian|
        ValueFlags::w_measure|ValueFlags::face_point|ValueFlags::map_face_value|
        ValueFlags::map_face_gradient|ValueFlags::map_face_hessian|
        ValueFlags::face_w_measure;

    ValueFlags pf_flags = fill_flag & common_flag;

    if (contains(fill_flag , ValueFlags::value) || contains(fill_flag , ValueFlags::face_value))
        pf_flags |= ValueFlags::tran_value;

    if (contains(fill_flag , ValueFlags::gradient) || contains(fill_flag , ValueFlags::face_gradient))
        pf_flags |= ValueFlags::tran_gradient;

    if (contains(fill_flag , ValueFlags::hessian) || contains(fill_flag , ValueFlags::face_hessian))
        pf_flags |= ValueFlags::tran_hessian;

    return pf_flags;
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
init_values(const ValueFlags fill_flag,
            const QuadratureType &quad)
{
    const ValueFlags ref_sp_flag =
        get_reference_space_accessor_fill_flags(fill_flag);
    RefElemAccessor::init_values(ref_sp_flag, quad);

    const ValueFlags pf_flag = get_push_forward_accessor_fill_flags(fill_flag);
    PfElemAccessor::init_values(pf_flag, quad);


    const Size n_basis = phys_space_->get_reference_space()->get_num_basis_per_element();
    elem_values_.reset(n_basis, quad, fill_flag);

    Index face_id = 0 ;
    const auto face_fill_flag = get_face_flags(fill_flag) ;
    for (auto& face_value : face_values_)
        face_value.reset(face_id++, n_basis, quad, face_fill_flag);
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
init_face_values(const Index face_id,
                 const ValueFlags fill_flag,
                 const QuadratureFaceType &quad)
{
    AssertThrow(false,ExcNotImplemented());
}



template< class PhysSpace >
void PhysicalSpaceElementAccessor<PhysSpace>::
fill_values()
{
    Assert(elem_values_.is_initialized(), ExcNotInitialized());

    PfElemAccessor::fill_values();
    RefElemAccessor::fill_values();

    if (elem_values_.fill_values_)
    {
        PfElemAccessor::template transform_values<RefSpace::dim_range,RefSpace::rank>
        (RefElemAccessor::get_basis_values(), elem_values_.D0phi_);
    }

    if (elem_values_.fill_gradients_)
    {
        if (transformation_type == Transformation::h_grad)
        {
            ValueTable<typename RefElemAccessor::Value> dummy;
            PfElemAccessor::
            template transform_gradients<PhysSpace::dim_range,PhysSpace::rank>(
                dummy,
                RefElemAccessor::get_basis_gradients(),
                elem_values_.D1phi_);
        }
        else
        {
            PfElemAccessor::
            template transform_gradients<PhysSpace::dim_range,PhysSpace::rank>(
                RefElemAccessor::get_basis_values(),
                RefElemAccessor::get_basis_gradients(),
                elem_values_.D1phi_);
        }
    }

    elem_values_.set_filled(true);
}



template< class PhysSpace >
void PhysicalSpaceElementAccessor<PhysSpace>::
fill_face_values(const Index face_id)
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    auto &face_value = face_values_[face_id] ;
    Assert(face_value.is_initialized(), ExcNotInitialized());

    PfElemAccessor::fill_face_values(face_id);
    RefElemAccessor::fill_face_values(face_id);

    if (face_value.fill_values_)
    {
        PfElemAccessor::template transform_face_values<RefSpace::dim_range,RefSpace::rank>
        (face_id, RefElemAccessor::get_face_basis_values(face_id), face_value.D0phi_);
    }

    if (face_value.fill_gradients_)
    {
        if (transformation_type == Transformation::h_grad)
        {
            ValueTable<typename RefElemAccessor::Value> dummy;
            PfElemAccessor::
            template transform_face_gradients<PhysSpace::dim_range,PhysSpace::rank>(
                face_id,
                dummy,
                RefElemAccessor::get_face_basis_gradients(face_id),
                face_value.D1phi_);
        }
        else
        {
            PfElemAccessor::
            template transform_face_gradients<PhysSpace::dim_range,PhysSpace::rank>(
                face_id,
                RefElemAccessor::get_face_basis_values(face_id),
                RefElemAccessor::get_face_basis_gradients(face_id),
                face_value.D1phi_);
        }
    }

    face_value.set_filled(true);
}



template< class PhysSpace >
const ValueVector<Real> &
PhysicalSpaceElementAccessor<PhysSpace>::
get_w_measures() const
{
    return PfElemAccessor::get_w_measures();
}



template< class PhysSpace >
const ValueVector<Real> &
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_w_measures(const Index face_id) const
{
    return PfElemAccessor::get_face_w_measures(face_id);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_point(const Index qp) const -> const Point<space_dim> &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    return (PfElemAccessor::get_values_map())[qp];
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_field(const std::vector<Real> &local_coefs) const -> ValueVector< Value >
{
    Assert(this->get_num_basis() == local_coefs.size(),
    ExcDimensionMismatch(this->get_num_basis(), local_coefs.size()));

    auto field_hat = RefElemAccessor::evaluate_field(local_coefs);

    ValueVector< Value > field(field_hat.size());

    PfElemAccessor::template
    transform_values<PhysSpace::dim_range,PhysSpace::rank>(field_hat, field);

    return field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_field_gradients(const std::vector<Real> &local_coefs) const -> ValueVector< Derivative<1> >
{
    Assert(this->get_num_basis() == local_coefs.size(),
    ExcDimensionMismatch(this->get_num_basis(), local_coefs.size()));

    auto D1field_hat = RefElemAccessor::evaluate_field_gradients(local_coefs);

    const auto n_quad_points = D1field_hat.size();

    ValueVector< typename RefElemAccessor::Value > D0field_hat(n_quad_points);
    if (transformation_type != Transformation::h_grad)
        D0field_hat = RefElemAccessor::evaluate_field(local_coefs);

    ValueVector< Derivative<1> > D1field(n_quad_points);
    PfElemAccessor::
    template transform_gradients<PhysSpace::dim_range,PhysSpace::rank>(
        D0field_hat, D1field_hat, D1field);

    return D1field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_field_hessians(const std::vector<Real> &local_coefs) const -> ValueVector< Derivative<2> >
{
    AssertThrow(false,ExcNotImplemented());

    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    Assert(elem_values_.fill_hessians_ == true, ExcInvalidState());


    ValueVector< Derivative<2> > D2field;

    return D2field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_face_field(const Index face_id,
                    const std::vector<Real> &local_coefs) const -> ValueVector< Value >
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(this->get_num_basis() == local_coefs.size(),
    ExcDimensionMismatch(this->get_num_basis(), local_coefs.size()));

    auto field_hat = RefElemAccessor::evaluate_face_field(face_id, local_coefs);

    ValueVector< Value > field(field_hat.size());

    PfElemAccessor::template
    transform_face_values<PhysSpace::dim_range,PhysSpace::rank>(face_id, field_hat, field);

    return field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_face_field_gradients(const Index face_id,
                              const std::vector<Real> &local_coefs) const -> ValueVector< Derivative<1> >
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(this->get_num_basis() == local_coefs.size(),
    ExcDimensionMismatch(this->get_num_basis(), local_coefs.size()));

    auto D1field_hat = RefElemAccessor::evaluate_face_field_gradients(face_id, local_coefs);

    const auto n_quad_points = D1field_hat.size();

    ValueVector< typename RefElemAccessor::Value > D0field_hat(n_quad_points);
    if (transformation_type != Transformation::h_grad)
        D0field_hat = RefElemAccessor::evaluate_face_field(face_id, local_coefs);

    ValueVector< Derivative<1> > D1field(n_quad_points);
    PfElemAccessor::
    template transform_face_gradients<PhysSpace::dim_range,PhysSpace::rank>(
        face_id, D0field_hat, D1field_hat, D1field);

    return D1field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
evaluate_face_field_hessians(const Index face_id,
                             const std::vector<Real> &local_coefs) const -> ValueVector< Derivative<2> >
{
    AssertThrow(false,ExcNotImplemented());

    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    Assert(face_values_[face_id].fill_hessians_ == true, ExcInvalidState());


    ValueVector< Derivative<2> > D2field;

    return D2field;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_values() const -> ValueTable<Value> const &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    Assert(elem_values_.fill_values_,ExcInvalidState());
    return elem_values_.D0phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_values(const Index func) const -> typename ValueTable<Value>::const_view
{
    return this->get_basis_values().get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_value(const Index func, const Index qp) const -> const Value &
{
    Assert(qp >= 0 && qp < elem_values_.n_points_,
           ExcIndexRange(qp,0,elem_values_.n_points_));

    return this->get_basis_values(func)[qp];
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_gradients() const -> ValueTable< Derivative<1> > const &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    Assert(elem_values_.fill_gradients_,ExcInvalidState());
    return elem_values_.D1phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_gradients(const Index func) const -> typename ValueTable< Derivative<1> >::const_view
{
    return this->get_basis_gradients().get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_gradient(const Index func, const Index qp) const -> const Derivative<1> &
{
    Assert(qp >= 0 && qp < elem_values_.n_points_,
    ExcIndexRange(qp,0,elem_values_.n_points_));

    return this->get_basis_gradients(func)[qp];
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_hessians() const -> ValueTable< Derivative<2> > const &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    Assert(elem_values_.fill_hessians_,ExcInvalidState());
    return elem_values_.D2phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_hessians(const Index func) const -> typename ValueTable< Derivative<2> >::const_view
{
    return this->get_basis_hessians().get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_hessian(const Index func, const Index qp) const -> const Derivative<2> &
{
    Assert(qp >= 0 && qp < elem_values_.n_points_,
    ExcIndexRange(qp,0,elem_values_.n_points_));

    return this->get_basis_hessians(func)[qp];
}



template< class PhysSpace >
Real
PhysicalSpaceElementAccessor<PhysSpace>::
get_basis_divergence(const Index func, const Index qp) const
{
    return (trace(this->get_basis_gradient(func,qp)));
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_values(const Index face_id) const -> ValueTable<Value> const &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    Assert(face_values_[face_id].fill_values_,ExcInvalidState());
    return face_values_[face_id].D0phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_values(const Index face_id, const Index func) const -> typename ValueTable<Value>::const_view
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    return this->get_face_basis_values(face_id).get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_value(const Index face_id, const Index func, const Index qp) const -> const Value &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(qp >= 0 && qp < face_values_[face_id].n_points_,
           ExcIndexRange(qp,0,face_values_[face_id].n_points_));

    return this->get_face_basis_values(face_id, func)[qp];
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_gradients(const Index face_id) const -> ValueTable< Derivative<1> > const &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    Assert(face_values_[face_id].fill_gradients_,ExcInvalidState());
    return face_values_[face_id].D1phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_gradients(const Index face_id, const Index func) const -> typename ValueTable< Derivative<1> >::const_view
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    return this->get_face_basis_gradients(face_id).get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_gradient(const Index face_id, const Index func, const Index qp) const -> const Derivative<1> &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(qp >= 0 && qp < face_values_[face_id].n_points_,
    ExcIndexRange(qp,0,face_values_[face_id].n_points_));

    return this->get_face_basis_gradients(face_id, func)[qp];
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_hessians(const Index face_id) const -> ValueTable< Derivative<2> > const &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    Assert(face_values_[face_id].fill_hessians_,ExcInvalidState());
    return face_values_[face_id].D2phi_;
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_hessians(const Index face_id, const Index func) const -> typename ValueTable< Derivative<2> >::const_view
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    return this->get_face_basis_hessians(face_id).get_function_view(func);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_hessian(const Index face_id, const Index func, const Index qp) const -> const Derivative<2> &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(qp >= 0 && qp < face_values_[face_id].n_points_,
    ExcIndexRange(qp,0,face_values_[face_id].n_points_));

    return this->get_face_basis_hessians(face_id, func)[qp];
}



template< class PhysSpace >
Real
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_basis_divergence(const Index face_id, const Index func, const Index qp) const
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    return (trace(this->get_face_basis_gradient(face_id,func,qp)));
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_points() const ->
const ValueVector< typename Mapping<dim, codim>::ValueType > &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    return PfElemAccessor::get_values_map();
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_map_gradient_at_points() const ->
const ValueVector< typename Mapping<dim, codim>::GradientType > &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    return PfElemAccessor::get_gradients_map();
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_points(const Index face_id) const ->
const ValueVector< typename Mapping<dim, codim>::ValueType > &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    return PfElemAccessor::get_face_values_map(face_id);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_map_gradient_at_points(const Index face_id) const ->
const ValueVector< typename Mapping<dim, codim>::GradientFaceType > &
{
    Assert(elem_values_.is_filled(), ExcCacheNotFilled());
    return PfElemAccessor::get_face_gradients_map(face_id);
}



template< class PhysSpace >
auto
PhysicalSpaceElementAccessor<PhysSpace>::
get_face_normals(const Index face_id) const ->
const ValueVector< typename Mapping<dim, codim>::ValueType > &
{
    Assert(face_id < n_faces && face_id >= 0, ExcIndexRange(face_id,0,n_faces));
    Assert(face_values_[face_id].is_filled(), ExcCacheNotFilled());
    return PfElemAccessor::get_face_normals(face_id);
}



template< class PhysSpace >
bool
PhysicalSpaceElementAccessor<PhysSpace>::
is_boundary() const
{
    return PfElemAccessor::is_boundary();
}


template< class PhysSpace >
bool
PhysicalSpaceElementAccessor<PhysSpace>::
is_boundary(const Index face) const
{
    return PfElemAccessor::is_boundary(face);
}



template< class PhysSpace >
const PhysSpace *
PhysicalSpaceElementAccessor<PhysSpace>::
get_physical_space() const
{
    return phys_space_;
}



template< class PhysSpace >
void
PhysicalSpaceElementAccessor<PhysSpace>::
operator++()
{
    RefElemAccessor::operator++();
    PfElemAccessor::operator++();
}



template< class PhysSpace >
bool
PhysicalSpaceElementAccessor<PhysSpace>::
operator==(const PhysicalSpaceElementAccessor <PhysSpace> &a) const
{
    return RefElemAccessor::get_flat_index() ==
           static_cast<RefElemAccessor>(a).get_flat_index();
}



template< class PhysSpace >
bool
PhysicalSpaceElementAccessor<PhysSpace>::
operator!=(const PhysicalSpaceElementAccessor <PhysSpace> &a) const
{
    return RefElemAccessor::get_flat_index() !=
           static_cast<RefElemAccessor>(a).get_flat_index();
}

IGA_NAMESPACE_CLOSE

#include <igatools/basis_functions/physical_space_element_accessor.inst>