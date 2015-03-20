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

#include <igatools/base/ig_function.h>
#include <igatools/base/function_element.h>

using std::shared_ptr;

IGA_NAMESPACE_OPEN

template<class Space>
IgFunction<Space>::
IgFunction(std::shared_ptr<Space> space,
           const CoeffType &coeff)
    :
    parent_t::Function(space->get_grid()),
    space_(space),
    coeff_(coeff),
    elem_(space->begin()),
    space_filler_(space->create_elem_handler())
{
    Assert(space_ != nullptr,ExcNullPtr());
    Assert(!coeff_.empty(),ExcEmptyObject());


    const auto dof_distribution = space_->get_dof_distribution();

#ifndef NDEBUG
    const auto n_dofs = dof_distribution->get_num_dofs(DofProperties::none);
    Assert(coeff.size() == n_dofs,ExcDimensionMismatch(coeff.size(),n_dofs));
#endif

    const auto &n_dofs_table = dof_distribution->get_num_dofs_table();
    int comp = 0;
    auto coeff_begin = coeff.cbegin();
    for (auto &coeff_new_comp : coeff_new_)
    {
        decltype(coeff_begin) coeff_end = coeff_begin + n_dofs_table[comp].flat_size();

        coeff_new_comp.resize(n_dofs_table[comp]);
        std::copy(coeff_begin, coeff_end, coeff_new_comp.begin());

        coeff_begin = coeff_end;
        ++comp;
    }


#if 0
    LogStream out;


    comp = 0;
    for (auto &coeff_new_comp : coeff_new_)
    {
        out.begin_item("Coeffs new[" + std::to_string(comp) + "]");
        coeff_new_comp.print_info(out);
        out.end_item();
        ++comp;
    }
    out.begin_item("Control points info (euclidean coordinates):");
    coeff_.print_info(out);
    out.end_item();
#endif
}


template<class Space>
IgFunction<Space>::
IgFunction(const self_t &fun)
    :
    parent_t::Function(fun.space_->get_grid()),
    space_(fun.space_),
    coeff_(fun.coeff_),
    coeff_new_(fun.coeff_new_),
    elem_(fun.space_->begin()),
    space_filler_(fun.space_->create_elem_handler())
{
    Assert(space_ != nullptr,ExcNullPtr());
    Assert(!coeff_.empty(),ExcEmptyObject());
}


template<class Space>
auto
IgFunction<Space>::
create(std::shared_ptr<Space> space,
       const CoeffType &coeff) ->  std::shared_ptr<self_t>
{
    auto ig_func = std::shared_ptr<self_t>(new self_t(space, coeff));

    Assert(ig_func != nullptr, ExcNullPtr());

    ig_func->create_connection_for_h_refinement(ig_func);

    return ig_func;
}



template<class Space>
void
IgFunction<Space>::
reset(const ValueFlags &flag, const eval_pts_variant &eval_pts)
{
    const std::set<int> elems_id =
        this->get_ig_space()->get_grid()->get_elements_id();

    this->reset_selected_elements(
        flag,
        eval_pts,
        vector<Index>(elems_id.begin(),elems_id.end()));
}


template<class Space>
void
IgFunction<Space>::
reset_selected_elements(
    const ValueFlags &flag,
    const eval_pts_variant &eval_pts,
    const vector<Index> &elements_flat_id)
{
    parent_t::reset(flag, eval_pts);
    reset_impl.flag = flag;
    reset_impl.space_handler_ = space_filler_.get();
    reset_impl.flags_ = &(this->flags_);
    reset_impl.elements_flat_id_ = &elements_flat_id;
    boost::apply_visitor(reset_impl, eval_pts);
}


template<class Space>
auto
IgFunction<Space>::
init_cache(ElementAccessor &elem, const topology_variant &k) -> void
{
    parent_t::init_cache(elem, k);
    init_cache_impl.space_handler_ = space_filler_.get();
    init_cache_impl.space_elem = &(*elem_);
    boost::apply_visitor(init_cache_impl, k);
}



