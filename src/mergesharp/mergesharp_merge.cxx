/// \file mergesharp_merge.cxx
/// Merge cubes containing sharp vertices.

/*
Copyright (C) 2012-2015 Arindam Bhattacharya and Rephael Wenger

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
(LGPL) as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <vector>
#include <algorithm>
#include <set>

#include "ijkcoord.txx"
#include "ijkgraph.txx"
#include "ijkmesh.txx"
#include "ijkgrid_macros.h"

#include "mergesharp_apply.txx"
#include "sharpiso_array.txx"

#include "mergesharp_types.h"
#include "mergesharp_datastruct.h"
#include "mergesharp_merge.h"
#include "mergesharp_extract.h"
#include "mergesharp_check_map.h"

#include "mergesharp_debug.h"

#define _USE_MATH_DEFINES
#include <math.h>

// *** DEBUG ***
#include "ijkprint.txx"

// forward declarations
namespace {

  typedef SHARPISO::FIXED_ARRAY<NUM_CUBE_NEIGHBORS3D, VERTEX_INDEX, NUM_TYPE>
  CUBE_CONNECTED_ARRAY;

	void determine_gcube_map
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   MERGESHARP::ISOVERT & isovert, 
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
   MERGESHARP::SHARPISO_INFO & sharpiso_info);

	void map_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void unmap_non_disk_isopatches
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
   MERGESHARP::SHARPISO_INFO & sharpiso_info);

	void determine_gcube_map_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   MERGESHARP::ISOVERT & isovert, 
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
   MERGESHARP::SHARPISO_INFO & sharpiso_info);

	void map_adjacent_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void unmap_non_disk_isopatches
		(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
		const SCALAR_TYPE isovalue,
		MERGESHARP::ISOVERT & isovert, 
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
		MERGESHARP::SHARPISO_INFO & sharpiso_info);

	void map_gcube_indices(const std::vector<VERTEX_INDEX> & gcube_map,
		std::vector<VERTEX_INDEX> & gcube_index);

  void store_map(const std::vector<VERTEX_INDEX> & gcube_map,
                 MERGESHARP::ISOVERT & isovert);

	void set_first_gcube_isov
		(const MERGESHARP::ISOVERT & isovert, 
		const std::vector<MERGESHARP::DUAL_ISOVERT> & iso_vlist,
		SHARPISO::NUM_TYPE * first_gcube_isov);

	void insert_vertex_list
		(const std::vector<VERTEX_INDEX> & vlist,
		MERGESHARP::VERTEX_HASH_TABLE & vertex_hash);

	void remap_vertex_list
    (const MERGESHARP::VERTEX_HASH_TABLE & vertex_hash,
		const std::vector<VERTEX_INDEX> & vlist,
		std::vector<VERTEX_INDEX> & new_vlist);

	void extend_mapping
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mappingII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mappingIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_to_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_to_cubeII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_to_cubeIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  void extend_map_facet_adjacent_pairs
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  void extend_map_edge_adjacent_pairs
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  void extend_map_triples
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_near_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX extend_from_cube,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_corner_cube_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_mapping_corner_cube_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   const MERGESHARP::MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_map_adjacent_pairs_covered
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_map_adjacent_pairs_covered
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_map_around_edges
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	void extend_map_around_edges
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	bool are_connected_by_iso_edge
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube_index1,
   const VERTEX_INDEX & cube_index2,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	bool are_connected_by_iso_quad
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube0_index,
   const VERTEX_INDEX & cube1_index,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended);

	void find_connected_sharp(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX cube0_index,
		const MERGESHARP::ISOVERT & isovert, 
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    CUBE_CONNECTED_ARRAY & connected_sharp);

	void add2list_connected_sharp(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX cube0_index,
		const MERGESHARP::ISOVERT & isovert, 
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    CUBE_CONNECTED_ARRAY & connected_sharp);

	bool is_cube_merge_permitted(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube_index,
		const INDEX_DIFF_TYPE to_gcube_index,
    const INDEX_DIFF_TYPE gcubeC_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

	bool is_corner_cube_merge_permitted(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube_index,
		const INDEX_DIFF_TYPE to_gcube_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  bool does_region_contain_cube
  (const MERGESHARP::GRID_BOX & region, const VERTEX_INDEX cube_index,
   const MERGESHARP::ISOVERT & isovert);

  bool does_region_contain_cube
  (const MERGESHARP::GRID_BOX & region, const GRID_COORD_TYPE cube_coord[DIM3],
   bool & is_region_vertex);

  void insert_selected_in_bin_grid
  (const SHARPISO_GRID & grid, const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const AXIS_SIZE_TYPE bin_width, BIN_GRID<VERTEX_INDEX> & bin_grid);

	bool check_connected_sharp
  ( const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const MERGESHARP::ISOVERT & isovert, 
    const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    const VERTEX_INDEX from_vertex,
    const VERTEX_INDEX to_vertex,
    const CUBE_CONNECTED_ARRAY & connected_sharp);

  void get_adjacent_unmapped_in_5x5x5_region
  (const VERTEX_INDEX icubeA, const VERTEX_INDEX icubeB,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   std::vector<VERTEX_INDEX> & cube_list);
}

// **************************************************
// Merge isosurface vertices:
//   Single vertex per cube
// **************************************************

// Merge isosurface vertices in cubes adjacent to selected sharp cubes.
void MERGESHARP::merge_sharp_iso_vertices
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
	const SCALAR_TYPE isovalue,
	ISOVERT & isovert,
	const MERGE_PARAM & merge_param,
	std::vector<VERTEX_INDEX> & isoquad_cube,
	std::vector<VERTEX_INDEX> & gcube_map, SHARPISO_INFO & sharpiso_info)
{
	const NUM_TYPE num_gcube = isovert.gcube_list.size();

	determine_gcube_map
		(scalar_grid, isovalue, isovert, merge_param, 
		gcube_map, sharpiso_info);

	// Count number merged isosurface vertices.
	NUM_TYPE num_merged = 0;
	for (int i = 0; i < num_gcube; i++) {
		if (gcube_map[i] != i) { num_merged++; }
	}
	sharpiso_info.num_merged_iso_vertices = num_merged;

	map_gcube_indices(gcube_map, isoquad_cube);

  store_map(gcube_map, isovert);
}

// Merge isosurface vertices in cubes adjacent to selected sharp cubes.
void MERGESHARP::merge_sharp_iso_vertices
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, const SCALAR_TYPE isovalue,
	ISOVERT & isovert, 
	const MERGE_PARAM & merge_param,
	std::vector<VERTEX_INDEX> & isoquad_cube,
	SHARPISO_INFO & sharpiso_info)
{
	const NUM_TYPE num_gcube = isovert.gcube_list.size();
	std::vector<VERTEX_INDEX> gcube_map(num_gcube);

	merge_sharp_iso_vertices
		(scalar_grid, isovalue, isovert, merge_param,
		isoquad_cube, gcube_map, sharpiso_info);
}

namespace {

	using namespace MERGESHARP;

	void determine_gcube_map
		(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		MERGESHARP::ISOVERT & isovert, 
		const MERGE_PARAM & merge_param,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
		MERGESHARP::SHARPISO_INFO & sharpiso_info)
	{
    std::vector<NUM_TYPE> selected_gcube_list;

		get_selected_cubes(isovert.gcube_list, selected_gcube_list);

		map_adjacent_cubes(scalar_grid, isovalue, isovert, gcube_map);

		if (merge_param.flag_map_extended){
			extend_mapping_corner_cube
        (scalar_grid, isovalue, merge_param, isovert, gcube_map);

			extend_mapping
        (scalar_grid, isovalue, selected_gcube_list, isovert, gcube_map);
		}

		if (merge_param.flag_check_disk) {
			unmap_non_disk_isopatches
				(scalar_grid, isovalue, isovert, gcube_map, sharpiso_info);
		}
	}
}


// **************************************************
// Merge isosurface vertices:
//   Allow multiple vertices per cube
// **************************************************

// Merge isosurface vertices in cubes adjacent to selected sharp cubes.
// Allows multiple isosurface vertices per cube.
void MERGESHARP::merge_sharp_iso_vertices_multi
(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
 const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
 const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
 const SCALAR_TYPE isovalue,
 const std::vector<DUAL_ISOVERT> & iso_vlist,
 ISOVERT & isovert,
 const MERGE_PARAM & merge_param,
 std::vector<VERTEX_INDEX> & poly_vert,
 std::vector<VERTEX_INDEX> & gcube_map, SHARPISO_INFO & sharpiso_info)
{
	const NUM_TYPE num_gcube = isovert.gcube_list.size();
	IJK::ARRAY<NUM_TYPE> first_gcube_isov(num_gcube);

	determine_gcube_map_multi
		(scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
     merge_param, gcube_map, sharpiso_info);

	// Count number merged isosurface vertices.
	NUM_TYPE num_merged = 0;
	for (int i = 0; i < num_gcube; i++) {
		if (gcube_map[i] != i) { num_merged++; }
	}
	sharpiso_info.num_merged_iso_vertices = num_merged;

	set_first_gcube_isov(isovert, iso_vlist, first_gcube_isov.Ptr());

	for (NUM_TYPE i = 0; i < poly_vert.size(); i++) {
		NUM_TYPE k = poly_vert[i];
		VERTEX_INDEX cube_index0 = iso_vlist[k].cube_index;
		NUM_TYPE gcube_index0 = isovert.GCubeIndex(cube_index0);
		VERTEX_INDEX gcube_index1 = gcube_map[gcube_index0];
		if (isovert.gcube_list[gcube_index1].flag == SELECTED_GCUBE) {
			// Reset poly_vert[i] to first isosurface vertex for cube_index1.
			poly_vert[i] = first_gcube_isov[gcube_index1];
		}
	}

  store_map(gcube_map, isovert);
}

// Merge isosurface vertices in cubes adjacent to selected sharp cubes.
// Allows multiple isosurface vertices per cube.
void MERGESHARP::merge_sharp_iso_vertices_multi
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
	const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
	const SCALAR_TYPE isovalue,
	const std::vector<DUAL_ISOVERT> & iso_vlist,
	ISOVERT & isovert,
  const MERGE_PARAM & merge_param,
	std::vector<VERTEX_INDEX> & poly_vert,
	SHARPISO_INFO & sharpiso_info)
{
	const NUM_TYPE num_gcube = isovert.gcube_list.size();
	std::vector<VERTEX_INDEX> gcube_map(num_gcube);

	merge_sharp_iso_vertices_multi
		(scalar_grid, isodual_table, ambig_info, isovalue, iso_vlist, isovert, 
		merge_param, poly_vert, gcube_map, sharpiso_info);
}

namespace {

	using namespace MERGESHARP;

	void determine_gcube_map_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
   MERGESHARP::SHARPISO_INFO & sharpiso_info)
	{
    MSDEBUG();
    flag_debug = false;

		map_adjacent_cubes_multi
      (scalar_grid, isodual_table, ambig_info, isovalue,
       merge_param, isovert, gcube_map);

		if (merge_param.flag_check_disk) {
			unmap_non_disk_isopatches
				(scalar_grid, isodual_table, isovalue, isovert, gcube_map, 
				sharpiso_info);
		}
	}

}

// **************************************************
// Map isosurface vertices
//   Single vertex per cube
// **************************************************

namespace {

	using namespace MERGESHARP;

  // forward declarations
	bool is_cube_edge_neighbor_connected_by_iso_edge
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube0_index,
   const NUM_TYPE k,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  bool is_unselected_cube_connected_to
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX & cube0_index,
   const VERTEX_INDEX & to_cube,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  void check_and_map_isov
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);

  bool is_cube_between
  (const ISOVERT & isovert, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3],
   const CUBE_CONNECTED_ARRAY cube_list);


	void map_iso_vertex
  (const GRID_CUBE_DATA_ARRAY & gcube_list,
   const INDEX_DIFF_TYPE from_gcube, const INDEX_DIFF_TYPE to_gcube,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		if (from_gcube != ISOVERT::NO_INDEX) {
      if (gcube_list[from_gcube].flag != SELECTED_GCUBE && 
          gcube_map[from_gcube] == from_gcube) {

        if (gcube_list[from_gcube].boundary_bits == 0) {
          // Map gcube_list[from_gcube] to isosurface vertex in cube to_gcube.
          gcube_map[from_gcube] = to_gcube;
        }
			}
		}
	}

  // Map cubes and boundary cubes.
  // Map boundary cubes if isosurface patch in cube does not 
  //   intersect the grid boundary.
	void map_iso_vertex
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const GRID_CUBE_DATA_ARRAY & gcube_list,
   const INDEX_DIFF_TYPE from_gcube,
   const INDEX_DIFF_TYPE to_gcube,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    BOUNDARY_BITS_TYPE boundary_bits;

		if (from_gcube != ISOVERT::NO_INDEX) {

			if (gcube_list[from_gcube].flag != SELECTED_GCUBE && 
          gcube_map[from_gcube] == from_gcube) {

        boundary_bits = gcube_list[from_gcube].boundary_bits;


        MSDEBUG();
        if (flag_debug) {
          using namespace std;
          VERTEX_INDEX to_cube = gcube_list[to_gcube].cube_index;
          VERTEX_INDEX from_cube = gcube_list[from_gcube].cube_index;
          cerr << "***Mapping " << from_cube << " ";
          ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
          cerr << " to " << to_cube << " ";
          ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
          cerr << endl;
        }

        if (boundary_bits == 0) {
          // Map gcube_list[from_gcube] to isosurface vertex in cube to_gcube.
          gcube_map[from_gcube] = to_gcube;
        }
        else {

          VERTEX_INDEX from_cube = gcube_list[from_gcube].cube_index;

          if (!is_gt_boundary_facet_min_le_facet_max
              (scalar_grid, from_cube, boundary_bits, isovalue)) {
            // Map gcube_list[from_gcube] to isosurface vertex in cube to_gcube.
            gcube_map[from_gcube] = to_gcube;
          }
        }
			}
		}
	}


  // Map cubes which are facet adjacent to cube to_cube.
	void map_facet_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    BOUNDARY_BITS_TYPE boundary_bits;

    if (to_gcube == ISOVERT::NO_INDEX) { return; }
    boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      // Map facet neighbors to to_cube.
      if (boundary_bits == 0) {
        // Cube cube_index is an interior cube.

        for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

          VERTEX_INDEX from_cube = isovert.grid.CubeNeighborF(to_cube, j);

          check_and_map_isov
            (scalar_grid, isovalue, from_cube, to_cube, isovert, gcube_map);
        }
      }
      else {
        // Boundary case.

        for (int d = 0; d < isovert.grid.Dimension(); d++) {
          for (int j = 0; j < 2; j++) {
            BOUNDARY_BITS_TYPE mask = (BOUNDARY_BITS_TYPE(1) << (2*d+j));

            if ((mask & boundary_bits) == 0) {
              VERTEX_INDEX from_cube =
                isovert.grid.AdjacentVertex(to_cube, d, j);

              check_and_map_isov
                (scalar_grid, isovalue, from_cube, to_cube, 
                 isovert, gcube_map);

            }

          }
        }

      }
    }
  }

  // Map cubes which are edge adjacent to cube to_cube.
	void map_edge_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);

    if (to_gcube == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      int boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

      if (boundary_bits == 0) {

        // Cube cube_index is an interior cube.
        //find the neighbors
        for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) {

          if (is_cube_edge_neighbor_connected_by_iso_edge
              (scalar_grid, to_cube, j, isovalue, isovert, gcube_map)) {

            VERTEX_INDEX from_cube = isovert.grid.CubeNeighborE(to_cube, j);

            check_and_map_isov
              (scalar_grid, isovalue, from_cube, to_cube, isovert, gcube_map);
          }
        }
      }
      else {
        // Handle boundary case.
      }
    }
  }

  // Map cubes which are vertex adjacent to cube to_cube.
	void map_vertex_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);

    if (to_gcube == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      int boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

      if (boundary_bits == 0) {

        // Cube cube_index is an interior cube.
        //find the neighbors
        for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsV(); j++) {

          VERTEX_INDEX from_cube = isovert.grid.CubeNeighborV(to_cube, j);

          check_and_map_isov
            (scalar_grid, isovalue, from_cube, to_cube, isovert, gcube_map);
        }
      }
      else {
        // Handle boundary case.
      }
    }
  }

	// Map isosurface vertices adjacent to isosurface in selected cubes
  //   with given number of eigenvalues.
  // @param Map to cubes with the given number of eigenvalues.
  // Handles SOME boundary cases.
	void map_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const int num_eigenvalues,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		std::vector<NUM_TYPE> sharp_gcube_list;
		using namespace MERGESHARP;

		const NUM_TYPE num_gcube = isovert.gcube_list.size();
		VERTEX_INDEX cube_index, neighbor_index;

		get_corner_or_edge_cubes(isovert.gcube_list, sharp_gcube_list);

		// Map cubes which share facets with selected cubes.
		for (NUM_TYPE i = 0; i < sharp_gcube_list.size(); i++) {

			NUM_TYPE to_gcube = sharp_gcube_list[i];

      if (isovert.gcube_list[to_gcube].num_eigenvalues == num_eigenvalues) {
        VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
        map_facet_adjacent_cubes
          (scalar_grid, isovalue, isovert, to_cube, gcube_map);
      }
		}

		// Set cubes which share edges with selected cubes.
		for (NUM_TYPE i = 0; i < sharp_gcube_list.size(); i++) {

			NUM_TYPE to_gcube = sharp_gcube_list[i];
      if (isovert.gcube_list[to_gcube].num_eigenvalues == num_eigenvalues) {
        VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
        map_edge_adjacent_cubes
          (scalar_grid, isovalue, isovert, to_cube, gcube_map);
      }
		}

		for (NUM_TYPE i = 0; i < sharp_gcube_list.size(); i++) {

			NUM_TYPE to_gcube = sharp_gcube_list[i];
      if (isovert.gcube_list[to_gcube].num_eigenvalues == num_eigenvalues) {
        VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
        map_vertex_adjacent_cubes
          (scalar_grid, isovalue, isovert, to_cube, gcube_map);
      }
		}

	}

	// Map isosurface vertices adjacent to selected cubes
	//  to the isosurface vertex in the selected cube.
  // Handles SOME boundary cases.
	void map_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		std::vector<NUM_TYPE> sharp_gcube_list;
		using namespace MERGESHARP;

		const NUM_TYPE num_gcube = isovert.gcube_list.size();
		VERTEX_INDEX cube_index, neighbor_index;

    // Initialize
		for (NUM_TYPE i = 0; i < num_gcube; i++)
		{ gcube_map[i] = i; }

    // Map to corner cubes.
    map_adjacent_cubes
      (scalar_grid, isovalue, isovert, 3, gcube_map);

    // Map to edge cubes.
    map_adjacent_cubes
      (scalar_grid, isovalue, isovert, 2, gcube_map);
	}

	void map_gcube_indices(const std::vector<VERTEX_INDEX> & gcube_map,
		std::vector<VERTEX_INDEX> & gcube_index)
	{
		for (VERTEX_INDEX i = 0; i < gcube_index.size(); i++) {
			VERTEX_INDEX gcube_index_i = gcube_index[i];
			gcube_index[i] = gcube_map[gcube_index_i];
		}
	}

	// Map cube_list[i] to target cube determined by gcube_map.
	void map_cube_list(const ISOVERT & isovert,
		const std::vector<VERTEX_INDEX> & gcube_map,
		std::vector<VERTEX_INDEX> & cube_list)
	{
		for (VERTEX_INDEX i = 0; i < cube_list.size(); i++) {
			VERTEX_INDEX cube_index0 = cube_list[i];
			VERTEX_INDEX gcube_index0 = isovert.GCubeIndex(cube_index0);
			VERTEX_INDEX gcube_index1 = gcube_map[gcube_index0];
			cube_list[i] = isovert.gcube_list[gcube_index1].cube_index;
		}
	}

  // Store map in GRID_CUBE_DATA
  void store_map(const std::vector<VERTEX_INDEX> & gcube_map,
                 MERGESHARP::ISOVERT & isovert)
  {
    IJK::PROCEDURE_ERROR error("store_map");

    for (NUM_TYPE i = 0; i < gcube_map.size(); i++) {
      INDEX_DIFF_TYPE to_gcube = gcube_map[i];
      if (to_gcube != ISOVERT::NO_INDEX) {
        isovert.gcube_list[i].maps_to_cube = isovert.CubeIndex(to_gcube);
      }
      else {
        error.AddMessage
          ("Programming error.  Illegal value for gcube_map[", i, "].");
        error.AddMessage(" gcube_map[", i, "] = ", gcube_map[i], ".");
        throw error;
      }
    }
  }

	// If cube0 containing isosurface vertex isov_index[i] is merged 
	//   with some other cube cube1, then map isov_index[i] 
	//   to the isosurface vertex in cube1.
	void map_isov_indices(const ISOVERT & isovert,
		const std::vector<VERTEX_INDEX> & gcube_map,
		const std::vector<DUAL_ISOVERT> & iso_vlist,
		std::vector<ISO_VERTEX_INDEX> & isov_index)
	{
		VERTEX_HASH_TABLE cube_hash;

		for (NUM_TYPE i = 0; i < iso_vlist.size(); i++) {
			VERTEX_INDEX cube_index = iso_vlist[i].cube_index;

			VERTEX_HASH_TABLE::iterator cube_iter = cube_hash.find(cube_index);
			if (cube_iter == cube_hash.end()) {
				cube_hash.insert(VERTEX_HASH_TABLE::value_type(cube_index, i));
			}
		}

		for (NUM_TYPE i = 0; i < isov_index.size(); i++) {
			ISO_VERTEX_INDEX j0 = isov_index[i];
			VERTEX_INDEX cube_index0 = iso_vlist[j0].cube_index;
			VERTEX_INDEX gcube_index0 = isovert.GCubeIndex(cube_index0);
			VERTEX_INDEX gcube_index1 = gcube_map[gcube_index0];
			if (gcube_index1 != gcube_index0 ||
				isovert.gcube_list[gcube_index1].flag == SELECTED_GCUBE) {
					VERTEX_INDEX cube_index1 = isovert.gcube_list[gcube_index1].cube_index;
					VERTEX_HASH_TABLE::iterator cube1_iter = cube_hash.find(cube_index1);
					isov_index[i] = cube1_iter->second;
			}
		}
	}


  /// Determine if isosurface quad dual to grid edge (iend0, edge_dir)
  ///   maps to cube0 or cube1.
  /// @param iend0 Lower endpoint of edge.
  /// @param edge_dir Edge direction.
  /// @pre Edge (iend0, edge_dir) is an internal edge.
  /// @pre Edge (iend0, edge_dir) is bipolar.
  void determine_quad_map_to_cubes
  (const VERTEX_INDEX iend0, const int edge_dir,
   const VERTEX_INDEX cube0_index,
   const VERTEX_INDEX cube1_index,
   const ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_maps_to_cube0,
   bool & flag_maps_to_cube1)
  {
    const NUM_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);
    const NUM_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);

    // Initialize
    flag_maps_to_cube0 = false;
    flag_maps_to_cube1 = false;

    int d1 = (edge_dir+1)%DIM3;
    int d2 = (edge_dir+2)%DIM3;

    VERTEX_INDEX iv0 = 
      iend0 - isovert.grid.AxisIncrement(d1) - isovert.grid.AxisIncrement(d2);
    for (int j = 0; j < NUM_VERT_PER_QUAD; j++) {
      VERTEX_INDEX cube2_index = isovert.grid.FacetVertex(iv0, edge_dir, j);
      NUM_TYPE gcube2_index = isovert.GCubeIndex(cube2_index);

      NUM_TYPE to_gcube = gcube_map[gcube2_index];
      if (to_gcube == gcube0_index) { flag_maps_to_cube0 = true; }
      else if (to_gcube == gcube1_index) { flag_maps_to_cube1 = true; }
    }
  }

  /// Determine if some isosurface quad dual to an edge of cube0
  ///   maps to both cube1 and cube2.
  void determine_quad_map_to_both_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0_index,
   const VERTEX_INDEX cube1_index,
   const VERTEX_INDEX cube2_index,
   const ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_maps_to_cube1,
   bool & flag_maps_to_both_cubes)
  {
    const INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);

    flag_maps_to_cube1 = false;
    flag_maps_to_both_cubes = false;

    if (gcube0_index == ISOVERT::NO_INDEX) { return; }

    BOUNDARY_BITS_TYPE boundary_bits = 
      isovert.gcube_list[gcube0_index].boundary_bits;

    if (boundary_bits == 0) {
      for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

        for (NUM_TYPE j = 0; j < NUM_CUBE_FACET_VERTICES3D; j++) {

          VERTEX_INDEX iend0 = 
            scalar_grid.FacetVertex(cube0_index, edge_dir, j);
          VERTEX_INDEX iend1 = scalar_grid.NextVertex(iend0, edge_dir);

          if (is_gt_min_le_max(scalar_grid, iend0, iend1, isovalue)) {

            bool flagB_maps_to_cube1;
            bool flagB_maps_to_cube2;
            determine_quad_map_to_cubes
              (iend0, edge_dir, cube1_index, cube2_index,
               isovert, gcube_map, flagB_maps_to_cube1, flagB_maps_to_cube2);

            /* DEBUG
            MSDEBUG();
            if (flag_debug) {

              cerr << "  Quad dual to edge (" << iend0 
                   << "," << iend1 << ") (Dir: " << edge_dir << ")."
                   << endl;
              cerr << "    Edge endpoints: ";
              ijkgrid_output_vertex_coord(cerr, scalar_grid, iend0);
              ijkgrid_output_vertex_coord(cerr, scalar_grid, iend1);
              cerr << endl;
              cerr << "    Maps to " << cube1_index
                   << ": " << int(flagB_maps_to_cube1);
              cerr << "  Maps to " << cube2_index
                   << ": " << int(flagB_maps_to_cube2);
              cerr << endl;
            }
            */

            if (flagB_maps_to_cube1) {
              flag_maps_to_cube1 = true;

              if (flagB_maps_to_cube2) {
                flag_maps_to_both_cubes = true;
                return;
              }
            }
          }
        }
      }
    }
    else {
      // Handle boundary case.
    }
  }

  /// Temporarily map from_gcube to to_gcube and then determine if some 
  ///   isosurface quad dual to an edge of cube0 maps to both cube1 and cube2.
  void temp_map_determine_quad_map_to_both_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const NUM_TYPE from_gcube_index,
   const NUM_TYPE to_gcube_index,
   const VERTEX_INDEX cube0_index,
   const VERTEX_INDEX cube1_index,
   const VERTEX_INDEX cube2_index,
   const ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_maps_to_cube1,
   bool & flag_maps_to_both_cubes)
  {
    const NUM_TYPE store_from_map = gcube_map[from_gcube_index];

    // temporarily set gcube_map[from_gcube_index] to to_gcube_index.
    gcube_map[from_gcube_index] = to_gcube_index;
    determine_quad_map_to_both_cubes
      (scalar_grid, isovalue, cube0_index, cube1_index, cube2_index,
       isovert, gcube_map, flag_maps_to_cube1, flag_maps_to_both_cubes);

    // restore gcube_map[from_gcube_index]
    gcube_map[from_gcube_index] = store_from_map;
  }


  /// Return true if cube cube0_index maps to cube to_cube.
  bool does_cube_map_to
  (const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const VERTEX_INDEX cube0_index,
   const VERTEX_INDEX to_cube)
  {
    const INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);
    const INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);

    if (gcube0_index == ISOVERT::NO_INDEX) { return(false); }

    if (gcube_map[gcube0_index] == to_gcube) { return(true); }

    return(false);
  }

  /// Count the number of edges around cube cube0_index
  ///   where one vertex maps to to_cube and the other does not.
  void count_edge_mappings_around_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0_index,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   NUM_TYPE & num_count)
  {
    const INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);

    num_count = 0;

    if (gcube0_index == ISOVERT::NO_INDEX) { return; }

    BOUNDARY_BITS_TYPE boundary_bits = 
      isovert.gcube_list[gcube0_index].boundary_bits;

    if (boundary_bits == 0) {
      for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

        int d1 = (edge_dir+1)%DIM3;
        int d2 = (edge_dir+2)%DIM3;

        for (int j1 = 0; j1 < 2; j1++) {
          for (int j2 = 0; j2 < 2; j2++) {

            VERTEX_INDEX iend0 = 
              cube0_index + j1*isovert.grid.AxisIncrement(d1) 
              + j2*isovert.grid.AxisIncrement(d2);
            VERTEX_INDEX iend1 = isovert.grid.NextVertex(iend0, edge_dir);

            if (is_gt_min_le_max(scalar_grid, iend0, iend1, isovalue)) {
              VERTEX_INDEX iv1 = 
                isovert.grid.AdjacentVertex(cube0_index, d1, j1);
              VERTEX_INDEX iv2 = 
                isovert.grid.AdjacentVertex(cube0_index, d2, j2);
              VERTEX_INDEX iv3 = 
                isovert.grid.AdjacentVertex(iv1, d2, j2);

              bool flag_v1 = 
                does_cube_map_to(isovert, gcube_map, iv1, to_cube);
              bool flag_v2 = 
                does_cube_map_to(isovert, gcube_map, iv2, to_cube);
              bool flag_v3 = 
                does_cube_map_to(isovert, gcube_map, iv3, to_cube);

              if (flag_v1 != flag_v3) { num_count++; }
              if (flag_v2 != flag_v3) { num_count++; }
            }
          }
        }
      }
    }
  }

  /// Returns true if mapping of adjacent cubes causes possible non-manifold.
  bool check_adjacent_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0_index,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const CUBE_CONNECTED_ARRAY & connected_sharp)
  {
    for (NUM_TYPE i = 0; i < connected_sharp.NumElements(); i++) {

      NUM_TYPE num_count;
      count_edge_mappings_around_cube
        (scalar_grid, isovalue, cube0_index, connected_sharp[i],
         isovert, gcube_map, num_count);

      MSDEBUG();
      if (flag_debug) {
        using namespace std;
        if (num_count > 2) {
          scalar_grid.PrintIndexAndCoord
            (cerr,  "Around cube: ", cube0_index, 
             " to cube ", connected_sharp[i], "");
          cerr << "  num edges mappings: " << num_count << endl;
        }
      }

      if (num_count > 2) { return(false); }
    }

    return(true);
  }

  /// Returns true if mapping of cubes adjacent to cubeA or cubeB causes 
  ///   possible non-manifold.
  bool check_adjacent_cubesII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const VERTEX_INDEX to_cube_index,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    IJK::PROCEDURE_ERROR error("check_adjacent_cubesII");
    const INDEX_DIFF_TYPE gcubeA_index = 
      isovert.GCubeIndex(cubeA_index, error);
    const INDEX_DIFF_TYPE gcubeB_index = 
      isovert.GCubeIndex(cubeB_index, error);
    const INDEX_DIFF_TYPE to_gcube_index = 
      isovert.GCubeIndex(to_cube_index, error);
    const VERTEX_INDEX cube_index[2] = { cubeA_index, cubeB_index };
    const INDEX_DIFF_TYPE gcube_index[2] = { gcubeA_index, gcubeB_index };
    NUM_TYPE store_map[2];
    static CUBE_CONNECTED_ARRAY connected_sharp;

    if (gcubeA_index == ISOVERT::NO_INDEX) { throw error; }
    if (gcubeB_index == ISOVERT::NO_INDEX) { throw error; }
    if (to_gcube_index == ISOVERT::NO_INDEX) { throw error; }

    for (int i = 0; i < 2; i++) 
      { store_map[i] = gcube_map[gcube_index[i]]; }

    for (int i0 = 0; i0 < 2; i0++) {
      
      int i1 = (i0+1) % 2;

      // Temporarily set gcube_index[i1] to map to to_cube.
      gcube_map[gcube_index[i1]] = to_gcube_index;

      find_connected_sharp(scalar_grid, isovalue, cube_index[i0], isovert, 
                           gcube_map, connected_sharp);

      bool flag_passed_check =
        check_adjacent_cubes
        (scalar_grid, isovalue, cube_index[i0], isovert, gcube_map,
         connected_sharp);

      // Reset gcube_map[]
      gcube_map[gcube_index[i1]] = store_map[i1];

      if (!flag_passed_check) { return(false); }
    }

    return(true);
  }

  /// Returns true if mapping of cubes adjacent to cube_index[i] causes
  ///   possible non-manifold.
  bool check_adjacent_cubesIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube_index,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    IJK::PROCEDURE_ERROR error("check_adjacent_cubesIII");
    const INDEX_DIFF_TYPE to_gcube_index = 
      isovert.GCubeIndex(to_cube_index, error);
    INDEX_DIFF_TYPE gcube_index[3];
    NUM_TYPE store_map[3];
    static CUBE_CONNECTED_ARRAY connected_sharp;

    if (to_gcube_index == ISOVERT::NO_INDEX) { throw error; }
    for (int i = 0; i < 3; i++) {
      gcube_index[i] = isovert.GCubeIndex(cube_index[i], error); 
      if (gcube_index[i] == ISOVERT::NO_INDEX) { throw error; }
      store_map[i] = gcube_map[gcube_index[i]];
    }

    connected_sharp.Clear();
    find_connected_sharp(scalar_grid, isovalue, cube_index[0], isovert, 
                         gcube_map, connected_sharp);
    add2list_connected_sharp
      (scalar_grid, isovalue, cube_index[1], isovert, 
       gcube_map, connected_sharp);
    add2list_connected_sharp
      (scalar_grid, isovalue, cube_index[2], isovert, 
       gcube_map, connected_sharp);


    for (int i0 = 0; i0 < 3; i0++) {
      
      int i1 = (i0+1) % 3;
      int i2 = (i0+2) % 3;

      // Temporarily set gcube_index[i1] and gcube_map[i2] to map to to_cube.
      gcube_map[gcube_index[i1]] = to_gcube_index;
      gcube_map[gcube_index[i2]] = to_gcube_index;

      bool flag_passed_check =
        check_adjacent_cubes
        (scalar_grid, isovalue, cube_index[i0], isovert, gcube_map,
         connected_sharp);

      // Reset gcube_map[]
      gcube_map[gcube_index[i1]] = store_map[i1];

      if (!flag_passed_check) { return(false); }
    }

    return(true);
  }

  // check for manifold violations caused by edges between sharp cubes.
  bool check_edges_between_sharp_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    static CUBE_CONNECTED_ARRAY connected_sharp;

		find_connected_sharp
      (scalar_grid, isovalue, from_cube, isovert, gcube_map, connected_sharp);

    for (NUM_TYPE i = 0; i < connected_sharp.NumElements(); i++) {

      if (connected_sharp[i] == to_cube) { continue; }

      const VERTEX_INDEX cubeA_index = connected_sharp[i];
      bool flag_maps_to_cubeA, flag_maps_to_both_cubes;
      determine_quad_map_to_both_cubes
        (scalar_grid, isovalue, from_cube, cubeA_index, to_cube,
         isovert, gcube_map, flag_maps_to_cubeA, flag_maps_to_both_cubes);

      if (flag_maps_to_cubeA && !flag_maps_to_both_cubes) {

        if (are_connected_by_iso_quad
            (scalar_grid, cubeA_index, to_cube,
             isovalue, isovert, gcube_map, flag_extended)) {

          MSDEBUG();
          if (flag_debug) {
            scalar_grid.PrintIndexAndCoord
              (cerr, "  Cubes ", cubeA_index, "");
            scalar_grid.PrintIndexAndCoord
              (cerr, " and ", to_cube, " are connected by isoquad.\n");
            scalar_grid.PrintIndexAndCoord
              (cerr, "  Merge of ", from_cube, "");
            scalar_grid.PrintIndexAndCoord
              (cerr, " to ", to_cube, " fails.\n");
          }
            

          return(false); 
        }
      }
    }

    return(true);
  }

  // check for manifold violations caused by edges between sharp cubes.
  //   caused by mapping from_cube0 to to_cube0 and from_cube1 to to_cube1.
  bool check_edges_between_sharp_cubesII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube0,
   const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX from_cube1,
   const VERTEX_INDEX to_cube1,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    IJK::PROCEDURE_ERROR error("check_edge_between_sharp_cubesII");
    static CUBE_CONNECTED_ARRAY connected_sharp;
    INDEX_DIFF_TYPE from_gcube0_index = isovert.GCubeIndex(from_cube0, error);
    INDEX_DIFF_TYPE from_gcube1_index = isovert.GCubeIndex(from_cube1, error);
    INDEX_DIFF_TYPE to_gcube0_index = isovert.GCubeIndex(to_cube0, error);
    INDEX_DIFF_TYPE to_gcube1_index = isovert.GCubeIndex(to_cube1, error);


    if (from_gcube0_index == ISOVERT::NO_INDEX) { throw error; }
    if (from_gcube1_index == ISOVERT::NO_INDEX) { throw error; }
    if (to_gcube0_index == ISOVERT::NO_INDEX) { throw error; }
    if (to_gcube1_index == ISOVERT::NO_INDEX) { throw error; }

		find_connected_sharp
      (scalar_grid, isovalue, from_cube0, isovert, gcube_map, connected_sharp);
    add2list_connected_sharp
      (scalar_grid, isovalue, from_cube1, isovert, 
       gcube_map, connected_sharp);

    for (NUM_TYPE i = 0; i < connected_sharp.NumElements(); i++) {

      const VERTEX_INDEX cubeA_index = connected_sharp[i];

      if (to_cube0 == to_cube1) {
        if (to_cube0 == cubeA_index) { 
          // Case I: to_cube0 == to_cube1 == cubeA_index
          continue; 
        }
      }

      bool flag_connected_cubeA_and_cube0(false);
      bool flag_connected_cubeA_and_cube1(false);

      if (to_cube0 != cubeA_index) {
        flag_connected_cubeA_and_cube0 =
          are_connected_by_iso_quad
          (scalar_grid, cubeA_index, to_cube0,
           isovalue, isovert, gcube_map, flag_extended);
      }

      if (to_cube1 == to_cube0) 
        { flag_connected_cubeA_and_cube1 = flag_connected_cubeA_and_cube0; }
      else if (to_cube1 != cubeA_index) {
        flag_connected_cubeA_and_cube1 =
          are_connected_by_iso_quad
          (scalar_grid, cubeA_index, to_cube1,
           isovalue, isovert, gcube_map, flag_extended);
      }

      if (to_cube0 == cubeA_index) {
        // Note: to_cube1 != cubeA_index
        if (!flag_connected_cubeA_and_cube1)
          { continue; }
      }
      else if (to_cube1 == cubeA_index) {
        // Note: to_cube0 != cubeA_index
        if (!flag_connected_cubeA_and_cube0)
          { continue; }
      }
      else {
        // to_cube0 != cubeA_index && to_cube1 != cubeA_index
        if (!flag_connected_cubeA_and_cube0 && !flag_connected_cubeA_and_cube1)
          { continue; }
      }

      bool flag0_maps_to_cubeA, flag0_maps_to_both_cubes;
      bool flag1_maps_to_cubeA, flag1_maps_to_both_cubes;
      bool flag0(true), flag1(true);

      if (flag_debug) {
        MSDEBUG();
        isovert.grid.PrintIndexAndCoord
          (cerr, "  -- Checking from ", from_cube0, " to ", to_cube0,
           " connected to ", cubeA_index, "\n");
      }

      if (to_cube0 != cubeA_index) {
        determine_quad_map_to_both_cubes
          (scalar_grid, isovalue, from_cube0, cubeA_index, to_cube0,
           isovert, gcube_map, flag0_maps_to_cubeA, flag0_maps_to_both_cubes);

        flag0 = (flag0_maps_to_cubeA && !flag0_maps_to_both_cubes);
      }

      if (flag_debug) {
        MSDEBUG();
        isovert.grid.PrintIndexAndCoord
          (cerr, "  -- Checking from ", from_cube1, " to ", to_cube1,
           " connected to ", cubeA_index, "\n");
      }

      if (to_cube1 != cubeA_index) {
        determine_quad_map_to_both_cubes
          (scalar_grid, isovalue, from_cube1, cubeA_index, to_cube1,
           isovert, gcube_map, flag1_maps_to_cubeA, flag1_maps_to_both_cubes);

        flag1 = (flag1_maps_to_cubeA && !flag1_maps_to_both_cubes);
      }

      // *** DEBUG ***
      if (flag_debug) {
        MSDEBUG();
        cerr << "  flag0_maps_to_cubeA: " << int(flag0_maps_to_cubeA)
             << "  flag0_maps_to_both_cubes: " << int(flag0_maps_to_both_cubes)
             << "  flag1_maps_to_cubeA: " << int(flag1_maps_to_cubeA)
             << "  flag1_maps_to_both_cubes: " << int(flag1_maps_to_both_cubes)
             << endl;
      }

      if (to_cube0 == cubeA_index) {
        // Note: to_cube1 != cubeA_index
        // Case IIa: to_cube0 == cubeA_index and to_cube0 != to_cube1
        if (flag1) { return(false); }
        else {
          temp_map_determine_quad_map_to_both_cubes
            (scalar_grid, isovalue, from_gcube0_index, to_gcube0_index,
             from_cube1, cubeA_index, to_cube1,
             isovert, gcube_map, flag1_maps_to_cubeA, flag1_maps_to_both_cubes);
          flag1 = (flag1_maps_to_cubeA && !flag1_maps_to_both_cubes);
          if (flag1) { return(false); }
          else { return(true); }
        }
      }
      else if (to_cube1 == cubeA_index) {
        // Note: to_cube0 != cubeA_index
        // Case IIb: to_cube1 == cubeA_index and to_cube0 != to_cube1
        if (flag0) { return(false); }
        else {
          temp_map_determine_quad_map_to_both_cubes
            (scalar_grid, isovalue, from_gcube1_index, to_gcube1_index,
             from_cube0, cubeA_index, to_cube0,
             isovert, gcube_map, flag0_maps_to_cubeA, flag0_maps_to_both_cubes);

          flag0 = (flag0_maps_to_cubeA && !flag0_maps_to_both_cubes);
          if (flag0) { return(false); }
          else { return(true); }
        }
      }
      else {
        // Case III: to_cube0 != cubeA_index && to_cube1 != cubeA_index
        if (flag0 && flag1) { 

          MSDEBUG();
          if (flag_debug) {
            isovert.grid.PrintIndexAndCoord
              (cerr, "  Both maps fail. From: ", from_cube0, " to ", 
               to_cube0, "\n");
            isovert.grid.PrintIndexAndCoord
              (cerr, "                  and ", from_cube1, " to ", 
               to_cube1, "\n");
          }

          return(false); 
        }

        if (flag0) {
          temp_map_determine_quad_map_to_both_cubes
            (scalar_grid, isovalue, from_gcube1_index, to_gcube1_index,
             from_cube0, cubeA_index, to_cube0,
             isovert, gcube_map, flag0_maps_to_cubeA, flag0_maps_to_both_cubes);

          flag0 = (flag0_maps_to_cubeA && !flag0_maps_to_both_cubes);
          if (flag0) { return(false); }
          else { return(true); }
        }
        else {
          // flag1 == true
          temp_map_determine_quad_map_to_both_cubes
            (scalar_grid, isovalue, from_gcube0_index, to_gcube0_index,
             from_cube1, cubeA_index, to_cube1,
             isovert, gcube_map, flag1_maps_to_cubeA, flag1_maps_to_both_cubes);
          flag1 = (flag1_maps_to_cubeA && !flag1_maps_to_both_cubes);
          if (flag1) { return(false); }
          else { return(true); }
        }
      }
    }

    return(true);
  }

  // check for manifold violations on a single edge with endpoints
  //   in (to_cube,cubeA_index) caused by mapping three cubes to to_cube.
  bool check_edge_manifoldIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube,
   const VERTEX_INDEX cubeA_index,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE gcube_index[3], to_gcube;
    NUM_TYPE store_map[3];
    bool flag_maps_to_cubeA[3], flag_maps_to_both_cubes[3];
    IJK::PROCEDURE_ERROR error("check_edge_manifoldIII");

    to_gcube = isovert.GCubeIndex(to_cube, error);
    for (int i = 0; i < 3; i++) {
      gcube_index[i] = isovert.GCubeIndex(cube_index[i], error); 
      if (gcube_index[i] == ISOVERT::NO_INDEX) { throw error; }
      store_map[i] = gcube_map[gcube_index[i]];
    }

    for (int i0 = 0; i0 < 3; i0++) {

      determine_quad_map_to_both_cubes
        (scalar_grid, isovalue, cube_index[i0], cubeA_index, to_cube,
         isovert, gcube_map, flag_maps_to_cubeA[0], flag_maps_to_both_cubes[0]);

      if (!flag_maps_to_cubeA[0] || flag_maps_to_both_cubes[0]) {

        gcube_map[gcube_index[i0]] = to_gcube;

        for (int k = 1; k <= 2; k++) {

          int i1 = (i0+k)%3;
          int i2 = (i0+2*k)%3;

          determine_quad_map_to_both_cubes
            (scalar_grid, isovalue, cube_index[i1], cubeA_index, to_cube, 
           isovert, gcube_map, flag_maps_to_cubeA[1], 
           flag_maps_to_both_cubes[1]);

          if (!flag_maps_to_cubeA[1] || flag_maps_to_both_cubes[1]) {
            gcube_map[gcube_index[i1]] = to_gcube;

            determine_quad_map_to_both_cubes
              (scalar_grid, isovalue, cube_index[i2], cubeA_index, to_cube, 
               isovert, gcube_map, flag_maps_to_cubeA[2], 
               flag_maps_to_both_cubes[2]);

            if (!flag_maps_to_cubeA[2] || flag_maps_to_both_cubes[2]) {

              // restore gcube_map
              gcube_map[gcube_index[i0]] = store_map[i0];
              gcube_map[gcube_index[i1]] = store_map[i1];
              return(true);
            }
          }
        }
      }

      // restore gcube_map
      gcube_map[gcube_index[i0]] = store_map[i0];
    }

    return(false);
  }

  // check for manifold violations caused by edges between sharp cubes.
  //   caused by mapping three cubes to to_cube.
  bool check_all_edges_between_sharp_cubesIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    INDEX_DIFF_TYPE gcube_index[3], to_gcube_index;
    static CUBE_CONNECTED_ARRAY connected_sharp;
    IJK::PROCEDURE_ERROR error("check_edge_between_sharp_cubesIII");

    to_gcube_index = isovert.GCubeIndex(to_cube, error);
    for (int i = 0; i < 3; i++) {
      gcube_index[i] = isovert.GCubeIndex(cube_index[i], error); 
      if (gcube_index[i] == ISOVERT::NO_INDEX) { throw error; }
    }

		find_connected_sharp
      (scalar_grid, isovalue, cube_index[0], isovert, gcube_map, 
       connected_sharp);
    add2list_connected_sharp
      (scalar_grid, isovalue, cube_index[1], isovert, 
       gcube_map, connected_sharp);
    add2list_connected_sharp
      (scalar_grid, isovalue, cube_index[2], isovert, 
       gcube_map, connected_sharp);

    for (NUM_TYPE i = 0; i < connected_sharp.NumElements(); i++) {

      const VERTEX_INDEX cubeA_index = connected_sharp[i];

      if (cubeA_index == to_cube) { continue; }

      if (!are_connected_by_iso_quad
          (scalar_grid, cubeA_index, to_cube, isovalue, isovert, gcube_map, 
           flag_extended)) { continue; }

      if (!check_edge_manifoldIII
          (scalar_grid, isovalue, cube_index, to_cube, cubeA_index,
           isovert, gcube_map))
        { return(false); }
    }

    return(true);
  }


  // Check for edges connecting two cubes separated 
  //   by a third selected edge cube.
  bool check_separating_cubes
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    static CUBE_CONNECTED_ARRAY connected_sharp;
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    GRID_BOX region(DIM3);


    if (to_gcube == ISOVERT::NO_INDEX) { return(false); }

    const GRID_COORD_TYPE * to_coord = 
      isovert.gcube_list[to_gcube].cube_coord;

		find_connected_sharp(scalar_grid, isovalue, from_cube, isovert, 
                         gcube_map, connected_sharp);

    MSDEBUG();
    if (flag_debug) {
      cerr << "In " << __func__;
      scalar_grid.PrintIndexAndCoord(cerr, " From cube: ", from_cube, "");
      scalar_grid.PrintIndexAndCoord(cerr, " To cube: ", to_cube, "\n");
      cerr << "  connected_sharp: ";
      for (int i = 0; i < connected_sharp.NumElements(); i++)
        { cerr << " " << connected_sharp[i]; }
      cerr << endl;
    }

    for (NUM_TYPE i = 0; i < connected_sharp.NumElements(); i++) {

      if (connected_sharp[i] == to_cube) { continue; }

      INDEX_DIFF_TYPE gcubeA = isovert.GCubeIndex(connected_sharp[i]);
      const GRID_COORD_TYPE * gcubeA_coord = 
        isovert.gcube_list[gcubeA].cube_coord;

      for (NUM_TYPE j = 0; j < connected_sharp.NumElements(); j++) {

        if (connected_sharp[j] == to_cube) { continue; }
        if (connected_sharp[j] == connected_sharp[i]) { continue; }

        region.SetCoord(to_coord, to_coord);
        region.Extend(gcubeA_coord);

        const NUM_TYPE gcubeB_index = isovert.GCubeIndex(connected_sharp[j]);
        if (gcubeB_index == ISOVERT::NO_INDEX) { continue; }

        // Skip corner cubes.
        // *** DEBUG ***
        if (isovert.NumEigenvalues(gcubeB_index) == 3) { continue; }
        // *** SHOULD CHECK ANGLE NOT REGION.

        const GRID_COORD_TYPE * coordB = 
          isovert.gcube_list[gcubeB_index].cube_coord;

        bool is_region_vertex;
        if (does_region_contain_cube(region, coordB, is_region_vertex)) {

          if (!is_region_vertex) {

            MSDEBUG();
            if (flag_debug) {
              scalar_grid.PrintIndexAndCoord
                (cerr, " --- Cube ",  connected_sharp[j], " is between\n");
              scalar_grid.PrintIndexAndCoord
                (cerr, "   ", to_cube, "");
              scalar_grid.PrintIndexAndCoord
                (cerr, " and ", connected_sharp[i], "\n");
            }
            
            return(false);
          }
        }
      }
    }

    return(true);
  }

  // Check for edges connecting two cubes separated 
  //   by a third selected edge cube.
  // Check for edges where two cubes are mapped
  bool check_separating_cubesII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0,
   const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX cube1,
   const VERTEX_INDEX to_cube1,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    NUM_TYPE store_map[2];
    INDEX_DIFF_TYPE gcube_index[2];
    INDEX_DIFF_TYPE to_gcube_index[2];
    bool flag_result;

    gcube_index[0] = isovert.GCubeIndex(cube0);
    gcube_index[1] = isovert.GCubeIndex(cube1);
    to_gcube_index[0] = isovert.GCubeIndex(to_cube0);
    to_gcube_index[1] = isovert.GCubeIndex(to_cube1);

    store_map[0] = gcube_map[gcube_index[0]];
    store_map[1] = gcube_map[gcube_index[1]];

    // temporarily set gcube_map[gcube_index[0]] to to_gcube_index[0].
    gcube_map[gcube_index[0]] = to_gcube_index[0];

    flag_result = 
      check_separating_cubes
      (scalar_grid, isovalue, cube1, to_cube1, isovert, gcube_map,
       flag_extended);

    // reset gcube_map[gcube_index[0]].
    gcube_map[gcube_index[0]] = store_map[0];

    if (!flag_result) { return(false); }

    // temporarily set gcube_map[gcube_index[1]] to to_gcube_index[1].
    gcube_map[gcube_index[1]] = to_gcube_index[1];

    flag_result = check_separating_cubes
      (scalar_grid, isovalue, cube0, to_cube0, isovert, gcube_map,
       flag_extended);

    // reset gcube_map[gcube_index[1]].
    gcube_map[gcube_index[1]] = store_map[1];

    if (!flag_result) { return(false); }

    return(true);
  }


  // check for some violations of manifold conditions by map
  // (Does not catch all violations.)
  bool check_map
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    if (!check_edges_between_sharp_cubes
        (scalar_grid, isovalue, from_cube, to_cube, 
         isovert, gcube_map, flag_extended))
      { return(false); }

    if (!is_unselected_cube_connected_to
        (scalar_grid, isovalue, isovert, from_cube, to_cube, gcube_map))
      {
        // *** DEBUG ***
        MSDEBUG();
        if (flag_debug) {
          scalar_grid.PrintIndexAndCoord
            (cerr, "*** Unselected cube ", from_cube,
             " NOT connected to ", to_cube, "\n");
        }

        return(false);
      }

    if (!check_separating_cubes
        (scalar_grid, isovalue, from_cube, to_cube, isovert, 
         gcube_map, flag_extended))
      { return(false); }

    return(true);
  }

  void check_and_map_isov
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const NUM_TYPE from_gcube = isovert.GCubeIndex(from_cube);

    if (from_gcube == ISOVERT::NO_INDEX) { return; }

    if (check_map(scalar_grid, isovalue, from_cube, to_cube,
                  isovert, gcube_map, false)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     from_gcube, to_gcube, gcube_map);
    }
  }

}


