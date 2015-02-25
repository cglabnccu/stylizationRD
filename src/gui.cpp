#include "gui.h"
#include <ctime>

bool MyApp::OnInit()
{
	MyFrame *frame = new MyFrame("CRD", wxPoint(50, 50), wxSize(800, 730));
	frame->Show(true);

	return true;
}

#pragma region MyPatternPicker
MyPatternPicker::MyPatternPicker(wxWindow* parent, const wxString & title)
	: wxFrame(parent, -1, title, wxDefaultPosition, wxSize(600, 540))
{
	this->SetSizeHints(wxSize(600, 540), wxSize(600, 540));

	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* right = new wxBoxSizer(wxVERTICAL);

	char cCurrentPath[FILENAME_MAX];
	getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = "";
	path.append(cCurrentPath).append("\\l_all.jpg");

	//left 
	picker = new Picker(this, path, wxBITMAP_TYPE_JPEG);
	left->Add(picker, 1, wxEXPAND);

	//right
	wxStaticText* s = new wxStaticText(this, NULL, "Preview", wxDefaultPosition, wxDefaultSize, 0);
	right->Add(s, 0, wxEXPAND);
	preview = new BasicDrawPane(this, Size(100, 100));
	right->Add(preview, 2, wxEXPAND);

	wxBoxSizer* right2 = new wxBoxSizer(wxVERTICAL);

	wxString ss;
	ss.Printf("Size : %.3f", preview->element.s);
	slider_s_t = new wxStaticText(this, wxID_ANY, ss, wxDefaultPosition, wxDefaultSize, 0);
	right2->Add(slider_s_t, 0, wxEXPAND | wxLEFT, 10);
	slider_s = new wxSlider(this, SLIDER_S_PICKER, int(preview->element.s * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	right2->Add(slider_s, 0, wxEXPAND | wxLEFT, 10);

	ss.Printf("F : %.4f", preview->element.f);
	slider_f_t = new wxStaticText(this, wxID_ANY, ss, wxDefaultPosition, wxDefaultSize, 0);
	right2->Add(slider_f_t, 0, wxEXPAND | wxLEFT, 10);

	ss.Printf("k : %.4f", preview->element.k);
	slider_k_t = new wxStaticText(this, wxID_ANY, ss, wxDefaultPosition, wxDefaultSize, 0);
	right2->Add(slider_k_t, 0, wxEXPAND | wxLEFT, 10);

	ss.Printf("l : %d", preview->element.l);
	slider_l_t = new wxStaticText(this, wxID_ANY, ss, wxDefaultPosition, wxDefaultSize, 0);
	right2->Add(slider_l_t, 0, wxEXPAND | wxLEFT, 10);

	right->Add(right2, 5, wxEXPAND);

	wxButton *select = new wxButton(this, BUTTON_Select, _T("SELECT!"), wxDefaultPosition, wxDefaultSize, 0);
	right->Add(select, 2, wxEXPAND);


	sizer->Add(left, 5, wxEXPAND);
	sizer->Add(right, 1, wxEXPAND);
	SetSizer(sizer);
	Centre();
}
void MyPatternPicker::OnSliderS(wxCommandEvent& event)
{
	wxString s;
	preview->element.s = slider_s->GetValue() / 1000.0;
	s.Printf("Size : %.3f", preview->element.s);
	slider_s_t->SetLabel(s);
}
void MyPatternPicker::OnSelect(wxCommandEvent& event)
{
	if (!((MyFrame *)GetParent())->Segmentation_cb->GetValue())
	{
		((MyFrame *)GetParent())->drawPane->element.l = preview->element.l;
		((MyFrame *)GetParent())->drawPane->element.f = preview->element.f;
		((MyFrame *)GetParent())->drawPane->element.k = preview->element.k;
		((MyFrame *)GetParent())->drawPane->element.s = preview->element.s;

		wxString s;
		s.Printf("Size : %.3f", ((MyFrame *)GetParent())->drawPane->element.s);
		((MyFrame *)GetParent())->slider_s_t->SetLabel(s);
		s.Printf("k : %.4f", ((MyFrame *)GetParent())->drawPane->element.k);
		((MyFrame *)GetParent())->slider_k_t->SetLabel(s);
		s.Printf("f : %.4f", ((MyFrame *)GetParent())->drawPane->element.f);
		((MyFrame *)GetParent())->slider_f_t->SetLabel(s);
		s.Printf("l : %d", ((MyFrame *)GetParent())->drawPane->element.l);
		((MyFrame *)GetParent())->slider_l_t->SetLabel(s);

	}

	//segmentation On
	else
	{
		int sr = ((MyFrame *)GetParent())->drawPane->regionSelected - 1;
		((MyFrame *)GetParent())->drawPane->element.segmentation[sr].l = preview->element.l;
		((MyFrame *)GetParent())->drawPane->element.segmentation[sr].F = preview->element.f;
		((MyFrame *)GetParent())->drawPane->element.segmentation[sr].k = preview->element.k;
		((MyFrame *)GetParent())->drawPane->element.segmentation[sr].size = preview->element.s;

		wxString s;
		s.Printf("Size : %.3f", ((MyFrame *)GetParent())->drawPane->element.segmentation[sr].size);
		((MyFrame *)GetParent())->slider_s_t->SetLabel(s);
		s.Printf("k : %.4f", ((MyFrame *)GetParent())->drawPane->element.segmentation[sr].k);
		((MyFrame *)GetParent())->slider_k_t->SetLabel(s);
		s.Printf("f : %.4f", ((MyFrame *)GetParent())->drawPane->element.segmentation[sr].F);
		((MyFrame *)GetParent())->slider_f_t->SetLabel(s);
		s.Printf("l : %d", ((MyFrame *)GetParent())->drawPane->element.segmentation[sr].l);
		((MyFrame *)GetParent())->slider_l_t->SetLabel(s);


		((MyFrame *)GetParent())->drawPane->element.UpdateControlMask();
	}

	((MyFrame *)GetParent())->slider_k->SetValue(int((preview->element.k - 0.03) / 0.04 * 1000));
	((MyFrame *)GetParent())->slider_f->SetValue(int((preview->element.f / 0.06) * 1000));
	((MyFrame *)GetParent())->slider_l->SetValue(preview->element.l);
	((MyFrame *)GetParent())->slider_s->SetValue(preview->element.s * 1000);

	((MyFrame *)GetParent())->activateRenderLoop(true);
	Close(true);
}
void MyPatternPicker::StartPreview()
{
	char cCurrentPath[FILENAME_MAX];
	getcwd(cCurrentPath, sizeof(cCurrentPath));
	string path = "";
	path.append(cCurrentPath).append("\\x.vfb");
	//path.append(cCurrentPath).append("\\512source.vfb");

	preview->element.ReadFlow(path);
	preview->processingS = "Thresholding";
	preview->processing.beta = 0.5;
	preview->processing.alpha = 0.5;
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MyPatternPicker::onIdle));
}
void MyPatternPicker::onIdle(wxIdleEvent& evt)
{
	preview->paintNow(true);
	evt.RequestMore(); // render continuously, not only once on idle
}
#pragma endregion 

