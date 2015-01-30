#include "gui.h"
#include <ctime>


bool MyApp::OnInit()
{
	MyFrame *frame = new MyFrame("CRD", wxPoint(50, 50), wxSize(800, 600));
	frame->Show(true);
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(ID_ONOPENSRC, "&Open SrcImg\tCtrl-O", "Open source image");
	menuFile->Append(ID_ONOPENVFB, "&Open Flow\tCtrl-F", "Open Flowfield file");
	menuFile->Append(ID_ONOPENETF, "&Open ETF\tCtrl-E", "Open image file");
	menuFile->Append(ID_ONOPENTEX, "&Open Texture\tCtrl-E", "Open texture file");
	menuFile->Append(ID_ONSAVE, "&Save\tCtrl-E", "Save Result");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu *menuTool = new wxMenu;
	menuTool->Append(ID_ONEdge2AddA, "&Edge2AddA\tCtrl-A", "add Edge to addition A");
	menuTool->Append(ID_ONEdge2AddB, "&Edge2AddB\tCtrl-B", "add Edge to addition B");
	menuTool->AppendSeparator();
	menuTool->Append(ID_ONOPEN_MASK, "&Open Mask Img\tCtrl-C", "Open Mask Img.");
	menuTool->Append(ID_ONOPEN_MASK_S, "&Open Mask_s Img\tCtrl-C", "Open Mask_s Img.");
	menuTool->AppendSeparator();
	menuTool->Append(ID_ONOPEN_PATTERN_PICKER, "&Open Pattern Picker\tCtrl-P", "Open Pattern Picker.");

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);


	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuTool, "&Tool");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
	CreateStatusBar();
	SetStatusText("");

	//start = new wxButton(this, BUTTON_Start, _T("Start"), wxDefaultPosition, wxDefaultSize, 0); 

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* leftside = new wxBoxSizer(wxVERTICAL);
	drawPane = new BasicDrawPane(this, Size(256, 256));

	// wxTextCtrl: http://docs.wxwidgets.org/trunk/classwx_text_ctrl.html
	log = new wxTextCtrl(this, ID_WXEDIT1, wxT(""), wxPoint(91, 43), wxSize(121, 21), wxTE_RICH2|wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, wxT("WxEdit1"));
	addlog("Hello CRD!", wxColour(*wxBLACK));
	leftside->Add(drawPane, 4, wxEXPAND);
	leftside->Add(log, 1, wxEXPAND);


	wxString s;
	wxBoxSizer* control = new wxBoxSizer(wxVERTICAL);
	start = new wxButton(this, BUTTON_Start, _T("Start"), wxDefaultPosition, wxDefaultSize, 0);
	control->Add(start, 0, wxEXPAND);
	fill = new wxButton(this, BUTTON_Fill, _T("Fill Ink"), wxDefaultPosition, wxDefaultSize, 0);
	control->Add(fill, 0, wxEXPAND);
	clean = new wxButton(this, BUTTON_Clean, _T("Clean"), wxDefaultPosition, wxDefaultSize, 0);
	control->Add(clean, 0, wxEXPAND);

	processingBox = new wxComboBox(this, COMBOBOX_Processing, "distribution_A", wxDefaultPosition, wxDefaultSize, 0);
	processingBox->Append("distribution_A");
	processingBox->Append("Motion_Illusion");
	processingBox->Append("dirTexture");
	processingBox->Append("adaThresholding");
	processingBox->Append("Thresholding");
	control->Add(processingBox, 0, wxEXPAND);

	controllingBox = new wxComboBox(this, COMBOBOX_Controlling, "seeds", wxDefaultPosition, wxDefaultSize, 0);
	controllingBox->Append("seeds");
	controllingBox->Append("addA");
	controllingBox->Append("addB");
	control->Add(controllingBox, 0, wxEXPAND);

	s.Printf("Size : %.4f", drawPane->element.s);
	slider_s_t = new wxStaticText(this, SLIDER_S_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_s_t, 0, 0);
	slider_s = new wxSlider(this, SLIDER_S, int(drawPane->element.s * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_s, 0, wxEXPAND);


	s.Printf("F : %.4f", drawPane->element.f);
	slider_f_t = new wxStaticText(this, SLIDER_F_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_f_t, 0, 0);
	slider_f = new wxSlider(this, SLIDER_F, int((drawPane->element.f / 0.06) * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_f, 0, wxEXPAND);


	s.Printf("k : %.4f", drawPane->element.k);
	slider_k_t = new wxStaticText(this, SLIDER_K_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_k_t, 0, 0);
	slider_k = new wxSlider(this, SLIDER_K, int((drawPane->element.k - 0.03) / 0.04 * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_k, 0, wxEXPAND);


	s.Printf("l : %d", drawPane->element.l);
	slider_l_t = new wxStaticText(this, SLIDER_L_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_l_t, 0, 0);
	slider_l = new wxSlider(this, SLIDER_L, int(drawPane->element.l), 0, 6, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_l, 0, wxEXPAND);


	s.Printf("theta0 : %d", drawPane->element.theta0);
	slider_theta0_t = new wxStaticText(this, SLIDER_Theta0_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_theta0_t, 0, 0);
	slider_theta0 = new wxSlider(this, SLIDER_Theta0, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_theta0, 0, wxEXPAND);

	s.Printf("addA : %.4f", drawPane->element.addA);
	slider_addA_t = new wxStaticText(this, SLIDER_AddA_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_addA_t, 0, 0);
	slider_addA = new wxSlider(this, SLIDER_AddA, int(drawPane->element.addA * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_addA, 0, wxEXPAND);

	s.Printf("addB : %.4f", drawPane->element.addB);
	slider_addB_t = new wxStaticText(this, SLIDER_AddB_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_addB_t, 0, 0);
	slider_addB = new wxSlider(this, SLIDER_AddB, int(drawPane->element.addB * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_addB, 0, wxEXPAND);

	s.Printf("alpha : %.4f", drawPane->processing.alpha);
	slider_alpha_t = new wxStaticText(this, SLIDER_Alpha_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_alpha_t, 0, 0);
	slider_alpha = new wxSlider(this, SLIDER_Alpha, int(drawPane->processing.alpha * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_alpha, 0, wxEXPAND);


	s.Printf("beta : %.4f", drawPane->processing.beta);
	slider_beta_t = new wxStaticText(this, SLIDER_Beta_T, s, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_beta_t, 0, 0);
	slider_beta = new wxSlider(this, SLIDER_Beta, int(drawPane->processing.beta * 1000), 0, 1000, wxDefaultPosition, wxDefaultSize, 0);
	control->Add(slider_beta, 0, wxEXPAND);

	sizer->Add(leftside, 4, wxEXPAND);
	sizer->Add(control, 1, wxEXPAND);
	SetSizer(sizer);

	slider_alpha->Disable();
	slider_beta->Disable();
	fill->Disable();

	render_loop_on = false;
	//activateRenderLoop(render_loop_on);
}

void MyFrame::addlog(wxString info, wxColour& color){
	time_t currentTime;// for logging current time
	struct tm *localTime;// for logging current time
	time(&currentTime);                   // Get the current time
	localTime = localtime(&currentTime);  // Convert the current time to the local time
	wxString s;
	s.Printf("%02d : %02d : %02d  |  %s\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec, info);
	log->SetDefaultStyle(wxTextAttr(color));
	log->AppendText(s);

}

void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}
void MyFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox("Wei-Ching Liu - Computer Science - National Chengchi University",	"About CRD", wxOK | wxICON_INFORMATION);
//	wxMessageBox("\
//---Style----------F-------k   | l\n\
//---------------------------------\n\
//semicircle    | .0375 | .0655 | 1\n\
//spot: spidle  | .0375 | .0655 | 2\n\
//hole : spidle | .0300 | .0546 | 2\n\
//stripe        | .0300 | .0620 | 2\n\
//triangle      | .0375 | .0655 | 3\n\
//spot : quad   | .0375 | .0655 | 4\n\
//water drop    | .0375 | .0655 | 2\n\
//sector        | .0375 | .0655 | 4\n",
//"About CRD", wxOK | wxICON_INFORMATION);

}

void MyFrame::OnOpenSrc(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open image file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL){
		addlog("Load Img Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else{
		wxString s;
		s.Printf("Load Img - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));
	}

	// proceed loading the file chosen by the user;
	// this can be done with e.g. wxWidgets input streams:
	wxFileInputStream input_stream(openFileDialog.GetPath());
	if (!input_stream.IsOk())
	{
		wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
		return;
	}
	drawPane->element.ReadSrc((const char*)openFileDialog.GetPath().mb_str());
	drawPane->SetSize(drawPane->element.Mask.cols, drawPane->element.Mask.rows);

	addlog("Img Loaded", wxColour(*wxBLACK));
}
void MyFrame::OnOpenTex(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open texture file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL){
		addlog("Load Texture Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else{
		wxString s;
		s.Printf("Load Texture - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));
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
}
void MyFrame::OnOpenVfb(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open vfb file"), "", "", "vfb files (*.vfb)|*.vfb", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL){
		addlog("Load Flow Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else{
		wxString s;
		s.Printf("Load Flow(.vfb) - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));
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
	addlog("Flow Loaded", wxColour(*wxBLACK));
}
void MyFrame::OnOpenETF(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open image file"), "", "", "image files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL){
		addlog("Load ETF Canceled", wxColour(*wxBLACK));
		return;     // the user changed idea...
	}
	else{
		wxString s;
		s.Printf("Load ETF - %s", openFileDialog.GetFilename());
		addlog(s, wxColour(*wxBLUE));
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
	addlog("ETF Loaded", wxColour(*wxBLACK));
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

void MyFrame::OnOpenMask(wxCommandEvent& event){
	if (drawPane->element.SrcLoaded) imshow("Maks Img", drawPane->element.Mask);
	else addlog("SrcImg didn't Load !", wxColour(*wxRED));
}
void MyFrame::OnOpenMaskS(wxCommandEvent& event){
	if (drawPane->element.SrcLoaded) imshow("Mask_S Img", drawPane->element.Mask_s);
	else addlog("SrcImg didn't Load !", wxColour(*wxRED));
}
void MyFrame::OnOpenPatternPicker(wxCommandEvent& event){
	wxFrame *patternpicker = new wxFrame(NULL, wxID_ANY, "Pattern Picker", wxPoint(100,100), wxSize(500,300));
	patternpicker->Show();
}

void MyFrame::OnStart(wxCommandEvent& event)
{
	render_loop_on = !render_loop_on;
	activateRenderLoop(render_loop_on);
	
	if (render_loop_on) fill->Enable();
	else fill->Disable();
}
void MyFrame::OnFill(wxCommandEvent& event)
{
	for (int i = 0; i < drawPane->element.c_B->cols/10; i ++){
		//for (int y = 0; y < drawPane->element.c_B->rows; y += drawPane->element.c_B->rows / 10){
		int x = rand() % drawPane->element.c_B->cols, y = rand() % drawPane->element.c_B->rows;
			ellipse(
				*drawPane->element.c_B, // img - Image.
				Point(x , y),           // center - Center of the ellipse.
				Size(1, 1),             // axes - Half of the size of the ellipse main axes.
				0,                      // angle - Ellipse rotation angle in degrees.
				0,                      // startAngle - Starting angle of the elliptic arc in degrees.
				360,                    // endAngle - Ending angle of the elliptic arc in degrees.
				Scalar(0.5, 0.5, 0.5),  // color - Ellipse color.
				3,                      // thickness - Thickness of the ellipse arc outline
				8                       // lineType - Type of the ellipse boundary. See the line() description.
				);
		//}
	}
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

void MyFrame::OnProcessingBox(wxCommandEvent& event)
{
	drawPane->processingS = processingBox->GetValue();
	
	if (drawPane->processingS == "dirTexture" && !drawPane->processing.TextureLoaded){
		addlog("Must Loaded Texture First! (dirTexture)", wxColour(*wxRED));
		drawPane->processingS = "none";
		processingBox->SetSelection(0);
	}

	if (processingBox->GetValue() == "none" || processingBox->GetValue() == "dirTexture"){
		slider_alpha->Disable();
		slider_beta->Disable();
	}
	else{
		slider_alpha->Enable();
		slider_beta->Enable();
	}
	drawPane->paintNow(true); //execute action
}
void MyFrame::OnControllingBox(wxCommandEvent& event)
{
	drawPane->controllingS = controllingBox->GetValue();
}

void MyFrame::OnSliderS(wxCommandEvent& event)
{
	drawPane->element.s = slider_s->GetValue() / 1000.0;
	wxString s;
	s.Printf("Size : %.4f", drawPane->element.s);
	slider_s_t->SetLabel(s);
}
void MyFrame::OnSliderF(wxCommandEvent& event)
{
	drawPane->element.f = slider_f->GetValue() / 1000.0*0.06;
	wxString s;
	s.Printf("F : %.4f", drawPane->element.f);
	slider_f_t->SetLabel(s);
}
void MyFrame::OnSliderK(wxCommandEvent& event)
{
	drawPane->element.k = slider_k->GetValue() / 1000.0*0.04 + 0.03;
	wxString s;
	s.Printf("k : %.4f", drawPane->element.k);
	slider_k_t->SetLabel(s);
}
void MyFrame::OnSliderL(wxCommandEvent& event)
{
	drawPane->element.l = slider_l->GetValue();
	wxString s;
	s.Printf("l : %d", drawPane->element.l);
	slider_l_t->SetLabel(s);
}
void MyFrame::OnSliderTheta0(wxCommandEvent& event)
{
	drawPane->element.theta0 = slider_theta0->GetValue();
	wxString s;
	s.Printf("theta0 : %d", drawPane->element.theta0);
	slider_theta0_t->SetLabel(s);
}
void MyFrame::OnSliderAddA(wxCommandEvent& event)
{
	drawPane->element.addA = slider_addA->GetValue() / 1000.0;
	wxString s;
	s.Printf("addA : %.4f", drawPane->element.addA);
	slider_addA_t->SetLabel(s);
}
void MyFrame::OnSliderAddB(wxCommandEvent& event)
{
	drawPane->element.addB = slider_addB->GetValue() / 1000.0;
	wxString s;
	s.Printf("addB : %.4f", drawPane->element.addB);
	slider_addB_t->SetLabel(s);
}
void MyFrame::OnSliderAlpha(wxCommandEvent& event)
{
	drawPane->processing.alpha = slider_alpha->GetValue() / 1000.0;
	wxString s;
	s.Printf("alpha : %f", drawPane->processing.alpha);
	slider_alpha_t->SetLabel(s);
}
void MyFrame::OnSliderBeta(wxCommandEvent& event)
{
	drawPane->processing.beta = slider_beta->GetValue() / 1000.0;
	wxString s;
	s.Printf("beta : %f", drawPane->processing.beta);
	slider_beta_t->SetLabel(s);
}

void MyFrame::activateRenderLoop(bool on)
{
	if (on )
	{
		start->SetLabel("Stop");
		Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(MyFrame::onIdle));
		render_loop_on = true;
		addlog("-------Start iteration-------", wxColour(*wxBLACK));
	}
	else 
	{
		start->SetLabel("Start");
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

//-------------------------------------------------------------------------------------------------------------------

BasicDrawPane::BasicDrawPane(wxFrame* parent, Size s) :
processing(s),
element(s),
wxPanel(parent)
{
	activateDraw = false;
}

void BasicDrawPane::MouseMove(wxMouseEvent &event)
{
	if (activateDraw)
	{
		if (controllingS == "addA")
			element.Addition_A.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
		else if (controllingS == "addB")
			element.Addition_B.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
		else
			ellipse(
				*element.c_B,         // img - Image.
				Point(event.m_x % element.c_B->cols, event.m_y % element.c_B->rows),// center - Center of the ellipse.
				Size(1, 1),           // axes - Half of the size of the ellipse main axes.
				0,                    // angle - Ellipse rotation angle in degrees.
				0,                    // startAngle - Starting angle of the elliptic arc in degrees.
				360,                  // endAngle - Ending angle of the elliptic arc in degrees.
				Scalar(0.5, 0.5, 0.5),// color - Ellipse color.
				3,                    // thickness - Thickness of the ellipse arc outline
				8                     // lineType - Type of the ellipse boundary. See the line() description.
			);
	}
	//((MyFrame *)GetParent())->addlog("Panit event - Mouse Down at (%.0f, %.0f)", wxColour(*wxBLACK));
}
void BasicDrawPane::MouseLDown(wxMouseEvent &event)
{
	if (controllingS == "addA")
		element.Addition_A.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
	else if (controllingS == "addB")
		element.Addition_B.at<float>(event.m_y%element.c_B->rows, event.m_x%element.c_B->cols) = 1.0;
	else
		ellipse(*element.c_B,
				Point(event.m_x%element.c_B->cols, event.m_y%element.c_B->rows),
				Size(1, 1),
				0,
				0,
				360,
				Scalar(0.5, 0.5, 0.5),
				3,
				8);
	activateDraw = true;

	wxString s;
	s.Printf("Panit event - Mouse Down at (%d, %d)", event.m_x%element.c_B->cols, event.m_y%element.c_B->rows);
	((MyFrame *)GetParent())->addlog(s, wxColour(*wxBLACK));
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

//Global variables for measuring time (in milli-seconds)
int	startTime;
int	prevTime;
int	updateInterval = 20;
//Main Render(iteration) Section
void BasicDrawPane::render(wxDC& dc, bool render_loop_on)
{
	// Measure the elapsed time
	int currTime = static_cast <float> (clock()) ;
	int timeSincePrevFrame = currTime - prevTime;
	int elapsedTime = currTime - startTime;
	prevTime = currTime;

	static int counter = 0;
	if (render_loop_on){
		element.FastGrayScott();
		counter++;
	}

	dis = element.c_A->clone();

	if (processingS == "Motion_Illusion"){
		processing.motionIllu(*element.c_A, element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "dirTexture"){
		processing.dirTexture(*element.c_A, element.Flowfield, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "adaThresholding"){
		processing.adaThresholding(*element.c_A, element.Mask, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else if (processingS == "Thresholding"){
		processing.Thresholding(*element.c_A, dis);
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_RGB2BGR);
	}
	else{
		dis.convertTo(dis, CV_8UC1, 255);
		cvtColor(dis, dis, CV_GRAY2BGR);
	}

	wxImage img(dis.cols, dis.rows, dis.data, true);
	wxBitmap bmp(img);
	dc.DrawBitmap(bmp, 0, 0);


	((MyFrame *)GetParent())->SetStatusText(wxString::Format("Fps: %.0f\t\tframeCounter: %i", 1000.0 / timeSincePrevFrame, counter), 0);
}