// **************************************************
// Map isosurface vertices
//   Allow multiple vertices per cube
// **************************************************

namespace {

  // data structure for passing mapping flags
  struct MAP_PARAM_FLAGS {
    bool extended;
    bool strict;
  };

  // forward declarations
  void check_and_map_isov_multi
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MAP_PARAM_FLAGS & param_flag,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  void check_and_map_isov_multi_region
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const GRID_BOX & region,
   const MAP_PARAM_FLAGS & param_flag,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  void check_and_map_ambig_pair
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  void check_and_map_ambig_pair_region
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const GRID_BOX & region,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  void map_facet_adjacent_pairs
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX cube0,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGESHARP::MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  void map_adjacent_cubes_to_different
  (const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX cube0,
   const MERGESHARP::ISOVERT & isovert,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);

  /// Map cubes which share a facet with cube to_cube.
  /// Only map cubes contained in the given region.
	void map_facet_adjacent_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   const bool flag_strict,
   const MERGE_PARAM & merge_param,
   const GRID_BOX & region,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    BOUNDARY_BITS_TYPE boundary_bits;
    const MAP_PARAM_FLAGS param_flags = { false, flag_strict };
    bool flag_map;

    if (to_gcube == ISOVERT::NO_INDEX) { return; }
    boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      apply_to_cubes_facet_adjacent_to
        (check_and_map_isov_multi_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags, merge_param, 
         gcube_map, flag_map);

