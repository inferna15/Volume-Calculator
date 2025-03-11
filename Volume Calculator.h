#pragma once

#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkAutoInit.h>