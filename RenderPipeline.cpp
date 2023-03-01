///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        RenderPipeline.cpp
//
// Author:      David Borland
//
// Description: Interface of the VTK rendering pipeline for Moleculizer.
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "RenderPipeline.h"

#include <vtkAlgorithm.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkCompositePolyDataMapper2.h>
#include <vtkCubeSource.h>
//#include <vtkGlyph3D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenciRenderWindowManager.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkTemporalDataSetCache.h>
#include <vtkTubeFilter.h>

#include "KeyPressCallback.h"


RenderPipeline::RenderPipeline(int argc, char* argv[]) {
    // Initialize animation variables
    timeStep = 0.0;
    timeIncrement = 1.0;
    numTimeSteps = 0;
    loop = false;
    animationState = PAUSE;

    // Initialize molecule variables
    numMolecules = 0;
    moleculeIndex = 0;


    // Need a composite data pipeline for time-series data
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype(prototype);


    // The file reader
    reader = vtkPDBTimeSeriesReader::New();


    // Cache for the time-series data
    vtkTemporalDataSetCache* cache = vtkTemporalDataSetCache::New();
    cache->SetInputConnection(reader->GetOutputPort());
    cache->SetCacheSize(100);


    // Scale for graphics objects
    double scale = 0.2;


    // Atoms
/*
    vtkSphereSource* sphere = vtkSphereSource::New();
    sphere->SetRadius(1.0);
    sphere->SetThetaResolution(6);
    sphere->SetPhiResolution(6);

    vtkGlyph3D* atom = vtkGlyph3D::New();
    atom->SetSourceConnection(sphere->GetOutputPort());
    atom->SetInputConnection(cache->GetOutputPort());
    atom->SetScaleModeToScaleByVector();
    atom->SetScaleFactor(scale);
    atom->SetColorModeToColorByScalar(); 

    vtkCompositePolyDataMapper2* atomMapper = vtkCompositePolyDataMapper2::New();
    atomMapper->SetInputConnection(atom->GetOutputPort());

    atomActor = vtkActor::New();
    atomActor->SetMapper(atomMapper); 
//    atomActor->GetProperty()->LoadMaterial("perPixelLighting.xml");
//    atomActor->GetProperty()->ShadingOn();
*/

    // Bonds
    vtkTubeFilter* bond = vtkTubeFilter::New();
    bond->SetInputConnection(cache->GetOutputPort());
    bond->CappingOn();
    bond->SetNumberOfSides(6);
    bond->SetRadius(scale);

    vtkCompositePolyDataMapper2* bondMapper = vtkCompositePolyDataMapper2::New();
    bondMapper->SetInputConnection(bond->GetOutputPort());

    bondActor = vtkActor::New();
    bondActor->SetMapper(bondMapper); 
//    bondActor->GetProperty()->LoadMaterial("perPixelLighting.xml");
//    bondActor->GetProperty()->ShadingOn();


    // Single molecule    
    connectivity = vtkPolyDataConnectivityFilter::New();
    connectivity->SetInputConnection(cache->GetOutputPort());
    connectivity->SetExtractionModeToSpecifiedRegions();
    connectivity->AddSpecifiedRegion(moleculeIndex);

    vtkTubeFilter* molecule = vtkTubeFilter::New();
    molecule->SetInputConnection(connectivity->GetOutputPort());
    molecule->SetNumberOfSides(6);
    molecule->SetRadius(scale * 1.5);

    vtkCompositePolyDataMapper2* moleculeMapper = vtkCompositePolyDataMapper2::New();
    moleculeMapper->SetInputConnection(molecule->GetOutputPort());
    moleculeMapper->ScalarVisibilityOff();

    moleculeActor = vtkActor::New();
    moleculeActor->SetMapper(moleculeMapper); 
    moleculeActor->GetProperty()->SetColor(1, 0, 0);
    moleculeActor->GetProperty()->SetAmbient(0.1);
    moleculeActor->GetProperty()->SetDiffuse(1.0);
    moleculeActor->GetProperty()->SetSpecular(0);


    // Outline
    vtkOutlineSource* outline = vtkOutlineSource::New();
    double cubeSize = 21.1175;
    outline->SetBounds(-cubeSize * 0.5, cubeSize * 0.5, 
                       -cubeSize * 0.5, cubeSize * 0.5,  
                       -cubeSize * 0.5, cubeSize * 0.5);

    vtkTubeFilter* outlineTube = vtkTubeFilter::New(); 
    outlineTube->SetInputConnection(outline->GetOutputPort());
    outlineTube->CappingOn();
    outlineTube->SetNumberOfSides(6);
    outlineTube->SetVaryRadiusToVaryRadiusOff();
    outlineTube->SetRadius(scale * 0.5);

    vtkPolyDataMapper* outlineMapper = vtkPolyDataMapper::New();
    outlineMapper->SetInputConnection(outlineTube->GetOutputPort());

    outlineActor = vtkActor::New();
    outlineActor->SetMapper(outlineMapper); 
    outlineActor->GetProperty()->SetAmbient(0.25);
    outlineActor->GetProperty()->SetDiffuse(0.25);
    outlineActor->GetProperty()->SetSpecular(0.0);


    // Outline faces
    vtkCubeSource* cube = vtkCubeSource::New();
    cube->SetXLength(cubeSize);
    cube->SetYLength(cubeSize);
    cube->SetZLength(cubeSize);

    vtkPolyDataMapper* cubeMapper = vtkPolyDataMapper::New();
    cubeMapper->SetInputConnection(cube->GetOutputPort());

    cubeActor = vtkActor::New();
    cubeActor->SetMapper(cubeMapper);
    cubeActor->GetProperty()->FrontfaceCullingOn();    
    cubeActor->GetProperty()->SetAmbient(0.1);
    cubeActor->GetProperty()->SetDiffuse(0.1);
    cubeActor->GetProperty()->SetSpecular(0.0);
    cubeActor->GetProperty()->SetOpacity(0.9);


    // Set the pipelines for time-series data
//    atomPipeline = vtkStreamingDemandDrivenPipeline::SafeDownCast(atom->GetExecutive());
    bondPipeline = vtkStreamingDemandDrivenPipeline::SafeDownCast(bond->GetExecutive());
    moleculePipeline = vtkStreamingDemandDrivenPipeline::SafeDownCast(molecule->GetExecutive());

    // Create the RENCI render window manager
    vtkRenciRenderWindowManager* manager = vtkRenciRenderWindowManager::New();

    // Create a renderer
    renderer = manager->GetRenderer();
    renderer->AddViewProp(outlineActor);
    renderer->AddViewProp(cubeActor);

    // Create the window
    window = manager->GetRenciRenderWindow(argc, argv);
    window->AddRenderer(renderer);
    window->SetWindowName("AniMOL");

    // Use a trackball camera style
    vtkInteractorStyleTrackballCamera* interactorStyle = vtkInteractorStyleTrackballCamera::New();

    // Create the interactor
    interactor = vtkRenderWindowInteractor::New();
    interactor->SetRenderWindow(window);
    interactor->SetInteractorStyle(interactorStyle);

    // Create the key press callback
    KeyPressCallback* keyPressCallback = KeyPressCallback::New();
    keyPressCallback->SetRenderPipeline(this);
    interactor->AddObserver(vtkCommand::KeyPressEvent, keyPressCallback);

    // Start interaction
    renderer->ResetCamera();
    interactor->Initialize();


    // Delete objects not needed any more
    prototype->Delete();
    reader->Delete();
    cache->Delete();
//    sphere->Delete();
//    atom->Delete();
//    atomMapper->Delete();
    bond->Delete();
    bondMapper->Delete();
    outline->Delete();
    outlineTube->Delete();
    outlineMapper->Delete();
    cube->Delete();
    cubeMapper->Delete();
    manager->Delete();
    interactorStyle->Delete();
    keyPressCallback->Delete();
}

