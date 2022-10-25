#include <vector>

using namespace std;

#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"

// This function will take the generated floor map and create the edge lists for both the
// wall and the roof objects too and add them to the main table ready for the next step.

// What I might do is to copy the original floor edge map for the main game...

void PROC_MAP::vbo_create()
{
	int		loop;
	int		v1, v2, v3;
	int		type;
	int		segment;

	char*	vedges;

	verts_count_floor = verts.size();
	edges_count_floor = edges.size();
	faces_count_floor = faces.size();

	// double the size of all the lists ready for duplicating
	verts.resize( verts_count_floor * 2 );
	edges.resize( edges_count_floor * 2 );
	faces.resize( faces_count_floor * 2 );

	// duplicate verts and add size max to the height Y
	_VERT*	vptr_src = &verts[0];
	_VERT*	vptr_dst = vptr_src + verts_count_floor; // halfway point

	memcpy( vptr_dst, vptr_src, sizeof(_VERT) * verts_count_floor );

	for ( loop = 0; loop < verts_count_floor; loop++ )
	{
		vptr_dst[ loop ].y = size_max;
	}

	// duplicate the edges and add the new vert references
	_EDGE*	eptr_src = &edges[0];
	_EDGE*	eptr_dst = eptr_src + edges_count_floor;

	memcpy( eptr_dst, eptr_src, sizeof(_EDGE) * edges_count_floor );

	for ( loop = 0; loop < edges_count_floor; loop++ )
	{
		eptr_dst[ loop ].v1 += verts_count_floor;
		eptr_dst[ loop ].v2 += verts_count_floor;
		// now alter the type (prob don't actually need this really)
		type = eptr_dst[ loop ].type;
		if ( type == PM_COUTER ) type = PM_COUTERR;
		else if ( type == PM_ROUTER ) type = PM_ROUTERR;
		eptr_dst[ loop ].type = type;
	}

	// duplicate the faces and add the new verts references
	_FACE*	fptr_src = &faces[0];
	_FACE*	fptr_dst = fptr_src + faces_count_floor;

	memcpy( fptr_dst, fptr_src, sizeof(_FACE) * faces_count_floor );

	for ( loop = 0; loop < faces_count_floor; loop++ )
	{
		v1 = fptr_dst[ loop ].v1 + faces_count_floor;
		v2 = fptr_dst[ loop ].v2 + faces_count_floor;
		v3 = fptr_dst[ loop ].v3 + faces_count_floor;
		// now reverse the face orientation
		fptr_dst[ loop ].v1 = v1;
		fptr_dst[ loop ].v2 = v3;
		fptr_dst[ loop ].v3 = v2;
		// change the faces type accordingly
		type = fptr_dst[ loop ].type;
		if ( type == PM_CFACE ) type = PM_CFACER;
		else type = PM_RFACER;
		fptr_dst[ loop ].type = type;
	}

	// now create the wall faces by joining the top and bottom verts on the outer edges
	// NEED STL FOR THIS BIT!!! - ALL OF IT!!! Argh!!!

	vedges = new char[ verts_count_floor ];		// just a small amount of memory needed
	memset( vedges, 0, verts_count_floor );

	for ( loop = 0; loop < edges_count_floor; loop++ )
	{
		// if an outer edge then create the face
		type = edges[ loop ].type;
		if ( type == PM_COUTER || type == PM_ROUTER )
		{
			segment = edges[ loop ].segment;

			if ( type == PM_COUTER ) type = PM_CFACEW;	// set to a wall face type
			else type = PM_RFACEW;

			v1 = edges[ loop ].v1;
			v2 = edges[ loop + edges_count_floor ].v2;
			v3 = edges[ loop ].v2;
			faces.push_back( _FACE( v1, v2, v3, type, segment ) );

			v3 = v2;
			v2 = edges[ loop + edges_count_floor ].v1;
			faces.push_back( _FACE( v1, v2, v3, type, segment ) );

			// now add edges for the walls
			if ( type == PM_CFACEW ) type = PM_COUTERW;	// set to a wall edge type
			else type = PM_ROUTERW;

			edges.push_back( _EDGE( v1, v3, type, segment ) );		// middle edge
			v2 = edges[ loop ].v2;

			// add left edge if it hasn't already been done
			if ( !vedges[ v1 ] )
			{
				edges.push_back( _EDGE( v1, edges[ loop + edges_count_floor ].v1, type, segment ) );
				vedges[ v1 ] = 1;
			}

			// add right edge if it hasn't already been done
			if ( !vedges[ v2 ] )
			{
				edges.push_back ( _EDGE( v2, edges[ loop + edges_count_floor ].v2, type, segment ) );
				vedges[ v2 ] = 1;
			}
		}
	} // EOF for loop

	delete[] vedges;

	loop = tess;

	while ( loop )	// well, it is a while loop
	{
		_TESSELLATE();
		loop--;
	}

	_NORMALISE();
	_DISPLAYLISTS();
	create_textures(6);
}

