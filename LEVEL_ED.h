#pragma once

class LEVEL_ED
{
public:

	//
	// all these are UPPER CASE in the dialog and
	// prefixed with IDC_EDIT_LS_
	//
	// main dialog is IDD_LEVEL_SETUP
	//

	short	segments;
	short	seed;

	short	len_min;
	short	len_max;
	
	short	ang_min;
	short	ang_max;
	
	short	seg_min;
	short	seg_max;
	
	short	ced_min;
	short	ced_max;

	short	cln_min;
	short	cln_max;

	short	map_hgt;
	short	tess;

private:
	PROC_MAP*	pm;
	short		zoom;
	EGL_CAM*	cam;

	// need for the dialog functions
	HINSTANCE	hInstance;
	HWND		hWnd;

public:
	LEVEL_ED(void);
	~LEVEL_ED(void);

	int		open_dialog();							// activate modal dialog
	void	view_map();								// draw 2d then view in 3d
	void	control_camera(int cam_spd, EGL_CAM* cam);	// camera controls for free flight

	void	set_win(HINSTANCE hInstance, HWND hWnd, EGL_CAM *cam);
};