RenderPipeline::~RenderPipeline() {
    // Clean up
    vtkAlgorithm::SetDefaultExecutivePrototype(0);

//    atomActor->Delete();
    bondActor->Delete();
    outlineActor->Delete();
    cubeActor->Delete();

    connectivity->Delete();

    renderer->Delete();
    window->Delete();
    interactor->Delete();
}


void RenderPipeline::Render() {
    renderer->ResetCameraClippingRange();
    interactor->Render();
}


/*
void RenderPipeline::ShowAtoms(bool show)  {
    atomActor->SetVisibility(show);
}
*/

void RenderPipeline::ShowBonds(bool show)  {
    bondActor->SetVisibility(show);
}

void RenderPipeline::ShowOutline(bool show)  {
    outlineActor->SetVisibility(show);
    cubeActor->SetVisibility(show);
}


void RenderPipeline::UpdateAnimation() {
    if (numTimeSteps <= 0 || animationState == PAUSE) return;

    // Increment the time step
    if (animationState == PLAY) {
        timeStep += timeIncrement;

        // Bounds checking
        if (timeStep > numTimeSteps - 1) {
            if (loop) {
                timeStep = 0;
            }
            else {
                timeStep = numTimeSteps - 1;
                Pause();
            }
        }
    }
    else if (animationState == REWIND) {
        timeStep -= timeIncrement;
    
        // Bounds checking
        if (timeStep < 0) {
            if (loop) {
                timeStep = numTimeSteps - 1;
            }
            else {
                timeStep = 0;
                Pause();
            }
        }
    }


    // Update the pipelines
    UpdatePipelines(timeStep);
}