void PROC_MAP::_TESSELLATE()
{
	//
	// TESSELLATION section now!!!
	//

	int		verts_count = verts.size();			// current sizes before it all happens
	int		edges_count = edges.size();
	int		faces_count = faces.size();

	// now extend the arrays sizes to hold the new tessellated data
	verts.resize( verts_count + edges_count );	// one extra per edge
	edges.resize( edges_count * 2 );			// double amount of edges
	faces.resize( faces_count * 4 );			// middle + 3 outer faces

	int*	vmid = new int[ edges_count * 3 ];	// temp references to mid verts references

	int		loop;
	float	x, y, z;
	int		v1, v2, v3;
	int		vpos, epos, fpos;
	int		m1, m2, m3;
	int		type, segment;

	_VERT*	vptr = &verts[0];
	_EDGE*	eptr = &edges[0];
	_FACE*	fptr = &faces[0];

	// now store the edges middle point verts and add the new edges
	for ( loop = 0; loop < edges_count; loop++ )
	{
		// positions of the new vert and edge data to be stored
		vpos = loop + verts_count;
		epos = loop + edges_count;

		// original edges vert refs
		v1 = eptr[loop].v1;
		v2 = eptr[loop].v2;

		// store the temp reference to the original edge data
		vmid[loop]						= v1;
		vmid[loop + edges_count]		= v2;
		vmid[loop + (edges_count * 2)]  = vpos;

		// store the new vert in the extended data
		vptr[vpos].x = ( vptr[v1].x + vptr[v2].x ) / 2.0f;
		vptr[vpos].y = ( vptr[v1].y + vptr[v2].y ) / 2.0f;
		vptr[vpos].z = ( vptr[v1].z + vptr[v2].z ) / 2.0f;

		// resize the original edge
		eptr[loop].v2		= vpos;

		// set the new edges data
		eptr[epos].v1		= vpos;
		eptr[epos].v2		= v2;
		eptr[epos].type		= eptr[loop].type;
		eptr[epos].segment	= eptr[loop].segment;
	}

	// and go through every face and split it up
	for ( loop = 0; loop < faces_count; loop++ )
	{
		// grab the original vert refs
		v1		= fptr[loop].v1;
		v2		= fptr[loop].v2;
		v3		= fptr[loop].v3;

		type	= fptr[loop].type;
		segment = fptr[loop].segment;

		// locate the mid vert refs
		m1		= _FINDMID(v1,v2,vmid,edges_count);
		m2		= _FINDMID(v2,v3,vmid,edges_count);
		m3		= _FINDMID(v3,v1,vmid,edges_count);

		// resize original face
		fptr[loop].v2 = m1;
		fptr[loop].v3 = m3;

		// store second face ( top )
		fpos = faces_count + ( loop * 3 );

		fptr[fpos].v1		= m1;
		fptr[fpos].v2		= v2;
		fptr[fpos].v3		= m2;
		fptr[fpos].type		= type;
		fptr[fpos].segment	= segment;

		// store third face ( right )
		fpos++;

		fptr[fpos].v1		= m3;
		fptr[fpos].v2		= m2;
		fptr[fpos].v3		= v3;
		fptr[fpos].type		= type;
		fptr[fpos].segment	= segment;

		// store middle face
		fpos++;

		fptr[fpos].v1		= m1;
		fptr[fpos].v2		= m2;
		fptr[fpos].v3		= m3;
		fptr[fpos].type		= type;
		fptr[fpos].segment	= segment;
	}

	delete[] vmid;
}

