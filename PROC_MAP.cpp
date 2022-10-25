#include <vector>

using namespace std;

//#include "darkgdk.h"
#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"
#include "time.h"

// Constructor - sets the default values
PROC_MAP::PROC_MAP(void)
{
	map_size	= PM_MAPSIZE;
	size_min	= PM_SIZEMIN;
	size_max	= PM_SIZEMAX;
	ang_min		= PM_ANGMIN;
	ang_max		= PM_ANGMAX;
	cseg_min	= PM_CSEGMIN;
	cseg_max	= PM_CSEGMAX;
	pn.Set( 0.80f, 0.01f, 1.20f, 8, PM_RNDSEED );
	noise_hgt	= PM_NOISEHGT;
	ObjID		= 0;
	room_min	= PM_ROOMMIN;
	room_max	= PM_ROOMMAX;
	rseg_min	= PM_RSEGMIN;
	rseg_max	= PM_RSEGMAX;

	map_seed	= PM_RNDSEED;
}

float PROC_MAP::get_noise(float x, float y)
{
	float res;

	res = pn.GetHeight( x / (float)PM_NOISE_SCALE, y / (float)PM_NOISE_SCALE ) * (float)noise_hgt;

	return res;
}

PROC_MAP::~PROC_MAP(void)
{
	verts.clear();
	edges.clear();
	faces.clear();

	clear_textures();

	clear_display_lists();
}

void PROC_MAP::start_poly()
{
	float len = size_max;	// made local to function (not really needed)

	// first 4 verts
	verts.push_back( _VERT( 0,   0,	0 ) );
	verts.push_back( _VERT( 0,   0,	len ) );
	verts.push_back( _VERT( len, 0, len ) );
	verts.push_back( _VERT( len, 0,	0 ) );

	// first corridor inner edge
	edges.push_back( _EDGE( 0, 2, PM_CINNER, curr_segment ) );

	// corridor outer edges
	edges.push_back( _EDGE( 0, 3, PM_COUTER, curr_segment ) );
	edges.push_back( _EDGE( 3, 2, PM_COUTER, curr_segment ) );
	edges.push_back( _EDGE( 2, 1, PM_COUTER, curr_segment ) );
	edges.push_back( _EDGE( 1, 0, PM_COUTER, curr_segment ) );

	// initial corridor faces (clockwise orientation)
	faces.push_back( _FACE( 0, 3, 2, PM_CFACE, curr_segment ) );
	faces.push_back( _FACE( 2, 1, 0, PM_CFACE, curr_segment ) );

	// No rooms defined yet so leave empty
}

void PROC_MAP::setup(int size, int seed, int lmin, int lmax, int amin, int amax)
{
	map_size = size;
	map_seed = seed;
	size_min = lmin;
	size_max = lmax;
	ang_min = amin;
	ang_max = amax;
}

void PROC_MAP::generate(int zoom)
{
	//
	// Added an STL optimisation here...
	//
	verts.reserve(10);
	edges.reserve(10);
	faces.reserve(10);

	curr_segment = 0;			// reset the current segment count to seperate limbs

	this->start_poly();			// create the start off poly

	int count = map_size;		// add this many map pieces

	_EDGE* edge;				// quicker access to the data than STL

	srand( map_seed );			// initialise random number generator

	while ( count > 0 )
	{
		if ( count == map_size )	// first segment is always a corridor
		{
			if ( add_corridor() )	// if successful then change counters
			{
				count--;		// decrease segment counter
				curr_segment++;	// next segment (for _FACE type)
			}
		}
		// add a room or a corridor?
		else if ( rand() & 1 )
		{
			// add corridor segments
			if ( add_corridor() )
			{
				count--;
				curr_segment++;
			}
		}
		else
		{
			// add room seg
			if ( add_room() )	// returns true if successfully added a room to the map
			{
				count--;
				curr_segment++;
			}
		}

		glutInitScreen();
		glutstart2d();
		glutPrint(4,12,"Generating...", 4, 1.0f, 1.0f, 1.0f);
		glutend2d();
		glutSwapBuffers();
	}
}

