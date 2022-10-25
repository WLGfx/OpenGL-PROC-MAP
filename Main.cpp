
// Dark GDK - The Game Creators - www.thegamecreators.com

// the wizard has created a very simple project that uses Dark GDK
// it contains the basic code for a GDK application

// whenever using Dark GDK you must ensure you include the header file
//#include "DarkGDK.h"
#include "math.h"
#include <vector>
#include "gl/glut.h"

// You're not supposed to use this outside of a function but what the hell
using namespace std;

// include the main classes
#include "_OGL.h"
#include "PROC_MAP.h"
#include "LEVEL_ED.h"

// DarkGDK globstruct so I can grab hInstance and hWnd for the win32api dialog
//#include "globstruct.h"



// free flight camera controls
void control_camera(int cam_spd, EGL_CAM *cam)
{
	if ( glut_keys[GLUT_KEY_LEFT] )		cam->StrafeRight( -cam_spd ); //dbMoveCameraLeft( 0, cam_spd );
	if ( glut_keys[GLUT_KEY_RIGHT] )	cam->StrafeRight( cam_spd ); //dbMoveCameraRight( 0, cam_spd );
	if ( glut_keys[GLUT_KEY_UP] )		cam->MoveForward( cam_spd ); //dbMoveCamera( cam_spd );
	if ( glut_keys[GLUT_KEY_DOWN] )		cam->MoveForward( -cam_spd ); //dbMoveCamera( -cam_spd );

	// if left mouse button is held then control the lookview
	if ( glut_mousebutton == 1 )
	{
		float x = glut_MouseMoveX() / 4.0f;
		float y = glut_MouseMoveY() / 4.0f;
		cam->RotateY( x );
		cam->RotateX( y );
		//dbYRotateCamera( dbWrapValue( dbCameraAngleY() + x ) );
		//dbXRotateCamera( dbWrapValue( dbCameraAngleX() + y ) );
	}
}

// the main entry point for the application is this function

// Dark GDK's globstruct (I need the hInstance and hWnd from this)
//extern GlobStruct* g_pGlob;

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hPI, LPTSTR lpCmdLn, int nCmdShw)
{
	_OGL main;
	EGL_CAM cam;

	glut_hinstance = hI;
	main.init((int*)lpCmdLn, nCmdShw, 800, 600);

	// the main level editor class setup
	LEVEL_ED ed;
	// pass from globstruct to my dialog controls
	ed.set_win(glut_hinstance, glut_hwnd, &cam);	// set windows stuff

	do {
		// draw the 2d map then view it in 3d
		ed.view_map();
		// exit is dialog says so or main window is closed
	} while ( glut_mainwindow == 0 && ed.open_dialog() == 0 );

	// return back to windows
	return true;
}