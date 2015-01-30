#include "gui.h"

BEGIN_EVENT_TABLE(BasicDrawPane, wxPanel)
	EVT_PAINT(BasicDrawPane::paintEvent)
	EVT_MOTION(BasicDrawPane::MouseMove)
	EVT_LEFT_DOWN(BasicDrawPane::MouseLDown)
	EVT_LEFT_UP(BasicDrawPane::MouseLUp)
END_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	// Menu - File
    EVT_MENU(ID_ONOPENSRC,   MyFrame::OnOpenSrc)
    EVT_MENU(ID_ONOPENVFB,   MyFrame::OnOpenVfb)
    EVT_MENU(ID_ONOPENETF,   MyFrame::OnOpenETF)
    EVT_MENU(ID_ONOPENTEX,   MyFrame::OnOpenTex)
    EVT_MENU(ID_ONSAVE,   MyFrame::OnSaveResult)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)

	// Menu - Tool
	EVT_MENU(ID_ONEdge2AddA, MyFrame::OnEdge2AddA)
	EVT_MENU(ID_ONEdge2AddB, MyFrame::OnEdge2AddB)
	EVT_MENU(ID_ONOPEN_MASK, MyFrame::OnOpenMask)
	EVT_MENU(ID_ONOPEN_MASK_S, MyFrame::OnOpenMaskS)

	// Menu - Help
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)

	// Control Panel
	EVT_BUTTON(BUTTON_Start, MyFrame::OnStart)
	EVT_BUTTON(BUTTON_Fill, MyFrame::OnFill)
	EVT_BUTTON(BUTTON_Clean, MyFrame::OnClean)
	EVT_COMBOBOX(COMBOBOX_Processing, MyFrame::OnProcessingBox)
	EVT_COMBOBOX(COMBOBOX_Controlling, MyFrame::OnControllingBox)
	EVT_SLIDER ( SLIDER_S, MyFrame::OnSliderS )
	EVT_SLIDER ( SLIDER_F, MyFrame::OnSliderF )
	EVT_SLIDER ( SLIDER_K, MyFrame::OnSliderK )
	EVT_SLIDER ( SLIDER_L, MyFrame::OnSliderL )
	EVT_SLIDER ( SLIDER_Theta0, MyFrame::OnSliderTheta0 )
	EVT_SLIDER ( SLIDER_AddA, MyFrame::OnSliderAddA )
	EVT_SLIDER ( SLIDER_AddB, MyFrame::OnSliderAddB )
	EVT_SLIDER ( SLIDER_Alpha, MyFrame::OnSliderAlpha )
	EVT_SLIDER ( SLIDER_Beta, MyFrame::OnSliderBeta )
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);