int PROC_MAP::_FINDMID(int v1, int v2, int *vptr, int cnt)
{
	int		flag = 1;
	int		pos = 0;
	int		t1,t2;
	do {
		t1 = vptr[pos];
		t2 = vptr[pos + cnt];
		if		( v1==t1 && v2==t2 ) flag = 0;
		else if ( v1==t2 && v2==t1 ) flag = 0;
		else	pos++;
	} while ( pos < cnt  && flag );	// pos should never go over cnt anyway
	return vptr[pos+(cnt*2)];
}

void PROC_MAP::_NORMALISE()
{
	//
	// Now to normalise all the verts
	//

	_VERT*	vptr = &verts[0];
	_FACE*	fptr = &faces[0];

	int		vsize = verts.size();
	int		fsize = faces.size();

	int		loop;

	// first clear all the normals
	//
	// Also be adding the noise height in here while I'm at it
	//
	// Oh and by the way, let's calculate the texture coords
	//

	for ( loop = 0; loop < vsize; loop++ )
	{
		vptr->nx = vptr->ny = vptr->nz = 0.0f;		// that's the normals cleared
		
		vptr->s = vptr->x / (float)size_max;		// setting texture coords here
		vptr->t = vptr->z / (float)size_max;

		vptr->y = vptr->y + get_noise( vptr->x, vptr->z );	// and the noise height

		vptr++;
	}

	// reset vptr
	vptr = &verts[0];

	// vars used for the normals calcs
	float	x1, y1, z1;
	float	x2, y2, z2;
	float	x3, y3, z3;
	float	normx, normy, normz;
	float	normlength;

	int		v1, v2, v3;

	int*	vcount = new int[vsize];			// counts the faces a vertex has
	memset( vcount, 0, sizeof(int)*vsize );		// set all values to 0

	// now loop through all the faces and get the normals and average the vert normals
	for ( loop = 0; loop < fsize; loop++ )
	{
		// reverse the faces for the CW face orientation
		v1 = fptr->v1;		v2 = fptr->v3;		v3 = fptr->v2;

		x1 = vptr[v1].x;	y1 = vptr[v1].y;	z1 = vptr[v1].z;
		x2 = vptr[v2].x;	y2 = vptr[v2].y;	z2 = vptr[v2].z;
		x3 = vptr[v3].x;	y3 = vptr[v3].y;	z3 = vptr[v3].z;

		normx = (z1-z2)*(y3-y2)-(y1-y2)*(z3-z2);
		normy = (x1-x2)*(z3-z2)-(z1-z2)*(x3-x2);
		normz = (y1-y2)*(x3-x2)-(x1-x2)*(y3-y2);

		// change this to a fast square root algorithm
		normlength = sqrt((normx*normx)+(normy*normy)+(normz*normz));

		normx /= normlength;
		normy /= normlength;
		normz /= normlength;

		// add this faces normal to all three verts
		vptr[v1].nx = ( vptr[v1].nx + normx );
		vptr[v1].ny = ( vptr[v1].ny + normy );
		vptr[v1].nz = ( vptr[v1].nz + normz );

		vptr[v2].nx = ( vptr[v2].nx + normx );
		vptr[v2].ny = ( vptr[v2].ny + normy );
		vptr[v2].nz = ( vptr[v2].nz + normz );

		vptr[v3].nx = ( vptr[v3].nx + normx );
		vptr[v3].ny = ( vptr[v3].ny + normy );
		vptr[v3].nz = ( vptr[v3].nz + normz );

		vcount[v1]++;	// increment the vert normal count for averaging
		vcount[v2]++;
		vcount[v3]++;

		fptr++;			// almost forgot to move the face pointer onwards
	}

	// now average the normals by the face count
	for ( loop = 0; loop < vsize; loop++ )
	{
		vptr[loop].nx = vptr[loop].nx / (float)vcount[loop];
		vptr[loop].ny = vptr[loop].ny / (float)vcount[loop];
		vptr[loop].nz = vptr[loop].nz / (float)vcount[loop];
	}

	// free the temp vertex faces count
	delete[] vcount;
}