#pragma region Picker
Picker::Picker(wxFrame* parent, wxString file, wxBitmapType format) :
wxPanel(parent)
{
	wxInitAllImageHandlers();
	image.LoadFile(file, format);
}
void Picker::MouseLDown(wxMouseEvent &event)
{

	((MyPatternPicker *)GetParent())->preview->element.k = 0.056 + 0.0000238*event.m_x;
	((MyPatternPicker *)GetParent())->preview->element.f = 0.0375;
	((MyPatternPicker *)GetParent())->preview->element.l = event.m_y / 70;
	((MyPatternPicker *)GetParent())->preview->element.s = 0.7;

	// clean preview
	*((MyPatternPicker *)GetParent())->preview->element.c_A = Mat::ones(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);
	*((MyPatternPicker *)GetParent())->preview->element.c_B = Mat::zeros(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);
	*((MyPatternPicker *)GetParent())->preview->element.p_A = Mat::ones(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);
	*((MyPatternPicker *)GetParent())->preview->element.p_B = Mat::zeros(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);
	((MyPatternPicker *)GetParent())->preview->element.Addition_B = Mat::zeros(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);
	((MyPatternPicker *)GetParent())->preview->element.Addition_A = Mat::zeros(((MyPatternPicker *)GetParent())->preview->element.Mask.size(), CV_32F);

	// fill preview with ink
	((MyPatternPicker *)GetParent())->preview->Seeds(25, true, .5);

	wxString ss;

	ss.Printf("F : %.4f", ((MyPatternPicker *)GetParent())->preview->element.f);
	((MyPatternPicker *)GetParent())->slider_f_t->SetLabel(ss);

	ss.Printf("k : %.4f", ((MyPatternPicker *)GetParent())->preview->element.k);
	((MyPatternPicker *)GetParent())->slider_k_t->SetLabel(ss);

	ss.Printf("l : %d", ((MyPatternPicker *)GetParent())->preview->element.l);
	((MyPatternPicker *)GetParent())->slider_l_t->SetLabel(ss);

}
void Picker::paintEvent(wxPaintEvent & evt)
{
	// depending on your system you may need to look at double-buffered dcs
	wxPaintDC dc(this);
	render(dc);
}
void Picker::paintNow()
{
	// depending on your system you may need to look at double-buffered dcs
	wxClientDC dc(this);
	render(dc);
}
void Picker::render(wxDC&  dc)
{
	dc.DrawBitmap(image, 0, 0, false);
}
#pragma endregion 

#pragma region Custome Function Preview
SimpleDrawPanel::SimpleDrawPanel(wxPanel* parent) :
wxPanel(parent)
{
	dmin = 0;
	dmax = 0;
}
void SimpleDrawPanel::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
void SimpleDrawPanel::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}
void SimpleDrawPanel::render(wxDC&  dc)
{
	dc.Clear();
	dc.SetPen(wxPen(wxColor(0, 0, 0), 1)); // 1-pixels-thick black outline
	dc.DrawCircle(wxPoint(37, 40), 25 /* radius */);
	dc.SetPen(wxPen(wxColor(255, 0, 0), 5)); // 5-pixels-thick red outline
	if (dmin < dmax)
	{
		dc.DrawEllipticArc(12, 15, 50, 50, dmin, dmax);
	}
	else if (dmin > dmax)
	{
		dc.DrawEllipticArc(12, 15, 50, 50, dmin, 360);
		if (dmax != 0) dc.DrawEllipticArc(12, 15, 50, 50, 0, dmax);
	}
	//dc.SetPen(wxPen(wxColor(0, 0, 255), 5)); // 5-pixels-thick red outline
	//if (dmin < dmax){
	//	dc.DrawEllipticArc(12, 15, 50, 50, dmax + 2 * ((270 - dmax) % 90), dmax + 2 * ((270 - dmax) % 90)+(dmax-dmin));
	//}
	//else if (dmin > dmax){
	//	dc.DrawEllipticArc(12, 15, 50, 50, dmax + 2 * ((270 - dmax) % 90), 360);
	//	if (dmax != 0) dc.DrawEllipticArc(12, 15, 50, 50, 0,(dmax + 2 * ((270 - dmax) % 90)+(dmax-dmin)));
	//}
}
#pragma endregion

#pragma region MyFrame
MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
#pragma region MenuBar
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_ONOPENSRC, "&Open SrcImg\tCtrl-O", "Open source image");
	menuFile->Append(ID_ONOPENVFB, "&Open Flow\tCtrl-F", "Open Flowfield file");
	menuFile->Append(ID_ONOPENETF, "&Open ETF\tCtrl-E", "Open image file");
	menuFile->Append(ID_ONOPENTEX, "&Open Texture\tCtrl-T", "Open texture file");
	menuFile->Append(ID_ONOPENCONTOLIMG, "&Open Control Image\tCtrl-C", "Open Control Image file");
	menuFile->Append(ID_ONSAVE, "&Save\tCtrl-E", "Save Result");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu *menuTool = new wxMenu;
	menuTool->Append(ID_ONEdge2AddA, "&Edge2AddA\tCtrl-A", "add Edge to addition A");
	menuTool->Append(ID_ONEdge2AddB, "&Edge2AddB\tCtrl-B", "add Edge to addition B");
	menuTool->Append(ID_ONMask2AddA, "&Mask2AddA", "add Mask to addition A");
	menuTool->Append(ID_ONMask2AddB, "&Mask2AddB", "add Mask to addition B");
	menuTool->Append(new wxMenuItem(menuTool, ID_ONCLAHE, wxString(wxT("&CLAHE")), "Contrast Limited Adaptive Histogram Equalization", wxITEM_CHECK))->Check(false);
	menuTool->AppendSeparator();
	menuTool->Append(ID_ONOPEN_MASK, "&Open Mask Img\tCtrl-M", "Open Mask Img.");
	menuTool->Append(ID_ONOPEN_MASK_S, "&Open Mask_s Img\tCtrl-S", "Open Mask_s Img.");
	menuTool->AppendSeparator();
	menuTool->Append(ID_ONOPEN_PATTERN_PICKER, "&Open Pattern Picker\tCtrl-P", "Open Pattern Picker.");

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT, "&About", "About the System");
	menuHelp->Append(new wxMenuItem(menuHelp, wxID_TOGGLE_LOG, wxString(wxT("&Log\tCtrl-L")), "Show/Hide the Log", wxITEM_CHECK))->Check(true);
	//menuHelp->Append(wxID_TOGGLE_LOG, "&Toggle Log\tCtrl-L", "Show/Hide the Log");

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuTool, "&Tool");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
	CreateStatusBar(4);
	SetStatusText("SrcImg: None", 0);
	SetStatusText("Flowfield: None", 1);
	SetStatusText("Texture: None", 2);
	SetStatusText("Control-Img: None", 3);