bool PROC_MAP::edge_intersect(float x1, float z1, float x2, float z2)
{
	int count = (int)edges.size();
	int pos = 0;

	_EDGE* e = &edges[0];	// faster 'not' using STL (loop could be huge)
	_VERT* v = &verts[0];

	bool flag = 0;

	float x3,z3,x4,z4;

	int v1, v2;

	while ( flag == 0 && pos < count )
	{
		v1 = e->v1;
		v2 = e->v2;

		x3 = v[v1].x;
		z3 = v[v1].z;
		x4 = v[v2].x;
		z4 = v[v2].z;

		flag = line_intersect(x1,z1,x2,z2,x3,z3,x4,z4);

		e++;		// next item in the edge list
		pos++;
	}

	return flag;
}

bool PROC_MAP::line_intersect(float x1, float y1, float x2, float y2,
						float x3, float y3, float x4, float y4) {
	float	d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	if ( d == 0 ) return 0;
	// Get the x and y
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
	float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
	// Check if the x and y coordinates are within both lines
	if ( x < min(x1, x2) || x > max(x1, x2) ||					// REMOVE STL min() and max()
		 x < min(x3, x4) || x > max(x3, x4) ) return 0;
	if ( y < min(y1, y2) || y > max(y1, y2) ||
		 y < min(y3, y4) || y > max(y3, y4) ) return 0;
	return 1;
}

// a handy function for return the centre of a circle by supplying points on its circumference
void PROC_MAP::vvv_circle(	float px, float py,
							float qx, float qy,
							float rx, float ry,
							float *x, float *y) //, float &r )
{
	float p1  = ( qx + px ) / 2.0f;
	float y11 = ( qy + py ) / 2.0f;
	float yd1 = qx - px;
	float pd1 = -( qy - py );

	float p2  = ( rx + qx ) / 2.0f;
	float y2  = ( ry + qy ) / 2.0f;
	float yd2 = rx - qx;
	float pd2 = -( ry - qy );

	float ox = ( y11 * pd1 * pd2 + p2 * pd1 * yd2 - p1 * yd1 * pd2 - y2 * pd1 * pd2 ) / ( pd1 * yd2 - yd1 * pd2 );
	float oy = ( ox - p1 ) * yd1 / pd1 + y11;

	float dx = ox - px;
	float yd = oy - py;

	//r = sqrt( dx * dx + yd * yd );
	*x = ox;
	*y = oy;
}

// returns the centre coords of an edge
void PROC_MAP::get_edge_centre(int edge, float *x, float *y, float *z)
{
	int v1 = edges[edge].v1;
	int v2 = edges[edge].v2;

	*x = ( verts[v1].x + verts[v2].x ) / 2.0f;
	*y = ( verts[v1].y + verts[v2].y ) / 2.0f;
	*z = ( verts[v1].z + verts[v2].z ) / 2.0f;
}

//
// I've decided to seperate the two functions that add corridors and rooms to the map
//

// PROC_MAP::add_room()
//
// So far seems to be working fine
//