template<class Space>
auto
IgFunction<Space>::
fill_cache(ElementAccessor &elem, const topology_variant &k, const int j) -> void
{
    parent_t::fill_cache(elem,k,j);

    elem_.move_to(elem.get_flat_index());

    fill_cache_impl.space_handler_ = space_filler_.get();
    fill_cache_impl.space_elem = &(*elem_);
    fill_cache_impl.func_elem = &elem;
    fill_cache_impl.function = this;


#if 0
    LogStream out;


    int comp = 0;
    for (auto &coeff_new_comp : coeff_new_)
    {
        out.begin_item("Coeffs new[" + std::to_string(comp) + "]");
        coeff_new_comp.print_info(out);
        out.end_item();
        ++comp;
    }
    out.begin_item("Control points info (euclidean coordinates):");
    coeff_.print_info(out);
    out.end_item();
#endif


    // TODO (pauletti, Nov 27, 2014): if code is in final state remove commented line else fix
    const auto elem_global_ids = elem_->get_local_to_global(DofProperties::none);
    vector<Real> loc_coeff;
#if 0
    for (const auto &id : elem_global_ids)
        loc_coeff.push_back(coeff_[id]);
    //    auto loc_coeff = coeff_.get_local_coefs(elem_->get_local_to_global());
#endif

    //-----------------------------------------------------
    const auto dof_distribution = space_->get_dof_distribution();
#if 0
    out.begin_item("Dof distribution");
    dof_distribution->print_info(out);
    out.end_item();
    out.begin_item("elem_global_ids");
    elem_global_ids.print_info(out);
    out.end_item();
#endif
    for (const auto &dof_global_id : elem_global_ids)
    {
        int comp;
        Index dof_id_in_comp;
        dof_distribution->global_to_comp_local(dof_global_id,comp,dof_id_in_comp);

//      out << "GID: " << dof_global_id << "    comp: " << comp << "    LID: " << dof_id_in_comp << std::endl;

        loc_coeff.push_back(coeff_new_[comp][dof_id_in_comp]);
    }
    //-----------------------------------------------------


    fill_cache_impl.loc_coeff = &loc_coeff;
    fill_cache_impl.j =j;

    boost::apply_visitor(fill_cache_impl, k);
}



template<class Space>
auto
IgFunction<Space>::
get_ig_space() const -> std::shared_ptr<const Space>
{
    return space_;
}



template<class Space>
auto
IgFunction<Space>::
get_coefficients() const -> const CoeffType &
{
    return coeff_;
}



template<class Space>
auto
IgFunction<Space>::
operator +=(const self_t &fun) -> self_t &
{
    Assert(coeff_.size() == fun.coeff_.size(),ExcDimensionMismatch(coeff_.size(),fun.coeff_.size()));
    /*
    const auto size = coeff_.size();
    for (int i=0; i<size; ++i)
        coeff_[i] += fun.coeff_[i];
    //*/

    // coeff_ += fun.coeff_
    std::transform(coeff_.begin(),coeff_.end(),
    fun.coeff_.begin(),
    coeff_.begin(),
    std::plus<int>());

    int comp = 0;
    for (auto &coeff_new_comp : coeff_new_)
    {
        Assert(coeff_new_comp.tensor_size() == fun.coeff_new_[comp].tensor_size(),
        ExcMessage("Different tensor sizes for the IgFunction coefficients."));

        // coeff_new_ += fun.coeff_new_
        std::transform(coeff_new_comp.begin(),coeff_new_comp.end(),
        fun.coeff_new_[comp].begin(),
        coeff_new_comp.begin(),
        std::plus<Real>());

        ++comp;
    }

    return *this;
}

