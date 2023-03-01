///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        KeyPressCallback.h
//
// Author:      David Borland
//
// Description: Key press callback for AniMOL application
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include <vtkCommand.h>

#include "RenderPipeline.h"


class KeyPressCallback : public vtkCommand {
public:
    KeyPressCallback();
    static KeyPressCallback* New();

    virtual void Execute(vtkObject* caller, unsigned long eventId, void* callData);

    void SetRenderPipeline(RenderPipeline* pipe);

private:
    RenderPipeline* pipeline;
};