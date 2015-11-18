#include "ExtractLabelSurfaces.h"

/*ExtractLabelSurfaces program*/

//Tool 1 : ExtractPointData label from a vtkfile -> create a file containing the label name/value for each point
int ExtractPointData ( std::string vtkLabelFile , std::string labelNameInfo , std::string arrayName )
{
    std::cout << "Start ExtractPointData..." << std::endl ;

    //Read VTK file
    vtkSmartPointer <vtkPolyData> polyData = vtkSmartPointer <vtkPolyData>::New() ;
    polyData = ReadVTKFile( vtkLabelFile ) ;

    //Checked that array name specified exists in this vtk file
    vtkPointData* pointdata =  polyData->GetPointData() ;
    int arrayId = 0 ;
    bool arrayFound = false ;

    //Search all arrays in file
    for( unsigned i = 0 ; i < pointdata->GetNumberOfArrays() ; i++ )
    {
        if( pointdata->GetArrayName(i) == arrayName )
        {
            arrayId = i ;
            arrayFound = true ;
        }
    }
    //Checked that the array specified exists
    if( arrayFound != true )
    {
        std::cout << "Array containing label specified not found" << std::endl ;
        return EXIT_FAILURE ;
    }

    //Find number of component for the array specified
    int arrayType = pointdata->GetArray(arrayId)->GetNumberOfComponents() ;
    std::cout << "Array name : " << arrayName << "\nNumber of components : " << arrayType << std::endl ;

    //Write in outputFile
    std::ofstream outputFile ;
    outputFile.open( labelNameInfo.c_str() , std::ofstream::out ) ;
    if( outputFile.good() )
    {
        outputFile << "#Text file containing label name information for each point - output of ExtractPointData tool \n" ;

        //Write in the file all labels corresponding to each points - in the same order
        for( vtkIdType j = 0 ; j < polyData->GetNumberOfPoints() ; j++)
        {
            double vec[arrayType] ;
            pointdata->GetArray(arrayId)->GetTuple(j,vec) ;
            for(int k = 0 ; k < arrayType ; k++)
            {
                outputFile << vec[k] ;
                if( k < arrayType-1 )
                {
                    outputFile << " " ;
                }
            }
            outputFile << "\n" ;
        }
        outputFile.close() ;
    }
    else
    {
        std::cout << "Unable to open file" << std::endl ;
        return EXIT_FAILURE ;
    }
    std::cout << "ExtractPointData Done !\n" << std::endl ;
    return EXIT_SUCCESS ;
}

//Tool 2 : TranslateToLabelNumber -> create a file containing the label number for each point
int TranslateToLabelNumber ( std::string labelNameInfo , std::string labelNumberInfo, bool useTranslationTable )
{
    std::cout << "Start TranslateToLabelNumber..." << std::endl ;

    std::ifstream inputFile ;
    inputFile.open( labelNameInfo.c_str() , std::ios::in ) ;
    std::ofstream outputFile ;
    outputFile.open( labelNumberInfo.c_str() , std::ios::out ) ;

    std::string labelLine ;
    //Define map containing labels and a list of points associated to them
    std::map <std::string , int> labelMap ;
    //Map iterator
    std::map <std::string , int >::const_iterator mit,mend ;
    std::vector <std::pair <int , std::string> > labelPair ;
    std::vector <std::pair <int , std::string> >::const_iterator vit,vend ;

    int labelNumber = 0 ;

    //Extract labels and points associated
    if( inputFile.good() ) //file can be open
    {
        if( outputFile.good() )
        {
            outputFile << "#Text file containing label number information for each point - output of TranslateToLabelNumber tool \n" ;
            do
            {
                getline( inputFile , labelLine ) ; //get first lines and checked if header of not
            }while( labelLine[0] == '#' ) ;

            do
            {
                std::string newlabel = labelLine ;
                if( labelMap.find(labelLine) != labelMap.end() )
                {
                    outputFile << labelMap[newlabel]<<"\n" ;
                }
                else
                {
                    labelNumber++ ;
                    labelMap[newlabel] = labelNumber ;
                    outputFile << labelMap[newlabel] << "\n" ;
                }
                getline( inputFile , labelLine ) ;
            }while( !inputFile.eof() ) ;

        }
        else
        {
            std::cout << "Cannot open  outputfile " << labelNumberInfo << std::endl ;
            return EXIT_FAILURE ;
        }
        outputFile.close() ;
    }
    else
    {
        std::cout << "Cannot open the inputfile " << std::endl ;
        return EXIT_FAILURE ;
    }

    //Create a vector of pair in order to sort labels by value in the log file
    for( mit = labelMap.begin() , mend = labelMap.end() ; mit != mend ; ++mit )
    {
        labelPair.push_back( std::make_pair( mit->second , mit->first ) ) ;
    }
    std::sort( labelPair.begin() , labelPair.end() ) ;


    //Write matching table between labels names and labels numbers
    std::ofstream logFile ;
    logFile.open( "logLabelTable" , std::ios::out ) ;

    if( logFile.good() )
    {
        logFile << "Matching table between labels names and labels numbers \n" ;
        for( vit = labelPair.begin() , vend = labelPair.end() ; vit != vend ; ++vit )
        {
            logFile << vit->first << " : "<< vit->second << "\n" ;
        }
    }
    else
    {
        std::cout << "Cannot open logFile" << labelNumberInfo << std::endl ;
        return EXIT_FAILURE ;
    }
    logFile.close();

    std::cout << "Number of labels : " << labelNumber << std::endl ;
    std::cout << "TranslateToLabelNumber Done !\n" << std::endl ;
    return EXIT_SUCCESS ;
}

