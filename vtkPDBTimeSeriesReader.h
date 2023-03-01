///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        vtkPDBTimeSeriesReader.h
//
// Author:      David Borland
//
// Description: A temporal source for a time series of PDB molecule files
//
///////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __vtkPDBTimeSeriesReader_h
#define __vtkPDBTimeSeriesReader_h

#include "vtkPDBReader.h"

#include "vtkStringArray.h"
#include <vtkstd/vector>

class VTK_IO_EXPORT vtkPDBTimeSeriesReader : public vtkPDBReader
{
public:
  vtkTypeRevisionMacro(vtkPDBTimeSeriesReader, vtkPDBReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkPDBTimeSeriesReader *New();

  void AddFileName(const char* fileName);
  void ClearFileNames();

protected:
  vtkPDBTimeSeriesReader();
  ~vtkPDBTimeSeriesReader();

  virtual int RequestInformation(vtkInformation*, 
                                 vtkInformationVector**, 
                                 vtkInformationVector*);

  virtual int RequestData(vtkInformation*, 
                          vtkInformationVector**, 
                          vtkInformationVector*);

  vtkstd::vector<vtkStdString> FileNames;
  vtkstd::vector<double> TimeSteps;

private:
  vtkPDBTimeSeriesReader(const vtkPDBTimeSeriesReader&);  // Not implemented
  void operator=(const vtkPDBTimeSeriesReader&);  // Not implemented
};

#endif