#pragma endregion

#pragma region ToolBar: Buttons(Start, Fill Ink, Clean), Combobox(processingBox, controllingBox)
	wxToolBar *toolbar1 = new wxToolBar(this, wxID_ANY);
	start = new wxButton(toolbar1, BUTTON_Start, _T("Start"), wxDefaultPosition, wxDefaultSize, 0);
	fill = new wxButton(toolbar1, BUTTON_Fill, _T("Fill Ink"), wxDefaultPosition, wxDefaultSize, 0);
	clean = new wxButton(toolbar1, BUTTON_Clean, _T("Clean"), wxDefaultPosition, wxDefaultSize, 0);

	processingBox = new wxComboBox(toolbar1, COMBOBOX_Processing, "distribution_A", wxDefaultPosition, wxDefaultSize, 0);
	processingBox->Append("distribution_A");
	processingBox->Append("distribution_B");
	processingBox->Append("LIC");
	processingBox->Append("Motion_Illusion");
	processingBox->Append("Texture");
	processingBox->Append("dirTexture");
	processingBox->Append("PolarTexture");
	processingBox->Append("adaThresholding");
	processingBox->Append("Thresholding");
	processingBox->Append("Color_mapping");

	controllingBox = new wxComboBox(toolbar1, COMBOBOX_Controlling, "paint_to_B", wxDefaultPosition, wxDefaultSize, 0);
	controllingBox->Append("paint_to_B");
	//controllingBox->Append("seeds");
	controllingBox->Append("paint_white");
	controllingBox->Append("paint_black");

	toolbar1->AddControl(start);
	toolbar1->AddControl(fill);
	toolbar1->AddControl(clean);
	toolbar1->AddControl(processingBox);
	toolbar1->AddControl(controllingBox);

	toolbar1->Realize();
	SetToolBar(toolbar1);
#pragma endregion 

	//Sizer of whole window
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	//Sizer of leftside
	wxBoxSizer* leftside = new wxBoxSizer(wxVERTICAL);
	//Sizer of rightside(control panel)
	wxBoxSizer* rightside = new wxBoxSizer(wxVERTICAL);

	wxPanel* controlpanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	controlpanel->SetMaxSize(wxSize(250, -1));
	controlpanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

	wxString s;

#pragma region Leftside: drawPane, log
	//drawing panel
	drawPane = new BasicDrawPane(this, Size(256, 256));

	// wxTextCtrl: http://docs.wxwidgets.org/trunk/classwx_text_ctrl.html
	log = new wxTextCtrl(this, ID_WXEDIT1, wxT(""), wxPoint(91, 43), wxSize(121, 21), wxTE_RICH2 | wxTE_MULTILINE | wxTE_READONLY, wxDefaultValidator, wxT("WxEdit1"));
	addlog("Hello CRD!", wxColour(*wxBLACK));

	leftside->Add(drawPane, 7, wxEXPAND);
	leftside->Add(log, 1, wxEXPAND);
#pragma endregion