      apply_to_cubes_facet_adjacent_to
        (check_and_map_ambig_pair_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags.extended, merge_param, 
         gcube_map, flag_map);
    }
  }

  /// Map cubes which share an edge with cube to_cube.
  /// Only map cubes contained in the given region.
	void map_edge_adjacent_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   const bool flag_strict,
   const MERGE_PARAM & merge_param,
   const GRID_BOX & region,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    const MAP_PARAM_FLAGS param_flags = { false, flag_strict };
    bool flag_map;

    if (to_gcube == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      int boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

      apply_to_cubes_edge_adjacent_to
        (check_and_map_isov_multi_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags, merge_param,
         gcube_map, flag_map);

      apply_to_cubes_edge_adjacent_to
        (check_and_map_ambig_pair_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags.extended, merge_param,
         gcube_map, flag_map);
    }
  }

  /// Map cubes which share a vertex with cube to_cube.
  /// Only map cubes contained in the given region.
	void map_vertex_adjacent_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX to_cube,
   const bool flag_strict,
   const MERGE_PARAM & merge_param,
   const GRID_BOX & region,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    const MAP_PARAM_FLAGS param_flags = { false, flag_strict };
    bool flag_map;

    if (to_gcube == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

      int boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

      apply_to_cubes_vertex_adjacent_to
        (check_and_map_isov_multi_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags, merge_param,
         gcube_map, flag_map);

      apply_to_cubes_vertex_adjacent_to
        (check_and_map_ambig_pair_region, isovert.grid, to_cube, 
         boundary_bits, scalar_grid, isodual_table, ambig_info, 
         isovalue, isovert, region, param_flags.extended, merge_param,
         gcube_map, flag_map);
    }
  }

  /// Find cube in cube_list whose coordinates are between coordA and coordB.
  /// Return true if such a cube exists.
  bool find_separating_cube
  (const ISOVERT & isovert, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3],
   const VERTEX_INDEX * cube_list, const NUM_TYPE list_length,
   VERTEX_INDEX & separating_cube)
  {
    GRID_BOX region(DIM3);
    bool flag_found = false;

    // Initialize
    separating_cube = 0;

    region.SetCoord(coordA, coordA);
    region.Extend(coordB);

    for (NUM_TYPE i = 0; i < list_length; i++) {
      const NUM_TYPE gcubeC_index = isovert.GCubeIndex(cube_list[i]);

      if (gcubeC_index == ISOVERT::NO_INDEX) { continue; }

      const GRID_COORD_TYPE * coordC = 
        isovert.gcube_list[gcubeC_index].cube_coord;

      if (region.Contains(coordC)) {
        separating_cube = isovert.CubeIndex(gcubeC_index);
        return(true);
      }
    }

    return(false);
  }

  /// Find cube in cube_list whose coordinates are between coordA and coordB.
  /// Return true if such a cube exists.
  bool find_separating_cube
  (const ISOVERT & isovert, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3],
   const std::vector<VERTEX_INDEX> & cube_list,
   VERTEX_INDEX & separating_cube)
  {
    bool flag =
      find_separating_cube
      (isovert, coordA, coordB, IJK::vector2pointer(cube_list),
       cube_list.size(), separating_cube);

    return(flag);
  }

  /// Return true if some cube in cube_list is between coordA and coordB.
  bool is_cube_between
  (const ISOVERT & isovert, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3],
   const VERTEX_INDEX * cube_list, const NUM_TYPE list_length)
  {
    VERTEX_INDEX separating_cube;

    bool flag = find_separating_cube(isovert, coordA, coordB, 
                                     cube_list, list_length, separating_cube);

    return(flag);
  }

  /// Return true if some cube in cube_list is between coordA and coordB.
  bool is_cube_between
  (const ISOVERT & isovert, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3],
   const CUBE_CONNECTED_ARRAY cube_list)
  {
    const VERTEX_INDEX * cube_list_ptr = cube_list.PtrConst();
    const NUM_TYPE list_length = cube_list.NumElements();

    return(is_cube_between(isovert, coordA, coordB, 
                           cube_list_ptr, list_length));
  }

  /// Construct small region (inside 3x3x3) around corner cube
  /// Note: Region is given by cube indices, not vertex coord.
  void construct_small_corner_cube_region
  (const SHARPISO_GRID & grid,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const ISOVERT & isovert,
   const VERTEX_INDEX corner_cube_index,
   GRID_BOX & region)
  {
    const INDEX_DIFF_TYPE corner_gcube_index = 
      isovert.GCubeIndex(corner_cube_index);
    std::vector<VERTEX_INDEX> selected_list;
    GRID_COORD_TYPE Linf_distance;
    IJK::PROCEDURE_ERROR error("construct_small_corner_cube_region");

    if (region.Dimension() != DIM3) {
      error.AddMessage("Programming error. Region dimension is not 3.");
      throw error;
    }

    if (corner_gcube_index == ISOVERT::NO_INDEX) {
      error.AddMessage
        ("Programming error.  Cube ", corner_cube_index, " is not active.");
      throw error;
    }

    const GRID_COORD_TYPE * corner_cube_coord = 
      isovert.gcube_list[corner_gcube_index].cube_coord;

    // get the selected vertices around corner_cube
    get_selected(grid, corner_cube_index, bin_grid, bin_width, selected_list);

    region.SetCoord(corner_cube_coord, corner_cube_coord);
    for (int d = 0; d < DIM3; d++) {
      if (corner_cube_coord[d] > 0) 
        { region.SetMinCoord(d, corner_cube_coord[d]-1); }
      if (corner_cube_coord[d]+2 < grid.AxisSize(d))
        { region.SetMaxCoord(d, corner_cube_coord[d]+1); }
    }

    // Contract region.
    for (int i = 0; i < selected_list.size(); i++) {

      VERTEX_INDEX selected_cube_index = selected_list[i];
      INDEX_DIFF_TYPE selected_gcube_index = 
        isovert.GCubeIndex(selected_cube_index);

      if (selected_gcube_index == ISOVERT::NO_INDEX) { 
        error.AddMessage
          ("Programming error.  Selected cube ", selected_cube_index,
           " is not active.");
        throw error;
      }

      const GRID_COORD_TYPE * selected_cube_coord =
        isovert.gcube_list[selected_gcube_index].cube_coord;

      IJK::compute_Linf_distance
        (DIM3, corner_cube_coord, selected_cube_coord, Linf_distance);

      if (Linf_distance != 3) { continue; }

      VERTEX_INDEX separating_cube_index;
      if (find_separating_cube
          (isovert, corner_cube_coord, selected_cube_coord, selected_list,
           separating_cube_index)) {

        INDEX_DIFF_TYPE separating_gcube_index = 
          isovert.GCubeIndex(separating_cube_index);
        if (separating_gcube_index == ISOVERT::NO_INDEX) {
          error.AddMessage
            ("Programming error.  Separating cube ", separating_cube_index,
             " is not active.");
          throw error;
        }

        const GRID_COORD_TYPE * separating_cube_coord =
          isovert.gcube_list[separating_gcube_index].cube_coord;

        for (int d = 0; d < DIM3; d++) {
          if  (separating_cube_coord[d] < corner_cube_coord[d]) 
            { region.SetMinCoord(d, corner_cube_coord[d]); }
          else if  (separating_cube_coord[d] > corner_cube_coord[d]) 
            { region.SetMaxCoord(d, corner_cube_coord[d]); }
        }
      }
    }
  }

	// Map isosurface vertices adjacent to isosurface in selected corner cubes
  // @param Map to cubes with the given number of eigenvalues.
  // Handles SOME boundary cases.
	void map_adjacent_corner_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & selected_corner_gcube_list,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const bool flag_strict(true);
    GRID_BOX region(DIM3);

    for (NUM_TYPE i = 0; i < selected_corner_gcube_list.size(); i++) {

      NUM_TYPE gcube_index = selected_corner_gcube_list[i];

      if (isovert.NumEigenvalues(gcube_index) != 3) { continue; }

      VERTEX_INDEX to_cube = isovert.CubeIndex(gcube_index);
      construct_small_corner_cube_region
        (isovert.grid, bin_grid, bin_width, isovert, to_cube, region);

      // Map cubes which share facets with selected cubes.
      map_facet_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Map cubes which share edges with selected cubes.
      map_edge_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Try again to map cubes which share facets with selected cubes.
      map_facet_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Try again to map cubes which share edges with selected cubes.
      map_edge_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Map cubes which share vertices with selected cubes.
      map_vertex_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Try again to map cubes which share facets with selected cubes.
      map_facet_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Try again to map cubes which share edges with selected cubes.
      map_edge_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);

      // Try again to map cubes which share vertices with selected cubes.
      map_vertex_adjacent_cubes_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert,
         to_cube, flag_strict, merge_param, region, gcube_map);
    }
	}

	// Map isosurface vertices adjacent to isosurface in selected cubes.
  // Simple version.  No repeat mapping.
	void map_adjacent_cubes_multi_simple
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const bool flag_strict,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const bool flag_extended(false);
    const MAP_PARAM_FLAGS param_flags = {flag_extended, flag_strict};
    bool flag;

    // Map cubes which share facets with selected cubes.
    apply_to_cubes_facet_adjacent_to_list
      (check_and_map_isov_multi, selected_gcube_list, 
       isovert, scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       param_flags, merge_param, gcube_map, flag);

    // Map cubes which share edges with selected cubes.
    apply_to_cubes_edge_adjacent_to_list
      (check_and_map_isov_multi, selected_gcube_list, 
       isovert, scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       param_flags, merge_param, gcube_map, flag);

    // Map cubes which share vertex with selected cubes.
    apply_to_cubes_vertex_adjacent_to_list
      (check_and_map_isov_multi, selected_gcube_list, 
       isovert, scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       param_flags, merge_param, gcube_map, flag);
  }

	// Map cube pairs which share an ambiguous facet
  // Simple version.  No repeat mapping.
	void map_ambiguous_pairs_simple
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const bool flag_strict,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const bool flag_extended(false);
    const MAP_PARAM_FLAGS param_flags = {flag_extended, flag_strict};
    bool flag;

    // Map cube pairs where one cube is facet adjacent to a cube in the list.
    apply_to_cubes_facet_adjacent_to_list
      (check_and_map_ambig_pair, selected_gcube_list, isovert,
       scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       flag_extended, merge_param, gcube_map, flag);

    // Map cube pairs where one cube is edge adjacent to a cube in the list.
    apply_to_cubes_edge_adjacent_to_list
      (check_and_map_ambig_pair, selected_gcube_list, isovert,
       scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       flag_extended, merge_param, gcube_map, flag);
  }

	// Map isosurface vertices adjacent to isosurface in selected cubes
  //   with given number of eigenvalues.
  // Vertices are in 3x3x3 region around selected cube.
  // Handles SOME boundary cases.
  // @param selected_gcube_list[] List of selected cubes
  //    sorted by number of large eigenvalues and then by distance 
  //    of sharp coord from cube center.
	void map_adjacent_cubes_multi_3x3x3
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const int TWO(2), THREE(3);
    const bool flag_extended(false);
    const bool flag_strict(true);
    const MAP_PARAM_FLAGS param_flags = {flag_extended, flag_strict};
    bool flag;

    // Map cubes which share facets with selected cubes.
    apply_to_cubes_facet_adjacent_to_list
      (check_and_map_isov_multi, selected_gcube_list, isovert, 
       scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       param_flags, merge_param, gcube_map, flag);

    // Map cube pairs which share an ambiguous facet.
    apply_to_cubes_facet_adjacent_to_list
      (check_and_map_ambig_pair, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);

		// Map cubes which share edges with selected cubes.
    apply_to_cubes_edge_adjacent_to_list
      (check_and_map_isov_multi, selected_gcube_list, isovert,  
       scalar_grid, isodual_table, ambig_info, isovalue, isovert,
       param_flags, merge_param, gcube_map, flag);

    // Map cube pairs which share an ambiguous facet.
    apply_to_cubes_edge_adjacent_to_list
      (check_and_map_ambig_pair, selected_gcube_list, isovert, scalar_grid,
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);

    // Map cube pairs (cube0, cube1) where cube0 is facet adjacent
    //   to a selected cube
    apply_to_cubes_facet_adjacent_to_list
      (map_facet_adjacent_pairs, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);

    // Map cubes which are facet, edge or vertex adjacent to selected cubes.
    map_adjacent_cubes_multi_simple
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, flag_strict, merge_param, gcube_map);

    // Map ambiguous pairs
    map_ambiguous_pairs_simple
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, flag_strict, merge_param, gcube_map);

    // Try again to map cube pairs (cube0, cube1) where cube0 
    //   is facet adjacent to a selected cube
    apply_to_cubes_facet_adjacent_to_list
      (map_facet_adjacent_pairs, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended,
       merge_param, gcube_map, flag);

    // Map cube pairs (cube0, cube1) where cube0 is edge adjacent
    //   to a selected cube
    apply_to_cubes_edge_adjacent_to_list
      (map_facet_adjacent_pairs, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);

    // Map two cubes to different cubes.
    apply_to_cubes_facet_adjacent_to_list
      (map_adjacent_cubes_to_different, selected_gcube_list, isovert, isovert,
       scalar_grid, isodual_table, ambig_info, isovalue, flag_extended, 
       merge_param, gcube_map, flag);

    // Map two cubes to different cubes.
    apply_to_cubes_edge_adjacent_to_list
      (map_adjacent_cubes_to_different, selected_gcube_list, isovert, isovert,
       scalar_grid, isodual_table, ambig_info, isovalue, flag_extended, 
       merge_param, gcube_map, flag);

    // Try again to map cubes which are facet, edge or vertex adjacent 
    //   to selected cubes.
    map_adjacent_cubes_multi_simple
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, flag_strict, merge_param, gcube_map);

    // Try again to map ambiguous pairs
    map_ambiguous_pairs_simple
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, flag_strict, merge_param, gcube_map);

    // Try again to map cube pairs (cube0, cube1) where cube0 
    //   is facet adjacent to a selected cube
    apply_to_cubes_facet_adjacent_to_list
      (map_facet_adjacent_pairs, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);

    // Map cube pairs (cube0, cube1) where cube0 is edge adjacent
    //   to a selected cube
    apply_to_cubes_edge_adjacent_to_list
      (map_facet_adjacent_pairs, selected_gcube_list, isovert, scalar_grid, 
       isodual_table, ambig_info, isovalue, isovert, flag_extended, 
       merge_param, gcube_map, flag);
	}


	// Map isosurface vertices adjacent to selected cubes
	//  to the isosurface vertex in the selected cube.
  // Handles SOME boundary cases.
	void map_adjacent_cubes_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const int bin_width = merge_param.bin_width;
    const bool flag_map_extended = merge_param.flag_map_extended;
		const NUM_TYPE num_gcube = isovert.gcube_list.size();
    std::vector<NUM_TYPE> selected_gcube_list;
    std::vector<NUM_TYPE> selected_corner_gcube_list;
    BIN_GRID<VERTEX_INDEX> bin_grid;
    bool flag_strict;

    // *** DEBUG ***
    using namespace std;
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

    // Initialize
		for (NUM_TYPE i = 0; i < num_gcube; i++)
		{ gcube_map[i] = i; }

		get_selected_cubes(isovert.gcube_list, selected_gcube_list);
		get_selected_corner_cubes(isovert.gcube_list, selected_corner_gcube_list);

    init_bin_grid(scalar_grid, bin_width, bin_grid);
    insert_selected_in_bin_grid
      (scalar_grid, isovert, selected_gcube_list, bin_width, bin_grid);

    // Map to corner cubes.
    map_adjacent_corner_cubes_multi
      (scalar_grid, isodual_table, ambig_info, bin_grid, bin_width,
       isovalue, isovert, selected_corner_gcube_list, merge_param, gcube_map);

    // Map to edge cubes.
    map_adjacent_cubes_multi_3x3x3
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, merge_param, gcube_map);

		if (flag_map_extended) {

      extend_map_adjacent_pairs_covered
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
         merge_param, gcube_map);

      extend_map_around_edges
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert, merge_param,
         gcube_map);

      // Redo map to adjacent cubes.
      flag_strict = true;
      map_adjacent_cubes_multi_simple
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
         selected_gcube_list, flag_strict, merge_param, gcube_map);

			extend_mapping_corner_cube_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         merge_param, isovert, gcube_map);

      extend_mapping
        (scalar_grid, isovalue, selected_gcube_list, isovert, gcube_map);

      extend_mappingII(scalar_grid, isovalue, selected_gcube_list, merge_param,
                       isovert, gcube_map);

      // *** COULD CALL extend_mappingIII HERE. ***

      // Redo map to adjacent cubes.
      map_adjacent_cubes_multi_simple
        (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
         selected_gcube_list, flag_strict, merge_param, gcube_map);
		}

    // Redo map to adjacent cubes.
    flag_strict = false;
    map_adjacent_cubes_multi_simple
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       selected_gcube_list, flag_strict, merge_param, gcube_map);
	}

	void set_first_gcube_isov
		(const ISOVERT & isovert, const std::vector<DUAL_ISOVERT> & iso_vlist,
		NUM_TYPE * first_gcube_isov)
	{
		// Scan iso_vlist from back to front.
		for (int i = 0; i < iso_vlist.size(); i++) {
			int j = iso_vlist.size()-i-1;
			VERTEX_INDEX cube_index = iso_vlist[j].cube_index;
			VERTEX_INDEX gcube_index = isovert.GCubeIndex(cube_index);
			first_gcube_isov[gcube_index] = j;
		}
	}

  // Check cubes with multiple isosurface vertices
  // Return true if passed check.
  bool check_cubes_with_multi_isov
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    const IJKDUALTABLE::TABLE_INDEX table_index = 
      isovert.gcube_list[from_gcube].table_index;

    if (isodual_table.NumIsoVertices(table_index) < 2) 
      { return(true); }

    const INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    const BOUNDARY_BITS_TYPE boundary_bits =
      isovert.gcube_list[from_gcube].boundary_bits;

    if (boundary_bits == 0) {

      for (NUM_TYPE jfacet = 0; jfacet < NUM_CUBE_FACETS3D; jfacet++) {

        if (ambig_info.IsFacetAmbiguous(table_index, jfacet)) {
          int orth_dir = IJK::cube_facet_orth_dir(DIM3, jfacet);
          int side = IJK::cube_facet_side(DIM3, jfacet);

          const VERTEX_INDEX adj_cube_index = 
            scalar_grid.AdjacentVertex(from_cube, orth_dir, side);
          const INDEX_DIFF_TYPE adj_gcube_index =
            isovert.GCubeIndex(adj_cube_index);
          if (adj_gcube_index == ISOVERT::NO_INDEX)
            { return(false); }

          IJKDUALTABLE::TABLE_INDEX adj_table_index =
            isovert.gcube_list[adj_gcube_index].table_index;

          if (isodual_table.NumIsoVertices(adj_table_index) == 1) {

            if (gcube_map[adj_gcube_index] == to_gcube) {

              if (isodual_table.NumIsoVertices(table_index) == 2) {
                // Some adjacent cube connects the two isovertices in from_cube
                //   to the isovertex in to_cube
                return(true);
              }
            }
            else
              { 
                if (flag_debug) {
                  MSDEBUG();
                  cerr << "    Multi isov check failed." << endl;
                  scalar_grid.PrintIndexAndCoord
                    (cerr, "    From ", from_cube, " to ", to_cube,
                     " adjacent ", adj_cube_index, "\n");
                  cerr << "    Cube " << from_cube
                       << " has " << isodual_table.NumIsoVertices(table_index)
                       << " isosurface vertices. (Table index: " 
                       << table_index << ")." << endl;
                }
                
                return(false); 
              }
          }
        }
      }
    }
    else {
      // Handle boundary.
    }

    return(true);
  }

  // check for some violations of manifold conditions by map
  // (Does not catch all violations.)
  bool check_map_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended,
   const bool flag_strict,
   const MERGE_PARAM & merge_param)
  {

    if (!is_unselected_cube_connected_to
        (scalar_grid, isovalue, isovert, from_cube, to_cube, gcube_map))
      { return(false); }

    if (!check_cubes_with_multi_isov
        (scalar_grid, isodual_table, ambig_info, isovalue,
         from_cube, to_cube, isovert, gcube_map)) {

      MSDEBUG();
      if (flag_debug) {
        cerr << "----- Ambig cube " << from_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
        cerr << " NOT connected to " << to_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
        cerr << endl;
      }

      return(false);
    }

    if (!check_edges_between_sharp_cubes
        (scalar_grid, isovalue, from_cube, to_cube, 
         isovert, gcube_map, flag_extended))
      { return(false); }

    if (!check_separating_cubes
        (scalar_grid, isovalue, from_cube, to_cube, isovert, 
         gcube_map, flag_extended))
      { return(false); }

    if (flag_strict) {
      if (!check_distortion_strict
          (scalar_grid, isovalue, isovert, gcube_map, 
           from_cube, to_cube, merge_param)) {

        if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (strict distortion). From: ", from_cube, 
           " to ", to_cube, "\n");
        }

        return(false);
      }
    }
    else {
      if (!check_distortion_loose
          (scalar_grid, isovalue, isovert, gcube_map, 
           from_cube, to_cube, merge_param)) {

        if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (loose distortion). From: ", from_cube, 
           " to ", to_cube, "\n");
        }

        return(false);
      }
    }

    return(true);
  }

  /// Check for some violations of manifold conditions by map
  /// (Does not catch all violations.)
  /// Only map cubes contained in the given region.
  bool check_map_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   const GRID_BOX & region,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended,
   const bool flag_strict,
   const MERGE_PARAM & merge_param)
  {
    if (!does_region_contain_cube(region, from_cube, isovert))
      { return(false); }

    bool flag_map = 
      check_map_multi
      (scalar_grid, isodual_table, ambig_info, isovalue,
       from_cube, to_cube, isovert, gcube_map, flag_extended, flag_strict,
       merge_param);
    return(flag_map);
  }

  void check_and_map_isov_multi
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MAP_PARAM_FLAGS & param_flag,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const NUM_TYPE from_gcube = isovert.GCubeIndex(from_cube);

    flag_map = false;

    if (from_gcube == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[from_gcube] != from_gcube) { return; }

    MSDEBUG();
    if (flag_debug) {
      cerr << "Checking mapping from " << from_cube << " ";
      ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
      cerr << " to " << to_cube << " ";
      ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
      cerr << endl;
    }

    if (check_map_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         from_cube, to_cube, isovert, gcube_map, 
         param_flag.extended, param_flag.strict, merge_param)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     from_gcube, to_gcube, gcube_map);
      flag_map = true;
    }
    else {

      MSDEBUG();
      if (flag_debug) {
        cerr << "*** Not mapping " << from_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
        cerr << " to " << to_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
        cerr << endl;
      }
    }
  }

  /// Only map cubes contained in the given region.
  void check_and_map_isov_multi_region
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const GRID_BOX & region,
   const MAP_PARAM_FLAGS & param_flag,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const NUM_TYPE from_gcube = isovert.GCubeIndex(from_cube);

    flag_map = false;

    if (from_gcube == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[from_gcube] != from_gcube) { return; }

    if (check_map_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         from_cube, to_cube, region, isovert, gcube_map, 
         param_flag.extended, param_flag.strict, merge_param)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     from_gcube, to_gcube, gcube_map);
      flag_map = true;
    }
  }

  // Get the cube sharing an ambiguous facet
  void get_cube_sharing_ambiguous_facet
  (const SHARPISO_GRID & grid,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const VERTEX_INDEX cube0_index,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   VERTEX_INDEX & cube1_index,
   bool & found_cube1)
  {
    const NUM_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);

    found_cube1 = false;

    if (gcube0_index == ISOVERT::NO_INDEX) { return; }

    const IJKDUALTABLE::TABLE_INDEX table_index = 
      isovert.gcube_list[gcube0_index].table_index;

    if (isodual_table.NumIsoVertices(table_index) < 2) { return; }

    if (ambig_info.NumAmbiguousFacets(table_index) != 1) { return; }

    const BOUNDARY_BITS_TYPE boundary_bits =
      isovert.gcube_list[gcube0_index].boundary_bits;

    for (NUM_TYPE jfacet = 0; jfacet < NUM_CUBE_FACETS3D; jfacet++) {

      if (ambig_info.IsFacetAmbiguous(table_index, jfacet)) {
        int orth_dir = IJK::cube_facet_orth_dir(DIM3, jfacet);
        int side = IJK::cube_facet_side(DIM3, jfacet);

        BOUNDARY_BITS_TYPE mask = (BOUNDARY_BITS_TYPE(1) << jfacet);
        if (boundary_bits & mask) { 
          found_cube1 = false;
          return;
        }

        cube1_index = grid.AdjacentVertex(cube0_index, orth_dir, side);
        found_cube1 = true;
      }
    }

  }

  // Check if cube0 maps to to_cube0 while cube1 maps to to_cube1.
  bool check_mapII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0,
   const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX cube1,
   const VERTEX_INDEX to_cube1,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    const NUM_TYPE gcube0 = isovert.GCubeIndex(cube0);
    const NUM_TYPE gcube1 = isovert.GCubeIndex(cube1);
    const NUM_TYPE to_gcube0 = isovert.GCubeIndex(to_cube0);
    const NUM_TYPE to_gcube1 = isovert.GCubeIndex(to_cube1);
    GRID_COORD_TYPE max_linf_distance;

    if (gcube0 == ISOVERT::NO_INDEX || gcube1 == ISOVERT::NO_INDEX)
      { return(false); }

    GRID_COORD_TYPE linf_distanceA, linf_distanceB;
    compute_Linf_distance_between_grid_vertices
      (scalar_grid, cube0, to_cube0, linf_distanceA);
    compute_Linf_distance_between_grid_vertices
      (scalar_grid, cube1, to_cube1, linf_distanceB);

    max_linf_distance = 1;
    if (flag_extended) { max_linf_distance = 2; }
    else { max_linf_distance = 1; }


    if (linf_distanceA > max_linf_distance ||
        linf_distanceB > max_linf_distance)
      { return(false); }

    bool flag_connectedA =
      is_unselected_cube_connected_to
      (scalar_grid, isovalue, isovert, cube0, to_cube0, gcube_map);

    bool flag_connectedB =
      is_unselected_cube_connected_to
      (scalar_grid, isovalue, isovert, cube1, to_cube1, gcube_map);

    if (!flag_connectedA && !flag_connectedB) { return(false); }

    if (!check_edges_between_sharp_cubesII
        (scalar_grid, isovalue, cube0, to_cube0, cube1, to_cube1,
         isovert, gcube_map, flag_extended)) {

      if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (manifold). From: ", cube0, 
           " to ", to_cube0, "\n");
        scalar_grid.PrintIndexAndCoord
          (cerr, "                         and ", cube1, 
           " to ", to_cube1, "\n");
      }

      return(false); 
    }

    if (!check_distortionII
        (scalar_grid, isovalue, isovert, gcube_map, 
         cube0, to_cube0, cube1, to_cube1, merge_param)) {

      if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (distortion). From: ", cube0, 
           " to ", to_cube0, "\n");
        scalar_grid.PrintIndexAndCoord
          (cerr, "                         and ", cube1, 
           " to ", to_cube1, "\n");
      }

      return(false);
    }

    if (!check_separating_cubesII
        (scalar_grid, isovalue, cube0, to_cube0, cube1, to_cube1,
         isovert, gcube_map, flag_extended)) 
      { return(false); }

    return(true);
  }

  // Check if three cubes map to to_cube.
  bool check_mapIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    const NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    GRID_COORD_TYPE max_linf_distance;

    for (int i = 0; i < 3; i++) {
      const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index[i]);
      if (gcube_index == ISOVERT::NO_INDEX) { return(false); }
    }

    // Check distances to to_cube
    max_linf_distance = 1;
    if (flag_extended) { max_linf_distance = 2; }

    bool flag_connected = false;
    for (int i = 0; i < 3; i++) {
      GRID_COORD_TYPE linf_distance;

      compute_Linf_distance_between_grid_vertices
        (scalar_grid, cube_index[i], to_cube, linf_distance);
    
      if (linf_distance > max_linf_distance) { return(false); }

      if (is_unselected_cube_connected_to
          (scalar_grid, isovalue, isovert, cube_index[i], to_cube, gcube_map))
        { flag_connected = true; }
    }

    if (!flag_connected) { 

      // *** DEBUG ***
      if (flag_debug) {
        MSDEBUG();
        cerr << "  No cube connected to " << to_cube << endl;
      }

      return(false); 
    }

    if (!check_all_edges_between_sharp_cubesIII
        (scalar_grid, isovalue, cube_index, to_cube, isovert, gcube_map, 
         flag_extended)) {

      if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (manifold). From (III): ", cube_index[0],
           ", ", cube_index[1], " and ", cube_index[2], "\n");
        scalar_grid.PrintIndexAndCoord
          (cerr, "                         to ", to_cube, "\n");
      }

      return(false); 
    }

    if (!check_distortionIII
        (scalar_grid, isovalue, isovert, gcube_map, 
         cube_index, to_cube, merge_param)) {

      if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "Map failed (distortion). From (III): ", cube_index[0],
           ", ", cube_index[1], " and ", cube_index[2], "\n");
        scalar_grid.PrintIndexAndCoord
          (cerr, "                         to ", to_cube, "\n");
      }

      return(false);
    }

    // *** Should add check for separating cubes.

    return(true);
  }

  // Check if ambig pair maps to to_cube.
  bool check_map_ambig_pair
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX adjacent_cube,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    const NUM_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    const NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    const NUM_TYPE adjacent_gcube = isovert.GCubeIndex(adjacent_cube);

    if (!(check_mapII
          (scalar_grid, isovalue, from_cube, to_cube, adjacent_cube, to_cube, 
           isovert, merge_param,gcube_map, flag_extended))) 
      { 
        if (flag_debug) {
          MSDEBUG();
          cerr << "In " << __func__ << ".  Check pair map fails." << endl;
        }

        return(false); 
      }

    IJKDUALTABLE::TABLE_INDEX table_indexA =
      isovert.gcube_list[from_gcube].table_index;
    IJKDUALTABLE::TABLE_INDEX table_indexB =
      isovert.gcube_list[adjacent_gcube].table_index;
    if (isodual_table.NumIsoVertices(table_indexA) > 1 &&
        isodual_table.NumIsoVertices(table_indexB) > 1) {

      if (flag_debug) {
        MSDEBUG();
        cerr << "    Ambig pair map "
             << from_cube << " and " << adjacent_cube
             << " to " << to_cube << " fails." << endl;
      }

      return(false);
    }

    return(true);
  }

  // Check if ambig pair maps to to_cube.
  /// Only map cubes contained in the given region.
  bool check_map_ambig_pair_region
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX adjacent_cube,
   const VERTEX_INDEX to_cube,
   const GRID_BOX & region,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    if (!does_region_contain_cube(region, from_cube, isovert))
      { return(false); }

    check_map_ambig_pair
      (scalar_grid, isodual_table, ambig_info, isovalue, 
       from_cube, adjacent_cube, to_cube, isovert, merge_param,
       gcube_map, flag_extended);
  }

  void check_and_map_ambig_pair
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    VERTEX_INDEX adjacent_cube;
    INDEX_DIFF_TYPE adjacent_gcube;
    bool adjacent_cube_found;

    flag_map = false;

    if (from_gcube == ISOVERT::NO_INDEX) { return; }

    get_cube_sharing_ambiguous_facet
      (scalar_grid, isodual_table, ambig_info, from_cube, isovert, gcube_map,
       adjacent_cube, adjacent_cube_found);
       
    if (!adjacent_cube_found) { return; }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "Checking ambiguous pair ", from_cube, " and ",
         adjacent_cube, " to ", to_cube, "\n");
    }


    adjacent_gcube = isovert.GCubeIndex(adjacent_cube);
    if (adjacent_gcube == ISOVERT::NO_INDEX) { return; }
    if ((gcube_map[from_gcube] != from_gcube) && 
        (gcube_map[adjacent_gcube] == gcube_map[from_gcube]))
      { return; }

    MSDEBUG();
    if (flag_debug) {
      if (gcube_map[from_gcube] != from_gcube ||
          gcube_map[adjacent_gcube] != adjacent_gcube) {

        cerr << "    xxx Cube " << from_cube <<" maps to "
             << isovert.gcube_list[gcube_map[from_gcube]].cube_index
             << " and cube " << adjacent_cube << " maps to "
             << isovert.gcube_list[gcube_map[adjacent_gcube]].cube_index
             << endl;
      }
    }

    if (check_map_ambig_pair
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         from_cube, adjacent_cube, to_cube, isovert, merge_param,
         gcube_map, flag_extended)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     from_gcube, to_gcube, gcube_map);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     adjacent_gcube, to_gcube, gcube_map);

      // *** DEBUG ***
      if (flag_debug) {
        using namespace std;
        cerr << "+++Mapping " << from_cube << " ";
        ijkgrid_output_vertex_coord(cerr, isovert.grid, from_cube);
        cerr << " and " << adjacent_cube << " ";
        ijkgrid_output_vertex_coord(cerr, isovert.grid, adjacent_cube);
        cerr << " to " << to_cube << " ";
        ijkgrid_output_vertex_coord(cerr, isovert.grid, to_cube);
        cerr << "  Flag extended: " << int(flag_extended);
        cerr << endl;
      }

      flag_map = true;
    }
    else {

      // *** DEBUG ***
      if (flag_debug) {
        using namespace std;
        cerr << "    Not mapping ambig pair " << from_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
        cerr << " to " << to_cube << " ";
        ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
      cerr << endl;
      }
    }
  }

  /// Only map cubes contained in the given region.
  void check_and_map_ambig_pair_region
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const GRID_BOX & region,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    VERTEX_INDEX adjacent_cube;
    INDEX_DIFF_TYPE adjacent_gcube;
    bool adjacent_cube_found;

    flag_map = false;

    if (from_gcube == ISOVERT::NO_INDEX) { return; }

    get_cube_sharing_ambiguous_facet
      (isovert.grid, isodual_table, ambig_info, from_cube, isovert, gcube_map,
       adjacent_cube, adjacent_cube_found);
       
    if (!adjacent_cube_found) { return; }

    adjacent_gcube = isovert.GCubeIndex(adjacent_cube);
    if (adjacent_gcube == ISOVERT::NO_INDEX) { return; }
    if ((gcube_map[from_gcube] != from_gcube) && 
        (gcube_map[adjacent_gcube] == gcube_map[from_gcube]))
      { return; }

    if (check_map_ambig_pair_region
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         from_cube, adjacent_cube, to_cube, region, isovert, merge_param,
         gcube_map, flag_extended)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     from_gcube, to_gcube, gcube_map);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     adjacent_gcube, to_gcube, gcube_map);

      // *** DEBUG ***
      /*
      using namespace std;
      cerr << "xxxxxx Mapping " << from_cube << " ";
      ijkgrid_output_vertex_coord(cerr, grid, from_cube);
      cerr << " and " << adjacent_cube << " ";
      ijkgrid_output_vertex_coord(cerr, grid, adjacent_cube);
      cerr << " to " << to_cube << " ";
      ijkgrid_output_vertex_coord(cerr, grid, to_cube);
      cerr << "  Flag extended: " << int(flag_extended);
      cerr << endl;
      */

      flag_map = true;
    }
    else {

      // *** DEBUG ***
      /*
      using namespace std;
      cerr << "*** Not mapping ambig pair " << from_cube << " ";
      ijkgrid_output_vertex_coord(cerr, scalar_grid, from_cube);
      cerr << " to " << to_cube << " ";
      ijkgrid_output_vertex_coord(cerr, scalar_grid, to_cube);
      cerr << endl;
      */
    }
  }

  NUM_TYPE get_num_ambiguous_facet
  (const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const MERGESHARP::ISOVERT & isovert,
   const VERTEX_INDEX cube_index)
  {
    const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index);

    if (gcube_index == ISOVERT::NO_INDEX) { return(false); }

    IJKDUALTABLE::TABLE_INDEX table_index =
      isovert.gcube_list[gcube_index].table_index;

    return(ambig_info.NumAmbiguousFacets(table_index));
  }

  /// Check and map cube0 to to_cube0 and cube1 to to_cube1.
  void check_and_mapII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube0,
   const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX cube1,
   const VERTEX_INDEX to_cube1,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcube0 = isovert.GCubeIndex(cube0);
    const INDEX_DIFF_TYPE gcube1 = isovert.GCubeIndex(cube1);

    flag_map = false;

    if (gcube0 == ISOVERT::NO_INDEX || gcube1 == ISOVERT::NO_INDEX) 
      { return; }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "Checking mapping of ", cube0, " to ", to_cube0, "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "                 and ", cube1, " to " , to_cube1, "\n");
    }

    if (check_mapII
        (scalar_grid, isovalue, cube0, to_cube0, cube1, to_cube1, 
         isovert, merge_param, gcube_map, flag_extended)) {

      NUM_TYPE to_gcube0 = isovert.GCubeIndex(to_cube0);
      NUM_TYPE to_gcube1 = isovert.GCubeIndex(to_cube1);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     gcube0, to_gcube0, gcube_map);
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     gcube1, to_gcube1, gcube_map);

      flag_map = true;

      MSDEBUG();
      if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "Mapping ", cube0, " to ", to_cube0, "");
      scalar_grid.PrintIndexAndCoord
        (cerr, " and ", cube1, " to " , to_cube1, "\n");
      }
    }
    else {
      MSDEBUG();
      if (flag_debug) {
        cerr << "   Pair not mapped.\n";
      }
    }
  }

  /// Check and map three cubes to cube_index.
  void check_and_mapIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    INDEX_DIFF_TYPE gcube_index[3];
    flag_map = false;

    for (int i = 0; i < 3; i++) {
      gcube_index[i] = isovert.GCubeIndex(cube_index[i]);
      if (gcube_index[i] == ISOVERT::NO_INDEX) 
        { return; }
    }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "Checking mapping (III) of ", cube_index[0], ", ", 
         cube_index[1], " and ", cube_index[2], "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "                 to cube ", to_cube, "\n");
    }

    if (check_mapIII
        (scalar_grid, isovalue, cube_index, to_cube, isovert, merge_param,
         gcube_map, flag_extended)) {

      NUM_TYPE to_gcube = isovert.GCubeIndex(to_cube);
      for (int i = 0; i < 3; i++) {
        map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                       gcube_index[i], to_gcube, gcube_map);
      }

      flag_map = true;

      MSDEBUG();
      if (flag_debug) {
        scalar_grid.PrintIndexAndCoord
          (cerr, "Mapping three cubes ", cube_index[0], ", ", cube_index[1], 
           " and ",  cube_index[2], "\n");
        scalar_grid.PrintIndexAndCoord
          (cerr, "      to " , to_cube, "\n");
      }
    }
    else {
      MSDEBUG();
      if (flag_debug) {
        cerr << "   Triplet not mapped.\n";
      }
    }
  }

  /// Check and map cube0, cube1 and cube2 to_cube
  void check_and_mapIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index0,
   const VERTEX_INDEX cube_index1,
   const VERTEX_INDEX cube_index2,
   const VERTEX_INDEX to_cube,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const VERTEX_INDEX cube_index[3] = 
      { cube_index0, cube_index1, cube_index2 };

    check_and_mapIII(scalar_grid, isovalue, cube_index, to_cube, isovert,
                     flag_extended, merge_param, gcube_map, flag_map);
  }

  /// Return true if cube is active, unmapped and unselected
  inline bool is_cube_unmapped_unselected_active
  (const VERTEX_INDEX cube_index,
   const MERGESHARP::ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index);

    if (gcube_index == ISOVERT::NO_INDEX) { return(false); }
    if (isovert.gcube_list[gcube_index].flag == SELECTED_GCUBE) 
      { return(false); }
    if (gcube_map[gcube_index] != gcube_index) { return(false); }

    return(true);
  }

  /// Map cube0 to to_cube0 and adjacent cube to some other cube.
  void map_adjacent_cubes_to_different
  (const VERTEX_INDEX to_cube0,
   const VERTEX_INDEX cube0,
   const MERGESHARP::ISOVERT & isovert,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcube0 = isovert.GCubeIndex(cube0);
    BOUNDARY_BITS_TYPE boundary_bits;

    flag_map = false;

    if (gcube0 == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcube0] != gcube0) { return; }

    // Check that if cube0 has multiple isosurface vertices,
    //   then any connecting adjacent cubes map to to_cube0.
    if (!check_cubes_with_multi_isov
        (scalar_grid, isodual_table, ambig_info, isovalue,
         cube0, to_cube0, isovert, gcube_map))
      { return; }

    boundary_bits = isovert.gcube_list[gcube0].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cube0 is an interior cube.

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

        const VERTEX_INDEX cube1 = isovert.grid.CubeNeighborF(cube0, j);
        INDEX_DIFF_TYPE gcube1 = isovert.GCubeIndex(cube1);

        if (gcube1 == ISOVERT::NO_INDEX) { continue; }
        if (isovert.gcube_list[gcube1].flag == SELECTED_GCUBE) { continue; }
        if (gcube_map[gcube1] != gcube1) { continue; }

        const VERTEX_INDEX to_cube1 = isovert.gcube_list[gcube1].covered_by;
        if (to_cube1 == cube1) { continue; }  // Not covered.
        if (to_cube1 == to_cube0) { continue; }  // Not different to_cube.

        // Check that if cube1 has multiple isosurface vertices,
        //   then any connecting adjacent cubes map to to_cube.
        if (!check_cubes_with_multi_isov
            (scalar_grid, isodual_table, ambig_info, isovalue,
             cube1, to_cube1, isovert, gcube_map))
          { continue; }

        check_and_mapII
          (scalar_grid, isovalue, cube0, to_cube0, cube1, to_cube1, 
           isovert, flag_extended, merge_param, gcube_map, flag_map);

        if (flag_map) { return; }
      }
    }
    else {
      // Handle boundary case
    }
  }

  /// Map cube pairs (cube0,cube1) to cube to_cube 
  ///   where cube1 is facet adjacent to cube0.
  void map_facet_adjacent_pairs
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX cube0,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcube0 = isovert.GCubeIndex(cube0);
    BOUNDARY_BITS_TYPE boundary_bits;

    flag_map = false;

    if (gcube0 == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcube0] != gcube0) { return; }

    // Check that if cube0 has multiple isosurface vertices,
    //   then any connecting adjacent cubes map to to_cube.
    if (!check_cubes_with_multi_isov
        (scalar_grid, isodual_table, ambig_info, isovalue,
         cube0, to_cube, isovert, gcube_map))
      { return; }

    boundary_bits = isovert.gcube_list[gcube0].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cube0 is an interior cube.

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

        const VERTEX_INDEX cube1 = isovert.grid.CubeNeighborF(cube0, j);
        if (!is_cube_unmapped_unselected_active(cube1, isovert, gcube_map))
          { continue; }

        // Check that if cube1 has multiple isosurface vertices,
        //   then any connecting adjacent cubes map to to_cube.
        if (!check_cubes_with_multi_isov
            (scalar_grid, isodual_table, ambig_info, isovalue,
             cube1, to_cube, isovert, gcube_map))
          { continue; }

        check_and_mapII
          (scalar_grid, isovalue, cube0, to_cube, cube1, to_cube, isovert,
           flag_extended, merge_param, gcube_map, flag_map);
        if (flag_map) { return; }
      }
    }
    else {
      // Handle boundary case
    }
  }

  /// Map cube pairs (cube0,cube1) to cube to_cube 
  ///   where cube1 is edge adjacent to cube0.
  void map_edge_adjacent_pairs
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX cube0,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcube0 = isovert.GCubeIndex(cube0);
    BOUNDARY_BITS_TYPE boundary_bits;

    flag_map = false;

    if (gcube0 == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcube0] != gcube0) { return; }

    // Check that if cube0 has multiple isosurface vertices,
    //   then any connecting adjacent cubes map to to_cube.
    if (!check_cubes_with_multi_isov
        (scalar_grid, isodual_table, ambig_info, isovalue,
         cube0, to_cube, isovert, gcube_map))
      { return; }

    boundary_bits = isovert.gcube_list[gcube0].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cube0 is an interior cube.

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) {

        const VERTEX_INDEX cube1 = isovert.grid.CubeNeighborE(cube0, j);
        if (!is_cube_unmapped_unselected_active(cube1, isovert, gcube_map))
          { continue; }

        // Check that if cube1 has multiple isosurface vertices,
        //   then any connecting adjacent cubes map to to_cube.
        if (!check_cubes_with_multi_isov
            (scalar_grid, isodual_table, ambig_info, isovalue,
             cube1, to_cube, isovert, gcube_map))
          { continue; }

        check_and_mapII
          (scalar_grid, isovalue, cube0, to_cube, cube1, to_cube, isovert,
           flag_extended, merge_param, gcube_map, flag_map);
        if (flag_map) { return; }
      }
    }
    else {
      // Handle boundary case
    }
  }

  /// Map cubes around an edge of cube0 to cube to_cube 
  void map_cubes_around_edge
  (const VERTEX_INDEX to_cube,
   const VERTEX_INDEX cube0_index,
   const MERGESHARP::ISOVERT & isovert,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const bool flag_extended,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);
    BOUNDARY_BITS_TYPE boundary_bits;

    flag_map = false;

    if (gcube0_index == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcube0_index] != gcube0_index) { return; }

    // Check that if cube0 has multiple isosurface vertices,
    //   then any connecting adjacent cubes map to to_cube.
    if (!check_cubes_with_multi_isov
        (scalar_grid, isodual_table, ambig_info, isovalue,
         cube0_index, to_cube, isovert, gcube_map))
      { return; }

    boundary_bits = isovert.gcube_list[gcube0_index].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cube0 is an interior cube.

      for (int d = 0; d < DIM3; d++) {

        const int d1 = (d+1) % DIM3;
        const int d2 = (d+2) % DIM3;

        for (int j1 = 0; j1 < 2; j1++) {

          const VERTEX_INDEX cube1_index =
            isovert.grid.AdjacentVertex(cube0_index, d1, j1);

          for (int j2 = 0; j2 < 2; j2++) {

            const VERTEX_INDEX cube2_index =
              isovert.grid.AdjacentVertex(cube1_index, d2, j2);
            const VERTEX_INDEX cube3_index =
              isovert.grid.AdjacentVertex(cube0_index, d2, j2);
        
            if (!is_cube_unmapped_unselected_active
                (cube2_index, isovert, gcube_map)) { continue; }

            // Check that if cube2 has multiple isosurface vertices,
            //   then any connecting adjacent cubes map to to_cube.
            if (!check_cubes_with_multi_isov
                (scalar_grid, isodual_table, ambig_info, isovalue,
                 cube2_index, to_cube, isovert, gcube_map))
              { continue; }

            if (is_cube_unmapped_unselected_active
                (cube1_index, isovert, gcube_map)) { 

              if (check_cubes_with_multi_isov
                  (scalar_grid, isodual_table, ambig_info, isovalue,
                   cube1_index, to_cube, isovert, gcube_map))

                check_and_mapIII
                  (scalar_grid, isovalue, cube0_index, cube1_index, cube2_index,
                   to_cube, isovert, flag_extended, merge_param,
                   gcube_map, flag_map);
              if (flag_map) { return; }
            }


            if (is_cube_unmapped_unselected_active
                (cube3_index, isovert, gcube_map)) { 

              if (check_cubes_with_multi_isov
                  (scalar_grid, isodual_table, ambig_info, isovalue,
                   cube3_index, to_cube, isovert, gcube_map))

                check_and_mapIII
                  (scalar_grid, isovalue, cube0_index, cube3_index, cube2_index,
                   to_cube, isovert, flag_extended, merge_param,
                   gcube_map, flag_map);
              if (flag_map) { return; }
            }
          }
        }
      }
    }
    else {
      // Handle boundary case
    }
  }

}



