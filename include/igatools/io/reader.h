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



#ifndef READER_H_
#define READER_H_

#include <igatools/base/config.h>
#include <igatools/geometry/mapping.h>
#include <igatools/basis_functions/nurbs_space.h>

#include <boost/property_tree/ptree.hpp>

#include <memory>



IGA_NAMESPACE_OPEN

//TODO: This function has to be documented
//TODO: Document the xml format

/**
 * Returns a string containing the format of the igatools XML input file.
 *
 * @ingroup input
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
std::string
get_xml_input_file_format(const std::string &filename);


/**
 * Reads an IgMapping from an xml file.
 *
 * @note The reference space for the IgMapping can be either BSplineSpace or NURBSSpace,
 * because this function returns a shared_ptr to the base class Mapping.
 *
 * @todo document the XML file formats (version 1.0 and 2.0) for IgMapping
 *
 * @ingroup input
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
template <int dim, int codim = 0>
std::shared_ptr< Mapping<dim,codim> >
get_mapping_from_file(const std::string &filename);


/**
 * Returns a CartesianGrid object (wrapped by a std::shared_ptr) from a Boost XML tree
 * containing exactly one node with the tag "CartesianGrid".
 * @note An assertion will be raised (in Debug and Release mode)
 * if no node or more than one node with the tag "CartesianGrid" are present in XML tree.
 * @ingroup input_v2
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
template <int dim>
std::shared_ptr< CartesianGrid<dim> >
get_cartesian_grid_from_xml(const boost::property_tree::ptree &tree);


/**
 * Returns a Mapping object (wrapped by a std::shared_ptr) from a Boost XML tree
 * containing exactly one node with the a tag decribing a supported mapping.
 * @warning The kind of instantiated mapping depends on the XML tag describing the mapping class.
 * Currently, only the IgMapping is supported (for which the specifying tag is "IgMapping").
 * @note An assertion will be raised (in Debug and Release mode)
 * if no node or more than one node with a tag with a supported mapping are present in XML tree.
 * @ingroup input_v2
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
template <int dim, int codim = 0>
std::shared_ptr< Mapping<dim,codim> >
get_mapping_from_xml(const boost::property_tree::ptree &tree);



/**
 * Returns a BSplineSpace object (wrapped by a std::shared_ptr) from a Boost XML tree
 * containing exactly one node with the tag "BSplineSpace".
 * @note An assertion will be raised (in Debug and Release mode)
 * if no node or more than one node with the tag "BSplineSpace" are present in XML tree.
 * @ingroup input_v2
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
template <int dim, int range, int rank>
std::shared_ptr< BSplineSpace<dim,range,rank> >
get_bspline_space_from_xml(const boost::property_tree::ptree &tree);


/**
 * Returns a NURBSSpace object (wrapped by a std::shared_ptr) from a Boost XML tree
 * containing exactly one node with the tag "NURBSSpace".
 * @note An assertion will be raised (in Debug and Release mode)
 * if no node or more than one node with the tag "NURBSSpace" are present in XML tree.
 * @ingroup input_v2
 * @author M. Martinelli
 * @date 04 Mar 2014
 */
template <int dim, int range, int rank>
std::shared_ptr< NURBSSpace<dim,range,rank> >
get_nurbs_space_from_xml(const boost::property_tree::ptree &tree);


IGA_NAMESPACE_CLOSE


#endif