void PROC_MAP::vbo_freedl()
{
	clear_textures();
	for ( int seg = 0; seg < map_size; seg++ )
	{
		glDeleteLists(seglist[seg].DL, 1);
	}
	seglist.clear();
}

//
// Now generate the segments display lists
//
// It will also keep a track an calculate the bounding box and centre of the segment
//

void PROC_MAP::_DISPLAYLISTS()
{
	int		curr_segment = 0;			// main loop segment count

	// resize the segment list to the correct size
	seglist.resize( map_size );

	while ( curr_segment < map_size )
	{
		// get GL to give me a display list ID
		seglist[curr_segment].DL = glGenLists(1);

		// get ready to start... GO...
		glNewList(seglist[curr_segment].DL, GL_COMPILE);

		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// enable texturing
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);

		// create floor, roof and walls
		_DLFLOOR(curr_segment);
		_DLROOF(curr_segment);
		_DLWALL(curr_segment);

		// disable stuff
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_2D);

		glPopMatrix();

		glEndList();

		curr_segment++;
	} // EOF while ( curr_segment < map_size )
}

void PROC_MAP::_DLFLOOR(int seg)
{
	// This function will also start off the bounding box info
	
	int		v1, v2, v3;
	float	x, y, z;
	int		type, segment;

	_VERT*	vp = &verts[0];
	_FACE*	fp = &faces[0];

	int		fc = faces.size();

	// I need to start the bounding box details so search for first matching segment entry
	while ( fp->segment != seg && fp->type != PM_CFACE && fp->type != PM_RFACE )
	{
		fp++;
		fc--;
	}

	// store bounding box info just from first coords
	v1 = fp->v1;
	seglist[seg].xmin = seglist[seg].xmax = vp[v1].x;
	seglist[seg].ymin = seglist[seg].ymax = vp[v1].y;
	seglist[seg].zmin = seglist[seg].zmax = vp[v1].z;

	// set the texture
	if ( fp->type == PM_CFACE )
		glBindTexture(GL_TEXTURE_2D, gltextureID[0]);
	else
		glBindTexture(GL_TEXTURE_2D, gltextureID[3]);

	// add faces from the main list now
	glBegin(GL_TRIANGLES);

	while ( fc )
	{
		if ( fp->segment == seg  && fp->type == PM_CFACE )	// just floor faces
		{
			v1 = fp->v1;
			v2 = fp->v2;
			v3 = fp->v3;

			glTexCoord2f(vp[v1].s,  vp[v1].t);
			glNormal3f  (vp[v1].nx, vp[v1].ny, vp[v1].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v2].s,  vp[v2].t);
			glNormal3f  (vp[v2].nx, vp[v2].ny, vp[v2].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v3].s,  vp[v3].t);
			glNormal3f  (vp[v3].nx, vp[v3].ny, vp[v3].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);
		}

		fp++;
		fc--;
	}

	glEnd();
}