//****************************************
// Check if vertices are connected
//****************************************

namespace {

	/// Compute the overlap region between two cube indices
  /// @param dist2boundary Distance from cube to region boundary
  //         region size = (2*dist2boundary+1)
	bool find_overlap(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const VERTEX_INDEX & cube_index1,
		const VERTEX_INDEX & cube_index2,
    const GRID_COORD_TYPE dist2boundary,
		GRID_COORD_TYPE rmin[],
		GRID_COORD_TYPE rmax[])
	{
		COORD_TYPE coord1[DIM3], coord2[DIM3];
		scalar_grid.ComputeCoord(cube_index1, coord1);
		scalar_grid.ComputeCoord(cube_index2, coord2);
		for (int d=0;d<DIM3;d++){
			rmin[d] = std::max(coord1[d]-dist2boundary, coord2[d]-dist2boundary);
      rmin[d] = std::max(rmin[d], 0);
			rmax[d] = std::min(coord1[d]+dist2boundary+1, coord2[d]+dist2boundary+1);
      rmax[d] = std::min(rmax[d], scalar_grid.AxisSize(d)-1);
		}

		// track the dimension of the tracked,
		// if the tracked regions has at least 2 dimension
		int dim_of_overlap=0;
		for (int d=0;d<DIM3;d++)
		{
			if(rmin[d] > rmax[d])
			{ return false; }
			if(rmin[d] < rmax[d])
				dim_of_overlap++;
		}
		if (dim_of_overlap>=2)
			return true;
		else
			return false;
	}

	bool find_overlap(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const VERTEX_INDEX & cube_index1,
		const VERTEX_INDEX & cube_index2,
		GRID_COORD_TYPE rmin[],
		GRID_COORD_TYPE rmax[])
  {
    return(find_overlap
           (scalar_grid, cube_index1, cube_index2, 1, rmin, rmax));
  }

	// Return true if two cube-indices are connected
  // INEXACT:  MAY RETURN true even if vertices are not connected.
	bool are_connected 
		(const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const VERTEX_INDEX & cube_index1,
		const VERTEX_INDEX & cube_index2,
		const SCALAR_TYPE isovalue )
	{
		// find the overlap region
		GRID_COORD_TYPE rmin[DIM3], rmax[DIM3];

		bool is_overlap =
			find_overlap(scalar_grid, cube_index1, cube_index2, rmin, rmax);
    

		if (!is_overlap) { return false; }

		COORD_TYPE vbase = scalar_grid.ComputeVertexIndex(rmin);
		VERTEX_INDEX vnext;
		VERTEX_INDEX v0,v1,v2;

		int d0=0,d1=0,d2=0;
		bool is_intersect=false;

		// if there is an overlap
		for(int d0=0;d0<DIM3;d0++){
			d1 = (d0+1)%3;
			d2 = (d0+2)%3;
			v1 = vbase;
			for (int i1=rmin[d1]; i1 <=rmax[d1]; i1++){
				v2=v1;
				for (int i2=rmin[d2];i2<=rmax[d2]; i2++){
					v0=v2;
					for(int i0=rmin[d0]; i0<rmax[d0]; i0++){
						vnext = scalar_grid.NextVertex(v0,d0);

            if (is_gt_min_le_max(scalar_grid, v0, vnext, isovalue))
              { return(true); }

						v0=vnext;
					}
					v2=scalar_grid.NextVertex(v2,d2);
				}
				v1=scalar_grid.NextVertex(v1,d1);
			}
		}

		return false;
	}


	// Return true if two cube-indices are connected
	bool are_connected_by_iso_edge
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube_index1,
   const VERTEX_INDEX & cube_index2,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const int dist2boundary = 2;  // Distance from cube to region boundary
    const NUM_TYPE gcube_index1 = isovert.GCubeIndex(cube_index1);
    const NUM_TYPE gcube_index2 = isovert.GCubeIndex(cube_index2);

		GRID_COORD_TYPE rmin[DIM3], rmax[DIM3];

		// find the overlap region
		bool is_overlap =
			find_overlap(scalar_grid, cube_index1, cube_index2, 
                   dist2boundary, rmin, rmax);

		if (!is_overlap) { return false; }

		VERTEX_INDEX vbase = scalar_grid.ComputeVertexIndex(rmin);
		VERTEX_INDEX vnext;
		VERTEX_INDEX v0,v1,v2;

		int d0=0,d1=0,d2=0;
		bool is_intersect=false;

		int num=0;
		// if there is an overlap
		for(int d0=0;d0<DIM3;d0++){
			d1 = (d0+1)%3;
			d2 = (d0+2)%3;
			v1 = vbase;
			for (int i1=rmin[d1]; i1 < rmax[d1];i1++){
				v2=v1;
				for (int i2=rmin[d2]; i2 < rmax[d2];i2++){
					v0=v2;
					for(int i0=rmin[d0]; i0 < rmax[d0];i0++){
						vnext = scalar_grid.NextVertex(v0,d0);

            VERTEX_INDEX gcube_v0 = isovert.GCubeIndex(v0);
            VERTEX_INDEX gcube_vnext = isovert.GCubeIndex(vnext);
            if (gcube_v0 != ISOVERT::NO_INDEX && 
                gcube_vnext != ISOVERT::NO_INDEX) {

              if ((gcube_map[gcube_v0] == gcube_index1 &&
                   gcube_map[gcube_vnext] == gcube_index2) ||
                  (gcube_map[gcube_v0] == gcube_index2 &&
                   gcube_map[gcube_vnext] == gcube_index1)) {
                // Assume some isosurface edge is dual to the facet
                //   between cube_index1 and cube_index2
                return(true);
              }
            }

            v0=vnext;
					}
					v2=scalar_grid.NextVertex(v2,d2);
				}
				v1=scalar_grid.NextVertex(v1,d1);
			}
		}

		return false;
  }


	// Return true if two cubes are connected by an isosurface quadrilaterals.
	bool are_connected_by_iso_quad
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube0_index,
   const VERTEX_INDEX & cube1_index,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const bool flag_extended)
  {
    int dist2boundary;      // Distance from cube to region boundary
		GRID_COORD_TYPE rmin[DIM3], rmax[DIM3];

    if (flag_extended) 
      { dist2boundary = 2; }
    else
      { dist2boundary = 1; }

		// find the overlap region
		bool is_overlap =
			find_overlap(scalar_grid, cube0_index, cube1_index, 
                   dist2boundary, rmin, rmax);

		if (!is_overlap) { return false; }

		VERTEX_INDEX vbase = scalar_grid.ComputeVertexIndex(rmin);

    for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

      int d1 = (edge_dir+1)%DIM3;
      int d2 = (edge_dir+2)%DIM3;

      VERTEX_INDEX v0 = vbase;
      for (GRID_COORD_TYPE x0 = rmin[edge_dir]; x0 < rmax[edge_dir]; x0++) {

        VERTEX_INDEX v1 = v0;
        for (GRID_COORD_TYPE x1 = rmin[d1]; x1 <= rmax[d1]; x1++) {

          if (x1 == 0 || x1 >= scalar_grid.AxisSize(d1)) { continue; }

          VERTEX_INDEX v2 = v1;
          for (GRID_COORD_TYPE x2 = rmin[d2]; x2 <= rmax[d2]; x2++) {

            if (x2 == 0 || x2 >= scalar_grid.AxisSize(d2)) { continue; }

            const VERTEX_INDEX iend0 = v2;
            const VERTEX_INDEX iend1 = scalar_grid.NextVertex(iend0, edge_dir);

            if (is_gt_min_le_max(scalar_grid, iend0, iend1, isovalue)) {

              bool flag_maps_to_cube0;
              bool flag_maps_to_cube1;

              determine_quad_map_to_cubes
                (iend0, edge_dir, cube0_index, cube1_index,
                 isovert, gcube_map, flag_maps_to_cube0, flag_maps_to_cube1);

              if (flag_maps_to_cube0 && flag_maps_to_cube1) {
                return(true); 
              }
            }

            v2 = scalar_grid.NextVertex(v2, d2);
          }

          v1 = scalar_grid.NextVertex(v1, d1);
        }

        v0 = scalar_grid.NextVertex(v0, edge_dir);
      }
    }

    return(false);
  }


  // Return true if unselected cube is connected to cube to_cube.
  bool is_unselected_cube_connected_to
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const VERTEX_INDEX & cube0_index,
   const VERTEX_INDEX & to_cube,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    BOUNDARY_BITS_TYPE boundary_bits;
    INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);

    if (to_gcube == ISOVERT::NO_INDEX) { return(false); }
    if (gcube0_index == ISOVERT::NO_INDEX) { return(false); }
    boundary_bits = isovert.gcube_list[gcube0_index].boundary_bits;
				
    // Check if some cube adjacent to cube0_index maps to to_cube
		if (boundary_bits == 0) {

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {
        VERTEX_INDEX cube1_index = isovert.grid.CubeNeighborF(cube0_index, j);
        INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);

        if (gcube1_index != ISOVERT::NO_INDEX) {

          // Note: Assumes j'th facet neighbor shares j'th facet with cube.
          if (is_gt_facet_min_le_facet_max
              (scalar_grid, cube0_index, j, isovalue)) {
            if (gcube_map[gcube1_index] == to_gcube) { return(true); }
          }
        }
      }
    }
    else {
      // Boundary case.

      for (int d = 0; d < isovert.grid.Dimension(); d++) {
        for (int j = 0; j < 2; j++) {
          BOUNDARY_BITS_TYPE mask = (BOUNDARY_BITS_TYPE(1) << (2*d+j));

          if ((mask & boundary_bits) == 0) {
            VERTEX_INDEX cube1_index = 
              isovert.grid.AdjacentVertex(cube0_index, d, j);
            INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);

            if (gcube1_index != ISOVERT::NO_INDEX) {

              // Note: Assumes j'th facet neighbor shares j'th facet with cube.
              if (is_gt_facet_min_le_facet_max
                  (scalar_grid, cube0_index, j, isovalue)) {
                if (gcube_map[gcube1_index] == to_gcube) { return(true); }
              }
            }
          }

        }
      }
    }

    return(false);
  }

	// Return true if k'th cube edge neighbor is connected by an iso-edge.
	bool is_cube_edge_neighbor_connected_by_iso_edge
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX & cube0_index,
   const NUM_TYPE k,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    VERTEX_INDEX cube1_index;

    cube1_index = isovert.grid.CubeNeighborE(cube0_index, k);
    NUM_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);

    if (gcube1_index == ISOVERT::NO_INDEX) { return(false); }
				
    // Check if edge is bipolar
    // Note: Computation of (iv0,iv1) relies on specific ordering
    //   of cube edge neighbors.
    int edge_dir = int(k/isovert.grid.NumFacetVertices());
    int k2 = k%isovert.grid.NumFacetVertices();
    VERTEX_INDEX iv0 = isovert.grid.FacetVertex(cube0_index, edge_dir, k2);
    VERTEX_INDEX iv1 = isovert.grid.NextVertex(iv0, edge_dir);

    if (is_gt_min_le_max(scalar_grid, iv0, iv1, isovalue)) 
      { return(true); }

    bool result =
      is_unselected_cube_connected_to
      (scalar_grid, isovalue, isovert, cube1_index, cube0_index, gcube_map);

    return(result);
  }

}

// ********************************************************
// Extend mapping of isosurface vertices from corner cubes
//   Single vertex per cube
// ********************************************************

namespace {

