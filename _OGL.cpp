#include "_OGL.h"

bool	glut_keys[256];
int		glut_mousex, glut_mousey;
int		glut_mousebutton;
int		glut_mousemovex, glut_mousemovey;

HWND	glut_hwnd;
HINSTANCE glut_hinstance;
bool	glut_mainwindow;

//
// Hidden internal functions
//

void handle_close()
{
	glut_mainwindow = 1;
}

void handle_glutdownkeys( unsigned char key, int x, int y )
{
	glut_keys[ key ] = true;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutupkeys( unsigned char key, int x, int y )
{
	glut_keys[ key ] = false;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutspecialkeys( int key, int x, int y )
{
	glut_keys[ key ] = true;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutupspecialkeys( int key, int x, int y )
{
	glut_keys[ key ] = false;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutmouse( int button, int state, int x, int y )
{
	int flags;
	switch ( button ) {
		case GLUT_LEFT_BUTTON:
			flags = 0x01;
			break;
		case GLUT_RIGHT_BUTTON:
			flags = 0x02;
			break;
		case GLUT_MIDDLE_BUTTON:
			flags = 0x04;
			break;
	}
	if ( state == GLUT_DOWN )
		glut_mousebutton |= flags;
	else
		glut_mousebutton &= flags;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutmotion( int x, int y )
{
	glut_mousemovex = x - glut_mousex;
	glut_mousemovey = y - glut_mousey;
	glut_mousex = x;
	glut_mousey = y;
}

void handle_glutpassivemotion( int x, int y )
{
	glut_mousemovex = x - glut_mousex;
	glut_mousemovey = y - glut_mousey;
	glut_mousex = x;
	glut_mousey = y;
}

int glut_MouseMoveX()
{
	int ret = glut_mousemovex;
	glut_mousemovex = 0;
	return ret;
}

int glut_MouseMoveY()
{
	int ret = glut_mousemovey;
	glut_mousemovey = 0;
	return ret;
}

void handle_idle()
{
	// When gl isn't doing anything then pass the HWND over
	static bool firstTime = true;
	if(firstTime == true)
	{
		firstTime = false;
		glut_hwnd = GetForegroundWindow();
	}
}

void handle_reshape (int width, int height) {
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,10000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}



//
// CAMERA
//

#include "math.h"
//#include <iostream>
//#include "windows.h"

#define SQR(x) (x*x)

#define NULL_VECTOR F3dVector(0.0f,0.0f,0.0f)

SF3dVector F3dVector ( GLfloat x, GLfloat y, GLfloat z )
{
	SF3dVector tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;
	return tmp;
}

GLfloat GetF3dVectorLength( SF3dVector * v)
{
	return (GLfloat)(sqrt(SQR(v->x)+SQR(v->y)+SQR(v->z)));
}

SF3dVector Normalize3dVector( SF3dVector v)
{
	SF3dVector res;
	float l = GetF3dVectorLength(&v);
	if (l == 0.0f) return NULL_VECTOR;
	res.x = v.x / l;
	res.y = v.y / l;
	res.z = v.z / l;
	return res;
}

SF3dVector operator+ (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x+u.x;
	res.y = v.y+u.y;
	res.z = v.z+u.z;
	return res;
}
SF3dVector operator- (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x-u.x;
	res.y = v.y-u.y;
	res.z = v.z-u.z;
	return res;
}


SF3dVector operator* (SF3dVector v, float r)
{
	SF3dVector res;
	res.x = v.x*r;
	res.y = v.y*r;
	res.z = v.z*r;
	return res;
}

SF3dVector CrossProduct (SF3dVector * u, SF3dVector * v)
{
	SF3dVector resVector;
	resVector.x = u->y*v->z - u->z*v->y;
	resVector.y = u->z*v->x - u->x*v->z;
	resVector.z = u->x*v->y - u->y*v->x;

	return resVector;
}
float operator* (SF3dVector v, SF3dVector u)	//dot product
{
	return v.x*u.x+v.y*u.y+v.z*u.z;
}




/***************************************************************************************/

EGL_CAM::EGL_CAM()
{
	//Init with standard OGL values:
	Position = F3dVector (0.0, 0.0,	0.0);
	ViewDir = F3dVector( 0.0, 0.0, -1.0);
	RightVector = F3dVector (1.0, 0.0, 0.0);
	UpVector = F3dVector (0.0, 1.0, 0.0);

	//Only to be sure:
	RotatedX = RotatedY = RotatedZ = 0.0;
}

void EGL_CAM::GetPosition(GLfloat *x, GLfloat *y, GLfloat *z)
{
	*x = Position.x;
	*y = Position.y;
	*z = Position.z;
}

void EGL_CAM::Move (SF3dVector Direction)
{
	Position = Position + Direction;
}

void EGL_CAM::Place (SF3dVector Pos)
{
	Position = Pos;
}

void EGL_CAM::Place (float x, float y, float z)
{
	Position.x = x;
	Position.y = y;
	Position.z = z;
}

void EGL_CAM::RotateX (GLfloat Angle)
{
	RotatedX += Angle;
	
	//Rotate viewdir around the right vector:
	ViewDir = Normalize3dVector(ViewDir*cos(Angle*PIdiv180)
								+ UpVector*sin(Angle*PIdiv180));

	//now compute the new UpVector (by cross product)
	UpVector = CrossProduct(&ViewDir, &RightVector)*-1;

	
}

void EGL_CAM::RotateY (GLfloat Angle)
{
	RotatedY += Angle;
	
	//Rotate viewdir around the up vector:
	ViewDir = Normalize3dVector(ViewDir*cos(Angle*PIdiv180)
								- RightVector*sin(Angle*PIdiv180));

	//now compute the new RightVector (by cross product)
	RightVector = CrossProduct(&ViewDir, &UpVector);
}

void EGL_CAM::RotateZ (GLfloat Angle)
{
	RotatedZ += Angle;
	
	//Rotate viewdir around the right vector:
	RightVector = Normalize3dVector(RightVector*cos(Angle*PIdiv180)
								+ UpVector*sin(Angle*PIdiv180));

	//now compute the new UpVector (by cross product)
	UpVector = CrossProduct(&ViewDir, &RightVector)*-1;
}

void EGL_CAM::Angle(float x, float y, float z)
{
	this->RotatedX = x;
	this->RotatedY = y;
	this->RotatedZ = z;

	// Might need to do those camera thingies as in the above cam functions
}

void EGL_CAM::Render( void )
{

	//The point at which the camera looks:
	SF3dVector ViewPoint = Position+ViewDir;

	//as we know the up vector, we can easily use gluLookAt:
	gluLookAt(	Position.x,Position.y,Position.z,
				ViewPoint.x,ViewPoint.y,ViewPoint.z,
				UpVector.x,UpVector.y,UpVector.z);

}

void EGL_CAM::MoveForward( GLfloat Distance )
{
	Position = Position + (ViewDir*-Distance);
}

void EGL_CAM::StrafeRight ( GLfloat Distance )
{
	Position = Position + (RightVector*Distance);
}

void EGL_CAM::MoveUpward( GLfloat Distance )
{
	Position = Position + (UpVector*Distance);
}

//
// MAIN CLASS HANDLER
//



_OGL::_OGL(void)
{
}

_OGL::~_OGL(void)
{
}

int _OGL::init(int* cmdl, int cmds, int wid, int hgt)
{
	// Initialise the window
	char *argv[] = {"foo", "bar"};
	int argc = 2; // must/should match the number of strings in argv

	glutInit((int*)&argc, argv);
	//glutInit( cmdl, (char**)cmds );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( wid, hgt );
	glutInitWindowPosition( 50, 50 );
	glutCreateWindow( "WLGfx Demonstration 1 in openGL and GLUT..." );
	glMatrixMode( GL_MODELVIEW );

	// Setup the keyboard function callback to register keyboard input
	//for ( int x = 0; x < 256; x++ )
	//	glut_keys[ x ] = false;
	memset( &glut_keys, 0, 256 );

	glut_mousex = glut_mousey = glut_mousebutton = glut_mainwindow = 0;

	glutKeyboardFunc( handle_glutdownkeys );
	glutKeyboardUpFunc( handle_glutupkeys );
	glutSpecialFunc( handle_glutspecialkeys );
	glutSpecialUpFunc( handle_glutupspecialkeys );
	glutMouseFunc( handle_glutmouse );
	glutMotionFunc( handle_glutmotion );
	glutPassiveMotionFunc( handle_glutpassivemotion );
	glutIdleFunc( handle_idle );
	glutCloseFunc( handle_close );

	// Setup the screen resize
	glutReshapeFunc( handle_reshape );
	glutPostRedisplay();		// hopefully this should initialise the HWND grabber
	handle_reshape(800,600);	// call it once
	glutMainLoopEvent();

	// Should make this check for an error
	return 0;
}

// Here are the fonts: 
LPVOID glutFonts[7] = { 
    GLUT_BITMAP_9_BY_15, 
    GLUT_BITMAP_8_BY_13, 
    GLUT_BITMAP_TIMES_ROMAN_10, 
    GLUT_BITMAP_TIMES_ROMAN_24, 
    GLUT_BITMAP_HELVETICA_10, 
    GLUT_BITMAP_HELVETICA_12, 
    GLUT_BITMAP_HELVETICA_18 
};

void glutPrint( int x, int y, const char *string, int font, float r, float g, float b )
{
	//glutstart2d();

	glColor3f (r,g,b);
	glRasterPos2f(x, y);

	for (int i = 0; i<strlen(string); ++i)
		glutBitmapCharacter(glutFonts[font], string[i]);

	//glutend2d();
}

void glutstart2d()
{
	//Assume we are in MODEL_VIEW already
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity();

	GLint viewport [4];
	glGetIntegerv (GL_VIEWPORT, viewport);
	gluOrtho2D (0,viewport[2], viewport[3], 0);
	
	glDepthFunc (GL_ALWAYS);
}

void glutend2d()
{
	glDepthFunc (GL_LESS);
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix ();
}

void glut2drect(int x, int y, int w, int h, float r, float g, float b)
{
	glColor3f(r,g,b);
	glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x+w,y);
		glVertex2i(x+w,y+h);
		glVertex2i(x,y+h);
	glEnd();
}

void glut2dtexture(int x, int y, int w, int h, GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(x+w,y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(x+w,y+h);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(x,y+h);
	glEnd();
}

void glutInitScreen()
{
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
}