bool PROC_MAP::add_room()
{
	// get the number of sides that make up the new room (within the range)
	int		sides = ( rand() % ( rseg_max - rseg_min ) ) + rseg_min;

	// use same calc to find the edge size
	int		edge_size = ( rand() % ( room_max - room_min ) ) + room_min;

	// setup an array to store the temp room verts
	_VERT*	v = new _VERT[sides+1];	// extra vert for centre of the room
	_EDGE*	curr_edge;

	// find an outer edge to attach the room to
	do {
		curr_edge = &edges[ ( rand() % (int)edges.size() ) ];
	} while ( curr_edge->type != PM_COUTER && curr_edge->type != PM_ROUTER );

	// get vert ID's for the original edge
	int		v1 = curr_edge->v1, v2 = curr_edge->v2;

	// get coords of original edge (using reverse direction)
	v[0].x = verts[v2].x;
	v[0].y = verts[v2].z;
	v[1].x = verts[v1].x;
	v[1].y = verts[v1].z;

	// define the starting angle and the step angle
	float	curr_angle = atan2( v[1].y - v[0].y, v[1].x - v[0].x );
	float	diff_angle = Deg2Rad( 360.0f / sides );

	// store edges angle in z
	v[1].z = curr_angle;

	int		loop;

	// store new verts for room
	for ( loop = 2; loop < sides; loop++ )
	{
		curr_angle = curr_angle + diff_angle;

		v[loop].x = v[loop-1].x + cos( curr_angle ) * edge_size;
		v[loop].y = v[loop-1].y + sin( curr_angle ) * edge_size;

		v[loop].z = curr_angle;
	}

	// now calc the centre of the room for the last vert
	vvv_circle( v[0].x, v[0].y, 
				v[1].x, v[1].y, 
				v[2].x, v[2].y, 
				&v[sides].x, &v[sides].y );

	float	x, y, inner_angle, z;

	// check new edges (not the last one though yet)
	for ( loop = 1; loop < sides - 1; loop++ )
	{
		// nudge the beginning coords a tad
		x = v[loop].x + cos( v[loop+1].z ) * 0.5f;
		y = v[loop].y + sin( v[loop+1].z ) * 0.5f;

		// if edge collides with map then exit
		if ( edge_intersect( x, y, v[loop+1].x, v[loop+1].y ) )
		{
			delete[] v;
			return 0;
		}
	}

	// now check all inner edges (using the centre coord of the room)
	for ( loop = 0; loop < sides; loop++ )
	{
		// get angle of current inner edge
		inner_angle = atan2( v[sides].y - v[loop].y, v[sides].x - v[loop].x );

		// nudge the start coords a little
		x = v[loop].x + cos( inner_angle ) * 0.5f;
		y = v[loop].y + sin( inner_angle ) * 0.5f;

		// if the edge collides with the map then exit
		if ( edge_intersect( x, y, v[sides].x, v[sides].y ) )
		{
			delete[] v;
			return 0;
		}
	}

	// now check the last outer edge of the room
	inner_angle = atan2( v[sides-1].y - v[0].y, v[sides-1].x - v[0].x );
	x = v[0].x + cos( inner_angle ) * 0.5f;
	y = v[0].y + sin( inner_angle ) * 0.5f;

	// if collides with map then just exit
	if ( edge_intersect( x, y, v[sides-1].x, v[sides-1].y ) )
	{
		delete[] v;
		return 0;
	}

	// Now it is safe to add the room to the main map
	int		vsize = (int)verts.size();
	int		esize = (int)edges.size();
	int		fsize = (int)faces.size();

	// change current edge to a room entry point (doorway to room)
	curr_edge->type = PM_RENTRY;

	// add centre of rooms coordinate
	x = v[sides].x;
	y = v[sides].y;
	verts.push_back( _VERT( x, 0, y ) ); 

	//
	// now add the original edge for the limb addition
	//
	verts.push_back( _VERT( verts[v1].x, verts[v1].y, verts[v1].z ) );
	verts.push_back( _VERT( verts[v2].x, verts[v2].y, verts[v2].z ) );

	// add this new edge
	edges.push_back( _EDGE( vsize + 1, vsize + 2, PM_RENTRY, curr_segment ) );

	// add first face ( current edge with centre of room )
	faces.push_back( _FACE( vsize + 2, vsize + 1, vsize, PM_RFACE, curr_segment ) );

	// now add the new edges to the list
	edges.push_back( _EDGE( vsize + 1, vsize, PM_RINNER, curr_segment ) );
	edges.push_back( _EDGE( vsize + 2, vsize, PM_RINNER, curr_segment ) );

	// vsize refers to centre room vert as well

	// this is used to record the last vert used in the next loop
	int		last_vert = vsize + 1;
	//int		v2 = curr_edge->v2;
	
	// now add the rest of the verts, edges and faces
	for ( loop = 2; loop < sides; loop++ )
	{
		x = v[loop].x;
		z = v[loop].y;
		y = 0;

		int pos = vsize + loop - 1 + 2;

		// add the new vert
		verts.push_back( _VERT( x, y, z ) );

		// add the face
		faces.push_back( _FACE( vsize, last_vert, pos, PM_RFACE, curr_segment ) );

		// add the edges
		edges.push_back( _EDGE( last_vert, vsize, PM_RINNER, curr_segment ) );
		edges.push_back( _EDGE( last_vert, pos, PM_ROUTER, curr_segment ) );

		// update last_vert
		last_vert = pos;
	}

	// fix the last face now
	last_vert = (int)verts.size() - 1;

	// The bug here was the verts array had been moved in memory so the curr_edge pointer
	// was now invalid at this point...

	edges.push_back( _EDGE( last_vert, vsize, PM_RINNER, curr_segment ) );
	edges.push_back( _EDGE( last_vert, vsize + 2, PM_ROUTER, curr_segment ) );

	faces.push_back( _FACE( last_vert, vsize + 2, vsize, PM_RFACE, curr_segment ) );

	delete[] v;

	return 1;	// 1 means success / 0 means failed
}

