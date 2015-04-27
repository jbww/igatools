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

/**
 *  Common code to use in the phys_space_iterators_* tests
 *
 *   author: pauletti
 *   date: 2015-04-17
 *
 */

#include "../tests.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/base/function.h>

#include <igatools/basis_functions/bspline_space.h>

#include <igatools/basis_functions/physical_space.h>
#include <igatools/basis_functions/physical_space_element.h>
#include <igatools/basis_functions/phys_space_element_handler.h>

#include <igatools/basis_functions/space_tools.h>

using space_tools::get_boundary_dofs;

template <int dim, int range=1, int rank=1, int codim = 0>
shared_ptr<PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>>
create_space(shared_ptr<CartesianGrid<dim>> grid,
             shared_ptr<MapFunction<dim,dim+codim>> map_func,
             const int deg=1)
{
    using BspSpace = BSplineSpace<dim, range, rank>;
    using Space = PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>;
    auto ref_space = BspSpace::create(deg, grid);
    return Space::create(ref_space, map_func);
}


struct DofProp
{
    static const std::string interior;
    static const std::string dirichlet;
    static const std::string neumman;
};

const std::string DofProp::interior = "interior";
const std::string DofProp::dirichlet  = "dirichlet";
const std::string DofProp::neumman  = "neumman";

enum  bc : boundary_id
{
    dir=0, neu
};

template <int dim, int range=1, int rank=1, int codim = 0>
shared_ptr<PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>>
create_space_prop(shared_ptr<CartesianGrid<dim>> grid,
             shared_ptr<MapFunction<dim,dim+codim>> map_func,
             const int deg=1)
{
    const int neu_face = 0;
    grid->set_boundary_id(neu_face, bc::neu);

    using BspSpace = BSplineSpace<dim, range, rank>;
    using Space = PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>;
    auto ref_space = BspSpace::create(deg, grid);
    auto space = Space::create(ref_space, map_func);

    std::set<boundary_id>  dir_ids = {bc::dir};
    auto dir_dofs = get_boundary_dofs<Space>(space, dir_ids);


    auto int_dofs = space->get_interior_dofs();


    std::set<boundary_id>  neu_ids = {bc::neu};
    auto neu_dofs = get_boundary_dofs<Space>(space, neu_ids);
    std::vector<Index> common(dim*range);
    auto end1 =
            std::set_intersection(neu_dofs.begin(), neu_dofs.end(),
                                  dir_dofs.begin(), dir_dofs.end(), common.begin());
    common.resize(end1-common.begin());
    for (auto &id : common)
        neu_dofs.erase(id);

    auto dof_dist = space->get_dof_distribution();
    dof_dist->add_dofs_property(DofProp::interior);
    dof_dist->add_dofs_property(DofProp::dirichlet);
    dof_dist->add_dofs_property(DofProp::neumman);


    dof_dist->set_dof_property_status(DofProp::interior, int_dofs, true);
    dof_dist->set_dof_property_status(DofProp::dirichlet, dir_dofs, true);
    dof_dist->set_dof_property_status(DofProp::neumman, neu_dofs, true);

    return space;
}



template <int dim, int k, int range=1, int rank=1, int codim = 0>
void elem_values(shared_ptr<PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>> space,
                 const int n_qp = 1,
                 const string &prop = DofProperties::active,
                 const bool no_boundary=true)
{
    using Space = PhysicalSpace<dim,range,rank,codim, Transformation::h_grad>;
    using ElementHandler = typename Space::ElementHandler;

    auto quad = QGauss<k>(n_qp);
    auto flag = ValueFlags::value|ValueFlags::gradient |
                ValueFlags::hessian | ValueFlags::point |
                ValueFlags::w_measure;

    ElementHandler sp_values(space);
    sp_values.template reset<k> (flag, quad);

    auto elem = space->begin();
    auto end  = space->end();
    sp_values.template init_cache<k>(*elem);
    for (; elem != end; ++elem)
    {
        if ((no_boundary) || (elem->is_boundary()) )
        {
            out.begin_item("Element " + std::to_string(elem->get_flat_index()));
            for (auto &s_id : UnitElement<dim>::template elems_ids<k>())
            {
                if ((no_boundary) || (elem->is_boundary(s_id)))
                {
                    out.begin_item("Sub element id:  " + std::to_string(s_id));
                    sp_values.template fill_cache<k>(*elem, s_id);

                    out.begin_item("Values: ");
                    elem->template get_basis<_Value, k>(s_id,prop).print_info(out);
                    out.end_item();

                    out.begin_item("Gradients: ");
                    elem->template get_basis<_Gradient, k>(s_id,prop).print_info(out);
                    out.end_item();

                    out.begin_item("Hessians: ");
                    elem->template get_basis<_Hessian, k>(s_id,prop).print_info(out);
                    out.end_item();

                    out.begin_item("W * Measures: ");
                    elem->template get_w_measures<k>(s_id).print_info(out);
                    out.end_item();
                    out.end_item();
                }
            }
            out.end_item();
        }
    }


}