template<class Space>
void
IgFunction<Space>::
refine_h_after_grid_refinement(
    const std::array<bool,dim> &refinement_directions,
    const CartesianGrid<dim> &grid_old)
{
    auto grid = this->get_grid();

    LogStream out;
    out.begin_item("Grid Old");
    grid_old.print_info(out);
    out.end_item();

    out.begin_item("Grid New");
    grid->print_info(out);
    out.end_item();

#if 0
    auto ref_space = data_->ref_space_;

    using bspline_space_t = BSplineSpace<RefSpace::dim,RefSpace::range,RefSpace::rank>;
    const bspline_space_t &bspline_space = get_bspline_space(*ref_space);

    auto knots_with_repetitions_pre_refinement = bspline_space.get_spline_space_previous_refinement()
                                                 ->compute_knots_with_repetition(
                                                     bspline_space.get_end_behaviour());
    auto knots_with_repetitions = bspline_space.compute_knots_with_repetition(
                                      bspline_space.get_end_behaviour());
#endif


#if 0
    for (int direction_id = 0 ; direction_id < dim ; ++direction_id)
    {
        if (refinement_directions[direction_id])
        {
            // knots in the refined grid along the selected direction
            vector<Real> knots_new = grid->get_knot_coordinates(direction_id);

            // knots in the original (unrefined) grid along the selected direction
            vector<Real> knots_old = grid_old->get_knot_coordinates(direction_id);

            vector<Real> knots_added(knots_new.size());

            // find the knots in the refined grid that are not present in the old grid
            auto it = std::set_difference(
                          knots_new.begin(),knots_new.end(),
                          knots_old.begin(),knots_old.end(),
                          knots_added.begin());

            knots_added.resize(it-knots_added.begin());


            for (int comp_id = 0 ; comp_id < space_dim ; ++comp_id)
            {
                const int p = ref_space->get_degree()[comp_id][direction_id];
                const auto &U = knots_with_repetitions_pre_refinement[comp_id].get_data_direction(direction_id);
                const auto &X = knots_added;
                const auto &Ubar = knots_with_repetitions[comp_id].get_data_direction(direction_id);

                const int m = U.size()-1;
                const int r = X.size()-1;
                const int a = space_tools::find_span(p,X[0],U);
                const int b = space_tools::find_span(p,X[r],U)+1;

                const int n = m-p-1;

                const auto &Pw = data_->ctrl_mesh_[comp_id];
                const auto old_sizes = Pw.tensor_size();
                Assert(old_sizes[direction_id] == n+1,
                       ExcDimensionMismatch(old_sizes[direction_id],n+1));


                auto new_sizes = old_sizes;
                new_sizes[direction_id] += r+1; // r+1 new weights in the refinement direction
                Assert(new_sizes[direction_id] ==
                       data_->ref_space_->get_num_basis(comp_id,direction_id),
                       ExcDimensionMismatch(new_sizes[direction_id],
                                            data_->ref_space_->get_num_basis(comp_id,direction_id)));

                DynamicMultiArray<Real,dim> Qw(new_sizes);

                for (Index j = 0 ; j <= a-p ; ++j)
                {
                    Qw.copy_slice(direction_id,j,
                                  Pw.get_slice(direction_id,j));
                }

                for (Index j = b-1 ; j <= n ; ++j)
                {
                    Qw.copy_slice(direction_id,j+r+1,
                                  Pw.get_slice(direction_id,j));
                }

                Index i = b + p - 1;
                Index k = b + p + r;
                for (Index j = r ; j >= 0 ; --j)
                {
                    while (X[j] <= U[i] && i > a)
                    {
                        Qw.copy_slice(direction_id,k-p-1,Pw.get_slice(direction_id,i-p-1));
                        k = k-1;
                        i = i-1;
                    }
                    Qw.copy_slice(direction_id,k-p-1,
                                  Qw.get_slice(direction_id,k-p));

                    for (Index l = 1 ; l <= p ; ++l)
                    {
                        Index ind = k-p+l;

                        Real alfa = Ubar[k+l] - X[j];
                        if (fabs(alfa) == 0.0)
                        {
                            Qw.copy_slice(direction_id,ind-1,Qw.get_slice(direction_id,ind));
                        }
                        else
                        {
                            alfa = alfa / (Ubar[k+l] - U[i-p+l]);

                            Qw.copy_slice(direction_id,ind-1,
                                          alfa  * Qw.get_slice(direction_id,ind-1) +
                                          (1.0-alfa) * Qw.get_slice(direction_id,ind));
                        }
                    } // end loop l
                    k = k-1;

                } // end loop j

                data_->ctrl_mesh_[comp_id] = Qw;
                //*/
            } // end loop comp_id
        } // end if (refinement_directions[direction_id])

    } // end loop direction_id



    //----------------------------------
    // copy the control mesh after the refinement
    data_->control_points_.resize(data_->ref_space_->get_num_basis());

    if (RefSpace::has_weights)
    {
#ifdef NURBS
        const auto weights_after_refinement = get_weights_from_ref_space(*(data_->ref_space_));

        Index ctrl_pt_id = 0;
        for (int comp_id = 0 ; comp_id < space_dim ; ++comp_id)
        {
            const auto &ctrl_mesh_comp = data_->ctrl_mesh_[comp_id];
            const auto &weights_after_refinement_comp = weights_after_refinement[comp_id];

            const Size n_dofs_comp = data_->ref_space_->get_num_basis(comp_id);
            for (Index loc_id = 0 ; loc_id < n_dofs_comp ; ++loc_id, ++ctrl_pt_id)
            {
                // if NURBS, transform the control points from  projective to euclidean coordinates
                const Real &w = weights_after_refinement_comp[loc_id];

                data_->control_points_[ctrl_pt_id] = ctrl_mesh_comp[loc_id] / w ;
            }
        }
#endif
    }
    else
    {
        Index ctrl_pt_id = 0;
        for (int comp_id = 0 ; comp_id < space_dim ; ++comp_id)
        {
            const auto &ctrl_mesh_comp = data_->ctrl_mesh_[comp_id];
            const Size n_dofs_comp = data_->ref_space_->get_num_basis(comp_id);
            for (Index loc_id = 0 ; loc_id < n_dofs_comp ; ++loc_id, ++ctrl_pt_id)
                data_->control_points_[ctrl_pt_id] = ctrl_mesh_comp[loc_id];
        }
    }
    //----------------------------------
#endif
    Assert(false,ExcNotImplemented());
    AssertThrow(false,ExcNotImplemented());
}


template<class Space>
void
IgFunction<Space>::
create_connection_for_h_refinement(std::shared_ptr<self_t> ig_function)
{
    using SlotType = typename CartesianGrid<dim>::SignalRefineSlot;

    auto refinement_func_igfunction =
        std::bind(&self_t::refine_h_after_grid_refinement,
                  ig_function.get(),
                  std::placeholders::_1,
                  std::placeholders::_2);
    this->functions_h_refinement_.connect_refinement_h_function(
        SlotType(refinement_func_igfunction).track_foreign(ig_function));
}



template<class Space>
void
IgFunction<Space>::
print_info(LogStream &out) const
{
    out.begin_item("Reference space info:");
    space_->print_info(out);
    out.end_item();
    out << std::endl;

#if 0
    out << "Control points info (projective coordinates):" << endl;

    //write the projective cooridnates if the reference space is NURBS
#endif

    out.begin_item("Control points info (euclidean coordinates):");
    coeff_.print_info(out);
    out.end_item();
}

IGA_NAMESPACE_CLOSE

#include <igatools/base/ig_function.inst>
