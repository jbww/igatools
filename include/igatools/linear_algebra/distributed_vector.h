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


#ifndef TRILINOS_VECTOR_H_
#define TRILINOS_VECTOR_H_

#include <igatools/base/config.h>
#include <igatools/base/logstream.h>

#include <igatools/linear_algebra/dense_vector.h>
//#include <igatools/linear_algebra/dof_tools.h>

#include <Tpetra_Vector.hpp>

#include <memory>


IGA_NAMESPACE_OPEN

/**
 * Numerical distributed Vector.
 * It's a wrapper to a Trilinos distributed vector.
 *
 * \author cavallini 2013
 * \author M. Martinelli 2013, 2014
 * \author pauletti 2013
 *
 * @todo (MM, Feb 2014) Re-design in order to be used with different linear algebra package
 * (Trilinos, PETSc,etc.)
 */
class Vector
{
public:

    /** @name Constructor and destructor */
    ///@{
    /**
     * Default constructor not allowed.
     */
    Vector() = delete;

    /**
     * Construct a vector that gets an consecutive indexing
     * for @p dof_ids degrees of freedom.
     * All entries are set to zero.
     */
    Vector(const Index size);

    /**
     * Construct a vector that gets an non consecutive indexing
     * for @p dof_ids degrees of freedom. An example could be the usage of
     * dof numbering provided from some external library.
     * All entries are set to zero.
     */
    Vector(const std::vector< Index > &dof_ids);

    /** Copy constructor. Performs a shallow copy of the object (i.e.)
     * only the smart pointers are copied, not the pointed objects. */
    Vector(const Vector &v) = default;

    /** Move constructor. */
    Vector(Vector &&v) = default;


    /** Destructor */
    ~Vector() = default;
    ///@}



    /** @name Assignment operators */
    ///@{

    /** Copy assignment operator */
    Vector &operator=(const Vector &v) = default;

    /** Move assignment operator */
    Vector &operator=(Vector &&v) = default;

    ///@}


    /**
     * @name Function for creating Vector objects
     * @note These methods implement the "create idiom" and return
     * a Vector object wrapped by a std::shared_ptr
     */
    ///@{

    /**
     * Create a vector that gets an consecutive indexing
     * for @p dof_ids degrees of freedom.
     * Initializing all entries to zero.
     */
    static std::shared_ptr<Vector> create(const Index size);

    /**
     * Create a vector that gets an non consecutive indexing
     * for @p dof_ids degrees of freedom. An example could be the usage of
     * dof numbering provided from some external library, IRIT as an example.
     * Initializing all entries to zero.
     */
    static std::shared_ptr<Vector> create(const std::vector<Index> &dof_ids);


    /**
     * Create a distributed vector with its index distribution
     * specified by the Space @p space.
     */
    template<class Space>
    static inline std::shared_ptr<Vector> create(const Space &space);
    ///@}



    /** @name Methods for getting and/or modifying the matrix entries */
    ///@{

    /**
     * Add the value @p input to the vector entry (i).
     * @note @p i is the global index of the entry (i).
     */
    void add_entry(const Index i, const Real input);


    /**
     * \brief This function add the @p local_vector values to the global vector.
     * The local-to-global ids are passed as input argument.
     * \param local_to_global The vector containing the global ids associated to the local vector entries
     * \param local_vector The local vector that must be added to the global vector.
     * \note The size of the local vector must be equal to the dimension of the
     * local_to_global vector, otherwise an exception will be raised.
     * \author M. Martinelli
     * \date 29 Jan 2013
     */
    void add_block(
        const std::vector< Index > &local_to_global,
        const DenseVector &local_vector);



    /**
     * Returns the const reference to the i-th entry of the vector.
     * @note @p i is the global index of the i-th entry.
     */
    const Real &operator()(const Index global_id) const;

    /**
     * Returns the reference to the i-th entry of the vector.
     * @note @p i is the global index of the i-th entry.
     */
    Real &operator()(const Index global_id);

    /** Returns the number of entries in the vector. */
    Index size() const;

    ///@}

    /**
     * Print the content of the vector, mostly for debug purposes.
     * @param out
     */
    void print(LogStream &out) const;



public:

    /** Type of Trilinos object wrapped by this class. */
    using WrappedVectorType = Tpetra::MultiVector<Real,Index,Index>;



    /** @name Methods for retrieving the Trilinos objects wrapped by this class */
    ///@{
    /**
     * Return the Trilinos RCP (Reference-Counted-Pointer, i.e. a smart pointer) wrapping the
     * concrete Trilinos distributed Vector. Const version.
     */
    Teuchos::RCP<const WrappedVectorType> get_trilinos_vector() const;

    /**
     * Return the Trilinos RCP (Reference-Counted-Pointer, i.e. a smart pointer) wrapping the
     * concrete Trilinos distributed Vector. Non-const version.
     */
    Teuchos::RCP<WrappedVectorType> get_trilinos_vector();
    ///@}

private:
    Teuchos::RCP<const Teuchos::Comm<int>> comm_ = Teuchos::createSerialComm<int>();

    /**
     * Smart pointer wrapping a Tpetra::Vector
     */
    Teuchos::RCP<WrappedVectorType> vector_;


    DeclException3(ExcVectorAccessToNonLocalElement,
                   Index, Index, Index,
                   << "You tried to access element (" << arg1 << ")"
                   << " of a distributed vector, but only rows "
                   << arg2 << " through " << arg2
                   << " are stored locally and can be accessed.");

};

IGA_NAMESPACE_CLOSE

#endif /* TRILINOS_VECTOR_H_ */