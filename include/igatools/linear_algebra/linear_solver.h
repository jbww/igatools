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

#ifndef LINEAR_SOLVER_H_
#define LINEAR_SOLVER_H_

#include <igatools/base/config.h>
#include <igatools/base/logstream.h>

#include <igatools/linear_algebra/distributed_matrix.h>

#include <BelosSolverManager.hpp>


IGA_NAMESPACE_OPEN


template < LinearAlgebraPackage linear_algebra_package>
class LinearSolver;




/**
 * Simple interface linear solver.
 *
 * It's basically a wrapper to a Trilinos::Belos solver.
 *
 * For an experiece user as well as for using advanced solver
 * features is recommended to interact directly with
 * the solvers offer by Trilinos.
 *
 * We provide this wrapper for simple problem for begginers
 * that have enough headaches learning igatools and postpone
 * learning Trilinos.
 *
 *
 * @todo (MM, Feb 2014) Re-design in order to be used with different linear algebra package
 * (Trilinos, PETSc,etc.)
 */
template<>
class LinearSolver<LinearAlgebraPackage::trilinos>
{
public:
    /**
     * Enum class used to specify the linear solver
     *
     * @note See the Trilinos::Belos package for their meaning.
     */
    enum class Type : int
    {
        GMRES = 0,
        FlexibleGMRES = 1,
        CG = 2,
        StochasticCG = 3,
        RecyclingCG = 4,
        RecyclingGMRES = 5,
        PseudoBlockGMRES = 6,
        PseudoBlockCG = 7,

        /** This entry tells how many elements we have in the current enum class */
        ENUM_SIZE = 8
    };

    /**
     * Constructor with a solver type.
     *
     * @param[in] solver_type Type of solver. Valid values are:
     * - SolverType::GMRES (default type)
     * - SolverType::FlexibleGMRES
     * - SolverType::CG
     * - SolverType::StochasticCG
     * - SolverType::RecyclingCG
     * - SolverType::RecyclingGMRES
     * - SolverType::PseudoBlockGMRES
     * - SolverType::PseudoBlockCG
     *
     * @param[in] tolerance The level that residual norms must reach to decide convergence.
     * Default is 1.0e-9.
     *
     * @param[in] max_num_iter The maximum number of iterations. Default is 1000.
     */
    LinearSolver(const Type solver_type = Type::GMRES,
                 const Real tolerance = 1.0e-9,
                 const int max_num_iter = 1000);

    /**
     * Solves the linear system.
     *
     * @note A and b should be const but Belos expects non const
     */
    void solve(Matrix &A, Vector &b, Vector &x);


    /** Return the tolerance achieved by the last solve() invocation. */
    Real get_achieved_tolerance() const ;

    /** Return Get the iteration count for the most recent call to solve(). */
    int get_num_iterations() const ;

    /**
     * Set the maximum number of iterations allowed.
     */
    void set_max_num_iterations(const int max_iter) ;

    /**
     * Set the level that residual norms must reach to decide convergence.
     */
    void set_tolerance(const Real tolerance) ;

    /**
     * Set the parameters for the solver.
     * @note The user is responsible to pass the parameter properly for the solver that is used.
     */
    void set_solver_parameters(Teuchos::RCP<Teuchos::ParameterList> solver_params);

private:
    using matrix_t = Tpetra::Operator<Real,Index,Index> ;
    using vector_t = Tpetra::MultiVector<Real,Index,Index> ;


    std::array<std::string,get_enum_size<Type>()> solver_type_enum_to_alias_ ;

    /** LinearSolver parameters */
    Teuchos::RCP<Teuchos::ParameterList> solver_params_;

    Teuchos::RCP<Belos::SolverManager<Real,vector_t,matrix_t> > solver_;
};



IGA_NAMESPACE_CLOSE


#endif /* LINEAR_SOLVER_H_ */