  // forward declaraions
	void check_face_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
		MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);
	void check_edge_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
		MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);
	void check_vertex_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
		MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);
	void check_extended_corner_cube_and_map
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map);


  /// Extend mapping of isosurface vertices to corner cube.
  /// @param extend_from Extend mapping from cube extend_from_cube.
  /// @param to_cube Extend mapping to cube to_cube.
	void extend_mapping_near_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX extend_from_cube,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE extend_from_gcube = isovert.GCubeIndex(extend_from_cube);
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);

    if (extend_from_gcube == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[extend_from_gcube] != to_gcube) { return; }
    if (isovert.NumEigenvalues(to_gcube) != 3) { return; }

    GRID_CUBE_FLAG gcube_flag = isovert.gcube_list[extend_from_gcube].flag;

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      cerr << "In " << __func__;
      scalar_grid.PrintIndexAndCoord
        (cerr, "  Extending from: ", extend_from_cube, "\n");
    }

    if (gcube_flag == COVERED_A_GCUBE || gcube_flag == COVERED_CORNER_GCUBE ) {

      if (isovert.gcube_list[extend_from_gcube].boundary_bits == 0) {

        // *** DEBUG ***
        using namespace std;
        if (flag_debug) { cerr << "  Checking face neighbors." << endl; }
        check_face_neighbors_near_corner_cube
          (scalar_grid, isovalue, extend_from_gcube, isovert, gcube_map);

        // *** DEBUG ***
        using namespace std;
        if (flag_debug) { cerr << "  Checking edge neighbors." << endl; }
        check_edge_neighbors_near_corner_cube
          (scalar_grid, isovalue, extend_from_gcube, isovert, gcube_map);


        // *** DEBUG ***
        using namespace std;
        if (flag_debug) { cerr << "  Checking vertex neighbors." << endl; }
        check_vertex_neighbors_near_corner_cube
          (scalar_grid, isovalue, extend_from_gcube, isovert, gcube_map);
      }
    }

  }

  /// Extend mapping of isosurface vertices
	void extend_mapping_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const int bin_width = merge_param.bin_width;
    std::vector<NUM_TYPE> sharp_gcube_list;
    BIN_GRID<VERTEX_INDEX> bin_grid;

    // *** DEBUG ***
    using namespace std;
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

		get_corner_or_edge_cubes(isovert.gcube_list, sharp_gcube_list);

    init_bin_grid(scalar_grid, bin_width, bin_grid);
    insert_selected_in_bin_grid
      (scalar_grid, isovert, sharp_gcube_list, bin_width, bin_grid);

    for (int i=0; i < sharp_gcube_list.size(); i++) {

      NUM_TYPE gcube_index = sharp_gcube_list[i];

      if (isovert.gcube_list[gcube_index].flag == SELECTED_GCUBE) {
        if (isovert.NumEigenvalues(gcube_index) == 3) {
          VERTEX_INDEX corner_cube_index = isovert.CubeIndex(gcube_index);

          extend_mapping_corner_cube
            (scalar_grid, bin_grid, bin_width, isovalue,
             corner_cube_index, isovert, gcube_map);
        }
      }
    }
  }

  // Return number of coordinates which differ by exactly two.
  int count_diff_two
  (const GRID_COORD_TYPE coord0[DIM3], const GRID_COORD_TYPE coord1[DIM3])
  {
    int num_diff_two = 0;

    for (int d = 0; d < DIM3; d++) {
      if (coord0[d]+2 == coord1[d] || coord0[d] == coord1[d]+2)
        { num_diff_two++; }
    }

    return(num_diff_two);
  }

  /// Construct extended region around corner cube.
  /// Note: Region is given by cube indices, not vertex coord.
  void construct_extended_corner_cube_region
  (const SHARPISO_GRID & grid,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const ISOVERT & isovert,
   const VERTEX_INDEX corner_cube_index,
   GRID_BOX & region)
  {
    const INDEX_DIFF_TYPE corner_gcube_index = 
      isovert.GCubeIndex(corner_cube_index);
    std::vector<VERTEX_INDEX> selected_list;
    GRID_COORD_TYPE Linf_distance;
    IJK::PROCEDURE_ERROR error("construct_extended_corner_cube_region");

    if (region.Dimension() != DIM3) {
      error.AddMessage("Programming error. Region dimension is not 3.");
      throw error;
    }

    if (corner_gcube_index == ISOVERT::NO_INDEX) {
      error.AddMessage
        ("Programming error.  Cube ", corner_cube_index, " is not active.");
      throw error;
    }

    const GRID_COORD_TYPE * corner_cube_coord = 
      isovert.gcube_list[corner_gcube_index].cube_coord;

    // get the selected vertices around corner_cube
    get_selected(grid, corner_cube_index, bin_grid, bin_width, selected_list);

    region.SetCoord(corner_cube_coord, corner_cube_coord);
    for (int d = 0; d < DIM3; d++) {
      if (corner_cube_coord[d] > 0) 
        { region.SetMinCoord(d, corner_cube_coord[d]-1); }
      if (corner_cube_coord[d]+2 < grid.AxisSize(d))
        { region.SetMaxCoord(d, corner_cube_coord[d]+1); }
    }

    // Extend region.
    for (int i = 0; i < selected_list.size(); i++) {
      VERTEX_INDEX selected_cube_index = selected_list[i];
      INDEX_DIFF_TYPE selected_gcube_index = 
        isovert.GCubeIndex(selected_cube_index);

      if (selected_gcube_index == ISOVERT::NO_INDEX) { 
        error.AddMessage
          ("Programming error.  Selected cube ", selected_cube_index,
           " is not active.");
        throw error;
      }

      const GRID_COORD_TYPE * selected_cube_coord =
        isovert.gcube_list[selected_gcube_index].cube_coord;

      IJK::compute_Linf_distance
        (DIM3, corner_cube_coord, selected_cube_coord, Linf_distance);
      if (Linf_distance != 3) { continue; }

      for (int d = 0; d < DIM3; d++) {
        if ((selected_cube_coord[d]+3) == corner_cube_coord[d])
          { region.SetMinCoord(d, selected_cube_coord[d]+1); }

        if ((corner_cube_coord[d]+3) == selected_cube_coord[d])
          { region.SetMaxCoord(d, corner_cube_coord[d]+2); }
      }
    }

    // Contract region.
    for (int i = 0; i < selected_list.size(); i++) {

      VERTEX_INDEX selected_cube_index = selected_list[i];
      INDEX_DIFF_TYPE selected_gcube_index = 
        isovert.GCubeIndex(selected_cube_index);

      if (selected_gcube_index == ISOVERT::NO_INDEX) { 
        error.AddMessage
          ("Programming error.  Selected cube ", selected_cube_index,
           " is not active.");
        throw error;
      }

      const GRID_COORD_TYPE * selected_cube_coord =
        isovert.gcube_list[selected_gcube_index].cube_coord;

      IJK::compute_Linf_distance
        (DIM3, corner_cube_coord, selected_cube_coord, Linf_distance);
      if (Linf_distance >= 3) { continue; }

      int num_diff_two = 
        count_diff_two(corner_cube_coord, selected_cube_coord);

      if (num_diff_two >= 2) { continue; }

      for (int d = 0; d < DIM3; d++) {
        if ((selected_cube_coord[d]+2) == corner_cube_coord[d])
          { region.SetMinCoord(d, selected_cube_coord[d]+1); }

        if ((corner_cube_coord[d]+2) == selected_cube_coord[d])
          { region.SetMaxCoord(d, corner_cube_coord[d]+1); }
      }
    }
  }

  /// Extend mapping of isosurface vertices to corner cube.
	void extend_mapping_corner_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE corner_gcube_index = 
      isovert.GCubeIndex(corner_cube_index);
    GRID_BOX region(DIM3);

    if (corner_gcube_index == ISOVERT::NO_INDEX) { return; }

    const GRID_COORD_TYPE * corner_cube_coord = 
      isovert.gcube_list[corner_gcube_index].cube_coord;

    construct_extended_corner_cube_region
      (isovert.grid, bin_grid, bin_width, isovert, corner_cube_index, region);

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      cerr << endl << "In " << __func__;
      scalar_grid.PrintIndexAndCoord
        (cerr, "  Corner cube: ", corner_cube_index, "\n");
      cerr << "  Region: ";
      IJK::print_coord3D(cerr, region.MinCoord());
      IJK::print_coord3D(cerr, region.MaxCoord());
      cerr << endl;
    }

    for (int d = 0; d < DIM3; d++) {

      if (corner_cube_coord[d]+2 == region.MaxCoord(d)) {
        VERTEX_INDEX neighbor_cube_index = 
          corner_cube_index + scalar_grid.AxisIncrement(d);

        extend_mapping_near_corner_cube
          (scalar_grid, isovalue, neighbor_cube_index,
           corner_cube_index, isovert, gcube_map);
      }

      if (region.MinCoord(d)+2 == corner_cube_coord[d]) {
        VERTEX_INDEX neighbor_cube_index = 
          corner_cube_index - scalar_grid.AxisIncrement(d);

        extend_mapping_near_corner_cube
          (scalar_grid, isovalue, neighbor_cube_index,
           corner_cube_index, isovert, gcube_map);
      }
    }

    // Try again.
    for (int d = 0; d < DIM3; d++) {

      if (corner_cube_coord[d]+2 == region.MaxCoord(d)) {
        VERTEX_INDEX neighbor_cube_index = 
          corner_cube_index + scalar_grid.AxisIncrement(d);

        extend_mapping_near_corner_cube
          (scalar_grid, isovalue, neighbor_cube_index,
           corner_cube_index, isovert, gcube_map);
      }

      if (region.MinCoord(d)+2 == corner_cube_coord[d]) {
        VERTEX_INDEX neighbor_cube_index = 
          corner_cube_index - scalar_grid.AxisIncrement(d);

        extend_mapping_near_corner_cube
          (scalar_grid, isovalue, neighbor_cube_index,
           corner_cube_index, isovert, gcube_map);
      }
    }

  }

	/*
	* Check FACE neighbors
	* @param covered_gcube_index, gcube_index of the sharp covered cube
	* @param covered_cube_index, cube index of the sharp covered cube
	*/
	void check_face_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
    MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		VERTEX_INDEX covered_cube_index= 
			isovert.gcube_list[covered_gcube_index].cube_index;
    NUM_TYPE to_gcube = gcube_map[covered_gcube_index];
    VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
    bool flag_map;

		VERTEX_INDEX neighbor_cube_index;
		for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) 
		{
			neighbor_cube_index = isovert.grid.CubeNeighborF(covered_cube_index, j);

      check_extended_corner_cube_and_map
        (scalar_grid, isovalue, neighbor_cube_index, 
         to_cube, isovert, gcube_map, flag_map);
		}
	}

	/*
	* Check EDGE neighbors
	* @param covered_gcube_index, gcube_index of the sharp covered cube
	* @param covered_cube_index, cube index of the sharp covered cube
	*/
	void check_edge_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
    MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		VERTEX_INDEX covered_cube_index= 
			isovert.gcube_list[covered_gcube_index].cube_index;
    NUM_TYPE to_gcube = gcube_map[covered_gcube_index];
    VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
    bool flag_map;

		VERTEX_INDEX neighbor_cube_index;
		for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) 
		{
			neighbor_cube_index = isovert.grid.CubeNeighborE(covered_cube_index, j);

      check_extended_corner_cube_and_map
        (scalar_grid, isovalue, neighbor_cube_index, 
         to_cube, isovert, gcube_map, flag_map);
		}
	}

	/*
	* Check VERTEX neighbors
	* @param covered_gcube_index, gcube_index of the sharp covered cube
	* @param covered_cube_index, cube index of the sharp covered cube
	*/
	void check_vertex_neighbors_near_corner_cube(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX covered_gcube_index,
		MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		VERTEX_INDEX covered_cube_index= 
			isovert.gcube_list[covered_gcube_index].cube_index;
    NUM_TYPE to_gcube = gcube_map[covered_gcube_index];
    VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
    bool flag_map;

		VERTEX_INDEX neighbor_cube_index;
		for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsV(); j++) 
		{
			neighbor_cube_index = isovert.grid.CubeNeighborV(covered_cube_index, j);

      // *** DEBUG ***
      if (flag_debug) {
        MSDEBUG();
        scalar_grid.PrintIndexAndCoord
          (cerr, "  Checking ", neighbor_cube_index, "\n");
      }

      check_extended_corner_cube_and_map
        (scalar_grid, isovalue, neighbor_cube_index, 
         to_cube, isovert, gcube_map, flag_map);
		}
	}

  /// Check extended mapping from corner_cube and map from cube from_cube 
  ///   to cube to_cube.
	void check_extended_corner_cube_and_map
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX from_cube,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    static CUBE_CONNECTED_ARRAY connected_sharp;

    flag_map = false;

    if (from_gcube == ISOVERT::NO_INDEX ||
        to_gcube == ISOVERT::NO_INDEX) { return; }

    GRID_CUBE_FLAG from_gcube_flag = isovert.gcube_list[from_gcube].flag;


    if (from_gcube_flag == SELECTED_GCUBE || 
        from_gcube_flag == COVERED_CORNER_GCUBE)
      { return; }

    if (gcube_map[from_gcube] != from_gcube) { return; }

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "*** Checking mapping from ", from_cube, " to " , to_cube, "\n");
    }

    if (!is_corner_cube_merge_permitted
        (scalar_grid, isovert, from_gcube, to_gcube, gcube_map))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed is_corner_cube_merge_permitted." << endl; }

        return; }

    if (!check_map(scalar_grid, isovalue, from_cube, to_cube,
                   isovert, gcube_map, true))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_map." << endl; }

        return; }

		find_connected_sharp(scalar_grid, isovalue, from_cube, isovert, 
                         gcube_map, connected_sharp);

    if (!check_adjacent_cubes(scalar_grid, isovalue, from_cube,
                              isovert, gcube_map, connected_sharp))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_adjacent_cubes." << endl; }
        
        return; }

    if (!check_connected_sharp
        (scalar_grid, isovalue, isovert, gcube_map, from_cube, to_cube,
         connected_sharp))
      { 
        // *** DEBUG ***
        if (flag_debug) {
          scalar_grid.PrintIndexAndCoord
            (cerr, "*** Failed check_connected_sharp. Cube: ", from_cube, "");
          scalar_grid.PrintIndexAndCoord
            (cerr, " to ", to_cube, "\n");
        }

        return; }

    map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                   from_gcube, to_gcube, gcube_map);
    isovert.gcube_list[from_gcube].flag = COVERED_B_GCUBE;

    flag_map = true;
	}

}


// ********************************************************
// Extend mapping of isosurface vertices from corner cubes
//   Allow multiple vertices per cube
// ********************************************************

namespace {

  // forward declaraion
	void extend_mapping_corner_region_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   const MERGESHARP::MERGE_PARAM & merge_param,
   const GRID_BOX & region,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);
	void extend_mapping_corner_cube_across_facets_multi
  ( const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
    const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
    const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX extend_from_cube,
    const GRID_BOX & region,
    const MERGESHARP::MERGE_PARAM & merge_param,
    MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);


  /// Extend mapping of isosurface vertices
	void extend_mapping_corner_cube_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const int bin_width = merge_param.bin_width;
    std::vector<NUM_TYPE> sharp_gcube_list;
    BIN_GRID<VERTEX_INDEX> bin_grid;

    // *** DEBUG ***
    using namespace std;
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

		get_corner_or_edge_cubes(isovert.gcube_list, sharp_gcube_list);

    init_bin_grid(scalar_grid, bin_width, bin_grid);
    insert_selected_in_bin_grid
      (scalar_grid, isovert, sharp_gcube_list, bin_width, bin_grid);

    for (int i=0; i < sharp_gcube_list.size(); i++) {

      NUM_TYPE gcube_index = sharp_gcube_list[i];

      if (isovert.gcube_list[gcube_index].flag == SELECTED_GCUBE) {
        if (isovert.NumEigenvalues(gcube_index) == 3) {
          VERTEX_INDEX corner_cube_index = isovert.CubeIndex(gcube_index);

          extend_mapping_corner_cube_multi
            (scalar_grid, isodual_table, ambig_info, 
             bin_grid, bin_width, isovalue, corner_cube_index, merge_param,
             isovert, gcube_map);
        }
      }
    }
  }

  /// Extend mapping of isosurface vertices to corner cube.
	void extend_mapping_corner_cube_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE corner_gcube_index = 
      isovert.GCubeIndex(corner_cube_index);
    GRID_BOX region(DIM3);

    if (corner_gcube_index == ISOVERT::NO_INDEX) { return; }

    const GRID_COORD_TYPE * corner_cube_coord = 
      isovert.gcube_list[corner_gcube_index].cube_coord;

    construct_extended_corner_cube_region
      (isovert.grid, bin_grid, bin_width, isovert, corner_cube_index, region);

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      cerr << endl << "In " << __func__;
      scalar_grid.PrintIndexAndCoord
        (cerr, "  Corner cube: ", corner_cube_index, "\n");
      cerr << "  Region: ";
      IJK::print_coord3D(cerr, region.MinCoord());
      IJK::print_coord3D(cerr, region.MaxCoord());
      cerr << endl;
    }

    extend_mapping_corner_region_multi
      (scalar_grid, isodual_table, ambig_info, bin_grid, bin_width, 
       isovalue, corner_cube_index, merge_param, region, isovert, gcube_map);

    // Try again.
    extend_mapping_corner_region_multi
      (scalar_grid, isodual_table, ambig_info, bin_grid, bin_width, 
       isovalue, corner_cube_index, merge_param, region, isovert, gcube_map);
  }

  /// Extend mapping of isosurface vertices in region to corner cube.
	void extend_mapping_corner_region_multi
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const BIN_GRID<VERTEX_INDEX> & bin_grid,
   const AXIS_SIZE_TYPE bin_width,
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX corner_cube_index,
   const MERGE_PARAM & merge_param,
   const GRID_BOX & region,   
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE corner_gcube_index = 
      isovert.GCubeIndex(corner_cube_index);

    if (corner_gcube_index == ISOVERT::NO_INDEX) { return; }

    const GRID_COORD_TYPE * corner_cube_coord = 
      isovert.gcube_list[corner_gcube_index].cube_coord;

    for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

      VERTEX_INDEX extend_from_cube = 
        isovert.grid.CubeNeighborF(corner_cube_index, j);

      extend_mapping_corner_cube_across_facets_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         extend_from_cube, region, merge_param, isovert, gcube_map);
    }

    for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) {

      VERTEX_INDEX extend_from_cube = 
        isovert.grid.CubeNeighborE(corner_cube_index, j);

      extend_mapping_corner_cube_across_facets_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         extend_from_cube, region, merge_param, isovert, gcube_map);
    }

    for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsV(); j++) {

      VERTEX_INDEX extend_from_cube = 
        isovert.grid.CubeNeighborV(corner_cube_index, j);

      extend_mapping_corner_cube_across_facets_multi
        (scalar_grid, isodual_table, ambig_info, isovalue, 
         extend_from_cube, region, merge_param, isovert, gcube_map);
    }

  }


  /// Extend corner cube mapping across facets.
	void extend_mapping_corner_cube_across_facets_multi
  ( const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
    const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
    const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX extend_from_cube,
    const GRID_BOX & region,
    const MERGE_PARAM & merge_param,
    MERGESHARP::ISOVERT & isovert,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    INDEX_DIFF_TYPE extend_from_gcube = isovert.GCubeIndex(extend_from_cube);

    if (extend_from_gcube == ISOVERT::NO_INDEX) { return; }

    NUM_TYPE to_gcube = gcube_map[extend_from_gcube];
    VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
    bool flag_map;

    if (isovert.gcube_list[extend_from_gcube].boundary_bits == 0) {

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

        VERTEX_INDEX from_cube = 
          isovert.grid.CubeNeighborF(extend_from_cube, j);
        INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);

        if (from_gcube == ISOVERT::NO_INDEX) { continue; }

        if (region.Contains(isovert.gcube_list[from_gcube].cube_coord)) {

          check_extended_corner_cube_and_map
            (scalar_grid, isovalue, from_cube, to_cube, isovert,
             gcube_map, flag_map);

          if (!flag_map) {
            check_and_map_ambig_pair
              (to_cube, from_cube, scalar_grid, isodual_table, 
               ambig_info, isovalue, isovert, true, merge_param,
               gcube_map, flag_map);
          }
        }
      }
    }
    else {
      // Handle boundary case.
    }
	}

}


// **************************************************
// Extend mapping of isosurface vertices
// **************************************************


namespace {

  // forward declaraions
	void check_extended_and_map
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map);


  /// Extend mapping of isosurface vertices
	void extend_mapping
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    MSDEBUG();
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

    for (NUM_TYPE i = 0; i < selected_gcube_list.size(); i++) {

      const NUM_TYPE gcube_index = selected_gcube_list[i];
      extend_mapping_to_cube
        (scalar_grid, isovalue, isovert.gcube_list[gcube_index], 
         isovert, gcube_map);
    }
	}


  /// Extend mapping of pairs of isosurface vertices
	void extend_mappingII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    MSDEBUG();
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

    for (NUM_TYPE i = 0; i < selected_gcube_list.size(); i++) {

      const NUM_TYPE gcube_index = selected_gcube_list[i];
      extend_mapping_to_cubeII
        (scalar_grid, isovalue, isovert.gcube_list[gcube_index], merge_param,
         isovert, gcube_map);
    }
	}

  // *** CURRENTLY NOT USED. ***
  /// Extend mapping of triples of isosurface vertices
	void extend_mappingIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const std::vector<NUM_TYPE> & selected_gcube_list,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    MSDEBUG();
    if (flag_debug)
      { cerr << "In " << __func__ << endl; }

    for (NUM_TYPE i = 0; i < selected_gcube_list.size(); i++) {

      const NUM_TYPE gcube_index = selected_gcube_list[i];
      extend_mapping_to_cubeIII
        (scalar_grid, isovalue, isovert.gcube_list[gcube_index], merge_param,
         isovert, gcube_map);
    }
	}

  /// Extend mapping of isosurface vertices to cube.
	void extend_mapping_to_cube
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE to_cube_index = to_gcube.cube_index;
    GRID_COORD_TYPE distance2boundary;
    scalar_grid.ComputeCubeDistanceToGridBoundary
      (to_gcube.cube_coord, distance2boundary);

    if (distance2boundary >= 2) {

      // Since dist2boundary >= 2, no need to worry about boundary.
      const BOUNDARY_BITS_TYPE boundary_bits = 0;

      for (int d0 = 0; d0 < DIM3; d0++) {
        for (int j0 = -1; j0 < 2; j0 += 2) {

          const VERTEX_INDEX cubeA_index =
            to_cube_index + 2*j0*scalar_grid.AxisIncrement(d0);

          check_extended_and_map
            (cubeA_index, cubeA_index, scalar_grid, isovalue, to_cube_index,
             isovert, gcube_map);

          apply_to_cubes_in_plane_facet_adjacent_to
            (check_extended_and_map, isovert.grid, cubeA_index, boundary_bits, 
             d0, scalar_grid, isovalue, to_cube_index, isovert, gcube_map);

          apply_to_cubes_in_plane_edge_adjacent_to
            (check_extended_and_map, isovert.grid, cubeA_index, boundary_bits, 
             d0, scalar_grid, isovalue, to_cube_index, isovert, gcube_map);
        }
      }
    }
  }


  /// Extend mapping of pairs of isosurface vertices to cube to_cube.
  /// @param cubeA_index Extend mapping from neighbors of cubeA_index.
  /// @param to_cube Extend mapping to cube to_cube.
	void extend_mapping_to_cubeII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE to_cube_index = to_gcube.cube_index;
    GRID_COORD_TYPE distance2boundary;
    scalar_grid.ComputeCubeDistanceToGridBoundary
      (to_gcube.cube_coord, distance2boundary);

    if (distance2boundary >= 2) {

      // Since dist2boundary >= 2, no need to worry about boundary.
      const BOUNDARY_BITS_TYPE boundary_bits = 0;

      for (int d0 = 0; d0 < DIM3; d0++) {
        for (int j0 = -1; j0 < 2; j0 += 2) {

          const VERTEX_INDEX cubeA_index =
            to_cube_index + 2*j0*scalar_grid.AxisIncrement(d0);

          extend_map_facet_adjacent_pairs
            (cubeA_index, cubeA_index, scalar_grid, isovalue,
             to_cube_index, merge_param, isovert, gcube_map);

          apply_to_cubes_in_plane_facet_adjacent_to
            (extend_map_facet_adjacent_pairs, isovert.grid, 
             cubeA_index, boundary_bits, d0, scalar_grid, isovalue, 
             to_cube_index, merge_param, isovert, gcube_map);

          apply_to_cubes_in_plane_edge_adjacent_to
            (extend_map_facet_adjacent_pairs, isovert.grid, 
             cubeA_index, boundary_bits, d0, scalar_grid, isovalue, 
             to_cube_index, merge_param, isovert, gcube_map);


          // *** COULD CALL extend_map_edge_adjacent_pairs HERE ***
        }
      }
    }

  }

  /// Extend mapping of triples of isosurface vertices to cube to_cube.
  /// @param cubeA_index Extend mapping from neighbors of cubeA_index.
  /// @param to_cube Extend mapping to cube to_cube.
	void extend_mapping_to_cubeIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const MERGESHARP::GRID_CUBE_DATA & to_gcube,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE to_cube_index = to_gcube.cube_index;
    GRID_COORD_TYPE distance2boundary;
    scalar_grid.ComputeCubeDistanceToGridBoundary
      (to_gcube.cube_coord, distance2boundary);

    // *** DEBUG ***
    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "In extend_mapping_to_cubeIII. Cube ", to_cube_index, "\n");
      cerr << "distance2boundary: " << distance2boundary << endl;
    }


    if (distance2boundary >= 2) {

      // Since dist2boundary >= 2, no need to worry about boundary.
      const BOUNDARY_BITS_TYPE boundary_bits = 0;

      for (int d0 = 0; d0 < DIM3; d0++) {
        for (int j0 = -1; j0 < 2; j0 += 2) {

          const VERTEX_INDEX cubeA_index =
            to_cube_index + 2*j0*scalar_grid.AxisIncrement(d0);

          extend_map_triples
            (cubeA_index, cubeA_index, scalar_grid, isovalue,
             to_cube_index, merge_param, isovert, gcube_map);

          apply_to_cubes_in_plane_facet_adjacent_to
            (extend_map_triples, isovert.grid, 
             cubeA_index, boundary_bits, d0, scalar_grid, isovalue, 
             to_cube_index, merge_param, isovert, gcube_map);

          apply_to_cubes_in_plane_edge_adjacent_to
            (extend_map_triples, isovert.grid, cubeA_index, boundary_bits, d0, 
             scalar_grid, isovalue, to_cube_index, merge_param,
             isovert, gcube_map);
        }
      }
    }

  }

  /// Extend mapping of pairs of adjacent cubes.
  /// One cube is covered while the other may not be covered.
	void extend_map_adjacent_pairs_covered
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const bool flag_extended = true;
    BOUNDARY_BITS_TYPE boundary_bits;

		for (NUM_TYPE i = 0; i < sharp_gcube_list.size(); i++) {

			NUM_TYPE to_gcube = sharp_gcube_list[i];

      if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {
        VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
        boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

        bool flag_map;
        apply_to_cubes_facet_adjacent_to
          (map_facet_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_edge_adjacent_to
          (map_facet_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_vertex_adjacent_to
          (map_facet_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_facet_adjacent_to
          (map_edge_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_edge_adjacent_to
          (map_edge_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_vertex_adjacent_to
          (map_edge_adjacent_pairs, isovert.grid, to_cube, boundary_bits,
           scalar_grid, isodual_table, ambig_info, isovalue, isovert,
           flag_extended, merge_param, gcube_map, flag_map);
      }
		}

  }

  /// Extend mapping of pairs of adjacent cubes.
  /// One cube is covered while the other may not be covered.
  /// Version which creates sharp_gcube_list[].
	void extend_map_adjacent_pairs_covered
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
		std::vector<NUM_TYPE> sharp_gcube_list;

		get_corner_or_edge_cubes(isovert.gcube_list, sharp_gcube_list);

    extend_map_adjacent_pairs_covered
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       sharp_gcube_list, merge_param, gcube_map);
  }

  /// Extend mapping around edges.  Try to map three cubes at the same time.
  /// One cube is covered while the other may not be covered.
	void extend_map_around_edges
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const bool flag_extended = true;
    BOUNDARY_BITS_TYPE boundary_bits;

		for (NUM_TYPE i = 0; i < sharp_gcube_list.size(); i++) {

			NUM_TYPE to_gcube = sharp_gcube_list[i];

      if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {
        VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
        boundary_bits = isovert.gcube_list[to_gcube].boundary_bits;

        bool flag_map;
        apply_to_cubes_facet_adjacent_to
          (map_cubes_around_edge, isovert.grid, to_cube, boundary_bits,
           isovert, scalar_grid, isodual_table, ambig_info, isovalue,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_edge_adjacent_to
          (map_cubes_around_edge, isovert.grid, to_cube, boundary_bits,
           isovert, scalar_grid, isodual_table, ambig_info, isovalue,
           flag_extended, merge_param, gcube_map, flag_map);

        apply_to_cubes_vertex_adjacent_to
          (map_cubes_around_edge, isovert.grid, to_cube, boundary_bits,
           isovert, scalar_grid, isodual_table, ambig_info, isovalue,
           flag_extended, merge_param, gcube_map, flag_map);
      }
		}

  }

  /// Extend mapping around edges.  Try to map three cubes at the same time.
  /// One cube is covered while the other may not be covered.
  /// Version which creates sharp_gcube_list[].
	void extend_map_around_edges
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
   const IJKDUALTABLE::ISODUAL_CUBE_TABLE_AMBIG_INFO & ambig_info,
   const SCALAR_TYPE isovalue,
   const MERGESHARP::ISOVERT & isovert, 
   const MERGE_PARAM & merge_param,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
		std::vector<NUM_TYPE> sharp_gcube_list;

		get_corner_or_edge_cubes(isovert.gcube_list, sharp_gcube_list);

    extend_map_around_edges
      (scalar_grid, isodual_table, ambig_info, isovalue, isovert, 
       sharp_gcube_list, merge_param, gcube_map);
  }

}


// **************************************************
// Routines for checking mappings
// **************************************************

namespace {

	// Add to list selected cubes whose vertices are "connected" 
  //   to vertex in cube0_index
	//[out] connected_sharp List of selected cubes.
	void add2list_connected_sharp(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX cube0_index,
		const MERGESHARP::ISOVERT & isovert, 
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    CUBE_CONNECTED_ARRAY & connected_sharp)
	{
    NUM_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);

    if (gcube0_index == ISOVERT::NO_INDEX) { return; }

		if (isovert.gcube_list[gcube0_index].boundary_bits == 0) {

			for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) 
			{  
				VERTEX_INDEX cube1_index = isovert.grid.CubeNeighborF(cube0_index, j);
        INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);

				if (gcube1_index == ISOVERT::NO_INDEX) { continue; }

				VERTEX_INDEX to_gcube = gcube_map[gcube1_index]; 

				if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

