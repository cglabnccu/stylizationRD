#include "gui.h"

BEGIN_EVENT_TABLE(SimpleDrawPanel, wxPanel)
	EVT_PAINT(SimpleDrawPanel::paintEvent)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(BasicDrawPane, wxPanel)
	EVT_PAINT(BasicDrawPane::paintEvent)
	EVT_MOTION(BasicDrawPane::MouseMove)
	EVT_LEFT_DOWN(BasicDrawPane::MouseLDown)
	EVT_LEFT_UP(BasicDrawPane::MouseLUp)
END_EVENT_TABLE()

// Pattern Picker Window
BEGIN_EVENT_TABLE(MyPatternPicker, wxFrame)
	EVT_BUTTON(BUTTON_Select, MyPatternPicker::OnSelect)
	EVT_SLIDER(SLIDER_S_PICKER, MyPatternPicker::OnSliderS)
END_EVENT_TABLE()

// Pattern Picker region
BEGIN_EVENT_TABLE(Picker, wxPanel)
	EVT_LEFT_DOWN(Picker::MouseLDown)
	EVT_PAINT(Picker::paintEvent)
END_EVENT_TABLE()



wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	// Menu - File
    EVT_MENU(ID_ONOPENSRC, MyFrame::OnOpenSrc)
    EVT_MENU(ID_ONOPENVFB, MyFrame::OnOpenVfb)
	EVT_MENU(ID_ONOPENETF, MyFrame::OnOpenETF)
	EVT_MENU(ID_ONOPENTEX, MyFrame::OnOpenTex)
	EVT_MENU(ID_ONOPENCONTOLIMG, MyFrame::OnOpenControlImg)
	EVT_MENU(ID_ONSAVE, MyFrame::OnSaveResult)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)

	// Menu - Tool
	EVT_MENU(ID_ONEdge2AddA, MyFrame::OnEdge2AddA)
	EVT_MENU(ID_ONEdge2AddB, MyFrame::OnEdge2AddB)
	EVT_MENU(ID_ONMask2AddA, MyFrame::OnMask2AddA)
	EVT_MENU(ID_ONMask2AddB, MyFrame::OnMask2AddB)
	EVT_MENU(ID_ONCLAHE, MyFrame::OnCLAHE)
	EVT_MENU(ID_ONHISTOGRAM, MyFrame::OnHISTOGRAM)
	EVT_MENU(ID_ONSIZEMASK, MyFrame::OnSIZEMASK)
	EVT_MENU(ID_ONOPEN_MASK, MyFrame::OnOpenMask)
	EVT_MENU(ID_ONOPEN_MASK_S, MyFrame::OnOpenMaskS)
	EVT_MENU(ID_ONOPEN_PATTERN_PICKER, MyFrame::OnOpenPatternPicker)

	// Menu - Help
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
	EVT_MENU(wxID_TOGGLE_LOG, MyFrame::OnToggleLog)

	// Toolbar
	EVT_BUTTON(BUTTON_Start, MyFrame::OnStart)
	EVT_BUTTON(BUTTON_Fill, MyFrame::OnFill)
	EVT_BUTTON(BUTTON_Clean, MyFrame::OnClean)
	EVT_COMBOBOX(COMBOBOX_Processing, MyFrame::OnProcessingBox)
	EVT_COMBOBOX(COMBOBOX_Controlling, MyFrame::OnControllingBox)

	// Control Panel
	EVT_SLIDER(SLIDER_BRUSH_SIZE, MyFrame::OnSliderBrushSize)
	EVT_SLIDER(SLIDER_AddA, MyFrame::OnSliderAddA)
	EVT_SLIDER(SLIDER_AddB, MyFrame::OnSliderAddB)

	EVT_SLIDER(SLIDER_S, MyFrame::OnSliderS)
	EVT_SLIDER(SLIDER_F, MyFrame::OnSliderF)
	EVT_SLIDER(SLIDER_K, MyFrame::OnSliderK)
	EVT_SLIDER(SLIDER_L, MyFrame::OnSliderL)
	EVT_SLIDER(SLIDER_Theta0, MyFrame::OnSliderTheta0)
	EVT_CHECKBOX(CHECKBOX_MODIFY_FUNCTION, MyFrame::OnCheckboxModifyToggle)
	EVT_SLIDER(SLIDER_MINDEGREE, MyFrame::OnSliderMinDegree)
	EVT_SLIDER(SLIDER_MAXDEGREE, MyFrame::OnSliderMaxDegree)
	EVT_CHECKBOX(CHECKBOX_SEGMENTATION, MyFrame::OnCheckboxSegmentation)
	EVT_CHOICE(COMBOBOX_Region, MyFrame::OnSegmentationBox)
	EVT_CHECKBOX(CHECKBOX_DISPLAY_REGION, MyFrame::OnCheckboxDisplayRegion)

	EVT_SLIDER(SLIDER_Alpha, MyFrame::OnSliderAlpha)
	EVT_SLIDER(SLIDER_Beta, MyFrame::OnSliderBeta)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);