//Tool 3 : CreateSurfaceLabelFiles -> create one ASCII file per label
int CreateSurfaceLabelFiles ( std::string vtkFile , std::string labelNumberInfo , std::string prefix, bool overlapping )
{
    std::cout << "Start CreateSurfaceLabelFiles..." << std::endl ;

    //Read labelInformation file
    std::ifstream inputFile ;
    inputFile.open( labelNumberInfo.c_str() , std::ios::in ) ;
    std::string labelLine ;
    int nbLinesLabels = 0 ;
    std::vector <std::string> labelVect ;
    //List of polygons
    vtkSmartPointer <vtkIdList> cellIdList = vtkSmartPointer <vtkIdList>::New() ;

    //Count number of label informations and checked if file can be open
    if( inputFile.good() )
    {
        do
        {
            getline( inputFile , labelLine ) ; //get information line
        }while( labelLine[0] == '#' ) ;

        do
        {
            if( !labelLine.empty() )
            {
                nbLinesLabels++ ;

            }
            getline( inputFile , labelLine ) ;
        }while( !inputFile.eof() ) ;
    }
    else
    {
        std::cout << "Cannot open the label file " << std::endl ;
        return EXIT_FAILURE ;
    }
    inputFile.close() ;
    // ReadFile vtkFile
    vtkSmartPointer <vtkPolyData> polyData = vtkSmartPointer <vtkPolyData>::New() ;
    polyData = ReadVTKFile( vtkFile ) ;
    int nbPoints = 0 ;
    nbPoints = polyData->GetNumberOfPoints() ;

    //Check nb point vtkFile = nb point labelNumber
    if(nbPoints != nbLinesLabels)
    {
        std::cout << "Number of points in the VTK file aren't match with the number of labels information in the text file" << std::endl ;
        return EXIT_FAILURE ;
    }

    //Add new array in vtkFile containing labelId informations for each points
    vtkSmartPointer <vtkIntArray> index = vtkSmartPointer <vtkIntArray>::New() ;
    index->SetNumberOfComponents(1) ;
    index->SetName( "indexLabel" ) ;
    inputFile.open( labelNumberInfo.c_str() , std::ios::in ) ;
    if( inputFile.good() )
    {
        do
        {
            getline( inputFile , labelLine ) ; //get information line
        }while( labelLine[0] =='#' ) ;
        //Create new array containing index label in the vtkFile

        for( int i=0 ; i < nbPoints ; i++ )
        {
            std::vector <std::string>::iterator it ;
            int valIndex = atoi( labelLine.c_str() ) ;
            it = find( labelVect.begin() , labelVect.end() ,labelLine ) ;
            if( it == labelVect.end() )
            {
                labelVect.push_back( labelLine ) ;
            }
            index->InsertNextValue( valIndex ) ;
            getline( inputFile , labelLine ) ;
        }
    }
    else
    {
        std::cout << "Cannot open the label file " << std::endl ;
        return EXIT_FAILURE ;
    }
    inputFile.close() ;
    polyData->GetPointData()->AddArray(index) ;

    //Get array "indexLabel" id
    vtkPointData* pointdata =  polyData->GetPointData() ;
    int arrayId = 0 ;
    for( unsigned i = 0 ; i < pointdata->GetNumberOfArrays() ; i++ )
    {
        if( pointdata->GetArrayName(i) == "indexLabel" )
        {
            arrayId = i ;
        }
    }
    std::string directory = "labelSurfaces" ;
    vtksys::SystemTools::MakeDirectory(directory) ;
    //Threshold the polyData
    for( int k=0 ; k < labelVect.size() ; k++ )
    {
        //Threshold points
        vtkSmartPointer <vtkThresholdPoints> thresholdPoints= vtkSmartPointer <vtkThresholdPoints>::New() ;
        thresholdPoints->SetInputData(polyData) ;
        thresholdPoints->ThresholdBetween(k+0.5,k+1.5) ;
        thresholdPoints->SetInputArrayToProcess( arrayId , 0 , 0 , vtkDataObject::FIELD_ASSOCIATION_POINTS , "indexLabel" ) ;
        thresholdPoints->Update() ;
        vtkPolyData* thresholdedPolydataPoints = thresholdPoints->GetOutput() ;

        std::ofstream outputFile ;
        std::string labelName ;
        labelName = directory + '/' ;

        //Vector containing all lines to write in surface file
        std::vector <std::string> listPoints ;
        std::vector <std::string> listPolys ;
        //Vector iterator to write into surface file
        std::vector <std::string>::const_iterator itPoints, endPoints ;
        std::vector <std::string>::const_iterator itPolys, endPolys ;

        //Add prefix
        if( prefix != "" )
        {
            labelName += prefix + '.' ;
        }
        labelName += labelVect.at(k) ;
        labelName += ".asc" ;

        std::list <int> listIdPoint ;

        for( vtkIdType i = 0 ; i < thresholdedPolydataPoints->GetNumberOfPoints() ; i++ )
        {
            double p[3] ;
            thresholdedPolydataPoints->GetPoint(i,p) ;
            std::string linePoints = writeLinePoints(p) ;
            listPoints.push_back(linePoints) ;
            int idPointVTK = polyData->FindPoint(p) ;
            listIdPoint.push_back(idPointVTK) ;
        }

        int nbPolys = 0 ;
        for( vtkIdType j = 0 ; j < polyData->GetNumberOfCells() ; j++ )
        {
            polyData->GetCellPoints(j,cellIdList) ;
            int nbCompoCell = 0 ;
            nbCompoCell = cellIdList->GetNumberOfIds() ;
            int cellValue[nbCompoCell] ;
            int polyFound = 0 ;

            std::list <int>::iterator iter[nbCompoCell] ;

            for( int i = 0 ; i < nbCompoCell ; i++ )
            {
                cellValue[i] = cellIdList->GetId(i) ;
                iter[i] = find( listIdPoint.begin() , listIdPoint.end() , cellValue[i] ) ;
                int valArray = index->GetValue(cellValue[i]) ;
                if( iter[i] != listIdPoint.end() && valArray == k+1 )
                {
                    polyFound++ ;
                }
            }
            //Polygons withour overlapping
            if( polyFound == nbCompoCell )
            {
                nbPolys++ ;
                std::string lineCells = "" ;
                for( int i = 0 ; i < nbCompoCell ; i++ )
                {
                    std::string cell = IntToString( std::distance( listIdPoint.begin() , iter[i]) ) ;
                    lineCells += cell + " " ;
                }
                lineCells += "0\n" ;
                listPolys.push_back(lineCells) ;
            }
            //Polygons with overlapping
            else if( polyFound >= 1 && polyFound < nbCompoCell && overlapping==1 )
            {
                std::string lineCells ;
                for( int i = 0 ; i < nbCompoCell ; i++ )
                {
                    double newPoint[3] ;
                    polyData->GetPoint( cellValue[i] , newPoint ) ;
                    std::string newLinePoint = writeLinePoints(newPoint) ;
                    std::vector <std::string> ::const_iterator it ;
                    it=find( listPoints.begin() , listPoints.end() , newLinePoint ) ;
                    if(it == listPoints.end())
                    {
                        listPoints.push_back(newLinePoint) ;
                        listIdPoint.push_back(cellValue[i]) ;
                    }
                    std::list<int>::iterator newIter ;
                    newIter=find( listIdPoint.begin() , listIdPoint.end() , cellValue[i] ) ;
                    std::string cell = IntToString( std::distance( listIdPoint.begin() , newIter ) ) ;
                    lineCells += cell + " " ;
                }
                lineCells += "0\n" ;
                listPolys.push_back(lineCells) ;
                nbPolys++ ;
            }
        }

        //Write in a file
        outputFile.open( labelName.c_str() , std::ios::out ) ;
        if( outputFile.good() )
        {
            //Write header
            outputFile << "#!ascii - generated by CreateLabelFiles project \n" ;
            outputFile << listPoints.size() << " " << nbPolys << "\n" ;
            //write list of points
            for( itPoints = listPoints.begin() , endPoints = listPoints.end() ; itPoints != endPoints ; ++itPoints )
            {
                outputFile << *itPoints ;
            }
            //Wirte list of polygons
            for( itPolys = listPolys.begin() , endPolys = listPolys.end() ; itPolys != endPolys ; ++itPolys)
            {
                outputFile << *itPolys ;
            }
        }
        else
        {
            std::cout << "Cannot open the label file " << std::endl ;
            return EXIT_FAILURE ;
        }
        outputFile.close() ;
        std::cout << "Creation of " << labelName << " done !" << std::endl ;
    }

    std::cout << "CreateSurfaceLabelFiles Done !\n" << std::endl ;
    return EXIT_SUCCESS ;
}

