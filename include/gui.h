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
	BasicDrawPane(wxFrame* parent,Size);
	void paintEvent(wxPaintEvent& evt);
	void paintNow(bool);
	void render( wxDC& dc,bool );
	void MouseMove(wxMouseEvent &event);
	void MouseLDown(wxMouseEvent &event);
	void MouseLUp(wxMouseEvent &event);
	DECLARE_EVENT_TABLE()
private:
	bool activateDraw;
};

class MyFrame: public wxFrame
{
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	void addlog(wxString info, wxColour& color);
private:
	BasicDrawPane *drawPane;
	wxTextCtrl *log; // Show the log
	wxButton *start;
	wxButton *fill;
	wxButton *clean;
	wxComboBox *processingBox;
	wxComboBox *controllingBox;
	wxSlider *slider_s;
	wxStaticText *slider_s_t;
	wxSlider *slider_f;
	wxStaticText *slider_f_t;
	wxSlider *slider_k;
	wxStaticText *slider_k_t;
	wxSlider *slider_l;
	wxStaticText *slider_l_t;
	wxSlider *slider_theta0;
	wxStaticText *slider_theta0_t;
	wxSlider *slider_addA;
	wxStaticText *slider_addA_t;
	wxSlider *slider_addB;
	wxStaticText *slider_addB_t;
	wxSlider *slider_alpha;
	wxStaticText *slider_alpha_t;
	wxSlider *slider_beta;
	wxStaticText *slider_beta_t;
	bool render_loop_on;
	void activateRenderLoop(bool on);
	void OnStart(wxCommandEvent& event);
	void OnFill(wxCommandEvent& event);
	void OnClean(wxCommandEvent& event);
	void OnProcessingBox(wxCommandEvent& event);
	void OnControllingBox(wxCommandEvent& event);
	void OnSliderS(wxCommandEvent& event);
	void OnSliderF(wxCommandEvent& event);
	void OnSliderK(wxCommandEvent& event);
	void OnSliderL(wxCommandEvent& event);
	void OnSliderTheta0(wxCommandEvent& event);
	void OnSliderAddA(wxCommandEvent& event);
	void OnSliderAddB(wxCommandEvent& event);
	void OnSliderAlpha(wxCommandEvent& event);
	void OnSliderBeta(wxCommandEvent& event);
	void OnOpenSrc(wxCommandEvent& event);
	void OnOpenVfb(wxCommandEvent& event);
	void OnOpenETF(wxCommandEvent& event);
	void OnOpenTex(wxCommandEvent& event);
	void OnSaveResult(wxCommandEvent& event);
	void OnEdge2AddA(wxCommandEvent& event);
	void OnEdge2AddB(wxCommandEvent& event);
	void OnOpenMask(wxCommandEvent& event);
	void OnOpenMaskS(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void onIdle(wxIdleEvent& evt);
	wxDECLARE_EVENT_TABLE();
};


enum
{
	ID_ONOPENSRC = 1,
	ID_ONOPENVFB,
	ID_ONOPENETF,
	ID_ONOPENTEX,
	ID_ONSAVE,

	ID_ONEdge2AddA,
	ID_ONEdge2AddB,
	ID_ONOPEN_MASK,
	ID_ONOPEN_MASK_S,

	ID_WXEDIT1,
	BUTTON_Start,
	BUTTON_Fill,
	BUTTON_Clean,
	COMBOBOX_Processing,
	COMBOBOX_Controlling,
	SLIDER_S,
	SLIDER_S_T,
	SLIDER_F,
	SLIDER_F_T,
	SLIDER_K,
	SLIDER_K_T,
	SLIDER_L,
	SLIDER_L_T,
	SLIDER_Theta0,
	SLIDER_Theta0_T,
	SLIDER_AddA,
	SLIDER_AddA_T,
	SLIDER_AddB,
	SLIDER_AddB_T,
	
	SLIDER_Alpha,
	SLIDER_Alpha_T,
	SLIDER_Beta,
	SLIDER_Beta_T
};