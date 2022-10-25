#include <vector>

using namespace std;

//#include "darkgdk.h"
#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"

void PROC_MAP::extend_wall_vbo()
{
	// gonna use some ansi C again

	int		type;
	int		loop;

	// count the outer edges in the main list
	int		edge_count	= 0;

	_EDGE*	pEdge_curr	= &edges[0];
	_EDGE*	pEdge_end	= &edges[edges_roof-1];

	while ( pEdge_curr <= pEdge_end )
	{
		type = pEdge_curr->type;

		if ( type == PM_COUTER || type == PM_ROUTER )
			edge_count++;

		pEdge_curr++;
	}

	// Copy the edges to the new temporary list
	_EDGE* tEdges	= new _EDGE[edge_count];

	pEdge_curr		= &edges[0];

	int		curr	= 0;

	while ( pEdge_curr <= pEdge_end )
	{
		type = pEdge_curr->type;

		if ( type == PM_COUTER || type == PM_ROUTER )
		{
			memcpy( &tEdges[curr], pEdge_curr, sizeof(_EDGE) );
			curr++;
		}

		pEdge_curr++;
	}

	// do a bubble sort on the array (no faffing about this time)
	pEdge_curr		= tEdges;
	_EDGE*	tedge;
	_EDGE* tEdges_end = &tEdges[edge_count-1];

	_EDGE	TEMP;

	while ( pEdge_curr < tEdges_end )
	{
		for ( tedge = pEdge_curr+1; tedge < tEdges_end; tedge++ )
		{
			if ( pEdge_curr->v2 == tedge->v1 )
			{
				memcpy( &TEMP, pEdge_curr, sizeof( _EDGE ) );
				memcpy( pEdge_curr, tedge, sizeof( _EDGE ) );
				memcpy( tedge, &TEMP, sizeof( _EDGE ) );
			}
		}
		pEdge_curr++;
	}

	// Now use this array to create the wall mesh
	pEdge_curr	= &tEdges[0];
	int vcurr	= verts.size();		// could use verts_wall
	float uv	= 0.0f;
	int vstep	= tess+2;

	int v1, v2, v3;
	float x1,y1,x2,y2,dx,dy,dist;

	// store first verts for walls edge
	//push_wall_verts( pEdge_curr->v1, uv );

	while ( pEdge_curr <= tEdges_end )
	{
		// each wall section will get it's own verts
		push_wall_verts( pEdge_curr->v1, uv );

		// update the uv tex coord by using the distance between verts
		v1 = pEdge_curr->v1;	
		v2 = pEdge_curr->v2;

		x1 = verts[v1].x;
		y1 = verts[v1].z;
		x2 = verts[v2].x;
		y2 = verts[v2].z;

		dx = x2-x1; dx = dx * dx;
		dy = y2-y1; dy = dy * dy;
		dist = sqrt( dx + dy );

		uv = uv + ( dist / (float)size_max );

		// store the new verts in the main list
		push_wall_verts( pEdge_curr->v2, uv );

		int tcount = 0;
		int type = pEdge_curr->type;
		if ( type == PM_COUTER )
			type = PM_CFACE;
		else
			type = PM_RFACE;

		while ( tcount < vstep  - 1 )
		{

			// add the faces using the new verts
			v1 = tcount + vcurr;
			v2 = tcount + vcurr + 1;
			v3 = tcount + vcurr + 1 + vstep;

			faces.push_back( _FACE(v1, v2, v3, type, pEdge_curr->segment) );

			v2 = v3;
			v3 = tcount + vcurr + vstep;

			faces.push_back( _FACE(v1, v2, v3, type, pEdge_curr->segment) );

			tcount++;
			//vcurr++;
		}

		vcurr += ( vstep << 1 );

		pEdge_curr++;
	}
}
