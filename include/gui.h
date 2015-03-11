#include "reactionDiffusion.h"
#include "postProcessing.h"
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filedlg.h>
#include <wx/wfstream.h>

class MyApp: public wxApp
{
public:
	virtual bool OnInit();
};

class BasicDrawPane : public wxPanel
{

public:
	RD element;
	PP processing;
	Mat dis;
	Mat temp;
	string processingS;
	string controllingS;
	string gradientTypeS;
	int regionSelected;
	int brushSize;
	int mindegree;
	int maxdegree;
	bool customAnisotropicFunction;
	bool regionOn;
	bool displayRegion;
	BasicDrawPane(wxFrame* parent,Size);
	void Seeds(int r, bool isoffset, float ratio);
	void paintEvent(wxPaintEvent& evt);
	void paintNow(bool);
	void render( wxDC& dc,bool );
	void MouseMove(wxMouseEvent &event);
	void MouseLDown(wxMouseEvent &event);
	void MouseLUp(wxMouseEvent &event);	
	DECLARE_EVENT_TABLE()
private:
	bool activateDraw;
	Point LastMousePosition;
	Point StartMousePosition;
};

class SimpleDrawPanel : public wxPanel
{
public:
	int dmin;
	int dmax;
	SimpleDrawPanel(wxPanel* parent);
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
	void render(wxDC& dc);
	DECLARE_EVENT_TABLE()
};

class Picker : public wxPanel
{
public:
	Picker(wxFrame* parent, wxString file, wxBitmapType format);
	wxBitmap image;
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
	void render(wxDC& dc);
	void MouseLDown(wxMouseEvent &event);
	DECLARE_EVENT_TABLE()
};

