#pragma once

//
// Occlusion culling has been added to the _FACE class
//

#include "perlin_noise.h"
#include "gl/freeglut.h"

class _VERT {
public:
	float	x,y,z;

	// these are used to setup the mesh info (makes the structure 32 bytes each)
	float	nx, ny, nz;
	float	s, t;

	inline	_VERT() { x=y=z=0; }
	inline	_VERT(float xx, float yy, float zz)		{ x=xx; y=yy; z=zz; }
	inline	void set(float xx, float yy, float zz)	{ x=xx; y=yy; z=zz; }
};

class _EDGE {
public:
	int		v1, v2;		// start and end
	int		type;		// corridor or room / inner or outer?
	int		segment;	// segment this edge belongs to

	inline	_EDGE()
	{ 
		v1=v2=type=segment=0;
	}

	inline	_EDGE(int v1, int v2, int type, int segment)
	{
		this->v1=v1;
		this->v2=v2;
		this->type=type;
		this->segment=segment;
	}

	inline	void set(int v1, int v2, int type, int segment)
	{ 
		this->v1=v1;
		this->v2=v2;
		this->type=type;
		this->segment=segment;
	}

	inline	void set(int type)
	{
		this->type=type;
	}

	inline	void set(int v1, int v2)
	{
		this->v1=v1;
		this->v2=v2;
	}
};

class _FACE {
public:
	int		v1, v2, v3;
	int		type;

	// An addition now is the segment number to be used in the occlusion culling to boost FPS
	// When the algorithm creates each limb it will create it's own verts and index list
	int		segment;

	inline	_FACE() { v1=v2=v3=0; }
	inline	_FACE(int p1, int p2, int p3, int type, int segment)		
	{ 
		v1 = p1; 
		v2 = p2; 
		v3 = p3; 
		this->type		= type;
		this->segment	= segment;
	}
	inline	void set(int p1, int p2, int p3)	
	{ 
		v1=p1; 
		v2=p2; 
		v3=p3; 
	}
	inline	void set(int p1, int p2, int p3, int type)
	{
		v1=p1;
		v2=p2;
		v3=p3;
		this->type=type;
	}
	inline	void set(int type)
	{
		this->type=type;
	}
};

// definitions for the edges list
#define PM_CINNER	0		// corridor inner edge
#define PM_COUTER	1		// corridor outer edge
#define PM_RINNER	2		// room inner edge
#define PM_ROUTER	3		// room outer edge
#define PM_RENTRY	4		// room entry edge (ie the doorway)

#define PM_COUTERR	5		// corridor outer edge on roof
#define PM_ROUTERR	6		// room outer edge on roof

#define PM_COUTERW	7		// corridor outer edge on wall
#define PM_ROUTERW	8		// room outer edge on wall


#define PM_CFACE	0		// corridor face
#define PM_RFACE	1		// room face

#define PM_CFACER	2		// corridor face roof
#define PM_RFACER	3		// room face on roof

#define PM_CFACEW	4		// corridor face on wall
#define PM_RFACEW	5		// room face on wall

//
// defaults for the map setup
//

#define PM_MAPSIZE	100		// default map size in segments to add

#define PM_SIZEMIN	40		// corridor wall length min and max range
#define PM_SIZEMAX	50
#define PM_ANGMIN	60		// new corridor angle min max range
#define PM_ANGMAX	120
#define PM_CSEGMIN	6		// add corridor segments min max range
#define PM_CSEGMAX	16

#define PM_NOISEHGT	200		// default height adjusted by the perlin noise

#define PM_ROOMMIN	80		// room edges range for size
#define PM_ROOMMAX	150
#define PM_RSEGMIN	7		// room edges count range (seems safe with min of 7)
#define PM_RSEGMAX	15

#define PM_RNDSEED	14		// random seed default (was originally zero)

#define PM_NOISE_SCALE 100.0f	// scale down the perlin noise to stop glitches

struct _temp_edge_list {
	int v1, v2, mid_vert;
};

struct _gl_seglist {
	int		DL;						// display list ID
	float	xmin, zmin;				// bounding box
	float	xmax, zmax;
	float	ymin, ymax;
	float	xcentre, ycentre;		// centre of the segment
};

using namespace std;

class PROC_MAP {
public:
	PROC_MAP(void);
	~PROC_MAP(void);

public:
	void	setup(int size, int seed, int lmin, int lmax, int amin, int amax);
	void	generate(int zoom);

	void	get_edge_centre(int edge, float *x, float *y, float *z);