#pragma region Paint Parameters
	wxStaticBox *st_paint = new wxStaticBox(controlpanel, -1, wxT("Paint"), wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	wxStaticBoxSizer *st_paint_sizer = new wxStaticBoxSizer(st_paint, wxVERTICAL);

	s.Printf("Brush Size : %d", drawPane->brushSize);
	slider_brushSize_t = new wxStaticText(controlpanel, SLIDER_BRUSH_SIZE_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_brushSize_t, 0, wxEXPAND | wxLEFT, 10);
	slider_brushSize = new wxSlider(controlpanel, SLIDER_BRUSH_SIZE, drawPane->brushSize, 1, 10, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_brushSize, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("addA : %.3f", drawPane->element.addA);
	slider_addA_t = new wxStaticText(controlpanel, SLIDER_AddA_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_addA_t, 0, wxEXPAND | wxLEFT, 10);
	slider_addA = new wxSlider(controlpanel, SLIDER_AddA, int(drawPane->element.addA * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_addA, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("addB : %.3f", drawPane->element.addB);
	slider_addB_t = new wxStaticText(controlpanel, SLIDER_AddB_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_addB_t, 0, wxEXPAND | wxLEFT, 10);
	slider_addB = new wxSlider(controlpanel, SLIDER_AddB, int(drawPane->element.addB * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_paint_sizer->Add(slider_addB, 0, wxEXPAND | wxLEFT, 10);

	rightside->Add(st_paint_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 3);
#pragma endregion

#pragma region Pattern Parameters
	wxStaticBox* st_pattern = new wxStaticBox(controlpanel, -1, wxT("Pattern"), wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	wxStaticBoxSizer* st_pattern_sizer = new wxStaticBoxSizer(st_pattern, wxVERTICAL);

	s.Printf("Size : %.3f", drawPane->element.s);
	slider_s_t = new wxStaticText(controlpanel, SLIDER_S_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_s_t, 0, wxEXPAND | wxLEFT, 10);
	slider_s = new wxSlider(controlpanel, SLIDER_S, int(drawPane->element.s * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_s, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("F : %.4f", drawPane->element.f);
	slider_f_t = new wxStaticText(controlpanel, SLIDER_F_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_f_t, 0, wxEXPAND | wxLEFT, 10);
	slider_f = new wxSlider(controlpanel, SLIDER_F, int((drawPane->element.f / 0.06) * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_f, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("k : %.4f", drawPane->element.k);
	slider_k_t = new wxStaticText(controlpanel, SLIDER_K_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_k_t, 0, wxEXPAND | wxLEFT, 10);
	slider_k = new wxSlider(controlpanel, SLIDER_K, int((drawPane->element.k - 0.03) / 0.04 * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_k, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("l : %d", drawPane->element.l);
	slider_l_t = new wxStaticText(controlpanel, SLIDER_L_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_l_t, 0, wxEXPAND | wxLEFT, 10);
	slider_l = new wxSlider(controlpanel, SLIDER_L, int(drawPane->element.l), 0, 6, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_l, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("theta0 : %d", drawPane->element.theta0);
	slider_theta0_t = new wxStaticText(controlpanel, SLIDER_Theta0_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_theta0_t, 0, wxEXPAND | wxLEFT, 10);
	slider_theta0 = new wxSlider(controlpanel, SLIDER_Theta0, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(slider_theta0, 0, wxEXPAND | wxLEFT, 10);

	Modify_cb = new wxCheckBox(controlpanel, CHECKBOX_MODIFY_FUNCTION, wxT("Customize Anisotropic Function"), wxPoint(20, 20));
	Modify_cb->SetValue(false);
	st_pattern_sizer->Add(Modify_cb, 0, wxEXPAND | wxLEFT, 10);

#pragma region Customize Anisotropic Function Panel
	//      Circle    min/Max Slider
	//	  |--------|-----------------|
	//    |	degree |  ========[]===  |
	//	  |   GUI  |  ====[]=======  |
	//	  |--------|-----------------|
	wxBoxSizer* degree_outer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* degree_slider = new wxBoxSizer(wxVERTICAL);
	s.Printf("min degree : %d", drawPane->mindegree);
	slider_mindegree_t = new wxStaticText(controlpanel, SLIDER_MINDEGREE_T, s, wxDefaultPosition, wxDefaultSize, 0);
	degree_slider->Add(slider_mindegree_t, 0, wxEXPAND | wxLEFT, 10);
	slider_mindegree = new wxSlider(controlpanel, SLIDER_MINDEGREE, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0);
	degree_slider->Add(slider_mindegree, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("max degree : %d", drawPane->maxdegree);
	slider_maxdegree_t = new wxStaticText(controlpanel, SLIDER_MAXDEGREE_T, s, wxDefaultPosition, wxDefaultSize, 0);
	degree_slider->Add(slider_maxdegree_t, 0, wxEXPAND | wxLEFT, 10);
	slider_maxdegree = new wxSlider(controlpanel, SLIDER_MAXDEGREE, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0);
	degree_slider->Add(slider_maxdegree, 0, wxEXPAND | wxLEFT, 10);


	degreeGUI = new SimpleDrawPanel(controlpanel);
	degree_outer->Add(degreeGUI, 1, wxEXPAND);
	degree_outer->Add(degree_slider, 2, wxEXPAND);

	st_pattern_sizer->Add(degree_outer, 0, wxEXPAND);
#pragma endregion

	Segmentation_cb = new wxCheckBox(controlpanel, CHECKBOX_SEGMENTATION, wxT("Activate Segmentation"), wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(Segmentation_cb, 0, wxEXPAND | wxLEFT, 10);

	wxArrayString m_choice1Choices;
	SegmentationBox = new wxChoice(controlpanel, COMBOBOX_Region, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0);
	SegmentationBox->SetSelection(0);
	//SegmentationBox->Append("kiul");
	st_pattern_sizer->Add(SegmentationBox, 0, wxEXPAND | wxLEFT, 10);
	DisplayRegion_cb = new wxCheckBox(controlpanel, CHECKBOX_DISPLAY_REGION, wxT("Display Region"), wxDefaultPosition, wxDefaultSize, 0);
	st_pattern_sizer->Add(DisplayRegion_cb, 0, wxEXPAND | wxLEFT, 10);

	rightside->Add(st_pattern_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 3);
#pragma endregion 

#pragma region Post Processing Parameters
	wxStaticBox *st_pp = new wxStaticBox(controlpanel, -1, wxT("Post Processing"), wxDefaultPosition, wxDefaultSize, wxTE_RICH2);
	wxStaticBoxSizer *st_pp_sizer = new wxStaticBoxSizer(st_pp, wxVERTICAL);
	s.Printf("alpha : %.3f", drawPane->processing.alpha);
	slider_alpha_t = new wxStaticText(controlpanel, SLIDER_Alpha_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pp_sizer->Add(slider_alpha_t, 0, wxEXPAND | wxLEFT, 10);
	slider_alpha = new wxSlider(controlpanel, SLIDER_Alpha, int(drawPane->processing.alpha * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_pp_sizer->Add(slider_alpha, 0, wxEXPAND | wxLEFT, 10);

	s.Printf("beta : %.3f", drawPane->processing.beta);
	slider_beta_t = new wxStaticText(controlpanel, SLIDER_Beta_T, s, wxDefaultPosition, wxDefaultSize, 0);
	st_pp_sizer->Add(slider_beta_t, 0, wxEXPAND | wxLEFT, 10);
	slider_beta = new wxSlider(controlpanel, SLIDER_Beta, int(drawPane->processing.beta * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	st_pp_sizer->Add(slider_beta, 0, wxEXPAND | wxLEFT, 10);

	rightside->Add(st_pp_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 3);
#pragma endregion

	//set portion of size: leftside & rightside(control)
	controlpanel->SetSizer(rightside);
	sizer->Add(leftside, 7, wxEXPAND);
	sizer->Add(controlpanel, 3, wxEXPAND);
	rightside->Fit(controlpanel);
	SetSizer(sizer);

	slider_alpha->Disable();
	slider_beta->Disable();

	SegmentationBox->Hide();
	DisplayRegion_cb->Hide();
	fill->Disable();
	degreeGUI->Hide();
	slider_mindegree->Hide();
	slider_maxdegree->Hide();
	slider_mindegree_t->Hide();
	slider_maxdegree_t->Hide();
	this->GetSizer()->Layout();

	render_loop_on = false;
	isCLAHE = false;
}
void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("Wei-Ching Liu - Computer Science - National Chengchi University", "About CRD", wxOK | wxICON_INFORMATION);
}
void MyFrame::OnToggleLog(wxCommandEvent& event)
{
	event.IsChecked() ? log->Show() : log->Hide();

	this->GetSizer()->Layout();
}

void MyFrame::OnOpenSrc(wxCommandEvent& event)
{
	render_loop_on = false;
	activateRenderLoop(render_loop_on);
	wxFileDialog openFileDialog(this, _("Open image file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		addlog("Load Img Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else
	{
		wxString s;
		s.Printf("Load Img - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));

		s.Printf("SrcImg: %s", openFileDialog.GetFilename());
		SetStatusText(s, 0);

		////If no Flow is Loaded, Read Same File as defaulf ETF
		//if (!drawPane->element.FlowLoaded && !drawPane->element.ETFLoaded)
		//{	
		//	drawPane->element.ETF((const char*)openFileDialog.GetPath().mb_str()); 
		//	s.Printf("Flowfield(ETF): %s", openFileDialog.GetFilename());
		//	SetStatusText(s, 1);
		//}
	}

	// proceed loading the file chosen by the user, this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->element.ReadSrc((const char*)openFileDialog.GetPath().mb_str());


	drawPane->SetSize(drawPane->element.Mask.cols, drawPane->element.Mask.rows);
	render_loop_on = true;
	activateRenderLoop(render_loop_on);
}
void MyFrame::OnOpenVfb(wxCommandEvent& event)
{
	render_loop_on = false;
	activateRenderLoop(render_loop_on);
	wxFileDialog openFileDialog(this, _("Open vfb file"), "", "", "vfb files (*.vfb)|*.vfb", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		addlog("Load Flow Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else
	{
		wxString s;
		s.Printf("Load Flow(.vfb) - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));

		s.Printf("Flowfield(vfb): %s", openFileDialog.GetFilename());
		SetStatusText(s, 1);
	}


	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->element.ReadFlow((const char*)openFileDialog.GetPath().mb_str());
	render_loop_on = true;
	activateRenderLoop(render_loop_on);
}
void MyFrame::OnOpenETF(wxCommandEvent& event)
{
	render_loop_on = false;
	activateRenderLoop(render_loop_on);
	wxFileDialog openFileDialog(this, _("Open image file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		addlog("Load ETF Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else
	{
		wxString s;
		s.Printf("Load ETF - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));

		s.Printf("Flowfield(ETF): %s", openFileDialog.GetFilename());
		SetStatusText(s, 1);
	}


	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->element.ETF((const char*)openFileDialog.GetPath().mb_str());
	render_loop_on = true;
	activateRenderLoop(render_loop_on);

}
void MyFrame::OnOpenTex(wxCommandEvent& event)
{
	render_loop_on = false;
	activateRenderLoop(render_loop_on);
	wxFileDialog openFileDialog(this, _("Open texture file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		addlog("Load Texture Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else
	{
		wxString s;
		s.Printf("Load Texture - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));

		s.Printf("Texture: %s", openFileDialog.GetFilename());
		SetStatusText(s, 2);
	}

	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->processing.ReadTexture((const char*)openFileDialog.GetPath().mb_str());

	addlog("Texture Loaded", wxColour(*wxBLACK));
	render_loop_on = true;
	activateRenderLoop(render_loop_on);

}
void MyFrame::OnOpenControlImg(wxCommandEvent& event)
{
	render_loop_on = false;
	activateRenderLoop(render_loop_on);

	wxFileDialog openFileDialog(this, _("Open image file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		addlog("Load Control Img Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else
	{
		wxString s;
		s.Printf("Load Control Img - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));

		s.Printf("Control Img: %s", openFileDialog.GetFilename());
		SetStatusText(s, 3);
	}

	// proceed loading the file chosen by the user, this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->element.ReadControlImg((const char*)openFileDialog.GetPath().mb_str());

	SegmentationBox->Clear();
	for (int i = 0; i < drawPane->element.segmentation.size(); i++)
	{
		wxString s;
		s.Printf("Region - %s", to_string(i + 1));
		SegmentationBox->Append(s);
	}
	render_loop_on = true;
	activateRenderLoop(render_loop_on);

}

void MyFrame::OnSaveResult(wxCommandEvent& event)
{
	wxFileDialog
		saveFileDialog(this, _("Save PNG file"), "", "",
		"PNG files (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...

	// save the current contents in the file;
	// this can be done with e.g. wxWidgets output streams:
	wxFileOutputStream output_stream(saveFileDialog.GetPath());
	if (!output_stream.IsOk())
	{
		wxLogError("Cannot save current contents in file '%s'.", saveFileDialog.GetPath());
		return;
	}

	cvtColor(drawPane->dis, drawPane->dis, CV_BGR2RGB);
	imwrite((const char*)saveFileDialog.GetPath().mb_str(), drawPane->dis);
}

void MyFrame::OnEdge2AddA(wxCommandEvent& event)
{
	drawPane->element.Addition_A = drawPane->element.Mask_s.clone();
}
void MyFrame::OnEdge2AddB(wxCommandEvent& event)
{
	drawPane->element.Addition_B = drawPane->element.Mask_s.clone();
}
void MyFrame::OnMask2AddA(wxCommandEvent& event)
{
	drawPane->element.Addition_A += 0.5*drawPane->element.Mask;
}
void MyFrame::OnMask2AddB(wxCommandEvent& event)
{
	drawPane->element.Addition_B += 0.5*drawPane->element.Mask;
}
void MyFrame::OnCLAHE(wxCommandEvent& event)
{
	isCLAHE = !isCLAHE;
}

void MyFrame::OnOpenMask(wxCommandEvent& event)
{
	if (drawPane->element.SrcLoaded)
	{
		imshow("Original Img", drawPane->element.Original_img);
		imshow("Maks Img", drawPane->element.Mask);
	}
	else addlog("SrcImg didn't Load !", wxColour(*wxRED));
}
void MyFrame::OnOpenMaskS(wxCommandEvent& event)
{
	if (drawPane->element.SrcLoaded) imshow("Mask_S Img", drawPane->element.Mask_s);
	else addlog("SrcImg didn't Load !", wxColour(*wxRED));
}
void MyFrame::OnOpenPatternPicker(wxCommandEvent& event)
{
	activateRenderLoop(false);
	patternpicker = new MyPatternPicker(this, wxT("Pattern Picker"));
	patternpicker->Show();
	patternpicker->StartPreview();
}

//Buttons
void MyFrame::OnStart(wxCommandEvent& event)
{
	render_loop_on = !render_loop_on;
	activateRenderLoop(render_loop_on);

	//if (render_loop_on) fill->Enable();
	//else fill->Disable();
}
void MyFrame::OnFill(wxCommandEvent& event)
{
	drawPane->Seeds(25, true, .5);
}
void MyFrame::OnClean(wxCommandEvent& event)
{
	*drawPane->element.c_A = Mat::ones(drawPane->element.Mask.size(), CV_32F);
	*drawPane->element.c_B = Mat::zeros(drawPane->element.Mask.size(), CV_32F);
	*drawPane->element.p_A = Mat::ones(drawPane->element.Mask.size(), CV_32F);
	*drawPane->element.p_B = Mat::zeros(drawPane->element.Mask.size(), CV_32F);
	drawPane->element.Addition_A = Mat::zeros(drawPane->element.Mask.size(), CV_32F);
	drawPane->element.Addition_B = Mat::zeros(drawPane->element.Mask.size(), CV_32F);

	drawPane->paintNow(true); //execute clean action
	addlog("Draw Panel Cleaned.", wxColour(*wxBLACK));
}

//Comboboxes
void MyFrame::OnProcessingBox(wxCommandEvent& event)
{
	drawPane->processingS = processingBox->GetValue();

	if (drawPane->processingS == "dirTexture" || drawPane->processingS == "PolarTexture")
	{
		wxString s;
		if (!drawPane->processing.TextureLoaded)
		{
			s.Printf("Must Loaded Texture First! (%s)", drawPane->processingS);
			addlog(s, wxColour(*wxRED));
			drawPane->processingS = "distribution_A";
			processingBox->SetSelection(0);
		}
		else if (!drawPane->element.FlowLoaded)
		{
			s.Printf("Must Loaded vfb for Flowfield! (%s)", drawPane->processingS);
			addlog(s, wxColour(*wxRED));
			drawPane->processingS = "distribution_A";
			processingBox->SetSelection(0);
		}
	}

	if (processingBox->GetValue() == "distribution_A" || processingBox->GetValue() == "distribution_B")
	{
		slider_alpha->Disable();
		slider_beta->Disable();
	}
	else
	{
		slider_alpha->Enable();
		slider_beta->Enable();
	}
	drawPane->paintNow(true); //execute action
}
void MyFrame::OnControllingBox(wxCommandEvent& event)
{
	drawPane->controllingS = controllingBox->GetValue();
}

//Slides: Pattern Parameter
void MyFrame::OnSliderS(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].size = slider_s->GetValue() / 1000.0;
		s.Printf("Size : %.3f", drawPane->element.segmentation[drawPane->regionSelected - 1].size);
		drawPane->element.UpdateControlMask();
	}
	else
	{
		drawPane->element.s = slider_s->GetValue() / 1000.0;
		s.Printf("Size : %.3f", drawPane->element.s);
	}
	slider_s_t->SetLabel(s);
}
void MyFrame::OnSliderF(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].F = slider_f->GetValue() / 1000.0*0.06;
		s.Printf("F : %.4f", drawPane->element.segmentation[drawPane->regionSelected - 1].F);
		drawPane->element.UpdateControlMask();
	}
	else
	{
		drawPane->element.f = slider_f->GetValue() / 1000.0*0.06;
		s.Printf("F : %.4f", drawPane->element.f);
	}
	slider_f_t->SetLabel(s);
}
void MyFrame::OnSliderK(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].k = slider_k->GetValue() / 1000.0*0.04 + 0.03;
		s.Printf("k : %.4f", drawPane->element.segmentation[drawPane->regionSelected - 1].k);
		drawPane->element.UpdateControlMask();
	}
	else
	{
		drawPane->element.k = slider_k->GetValue() / 1000.0*0.04 + 0.03;
		s.Printf("k : %.4f", drawPane->element.k);
	}
	slider_k_t->SetLabel(s);
}
void MyFrame::OnSliderL(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].l = slider_l->GetValue();
		s.Printf("l : %d", drawPane->element.segmentation[drawPane->regionSelected - 1].l);
		drawPane->element.UpdateControlMask();
	}
	else
	{
		drawPane->element.l = slider_l->GetValue();
		s.Printf("l : %d", drawPane->element.l);
	}
	slider_l_t->SetLabel(s);
}
void MyFrame::OnSliderTheta0(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].theta0 = slider_theta0->GetValue();
		s.Printf("theta0 : %.0f", drawPane->element.segmentation[drawPane->regionSelected - 1].theta0);

		drawPane->element.UpdateControlMask();
	}
	else
	{
		drawPane->element.theta0 = slider_theta0->GetValue();
		s.Printf("theta0 : %d", drawPane->element.theta0);
	}
	slider_theta0_t->SetLabel(s);
}
void MyFrame::OnCheckboxModifyToggle(wxCommandEvent& event)
{
	if (Modify_cb->GetValue())
	{
		if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
		{
			drawPane->element.segmentation[drawPane->regionSelected - 1].CAF = true;
		}

		drawPane->customAnisotropicFunction = true;
		degreeGUI->Show();
		slider_mindegree->Show();
		slider_maxdegree->Show();
		slider_mindegree_t->Show();
		slider_maxdegree_t->Show();
	}
	else
	{
		if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
		{
			drawPane->element.segmentation[drawPane->regionSelected - 1].dmin = 0;
			drawPane->element.segmentation[drawPane->regionSelected - 1].dmax = 0;

			drawPane->element.segmentation[drawPane->regionSelected - 1].CAF = false;
			drawPane->element.UpdateControlMask();
		}
		wxString s;
		slider_mindegree->SetValue(0);
		slider_maxdegree->SetValue(0);
		s.Printf("min degree : %d", 0);
		slider_mindegree_t->SetLabel(s);
		s.Printf("max degree : %d", 0);
		slider_maxdegree_t->SetLabel(s);
		degreeGUI->dmin = slider_mindegree->GetValue();
		degreeGUI->dmax = slider_maxdegree->GetValue();
		degreeGUI->paintNow();

		drawPane->customAnisotropicFunction = false;
		degreeGUI->Hide();
		slider_mindegree->Hide();
		slider_maxdegree->Hide();
		slider_mindegree_t->Hide();
		slider_maxdegree_t->Hide();
	}
	this->Layout();
}
void MyFrame::OnSliderMinDegree(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].dmin = slider_mindegree->GetValue();
		s.Printf("min degree : %d", drawPane->element.segmentation[drawPane->regionSelected - 1].dmin);
		drawPane->element.UpdateControlMask();
		degreeGUI->dmin = slider_mindegree->GetValue();
		degreeGUI->paintNow();
	}
	else
	{
		drawPane->mindegree = slider_mindegree->GetValue();
		s.Printf("min degree : %d", drawPane->mindegree);
		degreeGUI->dmin = slider_mindegree->GetValue();
		degreeGUI->paintNow();
	}
	slider_mindegree_t->SetLabel(s);
}
void MyFrame::OnSliderMaxDegree(wxCommandEvent& event)
{
	wxString s;
	if (Segmentation_cb->GetValue() && drawPane->regionSelected != 0)
	{
		drawPane->element.segmentation[drawPane->regionSelected - 1].dmax = slider_maxdegree->GetValue();
		s.Printf("max degree : %d", drawPane->element.segmentation[drawPane->regionSelected - 1].dmax);
		drawPane->element.UpdateControlMask();
		degreeGUI->dmax = slider_maxdegree->GetValue();
		degreeGUI->paintNow();
	}
	else
	{
		drawPane->maxdegree = slider_maxdegree->GetValue();
		s.Printf("max degree : %d", drawPane->maxdegree);
		degreeGUI->dmax = slider_maxdegree->GetValue();
		degreeGUI->paintNow();
	}
	slider_maxdegree_t->SetLabel(s);

}
void MyFrame::OnCheckboxSegmentation(wxCommandEvent& event)
{
	if (drawPane->element.ControlImgLoad)
	{
		if (Segmentation_cb->GetValue())
		{
			SegmentationBox->Show();
			DisplayRegion_cb->Show();
			drawPane->regionOn = true;
			//SegmentationBox->Select(0);
			drawPane->regionSelected = SegmentationBox->GetSelection() + 1;
		}
		else
		{
			SegmentationBox->Hide();
			DisplayRegion_cb->Hide();
			drawPane->regionOn = false;
		}
		this->Layout();
	}
	else
	{
		addlog("Must Load Control Img First!", wxColour(*wxRED));
		Segmentation_cb->SetValue(false);
	}

}
void MyFrame::OnCheckboxDisplayRegion(wxCommandEvent& event)
{
	if (DisplayRegion_cb->GetValue())
	{
		drawPane->displayRegion = true;
	}
	else
	{
		drawPane->displayRegion = false;
	}

}
void MyFrame::OnSegmentationBox(wxCommandEvent& event)
{
	drawPane->regionSelected = SegmentationBox->GetSelection() + 1;
	int sr = drawPane->regionSelected - 1;


	wxString s;
	slider_s->SetValue(drawPane->element.segmentation[sr].size * 1000.0 );
	slider_f->SetValue(drawPane->element.segmentation[sr].F * 1000.0 / 0.06);
	slider_k->SetValue((drawPane->element.segmentation[sr].k - 0.03) / 0.04 * 1000);
	slider_l->SetValue(drawPane->element.segmentation[sr].l);
	slider_theta0->SetValue((int)drawPane->element.segmentation[sr].theta0);
	if (drawPane->element.segmentation[sr].CAF)
	{
		Modify_cb->SetValue(true);
		slider_mindegree->SetValue(drawPane->element.segmentation[sr].dmin);
		slider_maxdegree->SetValue(drawPane->element.segmentation[sr].dmax);
		degreeGUI->dmin = slider_mindegree->GetValue();
		degreeGUI->dmax = slider_maxdegree->GetValue();

		degreeGUI->paintNow();

		degreeGUI->Show();
		slider_mindegree->Show();
		slider_maxdegree->Show();
		slider_mindegree_t->Show();
		slider_maxdegree_t->Show();
	}
	else
	{
		Modify_cb->SetValue(false);
		slider_mindegree->SetValue(0);
		slider_maxdegree->SetValue(0);
		degreeGUI->dmin = slider_mindegree->GetValue();
		degreeGUI->dmax = slider_maxdegree->GetValue();

		degreeGUI->paintNow();

		degreeGUI->Hide();
		slider_mindegree->Hide();
		slider_maxdegree->Hide();
		slider_mindegree_t->Hide();
		slider_maxdegree_t->Hide();
	}

	s.Printf("Size : %.3f", drawPane->element.segmentation[sr].size);
	slider_s_t->SetLabel(s);
	s.Printf("F : %.4f", drawPane->element.segmentation[sr].F);
	slider_f_t->SetLabel(s);
	s.Printf("k : %.4f", drawPane->element.segmentation[sr].k);
	slider_k_t->SetLabel(s);
	s.Printf("l : %d", drawPane->element.segmentation[sr].l);
	slider_l_t->SetLabel(s);
	s.Printf("theta0 : %.0f", drawPane->element.segmentation[sr].theta0);
	slider_theta0_t->SetLabel(s);
	s.Printf("min degree : %d", drawPane->element.segmentation[sr].dmin);
	slider_mindegree_t->SetLabel(s);
	s.Printf("max degree : %d", drawPane->element.segmentation[sr].dmax);
	slider_maxdegree_t->SetLabel(s);

	s.Printf("Region selected: %d", drawPane->regionSelected);
	addlog(s, wxColour(*wxBLUE));
	this->Layout();
}

//Slides: Paint Parameter
void MyFrame::OnSliderBrushSize(wxCommandEvent& event)
{
	drawPane->brushSize = slider_brushSize->GetValue();
	wxString s;
	s.Printf("Brush Size : %d", drawPane->brushSize);
	slider_brushSize_t->SetLabel(s);
}
void MyFrame::OnSliderAddA(wxCommandEvent& event)
{
	drawPane->element.addA = slider_addA->GetValue() / 1000.0;
	wxString s;
	s.Printf("addA : %.3f", drawPane->element.addA);
	slider_addA_t->SetLabel(s);
}
void MyFrame::OnSliderAddB(wxCommandEvent& event)
{
	drawPane->element.addB = slider_addB->GetValue() / 1000.0;
	wxString s;
	s.Printf("addB : %.3f", drawPane->element.addB);
	slider_addB_t->SetLabel(s);
}

//Slides: Post Processing Parameter
void MyFrame::OnSliderAlpha(wxCommandEvent& event)
{
	drawPane->processing.alpha = slider_alpha->GetValue() / 1000.0;
	wxString s;
	s.Printf("alpha : %.3f", drawPane->processing.alpha);
	slider_alpha_t->SetLabel(s);
	drawPane->paintNow(true); //execute action
}
void MyFrame::OnSliderBeta(wxCommandEvent& event)
{
	drawPane->processing.beta = slider_beta->GetValue() / 1000.0;
	wxString s;
	s.Printf("beta : %.3f", drawPane->processing.beta);
	slider_beta_t->SetLabel(s);
	drawPane->paintNow(true); //execute action
}

void MyFrame::addlog(wxString info, wxColour& color)
{
	time_t currentTime;// for logging current time
	struct tm *localTime;// for logging current time
	time(&currentTime);                   // Get the current time
	localTime = localtime(&currentTime);  // Convert the current time to the local time
	wxString s;
	s.Printf("\n%02d : %02d : %02d  |  %s", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, info);
	log->SetDefaultStyle(wxTextAttr(color));
	log->AppendText(s);

}
void MyFrame::activateRenderLoop(bool on)
{
	if (on)
	{
		start->SetLabel("Stop");
		fill->Enable();
		Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MyFrame::onIdle));
		render_loop_on = true;
		addlog("-------Start iteration-------", wxColour(*wxBLACK));
	}
	else
	{
		start->SetLabel("Start");
		fill->Disable();
		Disconnect(wxEVT_IDLE, wxIdleEventHandler(MyFrame::onIdle));
		//Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MyFrame::onIdle));
		render_loop_on = false;
		addlog("-------Stop iteration-------", wxColour(*wxBLACK));
	}
}
void MyFrame::onIdle(wxIdleEvent& evt)
{
	drawPane->paintNow(render_loop_on);
	evt.RequestMore(); // render continuously, not only once on idle
}
#pragma endregion 

#pragma region BasicDrawPane
BasicDrawPane::BasicDrawPane(wxFrame* parent, Size s) :
processing(s),
element(s),
wxPanel(parent)
{
	activateDraw = false;
	brushSize = 1;
	customAnisotropicFunction = false;
	mindegree = 0;
	maxdegree = 0;
	regionSelected = 0;
	regionOn = false;
	displayRegion = false;
}
void BasicDrawPane::Seeds(int r, bool isoffset, float ratio)
{
	int w = element.c_A->cols, h = element.c_A->rows;
	int hr = r * ratio;
	int w_offset = w % (int(2 * r)) / 2;
	int h_offset = h % (int(2 * r)) / 2;

	Point2i temp;
	for (int j = h_offset + hr, k = 0; j < h; j += 2 * hr, k++)
	{
		if (isoffset && k % 2 == 1)
		{
			for (int i = w_offset - r; i < w; i += 2 * r)
			{
				temp.x = i + r + hr * 2 * (((float)rand()) / RAND_MAX - 0.5);
				temp.y = j + hr * 2 * (((float)rand()) / RAND_MAX - 0.5);
				int thickness = -1;
				int lineType = 8;
				ellipse(*element.c_B,
					Point(temp.x%element.c_B->cols, temp.y%element.c_B->rows),
					Size(brushSize, brushSize),
					0,
					0,
					360,
					Scalar(0.5, 0.5, 0.5),
					thickness,
					lineType);
			}
		}
		else
		{
			for (int i = w_offset + r; i < w; i += 2 * r)
			{
				temp.x = i + hr * 2 * (((float)rand()) / RAND_MAX - 0.5);
				temp.y = j + hr * 2 * (((float)rand()) / RAND_MAX - 0.5);
				int thickness = -1;
				int lineType = 8;
				ellipse(*element.c_B,
					Point(temp.x%element.c_B->cols, temp.y%element.c_B->rows),
					Size(brushSize, brushSize),
					0,
					0,
					360,
					Scalar(0.5, 0.5, 0.5),
					thickness,
					lineType);
			}
		}
	}
}

void BasicDrawPane::MouseMove(wxMouseEvent &event)
{
	Point MousePosition(min(max(event.m_x, 0), element.c_B->cols), min(max(event.m_y, 0), element.c_B->rows));
	
	if (activateDraw)
	{
		if (controllingS == "paint_white")
		{
			line(element.Addition_A, LastMousePosition, MousePosition, Scalar(1, 1, 1), brushSize);
			//ellipse(element.Addition_A,
			//	Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
			//	Size(brushSize, brushSize),
			//	0,
			//	0,
			//	360,
			//	Scalar(1, 1, 1),
			//	-1,
			//	8);
		}
		else if (controllingS == "paint_black")
		{
			line(element.Addition_B, LastMousePosition, MousePosition, Scalar(1, 1, 1), brushSize);
			//ellipse(element.Addition_B,
			//	Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
			//	Size(brushSize, brushSize),
			//	0,
			//	0,
			//	360,
			//	Scalar(1, 1, 1),
			//	-1,
			//	8);
		}
		else
			line(*element.c_B, LastMousePosition, MousePosition, Scalar(1, 1, 1), brushSize);
			//ellipse(
			//*element.c_B,         // img - Image.
			//Point(event.m_x % element.c_B->cols, event.m_y % element.c_B->rows),// center - Center of the ellipse.
			//Size(brushSize, brushSize),           // axes - Half of the size of the ellipse main axes.
			//0,                    // angle - Ellipse rotation angle in degrees.
			//0,                    // startAngle - Starting angle of the elliptic arc in degrees.
			//360,                  // endAngle - Ending angle of the elliptic arc in degrees.
			//Scalar(0.5, 0.5, 0.5),// color - Ellipse color.
			//-1,                    // thickness - Thickness of the ellipse arc outline
			//8                     // lineType - Type of the ellipse boundary. See the line() description.
			//);
	}

	LastMousePosition = Point(min(max(event.m_x, 0), element.c_B->cols), min(max(event.m_y, 0), element.c_B->rows));

	//((MyFrame *)GetParent())->addlog("Panit event - Mouse Down at (%.0f, %.0f)", wxColour(*wxBLACK));
}
void BasicDrawPane::MouseLDown(wxMouseEvent &event)
{
	if (controllingS == "paint_white")
	{
		//element.Addition_A.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
		ellipse(element.Addition_A,
			Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
			Size(brushSize, brushSize),
			0,
			0,
			360,
			Scalar(1, 1, 1),
			-1,
			8);
	}
	else if (controllingS == "paint_black")
	{
		ellipse(element.Addition_B,
			Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
			Size(brushSize, brushSize),
			0,
			0,
			360,
			Scalar(1, 1, 1),
			-1,
			8);
		//element.Addition_B.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
	}
	else
	{
		ellipse(*element.c_B,
			Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
			Size(brushSize, brushSize),
			0,
			0,
			360,
			Scalar(0.5, 0.5, 0.5),
			-1,
			8);
	}

	activateDraw = true;
}
void BasicDrawPane::MouseLUp(wxMouseEvent &event)
{
	activateDraw = false;
}

//first frame
void BasicDrawPane::paintEvent(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	//render(dc);
	dis = element.c_A->clone();
	dis.convertTo(dis, CV_8UC1, 255);
	cvtColor(dis, dis, CV_GRAY2RGB);
	wxImage img(dis.cols, dis.rows, dis.data, true);
	wxBitmap bmp(img);
	dc.DrawBitmap(bmp, 0, 0);
}

//render loop
void BasicDrawPane::paintNow(bool render_loop_on)
{
	wxClientDC dc(this);
	render(dc, render_loop_on);
}

//Main Render(iteration) Section
void BasicDrawPane::render(wxDC& dc, bool render_loop_on)
{
	if (render_loop_on)
	{
		//if (customAnisotropicFunction){
			int steps = element.FastGrayScott(mindegree, maxdegree, false, regionOn);
			//((MyFrame *)GetParent())->SetStatusText(wxString::Format("%i", steps), 0); //preview does not have StatusText
		//}
		//else {
		//	element.FastGrayScott(0, 0, false, regionOn);
		//}

		////must read 500x500 input image first, and a flow field
		//element.GrayScottModel();
	}

	dis = element.c_A->clone();

	if (((MyFrame *)GetParent())->isCLAHE)
	{
		processing.CLAHE(dis);
	}

	//element.DrawHistogram(dis, *element.c_B);

	//post process
	if (processingS == "Motion_Illusion")
	{
		processing.motionIllu(*element.c_A, element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC3, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "distribution_B")
	{
		dis = element.c_B->clone();
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_GRAY2BGR);
	}
	else if (processingS == "LIC")
	{
		processing.LIC(element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_GRAY2BGR);
	}
	else if (processingS == "Texture")
	{
		processing.Texture(dis, dis);
		dis.convertTo(dis, CV_8UC3, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "dirTexture")
	{
		processing.dirTexture(dis, element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC3, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "PolarTexture")
	{
		processing.dirTexture_Polar(dis, element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC3, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "adaThresholding")
	{
		processing.adaThresholding(dis, element.Mask, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "Thresholding")
	{
		processing.Thresholding(dis, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "Color_mapping")
	{
		processing.Colormapping(dis, element.Mask, element.Original_img, dis);
		dis.convertTo(dis, CV_8UC3, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else
	{
		//if (((MyFrame *)GetParent())->Segmentation_cb->GetValue()){
		if (displayRegion && regionOn && regionSelected != 0)
		{
			element.DisplaySeg(dis, regionSelected - 1);
		}
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_GRAY2BGR);
	}

	wxImage img(dis.cols, dis.rows, dis.data, true);
	wxBitmap bmp(img);
	dc.DrawBitmap(bmp, 0, 0);

	//((MyFrame *)GetParent())->SetStatusText(wxString::Format("Fps: %.0f\t\tframeCounter: %i", 1000.0 / timeSincePrevFrame, counter), 0);
}
#pragma endregion 