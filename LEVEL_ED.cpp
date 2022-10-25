#include "windows.h"
#include <vector>

using namespace std;

#include "_OGL.h"
#include "PROC_MAP.h"
#include "LEVEL_ED.h"
#include "resource.h"
#include "time.h"


LEVEL_ED::LEVEL_ED(void)
{
	this->segments	= 50;
	this->seed		= 0;

	this->len_min	= 35;
	this->len_max	= 45;
	
	this->ang_min	= 60;
	this->ang_max	= 120;

	this->seg_min	= 6;
	this->seg_max	= 16;

	this->ced_min	= 7;
	this->ced_max	= 25;

	this->cln_min	= 40;
	this->cln_max	= 100;

	this->map_hgt	= 200;
	this->tess		= 1;

	this->pm		= NULL;
	this->zoom		= 8;
}

LEVEL_ED::~LEVEL_ED(void)
{
	if ( pm )
		delete this->pm;
}

void LEVEL_ED::view_map()
{
	// check if map already exists
	if ( pm )
		delete this->pm;

	// setup new map
	pm = new PROC_MAP;
	cam = new EGL_CAM;

	pm->set_map( segments, seed, map_hgt );
	pm->set_corridor( len_min, len_max, ang_min, ang_max, seg_min, seg_max );
	pm->set_room( cln_min, cln_max, ced_min, ced_max );

	pm->tess = this->tess;

	// Set the start of the timer

	clock_t time = clock();

	pm->generate( zoom );

	// old stuff
	//pm->tessellate( tess );	// level of tessellation
	//pm->create_textures(6);	// generate textures before generating the mesh
	//pm->gl_setup_vbo();		// generate the mesh display lists

	time = clock() - time;

	cam->Place(0,0,1400);
	cam->Angle(0,0,0);

	int spd = 5;

	GLuint gl2dmap = glGenLists(1);
	glNewList( gl2dmap, GL_COMPILE );
	pm->gl_draw2dmap();
	glEndList();

	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

	// some stats to print on the screen
	char	text[128];
	int		vcount = pm->verts_count();
	int		ecount = pm->edges_count();
	int		fcount = pm->faces_count();
	int		esize = ( vcount * sizeof(_VERT) );
			esize = ( esize + ( fcount * sizeof(_FACE) ) ) / 1024 + 1;

	// draw the 2d map
	do {
		glutInitScreen();
		
		cam->Render();
		//pm->gl_draw2dmap();
		glCallList( gl2dmap );

		glutstart2d();
		glut2drect(2, 2, 112, 114, 0.2f, 0.2f, 0.2f);
		glutPrint( 4, 12, "Space to edit...", 4, 1.0f, 1.0f, 0.0f );

		sprintf_s(text, 128, "Verts count = %d", vcount);
		glutPrint( 4, 36, text, 4, 1.0f, 1.0f, 1.0f );

		sprintf_s(text, 128, "Edges count = %d", ecount);
		glutPrint( 4, 48, text, 4, 1.0f, 1.0f, 1.0f );

		sprintf_s(text, 128, "Faces count = %d", fcount);
		glutPrint( 4, 60, text, 4, 1.0f, 1.0f, 1.0f );

		sprintf_s(text, 128, "Size estimate = %dKb", esize);
		glutPrint( 4, 72, text, 4, 1.0f, 1.0f, 1.0f );

		glutPrint( 4, 96, "Arrow keys / + - Zoom", 4, 1.0f, 1.0f, 0.0f );

		sprintf_s( text, 128, "Milliseconds - %d", (int)time );
		glutPrint( 4, 112, text, 4, 1.0f, 1.0f, 1.0f );

		glutend2d();

		glutSwapBuffers();
		glutMainLoopEvent();

		// now handle key movement
		if ( glut_keys[GLUT_KEY_LEFT] )
			cam->StrafeRight(-spd);
		if ( glut_keys[GLUT_KEY_RIGHT] )
			cam->StrafeRight(spd);

		if ( glut_keys[GLUT_KEY_UP] )
			cam->MoveUpward(spd);
		if ( glut_keys[GLUT_KEY_DOWN] )
			cam->MoveUpward(-spd);

		if ( glut_keys['+'] )
			cam->MoveForward(spd);
		if ( glut_keys['-'] )
			cam->MoveForward(-spd);

	} while ( glut_keys[ 32 ] == false );

	glDeleteLists( gl2dmap, 1 );

	do {
		glutMainLoopEvent();
	} while ( glut_keys[ ' ' ] == true );

	pm->vbo_create();

	float cx, cy, cz;
	pm->get_edge_centre(0, &cx, &cy, &cz);

	cam->Place(cx, cy+20, cz);

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CW );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat specular[] = {1.0f, 1.0f, 1.0f , 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	do {
		glutInitScreen();
		cam->Render();

		cam->GetPosition( &position[0], &position[1], &position[2] );
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		
		/*for ( int seg = 0; seg < pm->map_size; seg++ )
		{
			glCallList( pm->seglist[seg].DL );
		}*/
		pm->vbo_drawdl();

		glutSwapBuffers();

		control_camera( 2, cam );

		glutMainLoopEvent();
	} while ( glut_keys[ ' ' ] == false );

	do {
		glutMainLoopEvent();
	} while ( glut_keys[ ' ' ] == true );

	pm->vbo_freedl();

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHT0 );
	glDisable( GL_LIGHTING );
	
	delete cam;
}

