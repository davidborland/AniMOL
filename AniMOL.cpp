///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AniMOL.cpp
//
// Author:      David Borland
//
// Description: Sets up wxWidgets for AniMOL application
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "AniMOL.h"

#include <algorithm>
#include <functional>

#include <wx/dir.h>

#include "RenciLog.h"


// Create the event table for the main frame
BEGIN_EVENT_TABLE(MainFrame, wxFrame)    
    EVT_MENU(MainFrame::MENU_OPEN, MainFrame::OnOpen)
    EVT_MENU(MainFrame::MENU_EXIT, MainFrame::OnExit)

//    EVT_CHECKBOX(MainFrame::SHOW_ATOMS_CHECKBOX, MainFrame::OnCheckBox)
    EVT_CHECKBOX(MainFrame::SHOW_BONDS_CHECKBOX, MainFrame::OnCheckBox)
    EVT_CHECKBOX(MainFrame::SHOW_OUTLINE_CHECKBOX, MainFrame::OnCheckBox)

    EVT_BUTTON(MainFrame::PAUSE_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::PLAY_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::REWIND_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::STEP_FORWARD_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::STEP_BACKWARD_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::JUMP_FORWARD_BUTTON, MainFrame::OnButton)
    EVT_BUTTON(MainFrame::JUMP_BACKWARD_BUTTON, MainFrame::OnButton)

    EVT_SLIDER(MainFrame::TIME_SLIDER, MainFrame::OnSlider)
    
    EVT_CHECKBOX(MainFrame::LOOP_CHECKBOX, MainFrame::OnCheckBox)

    EVT_TIMER(ANIMATION_TIMER, MainFrame::OnTimer)
END_EVENT_TABLE()


// Start the application here
IMPLEMENT_APP(AniMOLApp)


/////////////////////////////////////////////////////////////////////////////////////////////
// AniMOLApp
/////////////////////////////////////////////////////////////////////////////////////////////

// Program execution starts here
bool AniMOLApp::OnInit() {
    // Create the main application window
    MainFrame* frame;

    frame = new MainFrame("AniMOL Controls", argc, argv);

    // Show it.  Frames, unlike simple controls, are not shown initially when created.
    frame->Show();

    // Tell wxWidgets that this is a main frame
    SetTopWindow(frame);

    // Success:  wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// MainFrame
/////////////////////////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame(const wxString& title, int argc, char* argv[], const wxPoint& pos, const wxSize& size) 
: wxFrame(NULL, wxID_ANY, title, pos, size) {
    // Create a log window for printing messages
    RenciLog* log = new RenciLog(this);
    log->Show();


    // Create the VTK render pipeline.  This will create a separate VTK render window.  Using
    // wxVTKRenderWindowInteractor does not work correctly with a borderless window, which is needed
    // for rendering in certain environments, such as RENCI's dome.
    pipeline = new RenderPipeline(argc, argv);


    // Create the file menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(MENU_OPEN, "&Open Files\tAlt+O", "Open files");
    fileMenu->Append(MENU_EXIT, "E&xit\tAlt+X", "Exit");

    // Add the menus to the menu bar
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);


    // Add a panel to hold widgets
    wxPanel* panel = new wxPanel(this);


    // Create the widgets
//    showAtomsCheckBox = new wxCheckBox(panel, SHOW_ATOMS_CHECKBOX, "Show &Atoms");
    showBondsCheckBox = new wxCheckBox(panel, SHOW_BONDS_CHECKBOX, "Show &Bonds");
    showOutlineCheckBox = new wxCheckBox(panel, SHOW_OUTLINE_CHECKBOX, "Show &Outline");

    beginButton = new wxButton(panel, JUMP_BACKWARD_BUTTON, "<<");
    rewindButton = new wxButton(panel, REWIND_BUTTON, "<");
    stepBackwardButton = new wxButton(panel, STEP_BACKWARD_BUTTON, "<");
    stepBackwardButton->SetForegroundColour(*wxLIGHT_GREY);
    pauseButton = new wxButton(panel, PAUSE_BUTTON, "\"");
    stepForwardButton = new wxButton(panel, STEP_FORWARD_BUTTON, ">");
    stepForwardButton->SetForegroundColour(*wxLIGHT_GREY);
    playButton = new wxButton(panel, PLAY_BUTTON, ">");
    endButton = new wxButton(panel, JUMP_FORWARD_BUTTON, ">>");
    wxBoxSizer* playSizer = new wxBoxSizer(wxHORIZONTAL);
    playSizer->Add(beginButton);
    playSizer->Add(rewindButton);
    playSizer->Add(stepBackwardButton);
    playSizer->Add(pauseButton);
    playSizer->Add(stepForwardButton);
    playSizer->Add(playButton);
    playSizer->Add(endButton);

    timeSlider = new wxSlider(panel, TIME_SLIDER, 0, 0, 0, wxDefaultPosition, wxDefaultSize, 
                              wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS | wxSL_TOP);

    loopCheckBox = new wxCheckBox(panel, LOOP_CHECKBOX, "Loop");


    // Add the widgets to a sizer
    int border = 5;
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
//    sizer->Add(showAtomsCheckBox, 0, wxEXPAND | wxALL, border);
    sizer->Add(showBondsCheckBox, 0, wxEXPAND | wxALL, border);
    sizer->Add(showOutlineCheckBox, 0, wxEXPAND | wxALL, border);
    sizer->Add(playSizer, 0, wxEXPAND | wxALL, border);
    sizer->Add(timeSlider, 0, wxEXPAND | wxALL, border);
    sizer->Add(loopCheckBox, 0, wxEXPAND | wxALL, border);

    panel->SetSizer(sizer);
    sizer->SetSizeHints(this);


    // Create a timer for animation
    animationTimer = new wxTimer(this, ANIMATION_TIMER);

    // Start the timer.  For some reason, creating a continuous timer results in an issue with closing
    // the render frame.  Just use a one-shot timer and restart it every timer callback.
    animationTimer->Start(100, wxTIMER_ONE_SHOT);


    // Default values for widgets
    ResetWidgets();
    SetPipelineFromWidgets();
}


