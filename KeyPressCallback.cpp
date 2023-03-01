///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        KeyPressCallback.cpp
//
// Author:      David Borland
//
// Description: Key press callback for AniMOL application
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "KeyPressCallback.h"

#include "RenderPipeline.h"
#include "vtkRenderWindowInteractor.h"


KeyPressCallback::KeyPressCallback() {
    pipeline = NULL;
}

KeyPressCallback* KeyPressCallback::New() {
    return new KeyPressCallback;
}


void KeyPressCallback::Execute(vtkObject* caller, unsigned long eventId, void* callData) {
    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (pipeline) {
        if (eventId == KeyPressEvent) {
            char c = interactor->GetKeyCode();
            
            if (c == 'b') {
                pipeline->MoleculeBackward();
            }
            else if (c == 'f') {
                pipeline->MoleculeForward();
            }
        }
    }
}


void KeyPressCallback::SetRenderPipeline(RenderPipeline* pipe) {
    pipeline = pipe;
}