          // Note: Assumes j'th facet neighbor shares j'th facet with cube.
          if (is_gt_facet_min_le_facet_max
              (scalar_grid, cube0_index, j, isovalue)) {

            NUM_TYPE to_cube = isovert.CubeIndex(to_gcube);
            if (!connected_sharp.Contains(to_cube)) 
              { connected_sharp.PushBack(to_cube); }
					}
				}
			}//for_end


			//edges
			for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) 
			{
				VERTEX_INDEX cube1_index = isovert.grid.CubeNeighborE(cube0_index, j);
        INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);
				
				if (gcube1_index == ISOVERT::NO_INDEX) {	continue; }

				VERTEX_INDEX to_gcube = gcube_map[gcube1_index];

				if (isovert.gcube_list[to_gcube].flag == SELECTED_GCUBE) {

          // Check that edge is bipolar
          // Note: Computation of (iv0,iv1) relies on specific ordering
          //   of cube edge neighbors.
          int edge_dir = int(j/isovert.grid.NumFacetVertices());
          int k = j%isovert.grid.NumFacetVertices();
          VERTEX_INDEX iv0 = isovert.grid.FacetVertex(cube0_index, edge_dir, k);
          VERTEX_INDEX iv1 = isovert.grid.NextVertex(iv0, edge_dir);

          if (is_gt_min_le_max(scalar_grid, iv0, iv1, isovalue)) {

            NUM_TYPE to_cube = isovert.CubeIndex(to_gcube);
            if (!connected_sharp.Contains(to_cube)) 
              { connected_sharp.PushBack(to_cube); }
          }
				}
			}
		}
	}

	// Find selected cubes whose vertices are "connected" to vertex in cube0_index
	//[out] connected_sharp List of selected cubes.
	void find_connected_sharp(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const VERTEX_INDEX cube0_index,
		const MERGESHARP::ISOVERT & isovert, 
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    CUBE_CONNECTED_ARRAY & connected_sharp)
	{
    connected_sharp.Clear();
    add2list_connected_sharp
      (scalar_grid, isovalue, cube0_index, isovert, gcube_map, 
       connected_sharp);
	}


	// Return true if cubeA_index is facet adjacent to two cubes
  //   which share an edge and map to two different selected cubes.
  // One of the selected should be cubeB_index.
  // @param[out] cubeC_index Other selected cube.
	bool check_facet_adjacent_maps(
    const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert, 
    const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    const VERTEX_INDEX cubeA_index,
    const VERTEX_INDEX cubeB_index,
    VERTEX_INDEX & cubeC_index)
	{
    const INDEX_DIFF_TYPE gcubeA_index = isovert.GCubeIndex(cubeA_index);
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);
    BOUNDARY_BITS_TYPE boundary_bits;

    cubeC_index = cubeB_index;

    if (gcubeA_index == ISOVERT::NO_INDEX) { return(false); }

    boundary_bits = isovert.gcube_list[gcubeA_index].boundary_bits;

    if (boundary_bits == 0) {

      for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

        int d1 = (edge_dir+1)%DIM3;
        int d2 = (edge_dir+2)%DIM3;

        for (int j1 = 0; j1 < 2; j1++) {
          VERTEX_INDEX cube1_index = grid.AdjacentVertex(cubeA_index, d1, j1);
          INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);
          if (gcube1_index == ISOVERT::NO_INDEX) { continue; }

          if (!isovert.gcube_list[gcube1_index].IsCoveredOrSelected())
            { continue; }

          GRID_CUBE_FLAG flag1 = isovert.gcube_list[gcube1_index].flag;
          if (flag1 == COVERED_B_GCUBE) { continue; }

          for (int j2 = 0; j2 < 2; j2++) {
            VERTEX_INDEX cube2_index = grid.AdjacentVertex(cubeA_index, d2, j2);
            INDEX_DIFF_TYPE gcube2_index = isovert.GCubeIndex(cube2_index);

            if (gcube2_index == ISOVERT::NO_INDEX) { continue; }
            if (!isovert.gcube_list[gcube2_index].IsCoveredOrSelected())
              { continue; }

            GRID_CUBE_FLAG flag2 = isovert.gcube_list[gcube2_index].flag;
            if (flag2 == COVERED_B_GCUBE) { continue; }

            if (gcube_map[gcube1_index] == gcubeB_index) {
              cubeC_index = isovert.CubeIndex(gcube_map[gcube2_index]);
              return(true);
            }
            else if (gcube_map[gcube2_index] == gcubeB_index) {
              cubeC_index = isovert.CubeIndex(gcube_map[gcube1_index]);
              return(true);
            }
          }
        }
      }
    }
    else {
      // Handle boundary case.
    }

    return(false);
	}


	/// Check that mapping does not create degenerate/thin triangle 
  ///   between connected, selected cubes.
	bool check_connected_sharp
  ( const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		const MERGESHARP::ISOVERT & isovert, 
    const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    const VERTEX_INDEX from_vertex,
    const VERTEX_INDEX to_vertex,
    const CUBE_CONNECTED_ARRAY & connected_sharp)
	{
		if (connected_sharp.NumElements() == 3) {

      for (int k = 0; k < 3; k++) {

        if (connected_sharp[k] == to_vertex) {
						
          VERTEX_INDEX k1 = (k+1)%3;
          VERTEX_INDEX k2 = (k+2)%3;

          VERTEX_INDEX cube0_index = connected_sharp[k];
          VERTEX_INDEX cube1_index = connected_sharp[k1];
          VERTEX_INDEX cube2_index = connected_sharp[k2];

          // Check that cube1_index and cube2_index are NOT connected.
          if (are_connected_by_iso_edge
              (scalar_grid, cube1_index, cube2_index, 
               isovalue, isovert, gcube_map))
            { return(false); }
        }
      }
    }

    return(true);
	}

  // Return true if array contains x.
  template <const int MAX_NUM_ELEMENTS, typename ETYPE, typename NTYPE>
  bool array_contains(const ETYPE x, const ETYPE array[MAX_NUM_ELEMENTS], 
                      const NTYPE num_elements)
  {
    for (NTYPE i = 0; i < num_elements; i++)
      { if (array[i] == x) { return(true); } }
    
    return(false);
  }

  // Insert x into array.
  // @pre num_elements < MAX_NUM_ELEMENTS
  template <const int MAX_NUM_ELEMENTS, typename ETYPE, typename NTYPE>
  void array_insert(const ETYPE x,ETYPE array[MAX_NUM_ELEMENTS], 
                    NTYPE & num_elements)
  {
    array[num_elements] = x;
    num_elements++;
  }
  
  // *** NOT TESTED ***
	// Check that cube is near two selected cubes.
	bool check_near_two_selected(
    const SHARPISO_GRID_NEIGHBORS & grid,
		const MERGESHARP::ISOVERT & isovert, 
    const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
    const VERTEX_INDEX cube0_index)
  {
    BOUNDARY_BITS_TYPE boundary_bits;
    const NUM_TYPE MAX_NEAR = 2;
    NUM_TYPE near_gcube_index[MAX_NEAR];
    NUM_TYPE num_near;
    const INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);

    if (gcube0_index == ISOVERT::NO_INDEX) { return(false); }

    boundary_bits = isovert.gcube_list[gcube0_index].boundary_bits;

    if (boundary_bits == 0) {

      num_near = 0;
      for (NUM_TYPE j = 0; j < grid.NumCubeNeighborsF(); j++) {

        VERTEX_INDEX cube1_index = grid.CubeNeighborF(cube0_index, j);
        INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);
        if (gcube1_index != ISOVERT::NO_INDEX) {
          NUM_TYPE to_gcube = gcube_map[gcube1_index];

          GRID_CUBE_FLAG gcube_flag =
            isovert.gcube_list[gcube1_index].flag;

          if (gcube_flag == COVERED_A_GCUBE ||
              gcube_flag == COVERED_CORNER_GCUBE) {

            if (!array_contains<MAX_NEAR>
                (to_gcube, near_gcube_index, num_near)) {

              if (num_near >= MAX_NEAR) { return(false); }

              array_insert<MAX_NEAR>(to_gcube, near_gcube_index, num_near);
            }
          }
        }
      }

      for (NUM_TYPE j = 0; j < grid.NumCubeNeighborsE(); j++) {

        VERTEX_INDEX cube1_index = grid.CubeNeighborE(cube0_index, j);
        INDEX_DIFF_TYPE gcube1_index = isovert.GCubeIndex(cube1_index);
        if (gcube1_index != ISOVERT::NO_INDEX) {
          NUM_TYPE to_gcube = gcube_map[gcube1_index];

          GRID_CUBE_FLAG gcube_flag =
            isovert.gcube_list[gcube1_index].flag;

          if (gcube_flag == COVERED_A_GCUBE ||
              gcube_flag == COVERED_CORNER_GCUBE) {

            if (!array_contains<MAX_NEAR>
                (to_gcube, near_gcube_index, num_near)) {

              if (num_near >= MAX_NEAR) { return(false); }

              array_insert<MAX_NEAR>(to_gcube, near_gcube_index, num_near);
            }
          }
        }
      }
    }
    else {
      // Handle boundary case.
    }

    if (num_near == 2) { return(true); }
    
    return(false);
  }

	/*
	* Check merge Against edge neighbors
	*/
  // *** NOT CURRENTLY USED ***
	bool is_cube_merge_permitted_edge_neighbors(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		MERGESHARP::ISOVERT & isovert,
		const VERTEX_INDEX from_cube_gcube_index,
		const VERTEX_INDEX to_cube_gcube_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
	
		COORD_TYPE from_cube_cc[DIM3] = {0.0,0.0,0.0};
		COORD_TYPE to_cube_cc[DIM3] = {0.0,0.0,0.0};
		VERTEX_INDEX to_cube_cube_index = isovert.gcube_list[to_cube_gcube_index].cube_index;
		VERTEX_INDEX from_cube_cube_index = isovert.gcube_list[from_cube_gcube_index].cube_index;

		scalar_grid.ComputeCoord(from_cube_cube_index, from_cube_cc);
		scalar_grid.ComputeCoord(to_cube_cube_index, to_cube_cc);

		for (int d = 0; d < DIM3; d++)
		{
			int d1 = (d+1)%3;
			int d2 = (d+2)%3;
			for (int j = -1; j < 2; j=j+2)
			{
				for (int k = -1; k < 2; k=k+2)
				{
					VERTEX_INDEX ca = from_cube_cube_index + j*scalar_grid.AxisIncrement(d1) 
						+ k*scalar_grid.AxisIncrement(d2);
					VERTEX_INDEX ca_gcube_index = isovert.GCubeIndex(ca);
					if(ca_gcube_index != ISOVERT::NO_INDEX)
					{
						if( gcube_map[ca_gcube_index] == ca_gcube_index)
						{
							COORD_TYPE x1 = from_cube_cc[d1] + j;
							COORD_TYPE x2 = from_cube_cc[d2] + k;

					

							if(j<0) 
							{
								if(to_cube_cc[d1] < x1) { return false; }
							}
							else {
								if(to_cube_cc[d1] > x1){ return false; }
							}
							if(k<0)
							{
								if(to_cube_cc[d2] < x2) {return false; }
							}
							else {
								if(to_cube_cc[d2] > x2){ return false; }
							}
						}
					}
				}
			}
		}
		return true;
	}

	/*
	* 
  *   
	*/
  /// Check if merge distorts triangle between vertices in cubes 
  ///   from_gcube_index, ca_gcube_index and cb_gcube_index.
  /// @param min_distance_between_isovert Points closer 
  ///          than min_distance_between_isovert are considered identical.
	bool is_triangle_distorted
  (const MERGESHARP::ISOVERT & isovert,
   const VERTEX_INDEX from_gcube_index,
   const VERTEX_INDEX to_gcube_index,
   const VERTEX_INDEX ca_gcube_index,
   const VERTEX_INDEX cb_gcube_index,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   const COORD_TYPE min_distance_between_isovert,
   const COORD_TYPE max_cos)
  {
    const COORD_TYPE * from_coord = isovert.IsoVertCoord(from_gcube_index);
    const COORD_TYPE * to_coord = isovert.IsoVertCoord(to_gcube_index);
    const VERTEX_INDEX ca_to_gcube = gcube_map[ca_gcube_index];
    const VERTEX_INDEX cb_to_gcube = gcube_map[cb_gcube_index];
    const COORD_TYPE * ca_coord = isovert.IsoVertCoord(ca_to_gcube);
    const COORD_TYPE * cb_coord = isovert.IsoVertCoord(cb_to_gcube);
    COORD_TYPE v0[DIM3], v1[DIM3], v2[DIM3];
    COORD_TYPE wA[DIM3], wB[DIM3];
    COORD_TYPE mag0, mag1, mag2;
    COORD_TYPE cos_angle;
    bool flag0_zero, flag1_zero, flag2_zero;

    IJK::subtract_coord_3D(to_coord, cb_coord, v0);
    IJK::subtract_coord_3D(ca_coord, cb_coord, v1);
    IJK::subtract_coord_3D(from_coord, cb_coord, v2);

    IJK::normalize_vector
      (DIM3, v0, min_distance_between_isovert, v0, mag0, flag0_zero);
    IJK::normalize_vector
      (DIM3, v1, min_distance_between_isovert, v1, mag1, flag1_zero);
    IJK::normalize_vector
      (DIM3, v2, min_distance_between_isovert, v2, mag2, flag2_zero);

    if (flag0_zero || flag1_zero || flag2_zero) {
      // Points are too close to determine distortion.
      // Assume distortion.

      return(true);
    }

    // Check that moving from_coord to to_coord does not create small angle.
    IJK::compute_inner_product(DIM3, v0, v1, cos_angle);
    
    // *** DEBUG **
    /*
    using namespace std;
    VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube_index);
    VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube_index);
    if (to_cube == 150825) {
      cerr << "from_cube: " << from_cube;
      cerr << "  cos_angle: " << cos_angle << "  max_cos: " << max_cos << endl;
    }
    */

    if (cos_angle > max_cos) 
      { return(true); }

    /* DISABLE
    // Check that moving from_coord to to_coord does not flip triangle.
    IJK::compute_cross_product_3D(v0, v1, wA);
    IJK::compute_cross_product_3D(v2, v1, wB);

    IJK::compute_inner_product(DIM3, wA, wB, cos_angle);
    if (cos_angle <= 0) { 
      // Angle between wA and wB is at least 90 degrees.
      return(true); 
    }
    */

    return(false);
  }

	/*
	* Check if merge distorts triangles
	*/
	bool does_cube_merge_distort_triangles
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const MERGESHARP::ISOVERT & isovert,
   const VERTEX_INDEX from_gcube_index,
   const VERTEX_INDEX to_gcube_index,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		const VERTEX_INDEX to_cube_index = isovert.CubeIndex(to_gcube_index);
		const VERTEX_INDEX from_cube_index = isovert.CubeIndex(from_gcube_index);
    const GRID_COORD_TYPE * from_cube_coord =
      isovert.gcube_list[from_gcube_index].cube_coord;
    VERTEX_INDEX cb;

    // REPLACE BY PARAM
    const COORD_TYPE min_distance_between_isovert = 0.001;
    //    const COORD_TYPE max_cos_angle = std::cos(10.0*(M_PI/180.0));
    const COORD_TYPE max_cos_angle = std::cos(5.0*(M_PI/180.0));

    // Check triangles formed by two facet neighbors
    for (int d = 0; d < DIM3; d++) {
			int d1 = (d+1)%3;
			int d2 = (d+2)%3;

			for (int j1 = -1; j1 < 2; j1=j1+2) {
        if ((j1 < 0) && (from_cube_coord[d1] == 0)) { continue; }
        if ((j1 > 0) && (from_cube_coord[d1]+2 == scalar_grid.AxisSize(d1)))
          { continue; }

        for (int j2 = -1; j2 < 2; j2=j2+2) {
          if ((j2 < 0) && (from_cube_coord[d2] == 0)) { continue; }
          if ((j2 > 0) && (from_cube_coord[d2]+2 == scalar_grid.AxisSize(d2)))
            { continue; }

					VERTEX_INDEX c1 = from_cube_index + j1*scalar_grid.AxisIncrement(d1);
          VERTEX_INDEX c2 = from_cube_index + j2*scalar_grid.AxisIncrement(d2);

					INDEX_DIFF_TYPE c1_gcube_index = isovert.GCubeIndex(c1);
					INDEX_DIFF_TYPE c2_gcube_index = isovert.GCubeIndex(c2);

          if (c1_gcube_index != ISOVERT::NO_INDEX &&
              c2_gcube_index != ISOVERT::NO_INDEX) {

            if (gcube_map[c1_gcube_index] != to_gcube_index &&
                gcube_map[c2_gcube_index] != to_gcube_index &&
                gcube_map[c1_gcube_index] != gcube_map[c2_gcube_index]) {

              // *** DEBUG ***
              /*
              using namespace std;
              GRID_COORD_TYPE coordA[DIM3];
              GRID_COORD_TYPE coordB[DIM3];
              scalar_grid.ComputeCoord(c1, coordA);
              scalar_grid.ComputeCoord(c2, coordB);
              if (to_cube_index == 150825) {
                cout << "  Facet neighbor 1: " << c1 << " ";
                IJK::print_coord3D(cerr, coordB);
                
                cout << "  Facet neighbor 2: " << c2 << " ";
                IJK::print_coord3D(cerr, coordA);
                cerr << endl;
              }
              */

              if (is_triangle_distorted
                  (isovert, from_gcube_index, to_gcube_index,
                   c1_gcube_index, c2_gcube_index, gcube_map,
                   min_distance_between_isovert, max_cos_angle)) {

                // *** DEBUG ***
                using namespace std;
                if (flag_debug) {
                  scalar_grid.PrintIndexAndCoord
                    (cerr, "  Mapping distorts triangle ", from_cube_index, "");
                  scalar_grid.PrintIndexAndCoord(cerr, " ", c1, "");
                  scalar_grid.PrintIndexAndCoord(cerr, " ", c2, "\n");
                }

                return(true); 
              }
            }
          }
        }
      }
    }


    // Check triangles formed by facet and edge neighbor.
		for (int d = 0; d < DIM3; d++)
		{
			int d1 = (d+1)%3;
			int d2 = (d+2)%3;
			for (int j1 = -1; j1 < 2; j1=j1+2) {
        if ((j1 < 0) && (from_cube_coord[d1] == 0)) { continue; }
        if ((j1 > 0) && (from_cube_coord[d1]+2 == scalar_grid.AxisSize(d1)))
          { continue; }

				for (int j2 = -1; j2 < 2; j2=j2+2) {
          if ((j2 < 0) && (from_cube_coord[d2] == 0)) { continue; }
          if ((j2 > 0) && (from_cube_coord[d2]+2 == scalar_grid.AxisSize(d2)))
            { continue; }

					VERTEX_INDEX ca = from_cube_index + j1*scalar_grid.AxisIncrement(d1) 
						+ j2*scalar_grid.AxisIncrement(d2);
					VERTEX_INDEX ca_gcube_index = isovert.GCubeIndex(ca);

					if (ca_gcube_index != ISOVERT::NO_INDEX) {

            if (gcube_map[ca_gcube_index] != to_gcube_index) {

              for (int k = 0; k < 2; k++) {

                if (k == 0) {
                  cb = from_cube_index + j1*scalar_grid.AxisIncrement(d1);
                }
                else {
                  cb = from_cube_index + j2*scalar_grid.AxisIncrement(d2);
                }
                INDEX_DIFF_TYPE cb_gcube_index = isovert.GCubeIndex(cb);

                if (cb_gcube_index != ISOVERT::NO_INDEX) {
                  if (gcube_map[cb_gcube_index] != to_gcube_index &&
                      gcube_map[ca_gcube_index] != gcube_map[cb_gcube_index]) {

                    // *** DEBUG ***
                    /*
                    using namespace std;
                    GRID_COORD_TYPE coordA[DIM3];
                    GRID_COORD_TYPE coordB[DIM3];
                    scalar_grid.ComputeCoord(ca, coordA);
                    scalar_grid.ComputeCoord(cb, coordB);
                    if (to_cube_index == 150825) {
                      cout << "  Facet neighbor: " << cb << " ";
                      IJK::print_coord3D(cerr, coordB);

                      cout << "  Edge neighbor: " << ca << " ";
                      IJK::print_coord3D(cerr, coordA);
                      cerr << endl;
                    }
                    */

                    if (is_triangle_distorted
                        (isovert, from_gcube_index, to_gcube_index,
                         ca_gcube_index, cb_gcube_index, gcube_map,
                         min_distance_between_isovert, max_cos_angle)) {

                      // *** DEBUG ***
                      using namespace std;
                      if (flag_debug) {
                        scalar_grid.PrintIndexAndCoord
                          (cerr, "  Mapping distorts triangle ", 
                           from_cube_index, "");
                        scalar_grid.PrintIndexAndCoord(cerr, " ", ca, "");
                        scalar_grid.PrintIndexAndCoord(cerr, " ", cb, "\n");
                      }

                      return(true); 
                    }
                  }
                }
              }
            }
					}
				}
			}
		}

		return false;
	}

	/*
	* check if merge reverses order of isosurface vertices along grid axes
	*/
	bool does_merge_reverse_isovert_order(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube_index,
		const INDEX_DIFF_TYPE to_gcube_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		VERTEX_INDEX to_cube_index = 
      isovert.gcube_list[to_gcube_index].cube_index;
		VERTEX_INDEX from_cube_index = 
      isovert.gcube_list[from_gcube_index].cube_index;

    const GRID_COORD_TYPE * from_cube_cc =
      isovert.gcube_list[from_gcube_index].cube_coord;
    const GRID_COORD_TYPE * to_cube_cc =
      isovert.gcube_list[to_gcube_index].cube_coord;

		if (isovert.gcube_list[from_gcube_index].boundary_bits == 0) {
			for (int d = 0; d < DIM3; d++)
			{
				VERTEX_INDEX prev_v_cube_index = 
          scalar_grid.PrevVertex(from_cube_index,d);
				COORD_TYPE prev_v_cc[DIM3] = {0.0,0.0,0.0};
				scalar_grid.ComputeCoord(prev_v_cube_index, prev_v_cc);

				INDEX_DIFF_TYPE prev_v_gcube_index
          = isovert.GCubeIndex(prev_v_cube_index);


				if (prev_v_gcube_index != ISOVERT::NO_INDEX)
				{
					if( gcube_map[prev_v_gcube_index] == prev_v_gcube_index)
					{
						/*x is the dth coord of prev_v_cc*/
						COORD_TYPE x = from_cube_cc[d]-1;
						if (to_cube_cc[d] < x) {

              // *** DEBUG ***
              using namespace std;
              if (flag_debug) {
                scalar_grid.PrintIndexAndCoord
                  (cerr, "  Cube ", prev_v_cube_index, " prevents mapping.\n");
              }

              return true;	
            }
					}
				}

				VERTEX_INDEX next_v_cube_index = 
          scalar_grid.NextVertex(from_cube_index,d);
				COORD_TYPE next_v_cc[DIM3] = {0.0,0.0,0.0};
				scalar_grid.ComputeCoord(next_v_cube_index, next_v_cc);

				INDEX_DIFF_TYPE next_v_gcube_index
					= isovert.GCubeIndex(next_v_cube_index);

				if (next_v_gcube_index != ISOVERT::NO_INDEX) {

					if (gcube_map[next_v_gcube_index] == next_v_gcube_index) {

						/*x is the dth coord of next_v_cc*/
						COORD_TYPE x = from_cube_cc[d]+1;
						if (to_cube_cc[d] > x) {

              // *** DEBUG ***
              using namespace std;
              if (flag_debug) {
                scalar_grid.PrintIndexAndCoord
                  (cerr, "  Cube ", prev_v_cube_index, " prevents mapping.\n");
              }

              return true;	
            }
					}
				}
			}
		}
		
		return false;
	}

  /// Check if merge maps cube to same axis coordinate as some facet neighbor.
  ///   *** MAYBE CHANGED ***
  ///   Ignore neighbors which map to the same vertex as cube.
	bool does_merge_identify_facet_adjacent_axis_coord(
		const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube,
		const INDEX_DIFF_TYPE to_gcube,
    const INDEX_DIFF_TYPE gcubeC_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
		VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube);
    const GRID_COORD_TYPE * to_coord = isovert.gcube_list[to_gcube].cube_coord;

		if (isovert.gcube_list[from_gcube].boundary_bits == 0) {
			for (int d = 0; d < DIM3; d++)
			{
				VERTEX_INDEX prev_cube = grid.PrevVertex(from_cube, d);
				INDEX_DIFF_TYPE prev_gcube = isovert.GCubeIndex(prev_cube);

				if (prev_gcube != ISOVERT::NO_INDEX) {

          NUM_TYPE to_gcubeB = gcube_map[prev_gcube];
					if (to_gcubeB != to_gcube) {

            VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
            const GRID_COORD_TYPE * to_coordB = 
              isovert.gcube_list[to_gcubeB].cube_coord;

            if (to_gcubeB == gcubeC_index) { 
              if (to_coord[d] < to_coordB[d]) { return(true); }
            }
            else if (to_coord[d] <= to_coordB[d])
              { return true;	}
					}
				}

				VERTEX_INDEX next_cube = grid.NextVertex(from_cube, d);
				INDEX_DIFF_TYPE next_gcube = isovert.GCubeIndex(next_cube);

				if (next_gcube != ISOVERT::NO_INDEX) {

          NUM_TYPE to_gcubeB = gcube_map[next_gcube];
					if (to_gcubeB != to_gcube) {

            VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
            const GRID_COORD_TYPE * to_coordB = 
              isovert.gcube_list[to_gcubeB].cube_coord;

            if (to_gcubeB != gcubeC_index) { 
              if (to_coord[d] > to_coordB[d]) { return true; }
            }
            else if (to_coord[d] >= to_coordB[d])
              { return true;	}
					}
				}
			}
		}
		
		return false;
	}

  /// Return true if order of coordA[d] and coordB[d] is correct.
  /// @param j If -1, coordA[d] should precede coordB[d].
  ///          If 1, coordA[d] should follow coordB[d].
  template <typename CTYPE0, typename CTYPE1>
  bool is_strict_order_correct
  (const int d, const int j, 
   const CTYPE0 coordA[DIM3], const CTYPE1 coordB[DIM3])
  {
    if (j < 0)
      { return((coordA[d] < coordB[d])); }
    else
      { return((coordA[d] > coordB[d])); }
  }

  /// Return true if order of coordA[d] and coordB[d] is correct.
  /// @param j If -1, coordA[d] should precede coordB[d].
  ///          If 1, coordA[d] should follow coordB[d].
  template <typename CTYPE0, typename CTYPE1>
  bool is_order_correct
  (const int d, const int j, 
   const CTYPE0 coordA[DIM3], const CTYPE1 coordB[DIM3])
  {
    if (j < 0)
      { return((coordA[d] <= coordB[d])); }
    else
      { return((coordA[d] >= coordB[d])); }
  }

  /// Return true if difference between coordA[d] and coordB[d] 
  ////  is bounded by 1.
  /// @param j If -1, return true if coordA[d] <= coordB[d]+1
  ///          If 1, return true if coordA[d]+1 >= coordB[d]
  bool is_difference_bounded
  (const int d, const int j, 
   const GRID_COORD_TYPE coordA[DIM3], const GRID_COORD_TYPE coordB[DIM3])
  {
    if (j < 0)
      { return((coordA[d] <= coordB[d]+1)); }
    else
      { return((coordA[d]+1 >= coordB[d])); }
  }

  /// Return true if 3 cubes in given direction from icubeA are mapped
  /// @param j1 Backward (-1) or forward (1).
  /// @param j2 Backward (-1) or forward (1).
  bool are_cubes_mapped_III
  (const SHARPISO_GRID & grid, const VERTEX_INDEX cubeA_index,
   const int d1, const int j1, const int d2, const int j2,
   const ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    VERTEX_INDEX cube_index[3];

    cube_index[0] = cubeA_index + grid.AxisIncrement(d1)*j1;
    cube_index[1] = cubeA_index + grid.AxisIncrement(d2)*j2;
    cube_index[2] = cube_index[0] + grid.AxisIncrement(d2)*j2;

    for (int i = 0; i < 3; i++) {
      const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index[i]);
      if (gcube_index == ISOVERT::NO_INDEX) { continue; }
      if (gcube_map[gcube_index] == gcube_index) { return(false); }
    }

    return(true);
  }

  // *** NOT CURRENTLY USED ***
  /// Return true if 7 cubes in given direction from icubeA are mapped
  /// @param j0 Backward (-1) or forward (1).
  /// @param j1 Backward (-1) or forward (1).
  /// @param j2 Backward (-1) or forward (1).
  bool are_cubes_mapped_VII
  (const SHARPISO_GRID & grid, const VERTEX_INDEX cubeA_index,
   const int j0, const int j1, const int j2,
   const ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    VERTEX_INDEX cube_index[7];

    cube_index[0] = cubeA_index + grid.AxisIncrement(0)*j0;
    cube_index[1] = cubeA_index + grid.AxisIncrement(1)*j1;
    cube_index[2] = cubeA_index + grid.AxisIncrement(2)*j2;
    cube_index[3] = cube_index[0] + grid.AxisIncrement(1)*j1;
    cube_index[4] = cube_index[0] + grid.AxisIncrement(2)*j2;
    cube_index[5] = cube_index[1] + grid.AxisIncrement(2)*j2;
    cube_index[6] = cube_index[3] + grid.AxisIncrement(2)*j2;

    for (int i = 0; i < 7; i++) {
      const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index[i]);
      if (gcube_index == ISOVERT::NO_INDEX) { continue; }
      if (gcube_map[gcube_index] == gcube_index) { 

        // *** DEBUG ***
        MSDEBUG();
        if (flag_debug) {
          isovert.grid.PrintIndexAndCoord
            (cerr, "Cube ", cube_index[i], " is not mapped.\n");
        }

        return(false); 
      }
    }

    // *** DEBUG ***
    MSDEBUG();
    if (flag_debug) {
      cerr << "Returning true from " << __func__ << endl;
    }

    return(true);
  }

  /// Return true if adjacent cube maps to to_cubeA or to_cubeB
  bool does_adjacent_cube_map_to
  ( const SHARPISO_GRID & grid, 
		const MERGESHARP::ISOVERT & isovert,
    const VERTEX_INDEX cube0_index,
    const int dir, const int side,
    const VERTEX_INDEX to_cubeA,
    const VERTEX_INDEX to_cubeB,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE gcube0_index = isovert.GCubeIndex(cube0_index);
    INDEX_DIFF_TYPE to_gcubeA = isovert.GCubeIndex(to_cubeA);
    INDEX_DIFF_TYPE to_gcubeB = isovert.GCubeIndex(to_cubeB);

    if (gcube0_index == ISOVERT::NO_INDEX) { return(false); }
    if (to_gcubeA == ISOVERT::NO_INDEX) { return(false); }
    if (to_gcubeB == ISOVERT::NO_INDEX) { return(false); }

    VERTEX_INDEX adj_cube_index = 
      grid.AdjacentVertex(cube0_index, dir, side);
    INDEX_DIFF_TYPE adj_gcube_index = isovert.GCubeIndex(adj_cube_index);
    if (adj_gcube_index == ISOVERT::NO_INDEX) { return(false); }

    if (gcube_map[adj_gcube_index] == to_gcubeA ||
        gcube_map[adj_gcube_index] == to_gcubeB)
      { return(true); }
    else
      { return(false); }
  }
    
  /// Check if merge maps cube to same axis coordinate as some edge neighbor.
  ///   Ignore neighbors which map to the same vertex as cube.
	bool does_merge_identify_edge_adjacent_axis_coord(
		const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube,
		const INDEX_DIFF_TYPE to_gcube,
    const INDEX_DIFF_TYPE gcubeC_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const VERTEX_INDEX * axis_increment = grid.AxisIncrement();
    const GRID_COORD_TYPE * to_coord = isovert.gcube_list[to_gcube].cube_coord;
		VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
		VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube);

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      cerr << "In " << __func__ << endl;
      grid.PrintIndexAndCoord(cerr, "  From cube: ", from_cube, "\n");
      grid.PrintIndexAndCoord(cerr, "  To cube: ", to_cube, "\n");
      grid.PrintIndexAndCoord
        (cerr, "  cubeC: ", isovert.CubeIndex(gcubeC_index), "\n");
    }

		if (isovert.gcube_list[from_gcube].boundary_bits == 0) {
			for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

        int d1 = (edge_dir+1)%DIM3;
        int d2 = (edge_dir+2)%DIM3;

        for (int j1 = -1; j1 < 2; j1+=2) {
          for (int j2 = -1; j2 < 2; j2+=2) {
            VERTEX_INDEX cubeB_index =
              from_cube + j1*axis_increment[d1] + j2*axis_increment[d2];
            INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);

            if (gcubeB_index == ISOVERT::NO_INDEX) { continue; }

            // *** DEBUG ***
            if (flag_debug) {
              grid.PrintIndexAndCoord
                (cerr, "  Adjacent cube: ", cubeB_index, "\n");
            }

            NUM_TYPE to_gcubeB = gcube_map[gcubeB_index];
            if (to_gcubeB != to_gcube) {

              VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
              const GRID_COORD_TYPE * to_coordB = 
                isovert.gcube_list[to_gcubeB].cube_coord;

              if (to_gcubeB == gcubeC_index) {

                if (are_cubes_mapped_III
                    (grid, from_cube, d1, j1, d2, j2, isovert, gcube_map))
                  { continue; }

                if (!is_order_correct(d1, j1, to_coordB, to_coord)) {

                  if (!does_adjacent_cube_map_to
                      (grid, isovert, from_cube, d2, j2, 
                       to_cube, to_cubeB, gcube_map))
                    { return(true); }
                }
                if (!is_order_correct(d2, j2, to_coordB, to_coord)) {

                  if (!does_adjacent_cube_map_to
                      (grid, isovert, from_cube, d1, j1, 
                       to_cube, to_cubeB, gcube_map))
                    { return(true); }
                }
              }
              else {
                if (!is_strict_order_correct(d1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(d2, j2, to_coordB, to_coord))
                  { return(true); }
              }
            }
          }
        }
			}
		}
		
		return false;
	}

  /// Check if merge maps cube to same axis coordinate as some edge neighbor.
  ///   Ignore neighbors which map to the same vertex as cube.
  ///   Use looser rules when vertices map to other cubes.
	bool does_merge_identify_edge_adjacent_axis_coord_B(
		const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube,
		const INDEX_DIFF_TYPE to_gcube,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const VERTEX_INDEX * axis_increment = grid.AxisIncrement();
    const GRID_COORD_TYPE * to_coord = isovert.gcube_list[to_gcube].cube_coord;
    const GRID_COORD_TYPE * from_coord = 
      isovert.gcube_list[from_gcube].cube_coord;
		VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
		VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube);

    // *** DEBUG ***
    using namespace std;
    if (flag_debug) {
      cerr << "In " << __func__ << endl;
      grid.PrintIndexAndCoord
        (cerr, "  From ", from_cube, " to ", to_cube, "\n");
    }

		if (isovert.gcube_list[from_gcube].boundary_bits == 0) {
			for (int edge_dir = 0; edge_dir < DIM3; edge_dir++) {

        int d1 = (edge_dir+1)%DIM3;
        int d2 = (edge_dir+2)%DIM3;

        for (int j1 = -1; j1 < 2; j1+=2) {
          for (int j2 = -1; j2 < 2; j2+=2) {
            VERTEX_INDEX cubeB_index =
              from_cube + j1*axis_increment[d1] + j2*axis_increment[d2];
            INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);

            if (gcubeB_index == ISOVERT::NO_INDEX) { continue; }

            // *** DEBUG ***
            if (flag_debug) {
              grid.PrintIndexAndCoord
                (cerr, "  Adjacent cube: ", cubeB_index, "\n");
            }

            NUM_TYPE to_gcubeB = gcube_map[gcubeB_index];
            if (to_gcubeB != to_gcube) {

              VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
              const GRID_COORD_TYPE * to_coordB = 
                isovert.gcube_list[to_gcubeB].cube_coord;

              // *** DEBUG ***
              /*
              MSDEBUG();
              if (flag_debug) {
                bool flag =
                  IJK::is_coord_in_rectangular_region
                  (DIM3, to_coordB, from_coord, to_coord);
                cerr << "     Region: ";
                IJK::print_coord3D(cerr, from_coord);
                IJK::print_coord3D(cerr, to_coord);
                if (flag) { cerr << " contains "; }
                else { cerr << " does not contain "; }
                IJK::print_coord3D(cerr, to_coordB);
                cerr << endl;
              }
              */

              if (to_gcubeB != gcubeB_index) {

                if (are_cubes_mapped_III
                    (grid, from_cube, d1, j1, d2, j2, isovert, gcube_map))
                  { continue; }

                if (!is_order_correct(d1, j1, to_coordB, to_coord)) {
                  if (!does_adjacent_cube_map_to
                      (grid, isovert, from_cube, d2, j2, 
                       to_cube, to_cubeB, gcube_map))
                    { return(true); }
                }
                if (!is_order_correct(d2, j2, to_coordB, to_coord)) {
                  if (!does_adjacent_cube_map_to
                      (grid, isovert, from_cube, d1, j1, 
                       to_cube, to_cubeB, gcube_map))
                    { return(true); }
                }
              }
              else {
                if (!is_strict_order_correct(d1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(d2, j2, to_coordB, to_coord))
                  { return(true); }
              }
            }
          }
        }
			}
		}
		
		return false;
	}

  /// Check if merge maps cube to same axis coordinate as some vertex neighbor.
  ///   Ignore neighbors which map to the same vertex as cube.
	bool does_merge_identify_vertex_adjacent_axis_coord(
		const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube,
		const INDEX_DIFF_TYPE to_gcube,
    const INDEX_DIFF_TYPE gcubeC_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const VERTEX_INDEX * axis_increment = grid.AxisIncrement();
		VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
		VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube);
    const GRID_COORD_TYPE * to_coord = 
      isovert.gcube_list[to_gcube].cube_coord;
    const GRID_COORD_TYPE * from_coord =
      isovert.gcube_list[from_gcube].cube_coord;

    MSDEBUG();
    if (flag_debug) {
      cerr << "In " << __func__ << endl;
      grid.PrintIndexAndCoord
        (cerr, "  From ", from_cube, " to ", to_cube, "\n");
    }

		if (isovert.gcube_list[from_gcube].boundary_bits == 0) {

      for (int j0 = -1; j0 < 2; j0+=2) {
        for (int j1 = -1; j1 < 2; j1+=2) {
          for (int j2 = -1; j2 < 2; j2+=2) {
            VERTEX_INDEX cubeB_index =
              from_cube + j0*axis_increment[0] +
              j1*axis_increment[1] + j2*axis_increment[2];
            INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);

            if (gcubeB_index == ISOVERT::NO_INDEX) { continue; }

            NUM_TYPE to_gcubeB = gcube_map[gcubeB_index];
            if (to_gcubeB != to_gcube) {

              const GRID_COORD_TYPE * coordB =
                isovert.gcube_list[gcubeB_index].cube_coord;

              if (IJK::is_coord_in_rectangular_region
                  (DIM3, from_coord, coordB, to_coord) && 
                  !IJK::is_some_coord_equal(DIM3, from_coord, coordB)) {

                // *** DEBUG ***
                MSDEBUG();
                if (flag_debug) {
                  grid.PrintIndexAndCoord
                    (cerr, "  From cube: ", from_cube, " between ",
                     cubeB_index, " and ", to_cube, "\n");
                }

                continue;
              }

              VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
              const GRID_COORD_TYPE * to_coordB = 
                isovert.gcube_list[to_gcubeB].cube_coord;

              // *** DEBUG ***
              if (flag_debug) {
                grid.PrintIndexAndCoord
                  (cerr, "  Adjacent cube: ", cubeB_index, 
                   " maps to ", to_cubeB, "\n");
                cerr << "     j0: " << j0 << "  j1: " << j1
                     << "  j2: " << j2 << endl;
              }

              if (to_gcubeB == gcubeC_index) {
                if (!is_order_correct(0, j0, to_coordB, to_coord))
                  { return(true); }
                if (!is_order_correct(1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_order_correct(2, j2, to_coordB, to_coord))
                  { return(true); }
              }
              else {
                if (!is_strict_order_correct(0, j0, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(2, j2, to_coordB, to_coord))
                  { return(true); }
              }
            }
          }
        }
			}
		}
		
		return false;
	}

  /// Check if merge maps cube to same axis coordinate as some vertex neighbor.
  ///   Ignore neighbors which map to the same vertex as cube.
  ///   Use looser rules when vertices map to other cubes.
	bool does_merge_identify_vertex_adjacent_axis_coord_B(
		const SHARPISO_GRID & grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube,
		const INDEX_DIFF_TYPE to_gcube,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
    const VERTEX_INDEX * axis_increment = grid.AxisIncrement();
		VERTEX_INDEX to_cube = isovert.CubeIndex(to_gcube);
		VERTEX_INDEX from_cube = isovert.CubeIndex(from_gcube);
    const GRID_COORD_TYPE * to_coord = 
      isovert.gcube_list[to_gcube].cube_coord;
    const GRID_COORD_TYPE * from_coord =
      isovert.gcube_list[from_gcube].cube_coord;

    MSDEBUG();
    if (flag_debug) {
      cerr << "In " << __func__ << endl;
      grid.PrintIndexAndCoord(cerr, "  From cube: ", from_cube, "\n");
      grid.PrintIndexAndCoord(cerr, "  To cube: ", to_cube, "\n");
    }

		if (isovert.gcube_list[from_gcube].boundary_bits == 0) {

      for (int j0 = -1; j0 < 2; j0+=2) {
        for (int j1 = -1; j1 < 2; j1+=2) {
          for (int j2 = -1; j2 < 2; j2+=2) {
            VERTEX_INDEX cubeB_index =
              from_cube + j0*axis_increment[0] +
              j1*axis_increment[1] + j2*axis_increment[2];
            INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);

            if (gcubeB_index == ISOVERT::NO_INDEX) { continue; }

            // *** DEBUG ***
            if (flag_debug) {
              grid.PrintIndexAndCoord
                (cerr, "  Adjacent cube: ", cubeB_index, "\n");
            }

            NUM_TYPE to_gcubeB = gcube_map[gcubeB_index];
            if (to_gcubeB != to_gcube) {

              VERTEX_INDEX to_cubeB = isovert.CubeIndex(to_gcubeB);
              const GRID_COORD_TYPE * to_coordB = 
                isovert.gcube_list[to_gcubeB].cube_coord;

              const GRID_COORD_TYPE * coordB =
                isovert.gcube_list[gcubeB_index].cube_coord;

              if (IJK::is_coord_in_rectangular_region
                  (DIM3, from_coord, coordB, to_coord) && 
                  !IJK::is_some_coord_equal(DIM3, from_coord, coordB)) {

                // *** DEBUG ***
                MSDEBUG();
                if (flag_debug) {
                  grid.PrintIndexAndCoord
                    (cerr, "  From cube: ", from_cube, " between ",
                     cubeB_index, " and ", to_cube, "\n");
                }

                continue;
              }

              // *** DEBUG ***
              if (flag_debug) {
                grid.PrintIndexAndCoord
                  (cerr, "    Maps to: ", to_cubeB, "\n");
                cerr << "     j0: " << j0 << "  j1: " << j1
                     << "  j2: " << j2 << endl;
              }

              if (to_gcubeB != cubeB_index) {

                // *** DEBUG ***
                MSDEBUG();
                if (flag_debug) {
                  cerr << "  Calling is_difference_bounded.";
                  grid.PrintIndexAndCoord
                    (cerr, "  to_cubeB: ", to_cubeB, "\n");
                }


                if (!is_difference_bounded(0, j0, to_coordB, to_coord))
                  { return(true); }
                if (!is_difference_bounded(1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_difference_bounded(2, j2, to_coordB, to_coord))
                  { return(true); }
              }
              else {
                if (!is_strict_order_correct(0, j0, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(1, j1, to_coordB, to_coord))
                  { return(true); }
                if (!is_strict_order_correct(2, j2, to_coordB, to_coord))
                  { return(true); }
              }
            }
          }
        }
			}
		}
		
		return false;
	}

	/*
	* check if mapping of vertex in from_cube to corner cube to_cube is permitted.
	*/
	bool is_corner_cube_merge_permitted(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube_index,
		const INDEX_DIFF_TYPE to_gcube_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    if (does_merge_reverse_isovert_order
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, gcube_map)) {

        // *** DEBUG ***
        using namespace std;
        if (flag_debug) { cerr << "  Failed reverse order test." << endl; }

        return(false); 
    }

    if (does_cube_merge_distort_triangles
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, gcube_map)) {

      // *** DEBUG ***
      using namespace std;
      if (flag_debug) { cerr << "  Failed distorts triangle test." << endl; }

      return(false);
    }

    return(true);
  }

  /// Check if mapping of vertex in from_cube to to_cube is permitted.
	bool is_cube_merge_permitted(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE from_gcube_index,
		const INDEX_DIFF_TYPE to_gcube_index,
    const INDEX_DIFF_TYPE gcubeC_index,
		const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    if (does_merge_identify_facet_adjacent_axis_coord
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, 
         gcubeC_index, gcube_map))
      { 
        // *** DEBUG ***
        using namespace std;
        if (flag_debug) { cerr << "  Failed facet adjacent test." << endl; }

        return(false); }

    if (isovert.NumEigenvalues(to_gcube_index) == 3) {
      if (does_merge_identify_edge_adjacent_axis_coord_B
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, gcube_map))
        { 
          // *** DEBUG ***
          using namespace std;
          if (flag_debug) { cerr << "  Failed edge adjacent test B." << endl; }

          return(false); }

      if (does_merge_identify_vertex_adjacent_axis_coord_B
          (scalar_grid, isovert, from_gcube_index, to_gcube_index, gcube_map))
        { 
          // *** DEBUG ***
          using namespace std;
          if (flag_debug) { cerr << "  Failed vertex adjacent test B." << endl; }

          return(false); }
    }
    else {
      if (does_merge_identify_edge_adjacent_axis_coord
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, 
         gcubeC_index, gcube_map))
        { 
          // *** DEBUG ***
          using namespace std;
          if (flag_debug) { cerr << "  Failed edge adjacent test." << endl; }

          return(false); }

      if (does_merge_identify_vertex_adjacent_axis_coord
          (scalar_grid, isovert, from_gcube_index, to_gcube_index, 
           gcubeC_index, gcube_map))
        { 
          // *** DEBUG ***
          using namespace std;
          if (flag_debug) { cerr << "  Failed vertex adjacent test." << endl; }

          return(false); }
    }



    if (does_cube_merge_distort_triangles
        (scalar_grid, isovert, from_gcube_index, to_gcube_index, gcube_map)) {
      return(false);
    }

    return(true);
  }

  /// Check if mapping of vertices in cubeA and cubeB to to_cube is permitted.
	bool is_cube_merge_permittedII(
		const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
		const MERGESHARP::ISOVERT & isovert,
		const INDEX_DIFF_TYPE gcubeA_index,
		const INDEX_DIFF_TYPE gcubeB_index,
		const INDEX_DIFF_TYPE to_gcube_index,
    const INDEX_DIFF_TYPE gcubeC_index,
    std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcube_index[2] = { gcubeA_index, gcubeB_index };
    NUM_TYPE store_map[2];

    for (int i = 0; i < 2; i++) 
      { store_map[i] = gcube_map[gcube_index[i]]; }

    for (int i0 = 0; i0 < 2; i0++) {
      
      int i1 = (i0+1) % 2;

      // Temporarily set gcube_index[i1] to map to to_cube.
      gcube_map[gcube_index[i1]] = to_gcube_index;

      bool flag_merge_permitted =
        is_cube_merge_permitted
        (scalar_grid, isovert, gcube_index[i0], to_gcube_index, gcubeC_index,
         gcube_map);

      // Reset gcube_map[]
      gcube_map[gcube_index[i1]] = store_map[i1];

      if (!flag_merge_permitted) { return(false); }
    }

    return(true);
  }

  /// Check if mapping of 3 vertices in 3 cubes to to_cube is permitted.
	bool is_cube_merge_permittedIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const MERGESHARP::ISOVERT & isovert,
   const INDEX_DIFF_TYPE gcube_index[3],
   const INDEX_DIFF_TYPE to_gcube_index,
   const INDEX_DIFF_TYPE gcubeB_index,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    NUM_TYPE store_map[3];

    for (int i = 0; i < 3; i++) 
      { store_map[i] = gcube_map[gcube_index[i]]; }

    for (int i0 = 0; i0 < 3; i0++) {
      
      int i1 = (i0+1) % 3;
      int i2 = (i0+2) % 3;

      // Temporarily set gcube_index[i1] and gcube_index[i2] to map to to_cube.
      gcube_map[gcube_index[i1]] = to_gcube_index;
      gcube_map[gcube_index[i2]] = to_gcube_index;

      bool flag_merge_permitted =
        is_cube_merge_permitted
        (scalar_grid, isovert, gcube_index[i0], to_gcube_index, gcubeB_index,
         gcube_map);

      // Reset gcube_map[]
      gcube_map[gcube_index[i1]] = store_map[i1];
      gcube_map[gcube_index[i2]] = store_map[i2];

      if (!flag_merge_permitted) { return(false); }
    }

    return(true);
  }

  /// Return true if region contains cube.
  /// Note: Region is given by cube indices, not vertex coord.
  bool does_region_contain_cube
  (const MERGESHARP::GRID_BOX & region, const VERTEX_INDEX cube_index,
   const MERGESHARP::ISOVERT & isovert)
  {
    const INDEX_DIFF_TYPE gcube_index = isovert.GCubeIndex(cube_index);
    if (gcube_index == ISOVERT::NO_INDEX) { return(false); }
    
    const GRID_COORD_TYPE * cube_coord = 
      isovert.gcube_list[gcube_index].cube_coord;

    return(region.Contains(cube_coord));
  }

  /// Return true if region contains cube.
  /// Note: Region is given by cube indices, not vertex coord.
  /// @param[out] is_region_vertex True if cube_coord is region vertex.
  bool does_region_contain_cube
  (const MERGESHARP::GRID_BOX & region, const GRID_COORD_TYPE cube_coord[DIM3],
   bool & is_region_vertex)
  {
    is_region_vertex = true;
    for (int d = 0; d < DIM3; d++) {
      if (cube_coord[d] < region.MinCoord(d) || 
          cube_coord[d] > region.MaxCoord(d)) {

        is_region_vertex = false;
        return(false);
      }

      if (cube_coord[d] > region.MinCoord(d) &&
          cube_coord[d] < region.MaxCoord(d))
        { is_region_vertex = false; }
    }

    return(true);
  }

  void insert_selected_in_bin_grid
  (const SHARPISO_GRID & grid, const ISOVERT & isovert, 
   const std::vector<NUM_TYPE> & sharp_gcube_list,
   const AXIS_SIZE_TYPE bin_width, BIN_GRID<VERTEX_INDEX> & bin_grid)
  {
    for (NUM_TYPE i=0; i < sharp_gcube_list.size(); i++) {

      NUM_TYPE gcube_index = sharp_gcube_list[i];

      if (isovert.gcube_list[gcube_index].flag == SELECTED_GCUBE) {
        VERTEX_INDEX cube_index = isovert.CubeIndex(gcube_index);
        bin_grid_insert(grid, bin_width, cube_index, bin_grid); 
      }
    }
  }

  /// Count number of coordinates differences
  /// *** NOT USED ***
  /// @param[out] num_diff1 Number of coordinates with difference 1.
  /// @param[out] num_diff2_or_more Number of coordinates at least 2.
  void count_num_coord_diff
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
   const VERTEX_INDEX icube0, const VERTEX_INDEX icube1,
   NUM_TYPE & num_diff1, NUM_TYPE & num_diff2_or_more)
  {
    GRID_COORD_TYPE coord0[DIM3];
    GRID_COORD_TYPE coord1[DIM3];

		scalar_grid.ComputeCoord(icube0, coord0);
		scalar_grid.ComputeCoord(icube1, coord1); 

    num_diff1 = 0;
    num_diff2_or_more = 0;

		for (int d = 0; d < DIM3; d++) {

      GRID_COORD_TYPE diff = abs(coord0[d]-coord1[d]);
      if (diff == 1) 
        { num_diff1++; }
      else if (diff > 1)
        { num_diff2_or_more++; }
		}
  }


  /// Check extended mapping and map from cube from_cube to cube to_cube.
  /// Cube from_cube is (facet, edge or vertex) adjacent to cubeA 
  ///   or equals cubeA.
	void check_extended_and_map
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX from_cube,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    INDEX_DIFF_TYPE from_gcube = isovert.GCubeIndex(from_cube);
    INDEX_DIFF_TYPE to_gcube = isovert.GCubeIndex(to_cube);
    VERTEX_INDEX cubeC_index;
    static CUBE_CONNECTED_ARRAY connected_sharp;

    if (from_gcube == ISOVERT::NO_INDEX ||
        to_gcube == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[from_gcube].IsCoveredOrSelected()) { return; }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "*** Checking mapping from ", from_cube,
         " to ", to_cube, "\n");
    }
      

    if (!check_facet_adjacent_maps
        (scalar_grid, isovert, gcube_map, from_cube, to_cube, cubeC_index))
      { 

        MSDEBUG();
        if (flag_debug)
          { cerr << "  Failed check_facet_adjacent_maps." << endl; }

        return; }

    INDEX_DIFF_TYPE gcubeC_index = isovert.GCubeIndex(cubeC_index);
    if (!is_cube_merge_permitted
        (scalar_grid, isovert, from_gcube, to_gcube, gcubeC_index, gcube_map))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed is_cube_merge_permitted." << endl; }

        return; }

    if (!check_map(scalar_grid, isovalue, from_cube, to_cube,
                   isovert, gcube_map, true))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_map." << endl; }

        return; }

		find_connected_sharp(scalar_grid, isovalue, from_cube, isovert, 
                         gcube_map, connected_sharp);

    if (!check_adjacent_cubes(scalar_grid, isovalue, from_cube,
                              isovert, gcube_map, connected_sharp))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_adjacent_cubes." << endl; }
        
        return; }

    // *** DOES check_map ALREADY CHECK THIS? ***
    if (!check_connected_sharp
        (scalar_grid, isovalue, isovert, gcube_map, from_cube, to_cube,
         connected_sharp))
      { 
        // *** DEBUG ***
        if (flag_debug) {
          scalar_grid.PrintIndexAndCoord
            (cerr, "*** Failed check_connected_sharp. Cube: ", from_cube, "");
          scalar_grid.PrintIndexAndCoord
            (cerr, " to ", to_cube, "\n");
        }

        return; }

    map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                   from_gcube, to_gcube, gcube_map);
    isovert.gcube_list[from_gcube].flag = COVERED_B_GCUBE;
	}


  /// Check extended mapping of cubeA and cubeB to to_cube and map.
	void check_extended_and_mapII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const VERTEX_INDEX to_cube_index,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    const INDEX_DIFF_TYPE gcubeA_index = isovert.GCubeIndex(cubeA_index);
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);
    const INDEX_DIFF_TYPE to_gcube_index = isovert.GCubeIndex(to_cube_index);
    VERTEX_INDEX to_cubeA_index, to_cubeB_index, to_cubeC_index;

    flag_map = false;

    if (gcubeA_index == ISOVERT::NO_INDEX || 
        gcubeB_index == ISOVERT::NO_INDEX ||
        to_gcube_index == ISOVERT::NO_INDEX) { return; }

    if (isovert.gcube_list[gcubeA_index].IsCoveredOrSelected() &&
        isovert.gcube_list[gcubeB_index].IsCoveredOrSelected())
      { return; }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "*** Checking (extended) mapping of ", cubeA_index, " and ",
         cubeB_index, "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "        to ", to_cube_index, "\n");
    }

    bool flag_fadj_map_A =
      check_facet_adjacent_maps
      (scalar_grid, isovert, gcube_map, cubeA_index, to_cube_index, 
       to_cubeA_index);
    bool flag_fadj_map_B =
      check_facet_adjacent_maps
      (scalar_grid, isovert, gcube_map, cubeB_index, to_cube_index, 
       to_cubeB_index);

    if (flag_fadj_map_A && flag_fadj_map_B) {
      if (to_cubeA_index != to_cubeB_index) {
        MSDEBUG();
        if (flag_debug) {
          cerr << "  No mapping. Adjacent cubes map to different selected cubes." << endl;  
        }

        return;
      }
    }

    if (flag_fadj_map_A) { to_cubeC_index = to_cubeA_index; }
    else if (flag_fadj_map_B) { to_cubeC_index = to_cubeB_index; }
    else {
      MSDEBUG();
      if (flag_debug) 
        { cerr << "  Failed check_facet_adjacent_maps." << endl;  }

      return; 
    }

    const INDEX_DIFF_TYPE to_gcubeC_index = isovert.GCubeIndex(to_cubeC_index);

    if (!is_cube_merge_permittedII
        (scalar_grid, isovert, gcubeA_index, gcubeB_index, 
         to_gcube_index, to_gcubeC_index, gcube_map)) {

      MSDEBUG();
      if (flag_debug) 
        { cerr << "  Failed is_cube_merge_permittedII." << endl; }

      return; 
    }

    if (!check_mapII(scalar_grid, isovalue, cubeA_index, to_cube_index,
                     cubeB_index, to_cube_index, isovert, merge_param,
                     gcube_map, true)) {

      MSDEBUG();
      if (flag_debug) 
        { cerr << "  Failed check_mapII." << endl; }

      return; 
    }

    if (!check_adjacent_cubesII
        (scalar_grid, isovalue, cubeA_index, cubeB_index, to_cube_index,
         isovert, gcube_map))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_adjacent_cubesII." << endl; }
        
        return; }

    if (flag_debug) {
      using namespace std;
      scalar_grid.PrintIndexAndCoord
        (cerr, "xxxMapping (extended) ", cubeA_index, 
         " and ", cubeB_index, "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "     to cube ", to_cube_index, "\n");
    }

    flag_map = true;
    map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                   gcubeA_index, to_gcube_index, gcube_map);
    map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                   gcubeB_index, to_gcube_index, gcube_map);
    isovert.gcube_list[gcubeA_index].flag = COVERED_B_GCUBE;
    isovert.gcube_list[gcubeB_index].flag = COVERED_B_GCUBE;
	}

  /// Check extended mapping of three cubes to to_cube and map.
	void check_extended_and_mapIII
  (const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX cube_index[3],
   const VERTEX_INDEX to_cube_index,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert, 
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   bool & flag_map)
  {
    IJK::PROCEDURE_ERROR error("check_extended_and_mapIII");
    INDEX_DIFF_TYPE gcube_index[3];
    const INDEX_DIFF_TYPE to_gcube_index = 
      isovert.GCubeIndex(to_cube_index, error);

    flag_map = false;

    for (int i = 0; i < 3; i++) {
      gcube_index[i] = isovert.GCubeIndex(cube_index[i], error);
      if (gcube_index[i] == ISOVERT:: NO_INDEX) { throw error; }
    }
    if (to_gcube_index == ISOVERT::NO_INDEX) { throw error; }

    MSDEBUG();
    if (flag_debug) {
      scalar_grid.PrintIndexAndCoord
        (cerr, "*** Checking (extended) mapping III of ", cube_index[0], 
         " and ", cube_index[1], "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "        and ", cube_index[2], " to ", to_cube_index, "\n");
    }

    bool flag_fadj_map = false;
    VERTEX_INDEX to_cubeB_index = 0;
    for (int i = 0; i < 3; i++) {
      VERTEX_INDEX to_cubeC_index;
      bool flag =
        check_facet_adjacent_maps
        (scalar_grid, isovert, gcube_map, cube_index[i], to_cube_index,
         to_cubeC_index);
      if (flag) {
        if (flag_fadj_map) {
          if (to_cubeC_index != to_cubeB_index) {

            MSDEBUG();
            if (flag_debug) {
              cerr << "Failed check_facet_adjacent_maps test." << endl;
              scalar_grid.PrintIndexAndCoord
                (cerr, "  Cubes near ", to_cubeB_index, " and ",
                 to_cubeC_index, "\n");
            }

            return; 
          }
        }

        to_cubeB_index = to_cubeC_index;
        flag_fadj_map = true;
      }
    }

    if (!flag_fadj_map) { 

      MSDEBUG();
      if (flag_debug)
        { cerr << "Failed check_facet_adjacent_maps test." << endl; }

      return; 
    }

    const INDEX_DIFF_TYPE to_gcubeB_index = isovert.GCubeIndex(to_cubeB_index);

    if (!is_cube_merge_permittedIII
        (scalar_grid, isovert, gcube_index,
         to_gcube_index, to_gcubeB_index, gcube_map)) {

      MSDEBUG();
      if (flag_debug) 
        { cerr << "  Failed is_cube_merge_permittedIII." << endl; }

      return; 
    }

    if (!check_mapIII(scalar_grid, isovalue, cube_index, to_cube_index,
                      isovert, merge_param, gcube_map, true)) {

      MSDEBUG();
      if (flag_debug) 
        { cerr << "  Failed check_mapIII." << endl; }

      return; 
    }

    if (!check_adjacent_cubesIII
        (scalar_grid, isovalue, cube_index, to_cube_index, isovert, gcube_map))
      { 
        /// *** DEBUG ***
        if (flag_debug) 
          { cerr << "  Failed check_adjacent_cubesIII." << endl; }
        
        return; }

    if (flag_debug) {
      using namespace std;
      scalar_grid.PrintIndexAndCoord
        (cerr, "xxxMapping (extended III) ", cube_index[0], 
         " and ", cube_index[1], "\n");
      scalar_grid.PrintIndexAndCoord
        (cerr, "     and ", cube_index[2], " to cube ", to_cube_index, "\n");
    }
  
    flag_map = true;
    for (int i = 0; i < 3; i++) {
      map_iso_vertex(scalar_grid, isovalue, isovert.gcube_list, 
                     gcube_index[i], to_gcube_index, gcube_map);
      isovert.gcube_list[gcube_index[i]].flag = COVERED_B_GCUBE;
    }
	}

  /// Map cube pairs (cubeB,cubeC) to cube to_cube using extended mapping
  ///   where cubeC is facet adjacent to cubeB.
  void extend_map_facet_adjacent_pairs
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);
    BOUNDARY_BITS_TYPE boundary_bits;

    if (gcubeB_index == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcubeB_index] != gcubeB_index) { return; }

    boundary_bits = isovert.gcube_list[gcubeB_index].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cubeB is an interior cube.

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {

        const VERTEX_INDEX cubeC_index = 
          isovert.grid.CubeNeighborF(cubeB_index, j);
        if (!is_cube_unmapped_unselected_active
            (cubeC_index, isovert, gcube_map))
          { continue; }

        bool flag_map;
        check_extended_and_mapII
          (scalar_grid, isovalue, cubeB_index, cubeC_index, to_cube,
           merge_param, isovert, gcube_map, flag_map);
        if (flag_map) { return; }
      }
    }
    else {
      // Handle boundary case
    }
  }

  // *** NOT USED/TESTED ***
  /// Map cube pairs (cubeB,cubeC) to cube to_cube using extended mapping
  ///   where cubeC is edge adjacent to cubeB.
  void extend_map_edge_adjacent_pairs
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);
    BOUNDARY_BITS_TYPE boundary_bits;

    if (gcubeB_index == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcubeB_index] != gcubeB_index) { return; }

    boundary_bits = isovert.gcube_list[gcubeB_index].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cubeB is an interior cube.

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) {

        const VERTEX_INDEX cubeC_index = 
          isovert.grid.CubeNeighborE(cubeB_index, j);
        if (!is_cube_unmapped_unselected_active
            (cubeC_index, isovert, gcube_map))
          { continue; }

        bool flag_map;
        check_extended_and_mapII
          (scalar_grid, isovalue, cubeB_index, cubeC_index, to_cube, 
           merge_param, isovert, gcube_map, flag_map);
        if (flag_map) { return; }
      }
    }
    else {
      // Handle boundary case
    }
  }

  /// Map cube triple to cube to_cube using extended mapping.
  void extend_map_triples
  (const VERTEX_INDEX cubeA_index,
   const VERTEX_INDEX cubeB_index,
   const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
   const SCALAR_TYPE isovalue,
   const VERTEX_INDEX to_cube_index,
   const MERGE_PARAM & merge_param,
   MERGESHARP::ISOVERT & isovert,
   std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(cubeB_index);
    VERTEX_INDEX cube_index[3];
    std::vector<VERTEX_INDEX> adjacent_cube_list;
    BOUNDARY_BITS_TYPE boundary_bits;

    if (gcubeB_index == ISOVERT::NO_INDEX) { return; }
    if (gcube_map[gcubeB_index] != gcubeB_index) { return; }

    boundary_bits = isovert.gcube_list[gcubeB_index].boundary_bits;

    if (boundary_bits == 0) {
      // Cube cubeB is an interior cube.

      get_adjacent_unmapped_in_5x5x5_region
        (cubeB_index, to_cube_index, isovert, gcube_map, adjacent_cube_list);

      cube_index[0] = cubeB_index;

      for (int i1 = 0; i1 < adjacent_cube_list.size(); i1++) {
        
        cube_index[1] = adjacent_cube_list[i1];
        if (!is_cube_unmapped_unselected_active
            (cube_index[1], isovert, gcube_map)) { continue; }

        for (int i2 = i1+1; i2 < adjacent_cube_list.size(); i2++) {

          cube_index[2] = adjacent_cube_list[i2];
          if (!is_cube_unmapped_unselected_active
              (cube_index[2], isovert, gcube_map)) { continue; }

          bool flag_map;
          check_extended_and_mapIII
            (scalar_grid, isovalue, cube_index, to_cube_index, merge_param,
             isovert, gcube_map, flag_map);
          if (flag_map) { return; }
        }
      }
    }
    else {
      // Handle boundary case
    }
  }

  /// Return true if cubeA is active, unmapped and in a 3x3x3 region 
  ///   around cubeB.
  bool is_unmapped_in_5x5x5_region
  (const VERTEX_INDEX icubeA, const VERTEX_INDEX icubeB, 
   const ISOVERT & isovert,
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
  {
    const INDEX_DIFF_TYPE gcubeA_index = isovert.GCubeIndex(icubeA);
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(icubeB);
    GRID_COORD_TYPE Linf_distance;

    if (gcubeA_index == ISOVERT::NO_INDEX) { return(false); }

    if (gcube_map[gcubeA_index] != gcubeA_index) { return(false); }

    const GRID_COORD_TYPE * cubeA_coord =
        isovert.gcube_list[gcubeA_index].cube_coord;
    const GRID_COORD_TYPE * cubeB_coord =
        isovert.gcube_list[gcubeB_index].cube_coord;

    IJK::compute_Linf_distance
      (DIM3, cubeA_coord, cubeB_coord, Linf_distance);

    if (Linf_distance > 3) { return(false); }

    return(true);
  }

  /// Get active, unmapped cubes which are adjacent to cubeA 
  ///   and in 5x5x5 region around cubeB
  void get_adjacent_unmapped_in_5x5x5_region
  (const VERTEX_INDEX icubeA, const VERTEX_INDEX icubeB,
   const ISOVERT & isovert, 
   const std::vector<SHARPISO::VERTEX_INDEX> & gcube_map,
   std::vector<VERTEX_INDEX> & cube_list)
  {
    IJK::PROCEDURE_ERROR error("get_adjacent_unmapped_in_3x3x3_region");
    const INDEX_DIFF_TYPE gcubeA_index = isovert.GCubeIndex(icubeA, error);
    const INDEX_DIFF_TYPE gcubeB_index = isovert.GCubeIndex(icubeB, error);

    cube_list.clear();

    if (gcubeA_index == ISOVERT::NO_INDEX) { throw error; };
    if (gcubeB_index == ISOVERT::NO_INDEX) { throw error; };

    const BOUNDARY_BITS_TYPE boundary_bits = 
      isovert.gcube_list[gcubeA_index].boundary_bits;

    if (boundary_bits == 0) {

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsF(); j++) {
        const VERTEX_INDEX icubeC = isovert.grid.CubeNeighborF(icubeA, j);

        if (is_unmapped_in_5x5x5_region(icubeC, icubeB, isovert, gcube_map))
          { cube_list.push_back(icubeC); }
      }

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsE(); j++) {
        const VERTEX_INDEX icubeC = isovert.grid.CubeNeighborE(icubeA, j);

        if (is_unmapped_in_5x5x5_region(icubeC, icubeB, isovert, gcube_map))
          { cube_list.push_back(icubeC); }
      }

      for (NUM_TYPE j = 0; j < isovert.grid.NumCubeNeighborsV(); j++) {
        const VERTEX_INDEX icubeC = isovert.grid.CubeNeighborV(icubeA, j);

        if (is_unmapped_in_5x5x5_region(icubeC, icubeB, isovert, gcube_map))
          { cube_list.push_back(icubeC); }
      }
    }
    
  }

}


