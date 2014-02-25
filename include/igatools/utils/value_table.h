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

#ifndef VALUE_TABLE_H_
#define __VALUE_TABLE_H_

#include <igatools/base/config.h>
#include <igatools/base/logstream.h>
#include <igatools/utils/value_vector.h>
#include <igatools/utils/dynamic_multi_array.h>
#include <igatools/utils/container_view.h>

#include <vector>

IGA_NAMESPACE_OPEN



/**
 * @class ValueTable
 * @brief This class represents a 2-dimensional dynamic array for objects of type T.
 *
 * Each entry of the 2-dimensional array are associated to a single function
 * at one single point.
 *
 * Internally the data are stored as single std::vector<T>
 * (through the inherithance from DynamicMultiArray<T,2>) of length
 * <tt num_functions * num_points</tt> and the memory is ordered is made as
 * <tt>num_functions</tt> chunks of length <tt>num_points</tt> objects of type @p T.
 * The array element with index @p i refers to the function index resulting by the
 * integer division <tt>i/num_points</tt> and to the point index resulting by
 * the modulo operation <tt>i%num_points</tt>: in other words,
 * the point-index runs faster than the function-index.
 *
 * The container can be iterated with the iterator ValueTable::iterator
 * or its const version ValueTable::const_iterator.
 * Moreover the class provides ``views'' (in const and non-const version),
 * i.e. special iterators that operates on entries that
 * can be grouped in accordance with some criteria.
 * We provide two types of ``views'' (i.e. two grouping criteria):
 * - <b>function view</b>: i.e. iterators that runs on entries related to the same function.
 *   For this kind of view, the number of entries are equal to the number of points.
 * - <b>point view</b>: i.e. iterators that runs on entries related to the same point
 *   For this kind of view, the number of entries are equal to the number of functions.
 *
 * The types associated with the views are ValueTable<T>::view and ValueTable<T>::const_view
 *
 *
 * @tparam T Type of the object to be stored in the table.
 * \todo Missing documentation.
 * @author M.Martinelli
 * @date 2013,2014
 */
template <class T>
class ValueTable :
    public DynamicMultiArray<T,2>
{
public :
    /** Type for the iterator (non-const version). */
    using iterator = typename DynamicMultiArray<T,2>::iterator ;

    /** Type for the iterator (const version). */
    using const_iterator = typename DynamicMultiArray<T,2>::const_iterator;

    /** Type for the view (non-const version). */
    using view = ContainerView<DynamicMultiArray<T,2>>;


    /** Type for the view (const version). */
    using const_view = ConstContainerView<DynamicMultiArray<T,2>>;


    /**
     * @name Constructors
     */
    ///@{
    /**
     * Default constructor.
     * Constructs an empty container, with no elements.
     */
    explicit ValueTable() ;

    /**
     * Constructor. Constructs a container for storing num_functions*num_points objects of type T.
     * @param[in] num_functions - Number of functions.
     * @param[in] num_points - Number of points.
     */
    explicit ValueTable(const Size num_functions, const Size num_points) ;

    /**
     * Copy constructor. Performs a deep copy of the ValueTable object @p table_in.
     * @warning Use this operator with caution because if the ValueTable to be copied is big,
     * this results in an CPU expensive operation.
     */
    ValueTable(const ValueTable<T> &table_in) = default ;

    /**
     * Move constructor.
     */
    ValueTable(ValueTable<T> &&table_in) = default ;


    /**
     * Destructor.
     */
    ~ValueTable() = default ;

    ///@}


    /**
     * @name Assignment operators
     */
    ///@{

    /**
     * Copy assignment operator. Performs a deep copy of the ValueTable object @p table_in.
     * @warning Use this operator with caution because if the ValueTable to be copied is big,
     * this results in an CPU expensive operation.
     */
    ValueTable<T> &operator=(const ValueTable<T> &table_in) = default ;


    /**
     * Move assignment operator.
     */
    ValueTable<T> &operator=(ValueTable<T> &&table_in) = default ;

    ///@}


    /**
     * @name Values initialization
     */
    ///@{

    /** Set all the values of the table to zero. */
    void zero() ;

    ///@}


    /**
     * @name Functions for resizing
     */
    ///@{

    /**
     * Resize the ValueTable in order to allocate space for @p num_functions functions and
     * @p num_points points.
     */
    void resize(const Size num_functions, const Size num_points) ;

    /**
     * Removes all elements from the ValueTable, leaving the container with a size of 0.
     */
    void clear() noexcept ;
    ///@}


    /**
     * @name Functions for getting size information
     */
    ///@{
    /**
     * Returns the number of elements in the ValueTable (= num_functions * num_points).
     */
    Size size() const;

    /**
     * Returns the number of points.
     */
    Size get_num_points() const noexcept ;

    /**
     * Returns the number of functions.
     */
    Size get_num_functions() const noexcept ;
    ///@}


    /**
     * @name Functions for getting views
     */
    ///@{
    /**
     * Return a view of the elements relative to the <tt>i</tt>-th function. Non-const version.
     * @note In Debug mode, the function index @p i is checked if it is in the correct
     * functions-index range.
     */
    view get_function_view(const int i);

    /**
     * Return a view of the elements relative to the <tt>i</tt>-th function. Const version.
     * @note In Debug mode, the function index @p i is checked if it is in the correct
     * functions-index range.
     */
    const_view get_function_view(const int i) const;

    /**
     * Return a view of the elements relative to the <tt>i</tt>-th point. Non-const version.
     * @note In Debug mode, the function index @p i is checked if it is in the correct
     * points-index range.
     */
    view get_point_view(const int i);

    /**
     * Return a view of the elements relative to the <tt>i</tt>-th point. Const version.
     * @note In Debug mode, the function index @p i is checked if it is in the correct
     * points-index range.
     */
    const_view get_point_view(const int i) const;
    ///@}


    /**
     * @name Printing info
     */
    ///@{

    /**
     * Prints the content of the ValueTable on the LogStream @p out.
     */
    void print_info(LogStream &out) const ;
    ///@}


    /**
     * Returns the linear combination of the function values (at each evaluation points).
     * The size of vector of the @p coefficients must be equal to the number of functions
     * represented in the ValueTable.
     */
    ValueVector<T> evaluate_linear_combination(const std::vector<Real> &coefficients) const ;


private:

    /**
     * Number of functions for which the objects in the ValueTable refers to.
     */
    Size num_functions_ ;


    /**
     * Number of points for which the objects in the ValueTable refers to.
     */
    Size num_points_ ;
} ;


IGA_NAMESPACE_CLOSE


#endif /* VALUE_TABLE_H_ */