	int		verts_count() { return verts.size(); }
	int		edges_count() { return edges.size(); }
	int		faces_count() { return faces.size(); }

public:
	void	set_map(int size, int seed, int height);
	void	set_corridor(int len_min, int len_max, int aang_min, int aang_max, int seg_min, int seg_max);
	void	set_room(int len_min, int len_max, int seg_min, int seg_max);

private:
	vector<_VERT>	verts;		// main verts list
	vector<_EDGE>	edges;		// main edges list
	vector<_FACE>	faces;		// main faces list

	//_temp_vertex_buffer* vbo;	// pointer to the normals and texture coords for the mesh

public:
	int		map_size;			// ammount of corridors and rooms that make the map
	int		map_seed;

	int		size_min, size_max;	// corridor length min and max range
	int		ang_min, ang_max;	// corridor angle range
	int		cseg_min, cseg_max;	// corridor segment range

	int		room_min, room_max;	// length of the edges
	int		rseg_min, rseg_max;	// number of sides to the room

	int		noise_hgt;			// height adjusted by perlin noise
	int		tess;				// tessellation count

private:
	PerlinNoise pn;				// used to slope the dungeon

	// more internal properties

	// Current segment number being generated
	// This will be used for the occlusion culling by creating seperate limbs for each
	// corridor and room. After calculating the extents of each limb and the center coords
	// a simple distance calc will determine whether it's hidden or not.
private:
	int		curr_segment;

	int		ObjID;

	int		texture_floor;
	int		texture_roof;
	int		texture_wall;

	int		bump_floor;

private:
	void	start_poly();

	int		add_corridor_seg( int edge );	// adds a single corridor segment

	bool	add_corridor();		// attempts to add a corridor
	bool	add_room();			// attempts to add a room

	// check if edge collides with the map
	bool	edge_intersect(float x1, float z1, float x2, float z2);
	// check if a line intersects another line
	bool	line_intersect(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);

	float	get_noise(float x, float y);	// get the Y height for the level verts

	// gets the centre of a circle using 3 points on its circumference
	void	vvv_circle(	float px, float py,
						float qx, float qy,
						float rx, float ry,
						float *x, float *y); //, float &r );

public:
	// used by the level editor
	void	gl_draw2dmap();
	void	tessellate(int levels);
	void	gl_setup_vbo();

private:
	void	create_texture(GLuint* id);
	int		find_temp_edge(int v1, int v2, _temp_edge_list* list, int size);

	GLuint	gl_floorVBO;

	void	add_normal(int face, float *nx, float *ny, float *nz);

	void	extend_roof_vbo();
	void	extend_wall_vbo_old();
	void	extend_wall_vbo();

	void	normalise_floor();
	void	normalise_roof();
	void	normalise_wall();

	void	push_wall_verts(int vert, float uv);

	void	generate_display_lists();
	void	clear_display_lists();

	int		verts_roof, verts_wall;		// start of the roof and wall verts indices
	int		edges_roof, edges_wall;		// start of the roof and wall edges lists
	int		faces_roof, faces_wall;		// same for the faces

	//
	// NEW UPDATES:
	// After calling generate to create the 2d floor mesh (without noise), then call:
	// vbo_create() to create the floor and wall face tables
	// vbo_tessellate() to tesselate all
	// vbo_normals() to calculate the mesh vertex normals
	//

public:
	// Make the mesh, tessellate it, calc normals, tex coords and create dl's
	void	vbo_create();
	void	vbo_freedl();
	void	vbo_drawdl();

	vector<GLuint>		gltextureID;
	vector<_gl_seglist>	seglist;

private:
	int		verts_count_floor;
	int		edges_count_floor;
	int		faces_count_floor;

	void	_TESSELLATE();
	int		_FINDMID(int v1, int v2, int* vptr, int cnt);
	void	_NORMALISE();
	void	_DISPLAYLISTS();
	void	_DLFLOOR(int curr_segment);
	void	_DLROOF(int curr_segment);
	void	_DLWALL(int curr_segment);
	void	_BOUNDCHECK(float x, float y, float z, int seg);

	void	create_textures(int count);
	void	clear_textures();
};

/*

I balls'd up a bit when creating the wall vbo.
I will need to create a temp list of the edges that actually fall on the outer edge.

Balls'd up a second time and dunno what happened.
3rd time lucky, taking each step at a time.
1. Count the outer edges.
2. Create the outer edges list and copy the data into it.
3. Bubble sort the array.

Another problem which I've only just figured out. 


1. Taken out the noise calc per vert from the initial generation function
2. Added new mesh setup function
3. Tessellation now takes place on floor, roof and walls at the same time
4. Texture coords done during normalisation function.
5. Noise height for Y coord added during normalisation function.

*/