// **************************************************
// Unmap non-disk isopatches
// **************************************************

namespace {

	void unmap_merged_cubes
		(MERGESHARP::ISOVERT & isovert, const VERTEX_INDEX cube_index0,
		const AXIS_SIZE_TYPE dist2cube,
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map)
	{
		const NUM_TYPE gcube_index0 = isovert.GCubeIndex(cube_index0);
		std::vector<VERTEX_INDEX> merged_cube_list;

		isovert.gcube_list[gcube_index0].flag = NON_DISK_GCUBE;

		get_merged_cubes(isovert.index_grid, isovert, cube_index0,
                     gcube_map, dist2cube,  merged_cube_list);
		for (NUM_TYPE i = 0; i < merged_cube_list.size(); i++) {
			VERTEX_INDEX cube_index1 = merged_cube_list[i];
			NUM_TYPE gcube_index1 = isovert.GCubeIndex(cube_index1);
			if (gcube_map[gcube_index1] == gcube_index0) {
				gcube_map[gcube_index1] = gcube_index1; 
				if (isovert.gcube_list[gcube_index1].flag == COVERED_A_GCUBE) {
					isovert.gcube_list[gcube_index1].flag = SMOOTH_GCUBE;
				}
			}
		}
	}

	// Reverse merges which create isopatches which are not disks.
	void unmap_non_disk_isopatches
		(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const SCALAR_TYPE isovalue,
		MERGESHARP::ISOVERT & isovert, 
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
		MERGESHARP::SHARPISO_INFO & sharpiso_info)
	{
		const NUM_TYPE num_gcube = isovert.gcube_list.size();

		const int dist2cube = 1;
		std::vector<ISO_VERTEX_INDEX> tri_vert;
		std::vector<ISO_VERTEX_INDEX> quad_vert;

		bool passed_all_disk_checks;
		do {
			passed_all_disk_checks = true;

			for (NUM_TYPE i = 0; i < num_gcube; i++) {
				if (isovert.gcube_list[i].flag == SELECTED_GCUBE) {
					VERTEX_INDEX cube_index = isovert.gcube_list[i].cube_index;

					extract_dual_isopatch_incident_on
						(scalar_grid, isovalue, isovert, cube_index,
						gcube_map, dist2cube, tri_vert, quad_vert);

					IJK::reorder_quad_vertices(quad_vert);

					if (!is_isopatch_disk3D(tri_vert, quad_vert)) {
						unmap_merged_cubes(isovert, cube_index, dist2cube, gcube_map);
						sharpiso_info.num_non_disk_isopatches++;
						passed_all_disk_checks = false;
					}
				}
			}
		}
		while (!passed_all_disk_checks);
	}