void LEVEL_ED::control_camera(int cam_spd, EGL_CAM* can)
{
	if ( glut_keys[GLUT_KEY_LEFT] )		cam->StrafeRight( -cam_spd );
	if ( glut_keys[GLUT_KEY_RIGHT] )	cam->StrafeRight(  cam_spd );
	if ( glut_keys[GLUT_KEY_UP] )		cam->MoveForward( -cam_spd );
	if ( glut_keys[GLUT_KEY_DOWN] )		cam->MoveForward(  cam_spd );

	// if left mouse button is held then control the lookview
	if ( glut_mousebutton == 1 )
	{
		float x = glut_MouseMoveX() / 4.0f;
		float y = glut_MouseMoveY() / 4.0f;
		cam->RotateY( -x );
		cam->RotateX( -y );
	}
}

void LEVEL_ED::set_win(HINSTANCE hInstance, HWND hWnd, EGL_CAM* cam)
{
	this->hInstance = hInstance;
	this->hWnd = hWnd;
	this->cam = cam;
}

//
// DIALOG HELPER FUNCTIONS
//

static char dlg_buff[32];

int GetDlgInt(HWND hWnd,int ID)				{GetDlgItemText(hWnd,ID,dlg_buff,32);	return atoi(dlg_buff); }
float GetDlgFloat(HWND hWnd,int ID)			{GetDlgItemText(hWnd,ID,dlg_buff,32);	return (float)atof(dlg_buff); }
void SetDlgText(HWND hWnd,int ID, int val)	{sprintf_s(dlg_buff,32,"%d",val);		SetDlgItemText(hWnd,ID,dlg_buff); }
void SetDlgText(HWND hWnd,int ID,float val)	{sprintf_s(dlg_buff,32,"%f",val);		SetDlgItemText(hWnd,ID,dlg_buff); }

static LEVEL_ED* lemap;		// static to make it only accessible to this .cpp file