//
// Just going to use the STL for this function for now.
//

bool PROC_MAP::add_corridor()
{
	// in case I need to delete them, store the original sizes of the arrays
	int		orig_vsize = (int)verts.size();
	int		orig_esize = (int)edges.size();
	int		orig_fsize = (int)faces.size();

	// how many segments am I going to add?
	int segs = rand() % ( cseg_max - cseg_min ) + cseg_min;

	// now find an edge to try out
	int		edge;
	do {
		edge = rand() % (orig_esize);
	} while ( edges[edge].type != PM_COUTER && edges[edge].type != PM_ROUTER );

	// keep the original edge number and type for the future
	int		orig_edge = edge;
	int		orig_edge_type = edges[edge].type;

	int		etype;

	// alter the original edges type accordingly
	if ( orig_edge_type != PM_ROUTER )
		etype = PM_CINNER;
	else
		etype = PM_RENTRY;

	edges[edge].type = etype;

	// now add this edge to the new segment so it can be made as a seperate limb
	int		v1 = edges[edge].v1;
	int		v2 = edges[edge].v2;

	// Don't reverse the new edge as it's only an inner edge and needs to keep its
	// orientation for the rest of the corridor algorithm
	verts.push_back( _VERT( verts[v1].x, verts[v1].y, verts[v1].z ) );
	verts.push_back( _VERT( verts[v2].x, verts[v2].y, verts[v2].z ) );

	// set the new edges type based on corridor or room (rooms will have doorways)
	edges.push_back( _EDGE( orig_vsize, orig_vsize + 1, etype, curr_segment ) );

	// now set the variable 'edge' to point to the new edge
	edge = orig_esize;

	// now keep trying to add the segments
	int		seg_count = 0;

	do {
		edge = add_corridor_seg( edge );
		seg_count++;
	} while ( edge != 0 && seg_count < segs );

	// have we added enough segs to let them stay on the map?
	if ( seg_count < cseg_min )
	{
		// okay, remove the data from the main map data
		while ( (int)verts.size() > orig_vsize )
			verts.erase( verts.begin() + orig_vsize );
		
		while ( (int)edges.size() > orig_esize )
			edges.erase( edges.begin() + orig_esize );

		while ( (int)faces.size() > orig_fsize )
			faces.erase( faces.begin() + orig_fsize );

		// restore the original edges type
		edges[orig_edge].type = orig_edge_type;

		// now exit returning NULL to calling function
		return 0;
	}

	// it actually looks I'm done and can exit
	return 1;
}

