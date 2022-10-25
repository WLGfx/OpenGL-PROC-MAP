#include <vector>

using namespace std;

#include "perlin_noise.h"
#include "PROC_MAP.h"
#include "_OGL.h"
#include "math.h"

void PROC_MAP::create_textures(int count)
{
	// check if textures already exist just in case of a leak
	clear_textures();

	gltextureID.resize( count );

	for ( int loop=0; loop<count; loop++ )
	{
		create_texture(&gltextureID[loop]);
	}
}

void PROC_MAP::clear_textures()
{
	if ( (int)gltextureID.size() )
	{
		for ( int loop=0; loop<(int)gltextureID.size(); loop++ )
		{
			glDeleteTextures(1, &gltextureID[loop]);
		}
		gltextureID.clear();
	}
}

#define TEXTURE_SIZE 32
#define TEXTURE_RANGE 64
#define TEXTURE_SUB 32

void PROC_MAP::create_texture(GLuint *id)
{
	char	texture[TEXTURE_SIZE * TEXTURE_SIZE * 3];
	char	*ptr=texture;
	int		size = TEXTURE_SIZE * TEXTURE_SIZE;

	// initial RGB values
	int		mr = rand() & 255;
	int		mg = rand() & 255;
	int		mb = rand() & 255;

	int		nr, ng, nb;

	while ( size )
	{
		nr = mr + ( rand() % TEXTURE_RANGE ) - TEXTURE_SUB;
		ng = mg + ( rand() % TEXTURE_RANGE ) - TEXTURE_SUB;
		nb = mb + ( rand() % TEXTURE_RANGE ) - TEXTURE_SUB;
		if ( nr > 255 ) nr = 255;
		if ( ng > 255 ) ng = 255;
		if ( nb > 255 ) nb = 255;
		if ( nr < 0 ) nr = 0;
		if ( ng < 0 ) ng = 0;
		if ( nb < 0 ) nb = 0;
		*ptr++ = nr;
		*ptr++ = ng;
		*ptr++ = nb;
		size--;
	}

	glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
}
