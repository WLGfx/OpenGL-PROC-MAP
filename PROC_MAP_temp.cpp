#include <vector>

using namespace std;

#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"

//
// Setup the Mesh infor for the level.
//
// This will also have to duplicate the floor mesh and raise it for the roof as well
// as reversing the faces on the roof too.
//
// Then claculate the vertex normals for both the floor and the roof.
//
// Then I will have the problem of creating the walls with tesselation.
//

void PROC_MAP::extend_roof_vbo()
{
	int		vsize = verts.size();
	int		esize = edges.size();
	int		fsize = faces.size();

	// set these here
	verts_roof = vsize;
	edges_roof = esize;
	faces_roof = fsize;

	// optimising code:
	// resize the verts data to double it's size then just move everything over
	verts.resize( vsize * 2 );
	_VERT* vptr1 = &verts[0];
	_VERT* vptr2 = &verts[vsize];
	memcpy( vptr2, vptr1, sizeof(_VERT) * vsize );

	int		v1, v2, v3;

	int		loop;

	for ( loop = 0; loop < vsize; loop ++ )
	{
		vptr2->y = vptr2->y + size_max;
		vptr2++;
	}

	// I can save a chunk of memory here by not extending the edges list
	// Only the vertices and face index list is required to create the final mesh

	// optimisation here too on the same thing

	faces.resize( fsize * 2 );
	_FACE* fptr1 = &faces[0];
	_FACE* fptr2 = &faces[fsize];
	memcpy( fptr2, fptr1, sizeof(_FACE) * fsize );

	for ( loop = 0; loop < fsize; loop++ )
	{
		// optimised here too
		v1 = fptr2->v3;
		fptr2->v3 = fptr2->v2;
		fptr2->v2 = v1;
		fptr2->v1 += vsize;
		fptr2->v2 += vsize;
		fptr2->v3 += vsize;
		fptr2++;
	}

	// set these here now I've got them
	verts_wall = verts.size();
	edges_wall = edges.size();
	faces_wall = faces.size();
}

void PROC_MAP::push_wall_verts(int vert, float uv)
{
	float x, y, z;
	float hgt = 0.0f;
	float step = 1.0f / ( 1.0f + tess );
	int loop = tess + 2;

	x = verts[vert].x;
	y = verts[vert].y;
	z = verts[vert].z;

	int pos = verts.size();

	//for ( hgt = 0.0f; hgt <= 1.0f; hgt = hgt + step )
	while ( loop-- )
	{
		verts.push_back( _VERT( x, y, z ) );

		y = y + ( (float)size_max * hgt );		// next step upwards

		verts[ pos ].s = uv;				// texture coords
		verts[ pos ].t = hgt;

		hgt = hgt + step;

		pos++;
	}
}

void PROC_MAP::extend_wall_vbo_old()
{
	// first off, I need to bubble sort the outer edges list from the first edge
	// I might bee able to throw everything into just one loop if my head can hold it all in

	vector<_EDGE> tedges;

	tedges.clear();

	for ( int loop = 0; loop < edges_roof; loop++ )
	{
		if ( edges[loop].type == PM_COUTER || edges[loop].type == PM_ROUTER )
		{
			tedges.push_back( _EDGE( edges[loop].v1, edges[loop].v2, edges[loop].type, edges[loop].segment ) );
		}
	}

	int		tsize = tedges.size();
	
	int		vcurr = verts_wall;

	int		vstep = tess + 2;
	
	// store the first set of tessellated verts with UV's
	float uv = 0.0f;

	push_wall_verts( tedges[0].v1, uv );

	// now sort the outer edges of the list (only need the original set though)
	// and add the faces

	int epos = 0;

	// vars to calc the next uv's
	int		v1, v2;
	float	x1,y1,x2,y2,dx,dy,dist;

	while ( epos < tsize )
	{
		//push_wall_verts( tedges[0].v1, uv );

		// increase the texture coord across the wall edge
		v1 = tedges[ epos ].v1;		v2 = tedges[ epos ].v2;
		x1 = verts[ v1 ].x;			y1 = verts[ v1 ].z;
		x2 = verts[ v2 ].x;			y2 = verts[ v2 ].z;
		dx = x2-x1; dx = dx * dx;	dy = y2-y1; dy = dy * dy;
		dist = sqrt( dx + dy );

		uv = uv + ( dist / (float)size_max );

		push_wall_verts( tedges[ epos ].v2, uv );

		// now add faces for the wall section
		for ( int tstep = 0; tstep < vstep; tstep++ )
		{
			// face 1
			int v1 = vcurr + tstep;
			int	v2 = vcurr + tstep + 1;
			int v3 = vcurr + tstep + vstep + 1;

			int type = tedges[ epos ].type;

			faces.push_back( _FACE( v1, v2, v3, type, tedges[ epos ].segment ) );
			
			// face 2
			v2 = vcurr + tstep + vstep;
			faces.push_back( _FACE( v1, v3, v2, type, tedges[ epos ].segment ) );
		}

		vcurr += vstep;

		// put the next edge next to this one if not at end of original edge list
		if ( epos < ( tsize - 2 ) )
		{
			_EDGE tedge;

			int curr = epos + 1;
			int search_for = tedges[ epos ].v2;

			while ( curr < tsize && tedges[ curr ].v1 != search_for )
			{
				curr++;
			}

			if ( curr < tsize ) //!= ( epos + 1 ) && curr < tsize )
			{
				memcpy( &tedge,				&tedges[epos+1],	sizeof( _EDGE ) );
				memcpy( &tedges[epos+1],	&tedges[curr],		sizeof( _EDGE ) );
				memcpy( &tedges[curr],		&tedge,				sizeof( _EDGE ) );
			}
		}

		epos++;
	}

	tedges.clear();
}

