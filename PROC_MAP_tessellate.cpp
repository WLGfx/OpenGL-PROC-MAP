#include <vector>

using namespace std;

#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"

void PROC_MAP::tessellate(int levels)
{
	while ( levels ) {
		// go through every face and tessellate it
		// it will also keep a temp record of the edges split and the new vert for the mid point

		int size = edges.size();
		int esize = size;		// original size of the temp edges list

		_temp_edge_list *edge_list = new _temp_edge_list[ size ];

		int v1, v2;
		float temp_vert_x, temp_vert_y, temp_vert_z;

		int vpos = verts.size(); // this is the new vert position

		while ( size ) {
			size--;

			v1 = edges[ size ].v1;
			v2 = edges[ size ].v2;

			// get the midpoint
			temp_vert_x = ( verts[ v1 ].x + verts[ v2 ].x ) / 2.0f;
			temp_vert_z = ( verts[ v1 ].z + verts[ v2 ].z ) / 2.0f;

			// get new noise height for the y coord
			temp_vert_y = 0;

			// store the new vert now
			verts.push_back( _VERT(temp_vert_x, temp_vert_y, temp_vert_z) );

			// change the current edges second vert
			edges[ size ].v2 = vpos;

			// add new edge
			edges.push_back( _EDGE(vpos, v2, edges[ size ].type, edges[ size ].segment) );
			//edges.push_back( _EDGE(vpos, v2, 0, edges[ size ].segment) );

			// add the details of the original edge to the struct
			edge_list[ size ].v1 = v1;
			edge_list[ size ].v2 = v2;
			edge_list[ size ].mid_vert = vpos;

			// increment the vpos for the next vert position mid point
			vpos++;
		}

		// now re-create the faces list
		size = faces.size();

		int v3, edge1, edge2, edge3;
		int mid1, mid2, mid3;
		int type, segment, etype;

		while ( size ) {
			size--;

			// store the faces original vertex indices for adding the new faces later
			v1 = faces[ size ].v1;
			v2 = faces[ size ].v2;
			v3 = faces[ size ].v3;

			// and the type and segment info
			type = faces[ size ].type;
			segment = faces[ size ].segment;

			// problem here:
			// I've stored the vertex index for the face instead of the edge index
			// so I'll have to find the original edge that matches both vertex indices
			// Luckily enough I've stored the info in the temp_edges list
			edge1 = find_temp_edge(v1, v2, edge_list, esize);
			edge2 = find_temp_edge(v2, v3, edge_list, esize);
			edge3 = find_temp_edge(v3, v1, edge_list, esize);

			mid1 = edge_list[ edge1 ].mid_vert;
			mid2 = edge_list[ edge2 ].mid_vert;
			mid3 = edge_list[ edge3 ].mid_vert;

			// alter original face first to new midpoints
			faces[ size ].v2 = mid1;
			faces[ size ].v3 = mid3;

			// now add the new faces
			faces.push_back( _FACE(mid1, v2, mid2, type, segment) );
			faces.push_back( _FACE(mid3, mid2, v3, type, segment) );
			faces.push_back( _FACE(mid1, mid2, mid3, type, segment) ); // centre face

			// set new new edges type for inner edges
			if ( type == PM_CFACE ) etype = PM_CINNER;
			else etype = PM_RINNER;

			// now add the centre faces edges to the edge_list
			edges.push_back( _EDGE(mid1, mid2, etype, segment) );
			edges.push_back( _EDGE(mid2, mid3, etype, segment) );
			edges.push_back( _EDGE(mid3, mid1, etype, segment) );
		}

		// remove the temp edge list
		delete[] edge_list;

		levels--;
	} // eof while ( levels )
}

int PROC_MAP::find_temp_edge(int v1, int v2, _temp_edge_list *list, int size)
{
	int pos = 0;
	while ( pos < size ) {
		
		if ( v1 == list->v1 && v2 == list->v2 ) return pos;
		if ( v1 == list->v2 && v2 == list->v1 ) return pos;	// double checking

		pos++;
		list++;
	}
	return 0;	// It really should not get to this point
}