	// Renumber vertices in vlist so that they are numbered starting at 0.
	// Construct cube list of distinct vertices in vlist.
	template <typename VTYPE0, typename VTYPE1>
	void construct_cube_list
		(std::vector<VTYPE0> & vlist, std::vector<VTYPE1> & cube_list)
	{
		VERTEX_HASH_TABLE vertex_hash;

		cube_list.clear();
		insert_vertex_list(vlist, vertex_hash);

		remap_vertex_list(vertex_hash, vlist, vlist);

		cube_list.resize(vertex_hash.size());
		for (VERTEX_HASH_TABLE::const_iterator vertex_iter = vertex_hash.begin();
			vertex_iter != vertex_hash.end(); vertex_iter++) {
				VERTEX_INDEX iv = vertex_iter->first;
				NUM_TYPE n = vertex_iter->second;
				cube_list[n] = iv;
		}
	}

	// Reverse merges which create isopatches which are not disks.
	void unmap_non_disk_isopatches
		(const SHARPISO_SCALAR_GRID_BASE & scalar_grid, 
		const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
		const SCALAR_TYPE isovalue,
		MERGESHARP::ISOVERT & isovert, 
		std::vector<SHARPISO::VERTEX_INDEX> & gcube_map, 
		MERGESHARP::SHARPISO_INFO & sharpiso_info)
	{
		const NUM_TYPE num_gcube = isovert.gcube_list.size();

		const int dist2cube = 1;
		std::vector<ISO_VERTEX_INDEX> tri_vert;
		std::vector<ISO_VERTEX_INDEX> quad_vert;

		bool passed_all_disk_checks;
		do {
			passed_all_disk_checks = true;

			for (NUM_TYPE i = 0; i < num_gcube; i++) {
				if (isovert.gcube_list[i].flag == SELECTED_GCUBE) {
					VERTEX_INDEX cube_index = isovert.gcube_list[i].cube_index;

					extract_dual_isopatch_incident_on_multi
						(scalar_grid, isodual_table, isovalue, isovert, 
						cube_index, gcube_map, dist2cube, tri_vert, quad_vert);
					IJK::reorder_quad_vertices(quad_vert);

					if (!is_isopatch_disk3D(tri_vert, quad_vert)) {
						unmap_merged_cubes(isovert, cube_index, dist2cube, gcube_map);
						isovert.gcube_list[i].flag = NON_DISK_GCUBE;
						sharpiso_info.num_non_disk_isopatches++;
						passed_all_disk_checks = false;
					}
				}
			}
		}
		while (!passed_all_disk_checks);
	}

	// Set table_index[i] of cube cube_list[i].
	// Table index is stored in isovert.
	template <typename VTYPE, typename ITYPE>
	void set_table_index
		(const MERGESHARP::ISOVERT & isovert, const std::vector<VTYPE> & cube_list,
		std::vector<ITYPE> & table_index)
	{
		table_index.resize(cube_list.size());

		for (NUM_TYPE i = 0; i < cube_list.size(); i++) {
			VTYPE cube_index = cube_list[i];
			VTYPE gcube_index = isovert.GCubeIndex(cube_index);
			table_index[i] = isovert.gcube_list[gcube_index].table_index;
		}
	}


}



// **************************************************
// ROUTINE: is_isopatch_disk3D
// **************************************************

// Forward declarations:
template <typename VTYPE>
void renumber_tri_quad_vertices
	(const std::vector<VTYPE> & tri_vert,
	const std::vector<VTYPE> & quad_vert,
	std::vector<VTYPE> & new_tri_vert,
	std::vector<VTYPE> & new_quad_vert,
	NUM_TYPE & num_vert);

// Search cycle starting at iv0.
// @pre All vertices in cycle containing iv0 have is_visited set to false.
// @pre All vertices have degree two.
void search_cycle
	(const VERTEX_INDEX iv0, std::vector<CYCLE_VERTEX> & cycle_vertex);

// Return true if isopatch incident on vertex is a disk.
// @param tri_vert Triangle vertices.
// @param quad_vert Quadrilateral vertices in order around quadrilateral.
// @pre Assumes the boundary of the isopatch is the link of some vertex.
bool MERGESHARP::is_isopatch_disk3D
	(const std::vector<ISO_VERTEX_INDEX> & tri_vert,
	const std::vector<ISO_VERTEX_INDEX> & quad_vert)
{
	const NUM_TYPE num_tri = tri_vert.size()/NUM_VERT_PER_TRI;
	const NUM_TYPE num_quad = quad_vert.size()/NUM_VERT_PER_QUAD;
	std::vector<ISO_VERTEX_INDEX> tri_vert2;
	std::vector<ISO_VERTEX_INDEX> quad_vert2;
	NUM_TYPE num_vert;

	VERTEX_HASH_TABLE vertex_hash;
	EDGE_HASH_TABLE edge_hash;

	// Renumber tri and quad vertices
	renumber_tri_quad_vertices
		(tri_vert, quad_vert, tri_vert2, quad_vert2, num_vert);

	// Check for edges in more than two isosurface polygons.
	insert_tri_quad_edges(tri_vert2, quad_vert2, edge_hash);

	for (EDGE_HASH_TABLE::const_iterator edge_iter = edge_hash.begin();
		edge_iter != edge_hash.end(); edge_iter++) {
			if (edge_iter->second > 2) {
				return(false); 
			}
	}

	// Check that boundary is a cycle or edge.
	std::vector<CYCLE_VERTEX> cycle_vertex(num_vert);

	construct_boundary_cycle(edge_hash, cycle_vertex);
	NUM_TYPE num_boundary_vertices = 0;
	NUM_TYPE first_adjacent = 0;
	for (NUM_TYPE i = 0; i < cycle_vertex.size(); i++) {
		NUM_TYPE num_adjacent = cycle_vertex[i].num_adjacent;
		if (num_adjacent == 2) {
			first_adjacent = i;
			num_boundary_vertices++;
		}
		else if (num_adjacent != 0) {
			return(false); 
		}
	}

	if (num_boundary_vertices < 3) { 
		// Disk must have at least three boundary cycle vertices.
		return(false); 
	}

	search_cycle(first_adjacent, cycle_vertex);

	for (NUM_TYPE i = 0; i < cycle_vertex.size(); i++) { 
		if (cycle_vertex[i].num_adjacent == 2) {
			if (!cycle_vertex[i].is_visited) { return(false); }
		}
	}

	return(true);
}


/// Get list of cubes merged with icube.
void MERGESHARP::get_merged_cubes
	(const SHARPISO_GRID & grid,
	const ISOVERT & isovert,
	const VERTEX_INDEX cube_index0,
	const std::vector<VERTEX_INDEX> & gcube_map,
	const AXIS_SIZE_TYPE dist2cube,
	std::vector<VERTEX_INDEX> & merged_cube_list)
{
	const int dimension = grid.Dimension();
	const VERTEX_INDEX gcube_index0 = isovert.GCubeIndex(cube_index0);
	VERTEX_INDEX region_iv0;
	IJK::ARRAY<AXIS_SIZE_TYPE> region_axis_size(dimension);

	IJK::compute_region_around_cube
		(cube_index0, dimension, grid.AxisSize(), dist2cube, 
		region_iv0, region_axis_size.Ptr());

	NUM_TYPE num_region_cubes;
	IJK::compute_num_grid_cubes
		(dimension, region_axis_size.PtrConst(), num_region_cubes);

	IJK::ARRAY<VERTEX_INDEX> region_cube_list(num_region_cubes);
	IJK::get_subgrid_cubes
		(dimension, grid.AxisSize(), region_iv0, region_axis_size.PtrConst(),
		region_cube_list.Ptr());

	for (NUM_TYPE i = 0; i < num_region_cubes; i++) {
		VERTEX_INDEX cube_index1 = region_cube_list[i];
		VERTEX_INDEX gcube_index1 = isovert.GCubeIndex(cube_index1);
		if (gcube_map[gcube_index1] == gcube_index0) 
		{ merged_cube_list.push_back(cube_index1); }
	}
}

/// Get edges on boundary of merged cubes.
void MERGESHARP::get_merged_boundary_edges
	(const SHARPISO_GRID & grid,
	const std::vector<VERTEX_INDEX> & merged_cube_list,
	std::vector<EDGE_INDEX> & boundary_edge_list)
{
	typedef SHARPISO_GRID::DIMENSION_TYPE DTYPE;
	typedef std::unordered_map<EDGE_INDEX, NUM_TYPE> HASH_TABLE;

	HASH_TABLE edge_hash;

	for (NUM_TYPE i = 0; i < merged_cube_list.size(); i++) {
		VERTEX_INDEX cube_index = merged_cube_list[i];
		for (DTYPE edge_dir = 0; edge_dir < DIM3; edge_dir++) {
			for (NUM_TYPE k = 0; k < NUM_CUBE_FACET_VERTICES3D; k++) {
				VERTEX_INDEX iv = grid.FacetVertex(cube_index, edge_dir, k);
				EDGE_INDEX iedge = iv*DIM3+edge_dir;

				HASH_TABLE::iterator edge_iter = edge_hash.find(iedge);
				if (edge_iter == edge_hash.end()) {
					edge_hash.insert(HASH_TABLE::value_type(iedge, 1));
				}
				else {
					edge_iter->second++;
				}
			}
		}
	}

	for (HASH_TABLE::const_iterator edge_iter = edge_hash.begin();
		edge_iter != edge_hash.end(); edge_iter++) {
			if (edge_iter->second != NUM_QUAD_VERTICES) 
			{ boundary_edge_list.push_back(edge_iter->first); }
	}

}

/// Select edges from edge list which are in the grid interior.
void select_interior_grid_edges
	(const SHARPISO_GRID & grid,
	const std::vector<EDGE_INDEX> & edge_list,
	std::vector<EDGE_INDEX> & interior_edge_list)
{
	const int dimension = grid.Dimension();

	for (NUM_TYPE i = 0; i < edge_list.size(); i++) {
		EDGE_INDEX edge_index = edge_list[i];
		EDGE_INDEX iend0 = edge_index/dimension;
		int edge_dir = edge_index%dimension;
		if (!IJK::is_edge_on_grid_boundary
			(iend0, edge_dir, dimension, grid.AxisSize())) {
				interior_edge_list.push_back(edge_index);
		}
	}
}

// Extract dual isosurface patch with vertex in merged cube.
void MERGESHARP::extract_dual_quad_isopatch_incident_on
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
	const SCALAR_TYPE isovalue,
	const ISOVERT & isovert,
	const VERTEX_INDEX cube_index0,
	const std::vector<VERTEX_INDEX> & gcube_map,
	const AXIS_SIZE_TYPE dist2cube,
	std::vector<ISO_VERTEX_INDEX> & isoquad_cube,
	std::vector<FACET_VERTEX_INDEX> & facet_vertex)
{
	std::vector<VERTEX_INDEX> merged_cube_list;
	std::vector<EDGE_INDEX> boundary_edge_list;
	std::vector<EDGE_INDEX> edge_list;

	get_merged_cubes(scalar_grid, isovert, cube_index0, gcube_map, dist2cube, 
		merged_cube_list);
	get_merged_boundary_edges(scalar_grid, merged_cube_list, boundary_edge_list);
	select_interior_grid_edges(scalar_grid, boundary_edge_list, edge_list);
	extract_dual_isopoly_from_list(scalar_grid, isovalue, edge_list, 
		isoquad_cube, facet_vertex);
}

// Extract dual isosurface patch with vertex in merged cube.
void MERGESHARP::extract_dual_isopatch_incident_on
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
	const SCALAR_TYPE isovalue,
	const ISOVERT & isovert,
	const VERTEX_INDEX cube_index0,
	const std::vector<VERTEX_INDEX> & gcube_map,
	const AXIS_SIZE_TYPE dist2cube,
	std::vector<ISO_VERTEX_INDEX> & tri_vert,
	std::vector<ISO_VERTEX_INDEX> & quad_vert)
{
	std::vector<ISO_VERTEX_INDEX> isoquad_cube;
	std::vector<FACET_VERTEX_INDEX> facet_vertex;

	tri_vert.clear();
	quad_vert.clear();

	extract_dual_quad_isopatch_incident_on
		(scalar_grid, isovalue, isovert, cube_index0, gcube_map, dist2cube,
		isoquad_cube, facet_vertex);

	map_cube_list(isovert, gcube_map, isoquad_cube);
	IJK::get_non_degenerate_quad_btlr(isoquad_cube, tri_vert, quad_vert);
}

// Extract dual isosurface patch with vertex in merged cube.
// Allow multiple isosurface vertices in each cube.
// Version returning iso_vlist.
void MERGESHARP::extract_dual_isopatch_incident_on_multi
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
	const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
	const SCALAR_TYPE isovalue,
	const ISOVERT & isovert,
	const VERTEX_INDEX cube_index,
	const std::vector<VERTEX_INDEX> & gcube_map,
	const AXIS_SIZE_TYPE dist2cube,
	std::vector<ISO_VERTEX_INDEX> & tri_vert,
	std::vector<ISO_VERTEX_INDEX> & quad_vert,
	std::vector<DUAL_ISOVERT> & iso_vlist)
{
	std::vector<ISO_VERTEX_INDEX> isoquad_cube;
	std::vector<FACET_VERTEX_INDEX> facet_vertex;
	std::vector<VERTEX_INDEX> cube_list;
	std::vector<ISO_VERTEX_INDEX> quad_vert2;
	std::vector<IJKDUALTABLE::TABLE_INDEX> table_index;

	extract_dual_quad_isopatch_incident_on
		(scalar_grid, isovalue, isovert, cube_index, gcube_map, dist2cube,
		isoquad_cube, facet_vertex);

	map_cube_list(isovert, gcube_map, isoquad_cube);

	construct_cube_list(isoquad_cube, cube_list);

	NUM_TYPE num_split;

	set_table_index(isovert, cube_list, table_index);

	IJK::split_dual_isovert
		(isodual_table, cube_list, table_index,
		isoquad_cube, facet_vertex, iso_vlist, quad_vert2, num_split);

	map_isov_indices(isovert, gcube_map, iso_vlist, quad_vert2);

	tri_vert.clear();
	quad_vert.clear();
	IJK::get_non_degenerate_quad_btlr(quad_vert2, tri_vert, quad_vert);
}

// Extract dual isosurface patch with vertex in merged cube.
// Allow multiple isosurface vertices in each cube.
void MERGESHARP::extract_dual_isopatch_incident_on_multi
	(const SHARPISO_SCALAR_GRID_BASE & scalar_grid,
	const IJKDUALTABLE::ISODUAL_CUBE_TABLE & isodual_table,
	const SCALAR_TYPE isovalue,
	const ISOVERT & isovert,
	const VERTEX_INDEX cube_index,
	const std::vector<VERTEX_INDEX> & gcube_map,
	const AXIS_SIZE_TYPE dist2cube,
	std::vector<ISO_VERTEX_INDEX> & tri_vert,
	std::vector<ISO_VERTEX_INDEX> & quad_vert)
{
	std::vector<DUAL_ISOVERT> iso_vlist;

	extract_dual_isopatch_incident_on_multi
		(scalar_grid, isodual_table, isovalue, isovert, cube_index, gcube_map,
		dist2cube, tri_vert, quad_vert, iso_vlist);
}

// Insert polygon edges in edge hash table.
void insert_poly_edges
	(const std::vector<ISO_VERTEX_INDEX> & poly_vert, 
	const NUM_TYPE num_vert_per_poly,
	EDGE_HASH_TABLE & edge_hash)
{
	NUM_TYPE num_poly = poly_vert.size()/num_vert_per_poly;

	for (NUM_TYPE i = 0; i < num_poly; i++) {
		for (int k0 = 0; k0 < num_vert_per_poly; k0++) {
			VERTEX_INDEX iv0 = poly_vert[i*num_vert_per_poly+k0];
			int k1 = (k0+1)%num_vert_per_poly;
			VERTEX_INDEX iv1 = poly_vert[i*num_vert_per_poly+k1];
			if (iv0 > iv1) { std::swap(iv0, iv1); }
			VERTEX_PAIR key = std::make_pair(iv0, iv1);

			EDGE_HASH_TABLE::iterator edge_iter = edge_hash.find(key);
			if (edge_iter == edge_hash.end()) {
				edge_hash.insert(EDGE_HASH_TABLE::value_type(key, 1));
			}
			else {
				edge_iter->second++;
			}
		}
	}
}

// Insert triangle and quadrilateral edges into edge hash table.
void MERGESHARP::insert_tri_quad_edges
	(const std::vector<ISO_VERTEX_INDEX> & tri_vert,
	const std::vector<ISO_VERTEX_INDEX> & quad_vert,
	EDGE_HASH_TABLE & edge_hash)
{
	insert_poly_edges(tri_vert, NUM_VERT_PER_TRI, edge_hash);
	insert_poly_edges(quad_vert, NUM_VERT_PER_QUAD, edge_hash);
}

namespace {

	// Insert or increment vertex count in vertex hash table.
	void insert_vertex(const VERTEX_INDEX iv,
		VERTEX_HASH_TABLE & vertex_hash)
	{
		VERTEX_HASH_TABLE::iterator vertex_iter = vertex_hash.find(iv);
		if (vertex_iter == vertex_hash.end()) {
			vertex_hash.insert(std::pair<VERTEX_INDEX, int>(iv, 1));
		}
		else {
			vertex_iter->second++;
		}
	}

	// Insert vertices from vlist into vertex hash table.
	// @param vertex_hash Maps vertex to unique number 
	//        in range [0-(vertex_hash.size()-1)].
	void insert_vertex_list
		(const std::vector<VERTEX_INDEX> & vlist,
		VERTEX_HASH_TABLE & vertex_hash)
	{
		for (NUM_TYPE i = 0; i < vlist.size(); i++) {
			VERTEX_INDEX iv = vlist[i];
			VERTEX_HASH_TABLE::iterator vertex_iter = vertex_hash.find(iv);
			if (vertex_iter == vertex_hash.end()) {
				NUM_TYPE n = vertex_hash.size();
				vertex_hash.insert(std::pair<VERTEX_INDEX, int >(iv, n));
			}
		}
	}

	// Remap vertices from vlist to values vertex hash table.
	// @pre Every vertex in vlist is in the vertex hash table.
	void remap_vertex_list
		(const VERTEX_HASH_TABLE & vertex_hash,
		const std::vector<VERTEX_INDEX> & vlist,
		std::vector<VERTEX_INDEX> & new_vlist)
	{
		new_vlist.resize(vlist.size());

		for (NUM_TYPE i = 0; i < vlist.size(); i++) {
			VERTEX_INDEX iv = vlist[i];
			VERTEX_HASH_TABLE::const_iterator vertex_iter = vertex_hash.find(iv);
			new_vlist[i] = vertex_iter->second;
		}
	}

	// Insert vertex in cycle.
	// @param vertex_loc Location of vertex in list cycle_vertex.
	void insert_cycle_vertex
		(const VERTEX_INDEX iv0, const VERTEX_INDEX iv1,
		std::vector<CYCLE_VERTEX> & cycle_vertex)
	{
		NUM_TYPE num_adjacent = cycle_vertex[iv0].num_adjacent;
		if (num_adjacent < 2) 
		{ cycle_vertex[iv0].adjacent[num_adjacent] = iv1; }
		cycle_vertex[iv0].num_adjacent++;
	}
}

// Construct list of boundary cycle vertices.
void MERGESHARP::construct_boundary_cycle
	(const EDGE_HASH_TABLE & edge_hash,
	std::vector<CYCLE_VERTEX> & cycle_vertex)
{
	for (EDGE_HASH_TABLE::const_iterator edge_iter = edge_hash.begin();
		edge_iter != edge_hash.end(); edge_iter++) {
			if (edge_iter->second == 1) {
				VERTEX_INDEX iv0 = (edge_iter->first).first;
				VERTEX_INDEX iv1 = (edge_iter->first).second;
				insert_cycle_vertex(iv0, iv1, cycle_vertex);
				insert_cycle_vertex(iv1, iv0, cycle_vertex);
			}
	}
}

// Renumber tri and quad vertices so that they are from 0 to num_vert-1.
template <typename VTYPE>
void renumber_tri_quad_vertices
	(const std::vector<VTYPE> & tri_vert,
	const std::vector<VTYPE> & quad_vert,
	std::vector<VTYPE> & new_tri_vert,
	std::vector<VTYPE> & new_quad_vert,
	NUM_TYPE & num_vert)
{
	VERTEX_HASH_TABLE vertex_hash;

	insert_vertex_list(tri_vert, vertex_hash);
	insert_vertex_list(quad_vert, vertex_hash);

	num_vert = vertex_hash.size();
	remap_vertex_list(vertex_hash, tri_vert, new_tri_vert);
	remap_vertex_list(vertex_hash, quad_vert, new_quad_vert);
}

// Search cycle starting at iv0.
// @pre All vertices in cycle containing iv0 have is_visited set to false.
// @pre All vertices have degree two.
void search_cycle
	(const VERTEX_INDEX iv0, std::vector<CYCLE_VERTEX> & cycle_vertex)
{
	VERTEX_INDEX iv = iv0;
	VERTEX_INDEX ivprev = cycle_vertex[iv0].adjacent[0];
	while (!cycle_vertex[iv].is_visited) {
		cycle_vertex[iv].is_visited = true;
		if (cycle_vertex[iv].adjacent[0] == ivprev) {
			ivprev = iv;
			iv = cycle_vertex[iv].adjacent[1];
		}
		else {
			ivprev = iv;
			iv = cycle_vertex[iv].adjacent[0];
		}
	}
}