vtkSmartPointer <vtkPolyData> ReadVTKFile ( std::string vtkFile )
{
    // ReadFile vtkFile
    vtkSmartPointer <vtkPolyDataReader> reader = vtkSmartPointer <vtkPolyDataReader>::New() ;
    vtkSmartPointer <vtkPolyData> polyData = vtkSmartPointer <vtkPolyData>::New() ;
    reader->SetFileName( vtkFile.c_str() ) ;
    reader->OpenVTKFile() ;
    reader->Update() ;
    polyData = reader->GetOutput() ;
    //Checked if surface file with color no containing errors
    if ( reader->GetErrorCode() != 0 )
    {
        std::cerr << "Unable to open input file: " << vtkFile.c_str() << std::endl ;
        return 0 ;
    }
    else
    {
        return polyData ;
    }
}

std::string IntToString ( int number )
{
    std::string result ;
    std::ostringstream convert ;
    convert << number ;
    result = convert.str() ;
    return result ;
}

std::string DoubleToString ( double number )
{
    std::string result ;
    std::ostringstream convert ;
    convert << number ;
    result = convert.str() ;
    return result ;
}

std::string writeLinePoints (double p[3])
{
    std::string linePoints = "" ;
    for( int j=0 ; j < 3 ; j++ )
    {
        std::string point = DoubleToString( p[j] ) ;
        linePoints += point + " " ;
    }
    linePoints += "0\n" ;
    return linePoints ;
}

