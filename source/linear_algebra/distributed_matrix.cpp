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

#include <igatools/linear_algebra/distributed_matrix.h>
#include <igatools/base/exceptions.h>

#include <boost/numeric/ublas/matrix_proxy.hpp>


using std::vector;
using std::shared_ptr;

IGA_NAMESPACE_OPEN

Matrix::
Matrix(const SparsityPattern &sparsity_pattern)
{
    init(sparsity_pattern) ;
};


void Matrix::init(const SparsityPattern &sparsity_pattern)
{
    //-------------------------------------------------------------------------------------
    row_space_map_.reset(new dofs_map_t(sparsity_pattern.get_num_row_dofs(),
                                        sparsity_pattern.get_row_dofs(),
                                        0,comm_));

    column_space_map_.reset(new dofs_map_t(sparsity_pattern.get_num_col_dofs(),
                                           sparsity_pattern.get_col_dofs(),
                                           0,comm_));
    Teuchos::ArrayRCP<const long unsigned int> n_overlapping_funcs_per_row =
        Teuchos::arcp(
            Teuchos::RCP< const vector<long unsigned int> >(
                new vector<long unsigned int>(sparsity_pattern.get_num_overlapping_funcs()))) ;

    matrix_.reset(new WrappedMatrixType(row_space_map_,
                                        column_space_map_,
                                        n_overlapping_funcs_per_row));
    //-------------------------------------------------------------------------------------



    //-------------------------------------------------------------------------------------
    // allocating the entries (to 0.0) in the matrix corresponding to the sparsitiy pattern
    // (this step is required by the Tpetra matrix, in order to use sumIntoGlobalValues()
    auto row     = sparsity_pattern.cbegin() ;
    auto row_end = sparsity_pattern.cend() ;
    for (; row != row_end ; ++row)
    {
        const Index row_id = row->first ;

        const vector<Index> columns_id(row->second.begin(),row->second.end()) ;

        const auto num_columns = columns_id.size() ;

        vector<Real> values(num_columns, 0.0) ;

        matrix_->insertGlobalValues(row_id, columns_id, values) ;
    }
    //-------------------------------------------------------------------------------------
};
//*/


shared_ptr<Matrix>
Matrix::
create(const SparsityPattern &sparsity_pattern)
{
    return std::make_shared<Matrix>(Matrix(sparsity_pattern));
}

void Matrix::add_entry(const Index row_id, const Index column_id, const Real value)
{
    Teuchos::Array<Index> columns_id(1,column_id) ;
    Teuchos::Array<Real> values(1,value) ;

    matrix_->sumIntoGlobalValues(row_id,columns_id,values);
};



void
Matrix::
add_block(
    const vector<Index> &rows_id,
    const vector<Index> &cols_id,
    const DenseMatrix &local_matrix)
{
    Assert(!rows_id.empty(), ExcEmptyObject()) ;
    Assert(!cols_id.empty(), ExcEmptyObject()) ;

    const Index n_rows = rows_id.size();
    const Index n_cols = cols_id.size();

    Assert(n_rows == Index(local_matrix.size1()),
           ExcDimensionMismatch(n_rows,Index(local_matrix.size1()))) ;
    Assert(n_cols == Index(local_matrix.size2()),
           ExcDimensionMismatch(n_cols,Index(local_matrix.size2()))) ;

    vector<Real> row_values(n_cols) ;
    for (int i = 0 ; i < n_rows ; ++i)
    {
        const auto row_local_matrix = local_matrix.get_row(i) ;
        for (int j = 0 ; j < n_cols ; ++j)
            row_values[j] = row_local_matrix(j) ;

        matrix_->sumIntoGlobalValues(rows_id[i],cols_id,row_values);
    }
};



void
Matrix::
fill_complete()
{
    matrix_->fillComplete();
};

void
Matrix::
resume_fill()
{
    matrix_->resumeFill();
};


auto
Matrix::
get_trilinos_matrix() -> Teuchos::RCP<WrappedMatrixType>
{
    return matrix_;
};

auto
Matrix::
get_trilinos_matrix() const -> Teuchos::RCP<const WrappedMatrixType>
{
    return matrix_;
};