// if successfull then returns pointer to the new edge to carry on adding onto
int PROC_MAP::add_corridor_seg( int edge )
{
	float d2r90 = Deg2Rad(90);

	// get the original verts of the current edge
	int v1 = edges[edge].v1;
	int v2 = edges[edge].v2;

	// these will be used when adding the new edges
	int v3, v4;

	float x1 = verts[v1].x;		// main edge coords start
	float z1 = verts[v1].z;
	float x2 = verts[v2].x;		// end point
	float z2 = verts[v2].z;

	// get the current edges angle
	float edge_angle = atan2( z2-z1, x2-x1 );

	float angle_adjust = Deg2Rad( (float)( ( rand() % ( ang_max - ang_min ) ) + ang_min ) );

	// get the middle position of the current edge
	float xmid = ( x1 + x2 ) / 2.0f;
	float zmid = ( z1 + z2 ) / 2.0f;

	float new_length = ( rand() % ( size_max -size_min ) ) + size_min;

	float new_angle = edge_angle - angle_adjust;

	// set the middle coords of the new parallel edge
	float newx = xmid + cos( new_angle ) * new_length;
	float newz = zmid + sin( new_angle ) * new_length;

	float half_new_length = new_length / 2.0f;

	// start vert pos of new parallel edge
	float x3 = newx + cos( new_angle - d2r90 ) * ( half_new_length );
	float z3 = newz + sin( new_angle - d2r90 ) * ( half_new_length );

	// end vert pos of the new parallel edge
	float x4 = newx + cos( new_angle + d2r90 ) * ( half_new_length );
	float z4 = newz + sin( new_angle + d2r90 ) * ( half_new_length );

	// nudge original verts a little ( for intersect check )
	// otherwise they will intersect with 3 edges as verts are the same
	x1 = x1 + cos( new_angle ) * 0.5f;
	z1 = z1 + sin( new_angle ) * 0.5f;
	x2 = x2 + cos( new_angle ) * 0.5f;
	z2 = z2 + sin( new_angle ) * 0.5f;

	// if no intersection with current edges then add new poly
	if ( edge_intersect( x1, z1, x3, z3 ) == 0 )
	{
		if ( edge_intersect( x3, z3, x4, z4 ) == 0 )
		{
			if ( edge_intersect( x4, z4, x2, z2 ) == 0 )
			{
				// change old edge type if needed
				if ( edges[edge].type != PM_RENTRY )
					edges[edge].type = PM_CINNER;

				// get verts end position
				int vcount = (int)verts.size();

				// add the new verts to the array
				verts.push_back( _VERT( x3, 0, z3 ) );
				verts.push_back( _VERT( x4, 0, z4 ) );

				/*int v1 = edges[edge].v1;
				int v2 = edges[edge].v2;*/

				// new vert indices
				v3 = vcount;
				v4 = vcount + 1;

				edges.push_back( _EDGE( v1, v3, PM_COUTER, curr_segment ) );
				edges.push_back( _EDGE( v4, v2, PM_COUTER, curr_segment ) );
				edges.push_back( _EDGE( v4, v1, PM_CINNER, curr_segment ) );
				edges.push_back( _EDGE( v3, v4, PM_COUTER, curr_segment ) );

				faces.push_back( _FACE( v1, v3, v4, PM_CFACE, curr_segment ) );
				faces.push_back( _FACE( v4, v2, v1, PM_CFACE, curr_segment ) );

				edge = (int)edges.size() - 1;

				return edge;
			}
		}
	}

	return 0;
}

//
// setup functions
//

void PROC_MAP::set_map(int size, int seed, int height)
{
	this->map_size	= size;
	this->map_seed	= seed;
	this->noise_hgt	= height;
}

void PROC_MAP::set_corridor(int len_min, int len_max, int ang_min, int ang_max, int seg_min, int seg_max)
{
	this->size_min	= len_min;
	this->size_max	= len_max;
	this->ang_min	= ang_min;
	this->ang_max	= ang_max;
	this->cseg_min	= seg_min;
	this->cseg_max	= seg_max;
}

void PROC_MAP::set_room(int len_min, int len_max, int seg_min, int seg_max)
{
	this->room_min	= len_min;
	this->room_max	= len_max;
	this->rseg_min	= seg_min;
	this->rseg_max	= seg_max;
}