void PROC_MAP::gl_setup_vbo()
{
	// first of all - extend the buffers for the roof
	extend_roof_vbo();

	// now extend the walls lists
	extend_wall_vbo();

	// now to normalise the floor, roof and walls
	// This was orignally done in one whole chunk but it was pointless as
	// the loops where searching through the entire data for the whole object
	// Now it will only normalise the separate sections.
	normalise_floor();
	normalise_roof();
	normalise_wall();

	// Now setup the segments display lists
	generate_display_lists();
}

void PROC_MAP::clear_display_lists()
{
	int loop = 0;

	while ( loop < seglist.size() )
	{
		if ( glIsList( seglist[loop].DL ) == GL_TRUE )
			glDeleteLists( seglist[loop].DL, 1 );

		loop++;
	}

	seglist.clear();
}

void PROC_MAP::generate_display_lists()
{
	// in case of anything in the future then clear them first (don't take long)
	clear_display_lists();

	seglist.resize( this->map_size );

	// At this point, to speed things up (might not do though) I may need to sort
	// the faces list so that I can just use one glDrawElements() call.

	// So far all the data for the faces are stored consequtive in the floor, roof and wall lists

	int		curr_segment = 0;
	int		curr_floor, curr_roof, curr_wall;
	int		curr_end = faces.size();

	int		v;
	int		type;

	while ( curr_segment < map_size )
	{
		seglist[curr_segment].DL = glGenLists( 1 );

		glNewList( seglist[curr_segment].DL, GL_COMPILE );
		{
			// Save stuff here
			glPushMatrix();

			// init enables
			glEnable( GL_TEXTURE_2D );
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );

			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			curr_floor	= 0;				// set start positions
			curr_roof	= faces_roof;
			curr_wall	= faces_wall;

			// find the first matching segment in a face (patched in to fix the textures)
			while ( faces[ curr_floor ].segment != curr_segment )
				curr_floor++;

			if ( faces[ curr_floor ].type == PM_CFACE )
				type = 0;
			else
				type = 3;

			//
			// FLOOR
			//

			glBindTexture(GL_TEXTURE_2D, gltextureID[0 + type] );

			glBegin( GL_TRIANGLES );
			
			while ( curr_floor < faces_roof )
			{
				if ( faces[ curr_floor ].segment == curr_segment )
				{
					v = faces[ curr_floor ].v1;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_floor ].v2;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_floor ].v3;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );
				}
				curr_floor++;
			}

			glEnd();

			//
			// ROOF
			//
			
			glBindTexture(GL_TEXTURE_2D, gltextureID[1 + type] );

			glBegin( GL_TRIANGLES );
			
			while ( curr_roof < faces_wall )
			{
				if ( faces[ curr_roof ].segment == curr_segment )
				{
					v = faces[ curr_roof ].v1;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_roof ].v2;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_roof ].v3;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );
				}
				curr_roof++;
			}

			glEnd();

			//
			// WALL
			//
			
			glBindTexture(GL_TEXTURE_2D, gltextureID[2 + type] );

			glBegin( GL_TRIANGLES );
			
			while ( curr_wall < curr_end )
			{
				if ( faces[ curr_wall ].segment == curr_segment )
				{
					v = faces[ curr_wall ].v1;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_wall ].v2;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );

					v = faces[ curr_wall ].v3;

					glNormal3f  ( verts[ v ].nx, verts[ v ].ny, verts[ v ].nz );
					glTexCoord2f( verts[ v ].s,  verts[ v ].t );
					glVertex3f  ( verts[ v ].x,  verts[ v ].y,  verts[ v ].z );
				}
				curr_wall++;
			}

			glEnd();

			// init enables
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_NORMALIZE );

			glPopMatrix();
		}
		glEndList();

		curr_segment++;
	}
}

