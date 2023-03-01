///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        vtkPDBTimeSeriesReader.cxx
//
// Author:      David Borland
//
// Description: A temporal source for a time series of PDB molecule files
//
///////////////////////////////////////////////////////////////////////////////////////////////


#include "vtkPDBTimeSeriesReader.h"

#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkPDBTimeSeriesReader, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkPDBTimeSeriesReader);

vtkPDBTimeSeriesReader::vtkPDBTimeSeriesReader()
{
}

vtkPDBTimeSeriesReader::~vtkPDBTimeSeriesReader()
{
}

void vtkPDBTimeSeriesReader::AddFileName(const char* fileName)
{
  this->FileNames.push_back(fileName);
  this->TimeSteps.push_back(this->TimeSteps.size());

  if (this->FileNames.size() == 1) 
    {
    this->SetFileName(fileName);
    }
}

void vtkPDBTimeSeriesReader::ClearFileNames() 
{
  this->FileNames.clear();
  this->TimeSteps.clear();

  this->SetFileName("");
}

void vtkPDBTimeSeriesReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Names and Time Steps:" << endl;
  for (int i = 0; i < (int)this->FileNames.size(); i++) 
    {
    os << indent << indent << this->FileNames[i] << indent << this->TimeSteps[i] << endl;
    }
}

int vtkPDBTimeSeriesReader::RequestInformation(vtkInformation* request, 
                                               vtkInformationVector** inVector, 
                                               vtkInformationVector* outVector)
{
  if(!this->Superclass::RequestInformation(request, inVector, outVector))
    {
    return 0;
    }

  // Check for multiple time steps
  if (this->TimeSteps.size() < 2)
    {
    return 1;
    }

  // Get the information object for populating with time information
  vtkInformation *outInfo = outVector->GetInformationObject(0);

  // Tell the caller that the filter can provide time varying data and 
  // tell it what range of times it can deal with
  double range[2];
  range[0] = this->TimeSteps.front();
  range[1] = this->TimeSteps.back();
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), range, 2);

  // Give the caller the available timesteps
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &this->TimeSteps[0], this->TimeSteps.size());

  return 1;
}

int vtkPDBTimeSeriesReader::RequestData(vtkInformation* request,
                                        vtkInformationVector** inVector,
                                        vtkInformationVector* outVector)
{
  vtkInformation* outInfo = outVector->GetInformationObject(0);
  vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    double requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())[0];

    // Find the time
    int t;
    for (t = 0; t < (int)this->TimeSteps.size(); t++) 
      {
      if (requestedTimeValue <= this->TimeSteps[t]) break;
      }

    output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &this->TimeSteps[t], 1);

    // Load this data
    this->SetFileName(this->FileNames[t]);
    }

  return Superclass::RequestData(request, inVector, outVector);
}