void PROC_MAP::_DLROOF(int seg)
{
	int		v1, v2, v3;
	float	x, y, z;
	int		type, segment;

	_VERT*	vp = &verts[0];
	_FACE*	fp = &faces[0];

	int		fc = faces.size();

	// I need to search for the first matching so I can set the texture
	while ( fp->segment != seg && fp->type != PM_CFACER && fp->type != PM_RFACER )
	{
		fp++;
		fc--;
	}

	// set the texture
	if ( fp->type == PM_CFACER )
		glBindTexture(GL_TEXTURE_2D, gltextureID[1]);
	else
		glBindTexture(GL_TEXTURE_2D, gltextureID[4]);

	// add faces from the main list now
	glBegin(GL_TRIANGLES);

	while ( fc )
	{
		if ( fp->segment == seg && fp->type == PM_CFACER )	// just roof segments
		{
			v1 = fp->v1;
			v2 = fp->v2;
			v3 = fp->v3;

			glTexCoord2f(vp[v1].s,  vp[v1].t);
			glNormal3f  (vp[v1].nx, vp[v1].ny, vp[v1].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v2].s,  vp[v2].t);
			glNormal3f  (vp[v2].nx, vp[v2].ny, vp[v2].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v3].s,  vp[v3].t);
			glNormal3f  (vp[v3].nx, vp[v3].ny, vp[v3].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);
		}

		fp++;
		fc--;
	}

	glEnd();
}

void PROC_MAP::_DLWALL(int seg)
{
	int		v1, v2, v3;
	float	x, y, z;
	int		type, segment;

	_VERT*	vp = &verts[0];
	_FACE*	fp = &faces[0];

	int		fc = faces.size();

	// I need to search for the first matching so I can set the texture
	while ( fp->segment != seg && fp->type != PM_CFACEW && fp->type != PM_RFACEW )
	{
		fp++;
		fc--;
	}

	// set the texture
	if ( fp->type == PM_CFACEW )
		glBindTexture(GL_TEXTURE_2D, gltextureID[2]);
	else
		glBindTexture(GL_TEXTURE_2D, gltextureID[5]);

	// add faces from the main list now
	glBegin(GL_TRIANGLES);

	while ( fc )
	{
		if ( fp->segment == seg && fp->type == PM_CFACEW )	// just wall faces
		{
			v1 = fp->v1;
			v2 = fp->v2;
			v3 = fp->v3;

			glTexCoord2f(vp[v1].s,  vp[v1].t);
			glNormal3f  (vp[v1].nx, vp[v1].ny, vp[v1].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v2].s,  vp[v2].t);
			glNormal3f  (vp[v2].nx, vp[v2].ny, vp[v2].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);

			glTexCoord2f(vp[v3].s,  vp[v3].t);
			glNormal3f  (vp[v3].nx, vp[v3].ny, vp[v3].nz);
			x = vp[v1].x; y = vp[v1].y; z = vp[v1].z;
			glVertex3f  (x, y, z);
			_BOUNDCHECK (x, y, z, seg);
		}

		fp++;
		fc--;
	}

	glEnd();
}

void PROC_MAP::_BOUNDCHECK(float x, float y, float z, int seg)
{
	if		( x < seglist[seg].xmin ) seglist[seg].xmin = x;
	else if ( x > seglist[seg].xmax ) seglist[seg].xmax = x;

	if		( y < seglist[seg].ymin ) seglist[seg].ymin = y;
	else if ( y > seglist[seg].ymax ) seglist[seg].ymax = y;

	if		( z < seglist[seg].zmin ) seglist[seg].zmin = z;
	else if ( z > seglist[seg].zmax ) seglist[seg].zmax = z;
}

void PROC_MAP::vbo_drawdl()
{
	int seg = 0;

	while ( seg < map_size )
	{
		glCallList(seglist[seg].DL);
		seg++;
	}
}