int main ( int argc, char *argv[] )
{
    PARSE_ARGS ;

    if( extractPointData == 0 && translateToLabelNumber == 0 && createSurfaceLabelFiles == 0 )
    {
        std::cout << "Nothing to do. Specified at leat one action (--extractPointData or --translateToLabelNumber or --createSurfaceLabelFiles)" << std::endl ;
        return EXIT_FAILURE ;
    }
    else if( extractPointData == 1 && translateToLabelNumber == 0 && createSurfaceLabelFiles == 0 )
    {
        std::cout << "Run ExtractPointData tool ...\n" << std::endl ;
        if( !vtkLabelFile.empty() && !labelNameInfo.empty() && !arrayName.empty() )
        {
            ExtractPointData( vtkLabelFile , labelNameInfo , arrayName ) ;
        }
        else
        {
            std::cout << "You need to specify --vtkLabelFile AND --arrayName AND --labelNameInfo " << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else if ( extractPointData == 1 && translateToLabelNumber == 1 && createSurfaceLabelFiles == 0 )
    {
        std::cout << "Run ExtractPointData and TranslateToLabelNumber tools ...\n" << std::endl ;
        if( !vtkLabelFile.empty() && !labelNameInfo.empty() && !labelNumberInfo.empty() && !arrayName.empty())
        {
            ExtractPointData( vtkLabelFile , labelNameInfo , arrayName ) ;
            TranslateToLabelNumber( labelNameInfo , labelNumberInfo, useTranslationTable ) ;
        }
        else
        {
            std::cout << "You need to specify --vtkLabelFile AND --arrayName AND --labelNameInfo AND --labelNumberInfo" << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else if ( extractPointData == 1 && translateToLabelNumber == 1 && createSurfaceLabelFiles == 1 )
    {
        std::cout << "Run ExtractPointData, TranslateToLabelNumber and CreateSurfaceLabelFiles tools ...\n" << std::endl ;
        if( !vtkLabelFile.empty() && !labelNameInfo.empty() && !labelNumberInfo.empty() && !vtkFile.empty() && !arrayName.empty() )
        {
            ExtractPointData( vtkLabelFile , labelNameInfo , arrayName ) ;
            TranslateToLabelNumber( labelNameInfo , labelNumberInfo, useTranslationTable ) ;
            CreateSurfaceLabelFiles( vtkFile , labelNumberInfo , prefix , overlapping ) ;
        }
        else
        {
            std::cout << "You need to specify --vtkLabelFile AND --arrayName AND --labelNameInfo AND --labelNumberInfo AND --vtkFile " << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else if ( extractPointData == 0 && translateToLabelNumber == 1 && createSurfaceLabelFiles == 0)
    {
        std::cout << "Run TranslateToLabelNumber tool ...\n" << std::endl ;
        if( !labelNameInfo.empty() && !labelNumberInfo.empty() )
        {
            TranslateToLabelNumber( labelNameInfo , labelNumberInfo, useTranslationTable ) ;
        }
        else
        {
            std::cout << "You need to specify --labelNameInfo AND --labelNumberInfo" << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else if ( extractPointData == 0 && translateToLabelNumber == 1 && createSurfaceLabelFiles == 1)
    {
        std::cout << "Run TranslateToLabelNumber and CreateSurfaceLabelFiles tools ...\n" << std::endl ;
        if( !labelNameInfo.empty() && !labelNumberInfo.empty() && !vtkFile.empty() )
        {
            TranslateToLabelNumber( labelNameInfo, labelNumberInfo, useTranslationTable ) ;
            CreateSurfaceLabelFiles( vtkFile , labelNumberInfo , prefix , overlapping ) ;
        }
        else
        {
            std::cout << "You need to specify --labelNameInfo AND --labelNumberInfo AND --vtkFile " << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else if ( extractPointData == 0 && translateToLabelNumber == 0 && createSurfaceLabelFiles == 1 )
    {
        std::cout << "Run CreateSurfaceLabelFiles tool ...\n" << std::endl ;
        if( !labelNumberInfo.empty() && !vtkFile.empty() )
        {
            CreateSurfaceLabelFiles( vtkFile , labelNumberInfo , prefix , overlapping ) ;
        }
        else
        {
            std::cout << "You need to specify --labelNumberInfo AND --vtkFile " << std::endl ;
            return EXIT_FAILURE ;
        }
    }
    else // case extractPointData==1 && translateToLabelNumber==0 && createSurfaceLabelFiles==1
    {
        std::cout << "Wrong call" ;
        return EXIT_FAILURE ;
    }
    std::cout << "Done !" << std::endl ;
    return EXIT_SUCCESS ;
}