class MyPatternPicker : public wxFrame
{
public:
	MyPatternPicker(wxWindow* parent, const wxString& title, const float pattern_size);
	BasicDrawPane *preview;
	wxSlider *slider_s;
	wxStaticText *slider_s_t;
	wxStaticText *slider_f_t;
	wxStaticText *slider_k_t;
	wxStaticText *slider_l_t;
	Picker *picker;
	void StartPreview();
private:
	void onIdle(wxIdleEvent& evt);
	void OnSliderS(wxCommandEvent& event);
	void OnSelect(wxCommandEvent& event);
	void MouseLDown(wxMouseEvent &event);
	void OnExit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class MyFrame: public wxFrame
{
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	MyPatternPicker *patternpicker;
	BasicDrawPane *drawPane;
	wxSlider *slider_s;
	wxSlider *slider_f;
	wxSlider *slider_k;
	wxSlider *slider_l;
	wxStaticText *slider_s_t;
	wxStaticText *slider_f_t;
	wxStaticText *slider_k_t;
	wxStaticText *slider_l_t;
	wxCheckBox *Segmentation_cb; 
	void addlog(wxString info, wxColour& color);
	void activateRenderLoop(bool on);

	bool isCLAHE;
	bool isHistogram;
	bool isSizeMask;

protected:
	bool render_loop_on;
	wxTextCtrl *log; // Show the log
	SimpleDrawPanel *degreeGUI;
	wxButton *start;
	wxButton *fill;
	wxButton *clean;
	wxComboBox *processingBox;
	wxComboBox *controllingBox;
	wxChoice *gradientType;
	wxStaticText *slider_theta0_t;
	wxStaticText *slider_brushSize_t;
	wxStaticText *slider_addA_t;
	wxStaticText *slider_addB_t;
	wxStaticText *slider_alpha_t;
	wxStaticText *slider_beta_t;
	wxStaticText *slider_mindegree_t;
	wxStaticText *slider_maxdegree_t;
	wxSlider *slider_theta0;
	wxSlider *slider_brushSize;
	wxSlider *slider_addA;
	wxSlider *slider_addB;
	wxSlider *slider_alpha;
	wxSlider *slider_beta;
	wxCheckBox *Modify_cb; // Is User Want to Use Modified Anisotropic function?
	wxSlider *slider_mindegree;
	wxSlider *slider_maxdegree;
	wxChoice *SegmentationBox;
	wxCheckBox *DisplayRegion_cb; // Is User Want to Use Modified Anisotropic function?

	void OnStart(wxCommandEvent& event);
	void OnFill(wxCommandEvent& event); 
	void OnClean(wxCommandEvent& event);

	void OnProcessingBox(wxCommandEvent& event);
	void OnControllingBox(wxCommandEvent& event);
	void OnGradientTypeBox(wxCommandEvent& event);

	void OnSliderBrushSize(wxCommandEvent& event);
	void OnSliderAddA(wxCommandEvent& event);
	void OnSliderAddB(wxCommandEvent& event);

	void OnSliderS(wxCommandEvent& event);
	void OnSliderF(wxCommandEvent& event);
	void OnSliderK(wxCommandEvent& event);
	void OnSliderL(wxCommandEvent& event);
	void OnSliderTheta0(wxCommandEvent& event);
	void OnCheckboxModifyToggle(wxCommandEvent& event);
	void OnSliderMinDegree(wxCommandEvent& event);
	void OnSliderMaxDegree(wxCommandEvent& event);
	void OnCheckboxSegmentation(wxCommandEvent& event);
	void OnSegmentationBox(wxCommandEvent& event);
	void OnCheckboxDisplayRegion(wxCommandEvent& event);

	void OnSliderAlpha(wxCommandEvent& event);
	void OnSliderBeta(wxCommandEvent& event);
	void OnOpenSrc(wxCommandEvent& event);
	void OnOpenVfb(wxCommandEvent& event);
	void OnOpenETF(wxCommandEvent& event);
	void OnOpenTex(wxCommandEvent& event);
	void OnOpenControlImg(wxCommandEvent& event);
	void OnOpenSizeImg(wxCommandEvent& event);
	void OnSaveResult(wxCommandEvent& event);

	void OnEdge2AddA(wxCommandEvent& event);
	void OnEdge2AddB(wxCommandEvent& event);
	void OnMask2AddA(wxCommandEvent& event);
	void OnMask2AddB(wxCommandEvent& event);
	void OnCLAHE(wxCommandEvent& event);
	void OnHISTOGRAM(wxCommandEvent& event);
	void OnSIZEMASK(wxCommandEvent& event);

	void OnOpenMask(wxCommandEvent& event);
	void OnOpenMaskS(wxCommandEvent& event);
	void OnOpenPatternPicker(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnToggleLog(wxCommandEvent& event);
	void onIdle(wxIdleEvent& evt);
	wxDECLARE_EVENT_TABLE();
};

enum
{
	ID_ONOPENSRC = 1,
	ID_ONOPENVFB,
	ID_ONOPENETF,
	ID_ONOPENTEX,
	ID_ONOPENCONTOLIMG,
	ID_ONOPENSIZEIMG,
	ID_ONSAVE,
	wxID_TOGGLE_LOG,

	ID_ONEdge2AddA,
	ID_ONEdge2AddB,
	ID_ONMask2AddA,
	ID_ONMask2AddB,
	ID_ONCLAHE,
	ID_ONHISTOGRAM,
	ID_ONSIZEMASK,
	ID_ONOPEN_MASK,
	ID_ONOPEN_MASK_S,
	ID_ONOPEN_PATTERN_PICKER,

	ID_WXEDIT1,
	BUTTON_Start,
	BUTTON_Fill,
	BUTTON_Clean,
	COMBOBOX_Processing,
	COMBOBOX_Controlling,

	SLIDER_S_PICKER,
	SLIDER_S,
	SLIDER_S_T,
	COMBOBOX_GRADIENT_TYPE,
	SLIDER_F,
	SLIDER_F_T,
	SLIDER_K,
	SLIDER_K_T,
	SLIDER_L,
	SLIDER_L_T,
	SLIDER_Theta0,
	SLIDER_Theta0_T,
	SLIDER_BRUSH_SIZE,
	SLIDER_BRUSH_SIZE_T,
	SLIDER_AddA,
	SLIDER_AddA_T,
	SLIDER_AddB,
	SLIDER_AddB_T,
	CHECKBOX_MODIFY_FUNCTION,
	SLIDER_MINDEGREE,
	SLIDER_MINDEGREE_T,
	SLIDER_MAXDEGREE,
	SLIDER_MAXDEGREE_T,
	CHECKBOX_SEGMENTATION,
	COMBOBOX_Region,
	CHECKBOX_DISPLAY_REGION,

	SLIDER_Alpha,
	SLIDER_Alpha_T,
	SLIDER_Beta,
	SLIDER_Beta_T,

	BUTTON_Select //pattern picker
};