void PROC_MAP::add_normal(int face, float *nx, float *ny, float *nz)
{
	_FACE*	fptr = &faces[face];

	int		v1 = fptr->v1;
	int		v2 = fptr->v2;
	int		v3 = fptr->v3;

	_VERT*	vptr = &verts[v1];

	int		x1 = vptr->x;
	int		y1 = vptr->y;
	int		z1 = vptr->z;

			vptr = &verts[v3];	// swapped around for CW face orientation

	int		x2 = vptr->x;
	int		y2 = vptr->y;
	int		z2 = vptr->z;

			vptr = &verts[v2];

	int		x3 = vptr->x;
	int		y3 = vptr->y;
	int		z3 = vptr->z;

	float	normx = (z1-z2)*(y3-y2)-(y1-y2)*(z3-z2);
	float	normy = (x1-x2)*(z3-z2)-(z1-z2)*(x3-x2);
	float	normz = (y1-y2)*(x3-x2)-(x1-x2)*(y3-y2);

	float	normlength = sqrt((normx*normx)+(normy*normy)+(normz*normz));

	normx /= normlength;
	normy /= normlength;
	normz /= normlength;

	*nx = *nx + normx;
	*ny = *ny + normy;
	*nz = *nz + normz;
}

void PROC_MAP::normalise_floor()
{
	float	nx, ny, nz;
	int		ncount;

	int		fsize, vsize;

	//
	// Process all the vertex normals ( a rewrite of the main _VERT structure has been done)
	//

	vsize = verts_roof;		// end the loop here
	fsize = faces_roof;

	int loop, floop;

	_VERT*	vptr_start	= &verts[0];
	_FACE*	fptr_start	= &faces[0];
	int		f_start		= 0;

	_VERT*	vptr = vptr_start;
	_FACE*	fptr;

	for ( loop=0; loop < vsize; loop++ )
	{
		vptr->s = fmod( (float)size_max, vptr->x / (float)size_max );	// texture coords
		vptr->t = fmod( (float)size_max, vptr->z / (float)size_max );

		ncount = 0;			// start to the normal averaging count
		nx = ny = nz = 0;	// reset this verts normal

		fptr = fptr_start;

		for ( floop = f_start; floop < fsize; floop++ )
		{
			// if a vert then in a face then add the normal to the current one
			if ( fptr->v1 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v2 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v3 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }

			fptr++;
		}

		// now average them and store
		vptr->nx = nx / (float)ncount;
		vptr->ny = ny / (float)ncount;
		vptr->nz = nz / (float)ncount;

		vptr++;
	}
}

void PROC_MAP::normalise_roof()
{
	float	nx, ny, nz;
	int		ncount;

	int		fsize, vsize;

	//
	// Process all the vertex normals ( a rewrite of the main _VERT structure has been done)
	//

	vsize = verts_wall;		// end the loop here
	fsize = faces_wall;

	int loop, floop;

	_VERT*	vptr_start	= &verts[verts_roof];
	_FACE*	fptr_start	= &faces[faces_roof];
	int		f_start		= faces_roof;

	_VERT*	vptr = vptr_start;
	_FACE*	fptr;

	for ( loop=verts_roof; loop < vsize; loop++ )
	{
		vptr->s = vptr->x / (float)size_max;	// texture coords
		vptr->t = vptr->z / (float)size_max;

		ncount = 0;			// start to the normal averaging count
		nx = ny = nz = 0;	// reset this verts normal

		fptr = fptr_start;

		for ( floop = f_start; floop < fsize; floop++ )
		{
			// if a vert then in a face then add the normal to the current one
			if ( fptr->v1 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v2 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v3 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }

			fptr++;
		}

		// now average them and store
		vptr->nx = nx / (float)ncount;
		vptr->ny = ny / (float)ncount;
		vptr->nz = nz / (float)ncount;

		vptr++;
	}
}

void PROC_MAP::normalise_wall()
{
	float	nx, ny, nz;
	int		ncount;

	int		fsize, vsize;

	//
	// Process all the vertex normals ( a rewrite of the main _VERT structure has been done)
	//

	vsize = verts.size();		// end the loop here
	fsize = faces.size();

	int loop, floop;

	_VERT*	vptr_start	= &verts[verts_wall];
	_FACE*	fptr_start	= &faces[faces_wall];
	int		f_start		= faces_wall;

	_VERT*	vptr = vptr_start;
	_FACE*	fptr;

	for ( loop=verts_wall; loop < vsize; loop++ )
	{
		//vptr->s = (float)size_max / vptr->x;	// texture coords
		//vptr->t = (float)size_max / vptr->z;

		ncount = 0;			// start to the normal averaging count
		nx = ny = nz = 0;	// reset this verts normal

		fptr = fptr_start;

		for ( floop = f_start; floop < fsize; floop++ )
		{
			// if a vert then in a face then add the normal to the current one
			if ( fptr->v1 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v2 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }
			if ( fptr->v3 == loop ) { add_normal(floop, &nx, &ny, &nz); ncount++; }

			fptr++;
		}

		// now average them and store
		vptr->nx = nx / (float)ncount;
		vptr->ny = ny / (float)ncount;
		vptr->nz = nz / (float)ncount;

		vptr++;
	}

}