static LRESULT CALLBACK dlg_level_editor(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		{
			SetDlgText(hWnd, IDC_EDIT_LS_SEGMENTS,	lemap->segments);
			SetDlgText(hWnd, IDC_EDIT_LS_SEED,		lemap->seed);
			SetDlgText(hWnd, IDC_EDIT_LS_LEN_MIN,	lemap->len_min);
			SetDlgText(hWnd, IDC_EDIT_LS_LEN_MAX,	lemap->len_max);
			SetDlgText(hWnd, IDC_EDIT_LS_ANG_MIN,	lemap->ang_min);
			SetDlgText(hWnd, IDC_EDIT_LS_ANG_MAX,	lemap->ang_max);
			SetDlgText(hWnd, IDC_EDIT_LS_SEG_MIN,	lemap->seg_min);
			SetDlgText(hWnd, IDC_EDIT_LS_SEG_MAX,	lemap->seg_max);
			SetDlgText(hWnd, IDC_EDIT_LS_CED_MIN,	lemap->ced_min);
			SetDlgText(hWnd, IDC_EDIT_LS_CED_MAX,	lemap->ced_max);
			SetDlgText(hWnd, IDC_EDIT_LS_CLN_MIN,	lemap->cln_min);
			SetDlgText(hWnd, IDC_EDIT_LS_CLN_MAX,	lemap->cln_max);
			SetDlgText(hWnd, IDC_EDIT_LS_MAP_HGT,	lemap->map_hgt);
			SetDlgText(hWnd, IDC_EDIT_LS_TESS,		lemap->tess);
			return true;
		}
	case WM_COMMAND:
		{
			switch ( LOWORD(wParam) )
			{
			case IDOK:	// generate and view button
				{
					EndDialog(hWnd, 0);
					return true;
				}
			case IDC_BUTTON_QUIT:			// quit button
				{
					EndDialog(hWnd, 1);
					return true;
				}
			case IDC_EDIT_LS_SEED:
				{
					lemap->seed = GetDlgInt(hWnd, IDC_EDIT_LS_SEED);
					return true;
				}
			case IDC_EDIT_LS_SEGMENTS:
				{
					lemap->segments = GetDlgInt(hWnd, IDC_EDIT_LS_SEGMENTS);
					/*if ( lemap->segments < 25 )
					{
						lemap->segments = 25;
						SetDlgText(hWnd, IDC_EDIT_LS_SEGMENTS, lemap->segments);
					}*/
					return true;
				}
			case IDC_EDIT_LS_LEN_MIN:
				{
					lemap->len_min = GetDlgInt(hWnd, IDC_EDIT_LS_LEN_MIN);
					return true;
				}
			case IDC_EDIT_LS_LEN_MAX:
				{
					lemap->len_max = GetDlgInt(hWnd, IDC_EDIT_LS_LEN_MAX);
					return true;
				}
			case IDC_EDIT_LS_ANG_MIN:
				{
					lemap->ang_min = GetDlgInt(hWnd, IDC_EDIT_LS_ANG_MIN);
					return true;
				}
			case IDC_EDIT_LS_ANG_MAX:
				{
					lemap->ang_max = GetDlgInt(hWnd, IDC_EDIT_LS_ANG_MAX);
					return true;
				}
			case IDC_EDIT_LS_SEG_MIN:
				{
					lemap->seg_min = GetDlgInt(hWnd, IDC_EDIT_LS_SEG_MIN);
					return true;
				}
			case IDC_EDIT_LS_SEG_MAX:
				{
					lemap->seg_max = GetDlgInt(hWnd, IDC_EDIT_LS_SEG_MAX);
					return true;
				}
			case IDC_EDIT_LS_CED_MIN:
				{
					lemap->ced_min = GetDlgInt(hWnd, IDC_EDIT_LS_CED_MIN);
					return true;
				}
			case IDC_EDIT_LS_CED_MAX:
				{
					lemap->ced_max = GetDlgInt(hWnd, IDC_EDIT_LS_CED_MAX);
					return true;
				}
			case IDC_EDIT_LS_CLN_MIN:
				{
					lemap->cln_min = GetDlgInt(hWnd, IDC_EDIT_LS_CLN_MIN);
					return true;
				}
			case IDC_EDIT_LS_CLN_MAX:
				{
					lemap->cln_max = GetDlgInt(hWnd, IDC_EDIT_LS_CLN_MAX);
					return true;
				}
			case IDC_EDIT_LS_MAP_HGT:
				{
					lemap->map_hgt = GetDlgInt(hWnd, IDC_EDIT_LS_MAP_HGT);
					return true;
				}
			case IDC_EDIT_LS_TESS:
				{
					lemap->tess = GetDlgInt(hWnd, IDC_EDIT_LS_TESS);
				}
			} // EOF switch ( LOWORD(wParam) )

		}
	}
	return false;
}

int LEVEL_ED::open_dialog()
{
	lemap = this;
	int flag = DialogBoxA(hInstance, MAKEINTRESOURCE(IDD_LEVEL_SETUP), hWnd, (DLGPROC)dlg_level_editor);
	return flag;
}

//
// Extension to the PROC_MAP
//

void PROC_MAP::gl_draw2dmap()
{
	_VERT*	vlist = &verts[0];
	_EDGE*	curr_edge = &edges[0];
	int		edge_count = edges.size();	// only need to draw the floor edges
	int		edge_pos	= 0;

	float	x, y;
	int		v1, v2;

	glBegin(GL_LINES);

	while ( edge_pos < edge_count )
	{
		switch ( curr_edge->type )
		{
		case PM_CINNER:
			glColor3f(1.0f, 0.0f, 0.0f);
			break;
		case PM_COUTER:
			glColor3f(1.0f, 1.0f, 1.0f);
			break;
		case PM_RINNER:
			glColor3f(0.0f, 1.0f, 0.0f);
			break;
		case PM_ROUTER:
			glColor3f(1.0f, 1.0f, 0.0f);
			break;
		case PM_RENTRY:
			glColor3f(0.0f, 0.5f, 1.0f);
			break;
		}

		v1 = curr_edge->v1;
		v2 = curr_edge->v2;

		x = ( vlist + v1 )->x;
		y = ( vlist + v1 )->z;

		glVertex2f( x, y );

		x = ( vlist + v2 )->x;
		y = ( vlist + v2 )->z;

		glVertex2f( x, y );

		curr_edge++;
		edge_pos++;
	}

	glEnd();
}