MainFrame::~MainFrame() {
    // Clean up
    delete animationTimer;
    delete pipeline;
}

// Comparison for correctly sorting file names in OnOpen()
struct FileNameCompare : public std::binary_function<std::string, std::string, bool> {
    bool operator()(const std::string& s1, const std::string& s2) { 
        return s1.length() == s2.length() ? s1 < s2 : s1.length() < s2.length();
    }
};

void MainFrame::OnOpen(wxCommandEvent& WXUNUSED(e)) {
    // Display the choose file dialog
    wxFileDialog* fileDialog = new wxFileDialog(this, "Open a file series", "", "", "PDB files (.pdb)|*.pdb",
                                                wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE | wxFD_CHANGE_DIR);
    fileDialog->ShowModal();

    // Get the directory
    std::string directory = fileDialog->GetDirectory().c_str();

    // Get the file names
    wxArrayString pathArray;
    fileDialog->GetFilenames(pathArray);

    // Done with the fileDialog
    fileDialog->Destroy();


    // Check for valid number
    if (pathArray.GetCount() <= 0) return;


    // If one file is selected, get all files in the directory
    if (pathArray.GetCount() == 1) {             
        pathArray.clear();
        wxDir::GetAllFiles(directory, &pathArray, "*.pdb", wxDIR_FILES);
    }

    
    // Sort the file names correctly    
    std::vector<std::string> fileNames;
    for (int i = 0; i < (int)pathArray.GetCount(); i++) {
        fileNames.push_back(pathArray[i].c_str());
    }
    std::sort(fileNames.begin(), fileNames.end(), FileNameCompare());


    // Set the range for the time slider
    timeSlider->SetRange(0, fileNames.size() - 1);
    
    // This will fire the sizer layout command to correctly show the slider range
    int w, h;
    GetSize(&w, &h);
    SetSize(w + 1, h);
    SetSize(w, h );

    // Load the data
    pipeline->LoadData(fileNames);
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(e)) {
    // True forces the frame to close
    Close(true);
}

void MainFrame::OnCheckBox(wxCommandEvent& e) {
/*
    if (e.GetId() == SHOW_ATOMS_CHECKBOX) {
        pipeline->ShowAtoms(e.IsChecked());
    }
*/
    if (e.GetId() == SHOW_BONDS_CHECKBOX) {        
        pipeline->ShowBonds(e.IsChecked());
    }
    else if (e.GetId() == SHOW_OUTLINE_CHECKBOX) {
        pipeline->ShowOutline(e.IsChecked());
    }
    else if (e.GetId() == LOOP_CHECKBOX) {
        pipeline->Loop(e.IsChecked());
    }
}

void MainFrame::OnButton(wxCommandEvent& e) {
    if (e.GetId() == PAUSE_BUTTON) {
        pipeline->Pause();
    }
    else if (e.GetId() == PLAY_BUTTON) {
        pipeline->Play();
    }
    else if (e.GetId() == REWIND_BUTTON) {
        pipeline->Rewind();
    }
    else if (e.GetId() == STEP_FORWARD_BUTTON) {
        pipeline->StepForward();
    }
    else if (e.GetId() == STEP_BACKWARD_BUTTON) {
        pipeline->StepBackward();
    }
    else if (e.GetId() == JUMP_FORWARD_BUTTON) {
        pipeline->JumpForward();
    }
    else if (e.GetId() == JUMP_BACKWARD_BUTTON) {
        pipeline->JumpBackward();
    }
}

void MainFrame::OnSlider(wxCommandEvent& e) {
    if (e.GetId() == TIME_SLIDER) {
        pipeline->SetTimeStep(timeSlider->GetValue());
    }
}

void MainFrame::OnTimer(wxTimerEvent& e) {
    if (e.GetId() == ANIMATION_TIMER) {
        pipeline->UpdateAnimation();
        pipeline->Render();

        timeSlider->SetValue(pipeline->GetTimeStep());

        // Set the timer.  -1 uses the current timer interval
        animationTimer->Start(-1, wxTIMER_ONE_SHOT);
    }
}


void MainFrame::ResetWidgets() {
//    showAtomsCheckBox->SetValue(true);
    showBondsCheckBox->SetValue(true);
    showOutlineCheckBox->SetValue(true);

    loopCheckBox->SetValue(false);
}


void MainFrame::SetPipelineFromWidgets() {
//    pipeline->ShowAtoms(showAtomsCheckBox->GetValue());
    pipeline->ShowBonds(showBondsCheckBox->GetValue());
    pipeline->ShowOutline(showOutlineCheckBox->GetValue());

    pipeline->Loop(loopCheckBox->GetValue());
}