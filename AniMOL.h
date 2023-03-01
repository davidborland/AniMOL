///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        AniMOL.h
//
// Author:      David Borland
//
// Description: Sets up wxWidgets for AniMOL application
//
///////////////////////////////////////////////////////////////////////////////////////////////


#ifndef AniMOL_H
#define AniMOL_H


#include <wx/wx.h>

#include "RenderPipeline.h"
#include "wxVTKRenderWindowInteractor.h"


// Forward declarations
class RenderFrame;


/////////////////////////////////////////////////////////////////////////////////////////////
// AniMOLApp
/////////////////////////////////////////////////////////////////////////////////////////////

class AniMOLApp : public wxApp {
public:
    // Program execution starts here
    bool OnInit();
};


/////////////////////////////////////////////////////////////////////////////////////////////
// MainFrame
/////////////////////////////////////////////////////////////////////////////////////////////

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title, int argc, char* argv[], const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MainFrame();

    // Event handlers
    void OnOpen(wxCommandEvent& e);
    void OnExit(wxCommandEvent& e);
    void OnCheckBox(wxCommandEvent& e);
    void OnButton(wxCommandEvent& e);
    void OnSlider(wxCommandEvent& e);
    void OnTimer(wxTimerEvent& e);

protected:
    // Render frame for rendering using VTK
    RenderFrame* renderFrame;

    // The VTK pipeline
    RenderPipeline* pipeline;

    // Widgets
//    wxCheckBox* showAtomsCheckBox;
    wxCheckBox* showBondsCheckBox;
    wxCheckBox* showOutlineCheckBox;

    wxButton* pauseButton;
    wxButton* playButton;
    wxButton* rewindButton;
    wxButton* stepForwardButton;
    wxButton* stepBackwardButton;
    wxButton* beginButton;
    wxButton* endButton;

    wxSlider* timeSlider;

    wxCheckBox* loopCheckBox;

    // Timer
    wxTimer* animationTimer;

    // Event IDs
    enum {
        // Menu
        MENU_OPEN,
        MENU_EXIT,

        // Widgets
//        SHOW_ATOMS_CHECKBOX,
        SHOW_BONDS_CHECKBOX,
        SHOW_OUTLINE_CHECKBOX,

        PAUSE_BUTTON,
        PLAY_BUTTON,
        REWIND_BUTTON,
        STEP_FORWARD_BUTTON,
        STEP_BACKWARD_BUTTON,
        JUMP_FORWARD_BUTTON,
        JUMP_BACKWARD_BUTTON,

        TIME_SLIDER,

        LOOP_CHECKBOX,

        // Timer
        ANIMATION_TIMER
    };

    void ResetWidgets();
    void SetPipelineFromWidgets();

    // Any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};


#endif