void RenderPipeline::Pause() {
    animationState = PAUSE;
}

void RenderPipeline::Play() {
    animationState = PLAY;
}

void RenderPipeline::Rewind() {
    animationState = REWIND;
}

void RenderPipeline::StepForward() {
    if (numTimeSteps <= 0) return;

    timeStep = (int)timeStep + 1;

    // Bounds checking
    if (timeStep > numTimeSteps - 1) {
        timeStep = numTimeSteps - 1;
    }

    Pause();

    // Update the pipelines
    UpdatePipelines(timeStep);
}

void RenderPipeline::StepBackward() {
    if (numTimeSteps <= 0) return;

    timeStep = (int)timeStep - 1;

    // Bounds checking
    if (timeStep < 0) {
        timeStep = 0;
    }

    Pause();

    // Update the pipelines
    UpdatePipelines(timeStep);
}

void RenderPipeline::JumpForward() {
    if (numTimeSteps <= 0) return;

    timeStep = numTimeSteps - 1;

    Pause();

    // Update the pipelines
    UpdatePipelines(timeStep);
}

void RenderPipeline::JumpBackward() {
    if (numTimeSteps <= 0) return;

    timeStep = 0;

    Pause();

    // Update the pipelines
    UpdatePipelines(timeStep);
}


void RenderPipeline::SetTimeStep(int time) {
    if (numTimeSteps <= 0) return;

    timeStep = time;

    Pause();

    // Update the pipelines
    UpdatePipelines(timeStep);
}

int RenderPipeline::GetTimeStep() {
    return timeStep;
}


void RenderPipeline::Loop(bool loopAnimation) {
    loop = loopAnimation;
}


void RenderPipeline::MoleculeForward() {
    connectivity->DeleteSpecifiedRegion(moleculeIndex);
    moleculeIndex++;
    moleculeIndex = moleculeIndex >= numMolecules ? 0 : moleculeIndex;
    connectivity->AddSpecifiedRegion(moleculeIndex);
}

void RenderPipeline::MoleculeBackward() {
    connectivity->DeleteSpecifiedRegion(moleculeIndex);
    moleculeIndex--;
    moleculeIndex = moleculeIndex < 0 ? numMolecules - 1 : moleculeIndex;
    connectivity->AddSpecifiedRegion(moleculeIndex);
}


void RenderPipeline::LoadData(const std::vector<std::string>& fileNames) {
    reader->ClearFileNames();
    numTimeSteps = 0;

    for (int i = 0; i < (int)fileNames.size(); i++) {
        reader->AddFileName(fileNames[i].c_str());
        numTimeSteps++;
    }
    reader->Update();
    timeStep = 0;

    // XXX : Should read this from connectivity?
    numMolecules = 64;

    Pause();

//    renderer->AddViewProp(atomActor);
    renderer->AddViewProp(bondActor);
    renderer->AddViewProp(moleculeActor);

    renderer->ResetCamera();
    Render();
}


void RenderPipeline::UpdatePipelines(double timeStep) {
//    atomPipeline->SetUpdateTimeStep(0, timeStep);
    bondPipeline->SetUpdateTimeStep(0, timeStep);
    moleculePipeline->SetUpdateTimeStep(0, timeStep);
}