Real
Matrix::
operator()(const Index row, const Index col) const
{
    const auto graph = matrix_->getGraph();

    const auto row_map = graph->getRowMap();
    const auto col_map = graph->getColMap();

    const auto local_row = row_map->getLocalElement(row);
    const auto local_col = col_map->getLocalElement(col);
//    std::cout << "Global=("<<row<<","<<col<<")   Local=("<<local_row<<","<<local_col<<")"<<std::endl;

    // If the data is not on the present processor, we throw an exception.
    // This is one of the two tiny differences to the el(i,j) call,
    //which does not throw any assertions, but returns 0.0.
    Assert(local_row != Teuchos::OrdinalTraits<Index>::invalid(),
           ExcAccessToNonLocalElement(row,col,
                                      row_map->getMinGlobalIndex(),
                                      row_map->getMaxGlobalIndex()+1));

    Teuchos::ArrayView<const Index> local_col_ids ;
    Teuchos::ArrayView<const Real> values ;
    matrix_->getLocalRowView(local_row,local_col_ids,values) ;

//    std::cout << local_col_ids <<std::endl ;
//    std::cout << "local_col=" << local_col << std::endl;
    // Search the index where we look for the value, and then finally get it.
    const Index *col_find = std::find(local_col_ids.begin(),local_col_ids.end(),local_col);


    // This is actually the only difference to the el(i,j) function,
    // which means that we throw an exception in this case instead of just
    // returning zero for an element that is not present in the sparsity pattern.
    Assert(col_find != local_col_ids.end(), ExcInvalidIndex(row,col));

    const Index id_find = static_cast<Index>(col_find-local_col_ids.begin());
//    std::cout << "id_find="<<id_find<<std::endl;

    return values[id_find];
}


void Matrix::clear_row(const Index row)
{
    const auto graph = matrix_->getGraph();

    const auto row_map = graph->getRowMap();
    const auto local_row = row_map->getLocalElement(row);

    // Only do this on the rows owned locally on this processor.
    if (local_row != Teuchos::OrdinalTraits<Index>::invalid())
    {
        auto n_entries_row = graph->getNumEntriesInLocalRow(local_row);

        vector<Index> local_col_ids(n_entries_row);

        graph->getLocalRowCopy(local_row,local_col_ids,n_entries_row);
        Assert(n_entries_row == graph->getNumEntriesInLocalRow(local_row),
               ExcDimensionMismatch(n_entries_row,graph->getNumEntriesInLocalRow(local_row)));

        vector<Real> zeros(n_entries_row,0.0);
        matrix_->replaceLocalValues(local_row,local_col_ids,zeros);
    }
}



void Matrix::print(LogStream &out) const
{
    using std::endl;

    const auto n_global_rows = this->get_num_rows();

    out << "-----------------------------" << endl;
    // Commented as different trilinos version show different information here
    //   out << *matrix_ ;

    out << "Row Index        Col Index        Value" << endl;
    for (Index row_id = 0 ; row_id < n_global_rows ; ++row_id)
    {
        auto n_entries_row = matrix_->getNumEntriesInGlobalRow(row_id);

        vector<Index> columns_id(n_entries_row);

        vector<Real> values(n_entries_row) ;

        matrix_->getGlobalRowCopy(row_id,columns_id,values,n_entries_row);

        for (uint row_entry = 0 ; row_entry < n_entries_row ; ++row_entry)
            out << row_id << "       " <<
                columns_id[row_entry] << "        " <<
                values[row_entry] << endl;
    }
    out << "-----------------------------" << endl;

}





auto
Matrix::
get_num_rows() const -> Index
{
    return matrix_->getGlobalNumRows() ;
}

auto
Matrix::
get_num_columns() const -> Index
{
    return matrix_->getGlobalNumCols() ;
}


void
Matrix::
multiply_by_right_vector(const Vector &x,Vector &y,const Real alpha,const Real beta) const
{
    matrix_->apply(*x.get_trilinos_vector(),
                   *y.get_trilinos_vector(),
                   Teuchos::NO_TRANS,alpha,beta);
}

void
Matrix::
multiply_by_left_vector(const Vector &x,Vector &y,const Real alpha,const Real beta) const
{
    matrix_->apply(*x.get_trilinos_vector(),
                   *y.get_trilinos_vector(),
                   Teuchos::TRANS,alpha,beta);
}


IGA_NAMESPACE_CLOSE


