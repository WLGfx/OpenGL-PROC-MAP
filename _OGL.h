#pragma once

#include <gl\freeglut.h>		// Need to include it here because the GL* types are required
#include <windows.h>

extern bool		glut_keys[256];
extern int		glut_mousex, glut_mousey;
extern int		glut_mousebutton;
extern int		glut_mousemovex, glut_mousemovey;

extern int		glut_MouseMoveX();
extern int		glut_MouseMoveY();

extern HWND		glut_hwnd;
extern HINSTANCE glut_hinstance;
extern bool		glut_mainwindow;

extern void		glutInitScreen();

extern void		glutstart2d();
extern void		glutend2d();
extern void		glut2drect(int x, int y, int w, int h, float r, float g, float b);
extern void		glutPrint( int x, int y, const char *string, int font, float r, float g, float b );
extern void		glut2dtexture(int x, int y, int w, int h, GLuint texture);

#define PI 3.1415926535897932384626433832795f
#define PIdiv180 (PI/180.0f)

//
// HELPER MACROS
//

// helper functions to convert from radians to degrees (GDK uses degrees)
inline float Rad2Deg (float Angle) {
  static float _degratio = 180.0f / PI;
  return Angle * _degratio;
}
inline float Deg2Rad (float Deg) {
	static float _radratio = PI / 180.0f;
	return Deg * _radratio;
}


//
// CAMERA
//

/////////////////////////////////
//Note: All angles in degrees  //
/////////////////////////////////

struct SF3dVector  //Float 3d-vect, normally used
{
	GLfloat x,y,z;
};
struct SF2dVector
{
	GLfloat x,y;
};
SF3dVector F3dVector ( GLfloat x, GLfloat y, GLfloat z );

class EGL_CAM
{
private:
	
	SF3dVector ViewDir;
	SF3dVector RightVector;	
	SF3dVector UpVector;
	SF3dVector Position;

	GLfloat RotatedX, RotatedY, RotatedZ;	
	
public:
	EGL_CAM();				//inits the values (Position: (0|0|0) Target: (0|0|-1) )

	void Render ( void );	//executes some glRotates and a glTranslate command
							//Note: You should call glLoadIdentity before using Render

	void Move ( SF3dVector Direction );
	void Place ( SF3dVector Pos );
	void Place ( float x, float y, float z );
	void RotateX ( GLfloat Angle );
	void RotateY ( GLfloat Angle );
	void RotateZ ( GLfloat Angle );
	void Angle ( float x, float y, float z );

	void MoveForward ( GLfloat Distance );
	void MoveUpward ( GLfloat Distance );
	void StrafeRight ( GLfloat Distance );

	void GetPosition(GLfloat *x, GLfloat *y, GLfloat *z);
};

//
// MAIN CLASS HANDLER
//

class _OGL
{
public:
	int		init(int* cmdl, int cmds, int wid, int hgt);

	EGL_CAM* cam;

	_OGL(void);
	~_OGL(void);
};

