#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkCellData.h>
#include <vtkTriangle.h>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vtkPointData.h>
#include <ExtractLabelSurfacesCLP.h>
#include <list>
#include <map>
#include <stdlib.h>
#include <sstream>

int ExtractPointData(std::string vtkLabelFile, std::string labelNameInfo, std::string arrayName) ;
int TranslateToLabelNumber(std::string labelNameInfo, std::string labelNumberInfo) ;
int CreateSurfaceLabelFiles(std::string vtkFile, std::string labelNumberInfo) ;
vtkSmartPointer<vtkPolyData> ReadVTKFile(std::string vtkFile) ;
