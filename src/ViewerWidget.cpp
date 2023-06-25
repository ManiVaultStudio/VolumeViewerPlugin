/** General headers */
#include <math.h>
#include <algorithm> 
#include <vector>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
/** Plugin headers */
#include <ViewerWidget.h>
#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
/** HDPS headers */
#include <Dataset.h>
#include "PointData/PointData.h"
#include "ClusterData/ClusterData.h"
/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
#include <qvector.h>
/** VTK headers */
#include <QVTKOpenGLNativeWidget.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkIntArray.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkNamedColors.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkNamedColors.h>
#include <vtkCellPicker.h>
#include <vtkCellLocator.h>
#include <vtkPoints.h>
#include <vtkImageThreshold.h>



#include <vtkPointSource.h>
#include <vtkCellArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkLookupTable.h>

//#include <vtkIdTypeArray.h>
//#include <vtkSelectionNode.h>
//#include <vtkSelection.h>
//#include <vtkExtractSelection.h>
//#include <vtkUnstructuredGrid.h>


using namespace hdps;
using namespace hdps::gui;

namespace {
    // Catch mouse events
    class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
    {
    public:
        static MouseInteractorStyle* New();
        MouseInteractorStyle()
        {
            selectedMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
            selectedActor = vtkSmartPointer<vtkActor>::New();
        }
        virtual void OnLeftButtonDown() override
        {
            vtkNew<vtkNamedColors> colors;

            // Get the location of the click (in window coordinates)
            int* pos = this->GetInteractor()->GetEventPosition();

            vtkNew<vtkCellPicker> picker;
            picker->SetTolerance(0.0005);

            // Pick from this location.
            picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());
            double* worldPosition = picker->GetPickPosition();
            int cellID = picker->GetCellId();
            auto xyz = picker->GetCellIJK();
            _widget->setSelectedCell(cellID, xyz);

            // Forward events
            vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
        }

        void setViewer(ViewerWidget* widget) {
            _widget = widget;
        }
        ViewerWidget* _widget;
        vtkSmartPointer<vtkGPUVolumeRayCastMapper> selectedMapper;
        vtkSmartPointer<vtkActor> selectedActor;
    };

    vtkStandardNewMacro(MouseInteractorStyle);


} // namespace


ViewerWidget::ViewerWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent) :
    QWidget(parent),
    mRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
    mRenderer(vtkSmartPointer<vtkRenderer>::New()),
    mInteractor(vtkSmartPointer<QVTKInteractor>::New()),
    //mInteractorStyle(vtkSmartPointer<MouseInteractorStyle>::New()),
    _labelMap(vtkSmartPointer<vtkImageData>::New()),
    _imData(vtkSmartPointer<vtkImageData>::New()),
    numDimensions(),
    numPoints(),
    _VolumeViewerPlugin(VolumeViewerPlugin),
    _openGLWidget(),
    _dataSelected(false),
    _selectedCell(),
    _selectedCellCoordinate(),
    _thresholded(false),
    _lowerThreshold(),
    _upperThreshold(),
    _pointData(vtkSmartPointer<vtkPoints>::New()),
    _values(vtkSmartPointer<vtkFloatArray>::New()),
    _clusterData(),
    _clusterLoaded(false),
    _pointsColorData(),
    _pointsLoaded(false),
    _opacityLoaded(false),
    _valuesSelected()

{
    
    setAcceptDrops(true);
    // Initiate the QVTKOpenGLWidget.
    _openGLWidget = new QVTKOpenGLNativeWidget(this);
    vtkNew<MouseInteractorStyle> mStyle;
    // Setup the Renderwindow.
    mRenderWindow->AddRenderer(mRenderer);
    mInteractor->SetRenderWindow(mRenderWindow);
    _openGLWidget->setRenderWindow(mRenderWindow);
    mInteractor->Initialize();
    mStyle->SetDefaultRenderer(mRenderer);
    mStyle->setViewer(this);
    //mInteractorStyle->get
    mInteractor->SetInteractorStyle(mStyle);

    // Set the background color.
    mRenderer->SetBackground(0, 0, 0);
    numDimensions = 0;
    numPoints = 0;
    _selectedCell = -1;

}

ViewerWidget::~ViewerWidget()
{

}

vtkSmartPointer<vtkImageData> ViewerWidget::setData(Points& data, int chosenDim, std::string interpolationOption, std::string colorMap)
{
    // Get number of points from points dataset.
    numPoints = data.getNumPoints();

    // Get number of dimensions from points dataset.
    numDimensions = data.getNumDimensions();

    // Get x, y and z size from the points dataset.
    QVariant xQSize = data.getProperty("xDim");
    int xSize = xQSize.toInt();
    QVariant yQSize = data.getProperty("yDim");
    int ySize = yQSize.toInt();
    QVariant zQSize = data.getProperty("zDim");
    int zSize = zQSize.toInt();
    int dim;
    _xSize, _ySize, _zSize = xSize, ySize, zSize;
    // Create empty floatarray for reading from pointsdata.
    vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();

    // Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors.
    vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
    imData->SetDimensions(xSize, ySize, zSize);
    imData->AllocateScalars(VTK_FLOAT, 1);


    bool pointCloud = _VolumeViewerPlugin.getPointCloudEndabled();
    if (pointCloud) {
        // Set the number of values in the dataArray equal to the number of points in the pointsdataset.
        _values->SetNumberOfValues(numPoints);

        vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();

        vtkSmartPointer<vtkPoints> vtkPointObject = vtkSmartPointer<vtkPoints>::New();
        vtkPointObject->SetNumberOfPoints(numPoints);
        int pointCounter = 0;
        for (int i = 0; i < numPoints * numDimensions; i++)
        {
            if (i == 0 || i % numDimensions == 0) {
                double p[3] = { data.getValueAt(i), data.getValueAt(i + 1), data.getValueAt(i + 2) };

                vtkPointObject->SetPoint(pointCounter, p);

                vertices->InsertNextCell(pointCounter);

                _values->SetValue(pointCounter, 1);

                pointCounter++;
            }
        }
        _pointData = vtkPointObject;


        //_vertices = vertices;

    }
    else {
        // Set the number of values in the dataArray equal to the number of points in the pointsdataset.
        dataArray->SetNumberOfValues(numPoints);
        //polyData->s
        // Counter for the amount of values that have been read.
        int j = 0;

        // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;
            if (chosenDim == dim) {
                // write value into the dataArray
                dataArray->SetValue(j, data.getValueAt(i));
                j++;
            }
        }

        // Give the Points in the ImageData object the dataArray values.
        imData->GetPointData()->SetScalars(dataArray);

        _labelMap->SetOrigin(imData->GetOrigin());
        _labelMap->SetSpacing(imData->GetSpacing());
        _labelMap->SetDimensions(imData->GetDimensions());
        _labelMap->AllocateScalars(VTK_UNSIGNED_CHAR, 0);

        // Create an empty planeCollection in order to conform to the requirements off callinf renderData. (due to implementation of slicing action)
        vtkSmartPointer<vtkPlaneCollection> planeCollection = vtkSmartPointer<vtkPlaneCollection>::New();
        _imData = imData;
        // Return the imData object for later use in VolumeViewerPlugin.
    }
    return imData;
}

void ViewerWidget::renderData(vtkSmartPointer<vtkPlaneCollection> planeCollection, vtkSmartPointer<vtkImageData> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters) {
    // Store data parameters with clear names.
    double dataMinimum = imData->GetScalarRange()[0] + 1;
    double background = imData->GetScalarRange()[0];
    double dataMaximum = imData->GetScalarRange()[1];

    // Empty the renderer to avoid overlapping visualizations.
    mRenderer->RemoveAllViewProps();

    // Create color transfer function.
    vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(background, 0, 0, 0, 1, 1);

    // Get the colormap action.
    auto& colorMapAction = _VolumeViewerPlugin.getRendererSettingsAction().getColoringAction().getColorMapAction();

    // Get the colormap image.
    auto colorMapImage = colorMapAction.getColorMapImage();

    // Get background enabled parameter.
    bool backgroundEndabled = _VolumeViewerPlugin.getBackgroundEndabled();

    // Loop to read in colors from the colormap qimage.
    for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
        const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
        const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
        color->AddRGBPoint(normalizedPixelX * (dataMaximum - dataMinimum) + dataMinimum, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());
    }

    vtkSmartPointer<vtkPiecewiseFunction> colormapOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    // Set the opacity of the non-object voxels to 0.
    colormapOpacity->AddPoint(background, 0, 1, 1);

    // Loop to read in colors from the colormap qimage.
    for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
        const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
        const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
        colormapOpacity->AddPoint(normalizedPixelX * (dataMaximum - dataMinimum) + dataMinimum, pixelColor.alphaF());

    }


// Creates a volumeMapper with its input being the current imageData object in the vector.
    vtkSmartPointer<vtkGPUVolumeRayCastMapper > volMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper >::New();
    volMapper->SetBlendModeToComposite();
    volMapper->SetInputData(imData);
    volMapper->SetMaskInput(_labelMap);

    // Create volumeProperty for collormapping and opacitymapping.
    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

    // Set interpolation type.       
    if (interpolationOption == "NN") {
        volumeProperty->SetInterpolationType(VTK_NEAREST_INTERPOLATION);
    }
    else if (interpolationOption == "LIN") {
        volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
    }
    else {
        qDebug() << "Interpolation option invalid, using default Nearest Neighbor interpolation";
    }

    // Create volumeActor.
    vtkSmartPointer<vtkVolume> volActor = vtkSmartPointer<vtkVolume>::New();
    // Set volumeMapper .
    volActor->SetMapper(volMapper);
    // Set opacity and color table.
    volActor->SetProperty(volumeProperty);
    // Set the clipping planes.
    volMapper->SetClippingPlanes(planeCollection);
    volMapper->Update();

    // Create piecewise function for opacity table.
    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacityBackground = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compositeOpacityBackground->AddPoint(background, 0, 1, 1);

    // Checks if there is data is selected.
    if (!_dataSelected) {
        // Use the opacity information indicated in the colormap.
        compositeOpacity = colormapOpacity;
        // Add the Opacity options to volumeproperty.
        volumeProperty->SetScalarOpacity(compositeOpacity);
    }
    else {

        if (backgroundEndabled) {
            // Get background alpha parameter.
            float backgroundAlpha = _VolumeViewerPlugin.getBackgroundAlpha();

            // Set the nonselected data as semi-transparent.
            compositeOpacityBackground->AddSegment(dataMinimum, backgroundAlpha, dataMaximum, backgroundAlpha);

            //Check the currently selected option for selection opacity
            if (_VolumeViewerPlugin.getSelectionOpaque()) {
                // Set object values as opague.
                compositeOpacity->AddSegment(dataMinimum, 1, dataMaximum, 1);
            }
            else {
                // Use the transfer function values
                compositeOpacity = colormapOpacity;
            }
        }
        else {
            //Check the currently selected option for selection opacity
            if (_VolumeViewerPlugin.getSelectionOpaque()) {
                // Set object values as opague.
                compositeOpacity->AddSegment(dataMinimum, 1, dataMaximum, 1);
            }
            else {
                // Use the transfer function values
                compositeOpacity = colormapOpacity;
            }
            // Set the nonselected data as transparent.
            compositeOpacityBackground->AddSegment(dataMinimum, 0, dataMaximum, 0);
        }
        // Use the background alpha for all non labeled datapoints.
        volumeProperty->SetScalarOpacity(compositeOpacityBackground);
        // Use selected data alpha for all labeled datapoints.
        volumeProperty->SetLabelScalarOpacity(1, compositeOpacity);
    }

    // Add colortransferfunction to volumeproperty.
    volumeProperty->SetColor(color);

    // Check whether shading has been turned on or off and apply the shading parameters.
    if (shadingEnabled) {
        volumeProperty->ShadeOn();
        volumeProperty->SetAmbient(shadingParameters[0]);
        volumeProperty->SetDiffuse(shadingParameters[1]);
        volumeProperty->SetSpecular(shadingParameters[2]);
    }
    else {
        volumeProperty->ShadeOff();
        volumeProperty->SetAmbient(1);
        volumeProperty->SetDiffuse(0);
        volumeProperty->SetSpecular(0);
    }


    bool pointCloud = _VolumeViewerPlugin.getPointCloudEndabled();
    if (pointCloud) {
        vtkNew<vtkLookupTable> lut;
        vtkSmartPointer<vtkPolyData> pointsPolyData = vtkSmartPointer<vtkPolyData>::New();
        vtkNew<vtkPolyDataMapper> inputMapper;
        
        if (_clusterLoaded) {
            vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
            dataArray->SetNumberOfValues(_values->GetNumberOfValues());
            
            int k = 1;
            int numberOfClusters = _clusterData->getClusters().size();
            
            lut->SetValueRange(0, numberOfClusters);
            lut->SetRange(0, numberOfClusters);
            lut->SetNumberOfColors(numberOfClusters+1);
            lut->SetNumberOfTableValues(numberOfClusters+1);
            
            
            
            lut->SetTableValue(0, 1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());
            for (const auto& cluster : _clusterData->getClusters()) {
                auto indices = cluster.getIndices();
                for (auto index : indices) {
                    
                    dataArray->SetValue(index, k);
                    
                }
                
                auto currentColor = cluster.getColor();

                
                lut->SetTableValue(k, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), 1);
                
                k++;
            }
            lut->Build();
            if (_dataSelected) {
                for (int i = 0; i < _values->GetNumberOfValues(); i++)
                {
                    if (_valuesSelected->GetValue(i) == 0) {
                        dataArray->SetValue(i, 0);
                    }
                    
                }
                
            }
            pointsPolyData->SetPoints(_pointData);
            pointsPolyData->GetPointData()->SetScalars(dataArray);

            vtkSmartPointer<vtkVertexGlyphFilter> vertexFilt = vtkSmartPointer<vtkVertexGlyphFilter>::New();
            vertexFilt->SetInputData(pointsPolyData);
            vertexFilt->Update();
            vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->ShallowCopy(vertexFilt->GetOutput());
            
            inputMapper->SetInputData(polyData);
            inputMapper->SetLookupTable(lut);
            inputMapper->SetColorModeToMapScalars();
            inputMapper->SetScalarRange(0, numberOfClusters);
            
            inputMapper->Update();
        }
        else if (_pointsLoaded) {
            vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
            dataArray->SetNumberOfValues(_pointsColorData->getNumPoints());

            float dataMin = _pointsColorData->getValueAt(0);
            float dataMax = _pointsColorData->getValueAt(0);

            for (int i = 0; i < _pointsColorData->getNumPoints(); i++)
            {

                for (int dim = 0; dim < _pointsColorData->getNumDimensions(); dim++)
                {
                    if (dim == 0)
                    {

                        if (_pointsColorData->getValueAt(i * _pointsColorData->getNumDimensions()) < dataMin)
                        {
                            dataMin = _pointsColorData->getValueAt(i * _pointsColorData->getNumDimensions());
                        }
                        else if (_pointsColorData->getValueAt(i * _pointsColorData->getNumDimensions()) > dataMax) {
                            dataMax = _pointsColorData->getValueAt(i * _pointsColorData->getNumDimensions());
                        }
                        dataArray->SetValue(i, _pointsColorData->getValueAt(i * _pointsColorData->getNumDimensions()));
                    }
                }
            }

            // Create color transfer function.
            vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
            color->AddRGBPoint(-1, 1, 1, 1, 1, 1);
            
            

            // Get the colormap action.
            auto& colorMapAction = _VolumeViewerPlugin.getRendererSettingsAction().getColoringAction().getColorMapAction();

            // Get the colormap image.
            auto colorMapImage = colorMapAction.getColorMapImage();

            // Get background enabled parameter.
            bool backgroundEndabled = _VolumeViewerPlugin.getBackgroundEndabled();

            // Loop to read in colors from the colormap qimage.
            for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
                const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
                const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
                color->AddRGBPoint(normalizedPixelX * (dataMax - dataMin) + dataMin, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());
            }


            lut->SetValueRange(dataMin, dataMax);
            lut->SetRange(0, colorMapImage.width());
            lut->SetNumberOfColors(colorMapImage.width());
            lut->SetNumberOfTableValues(colorMapImage.width());
            lut->SetNanColor(1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());



            //lut->SetTableValue(0, 1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());
            
            if (_opacityLoaded) {
                for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {

                    const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);

                    lut->SetTableValue(pixelX, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF(), static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width()));
                }
                lut->Build();
            }
            else {
                // Loop to read in colors from the colormap qimage.
                for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {

                    const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);

                    lut->SetTableValue(pixelX, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF(), 0.8);
                }
                lut->Build();
            }
            

           /* int k = 1;
            int numberOfClusters = _clusterData->getClusters().size();

            lut->SetValueRange(0, numberOfClusters);
            lut->SetRange(0, numberOfClusters);
            lut->SetNumberOfColors(numberOfClusters + 1);
            lut->SetNumberOfTableValues(numberOfClusters + 1);



            lut->SetTableValue(0, 1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());
            for (const auto& cluster : _clusterData->getClusters()) {
                auto indices = cluster.getIndices();
                for (auto index : indices) {

                    dataArray->SetValue(index, k);

                }

                auto currentColor = cluster.getColor();


                lut->SetTableValue(k, currentColor.redF(), currentColor.greenF(), currentColor.blueF(), 1);

                k++;
            }*/


            //lut->Build();
            if (_dataSelected) {
                for (int i = 0; i < _values->GetNumberOfValues(); i++)
                {
                    if (_valuesSelected->GetValue(i) == 0) {
                        dataArray->SetValue(i, NAN);
                    }

                }

            }
            pointsPolyData->SetPoints(_pointData);
            pointsPolyData->GetPointData()->SetScalars(dataArray);


            

            vtkSmartPointer<vtkVertexGlyphFilter> vertexFilt = vtkSmartPointer<vtkVertexGlyphFilter>::New();
            vertexFilt->SetInputData(pointsPolyData);
            vertexFilt->Update();
            vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->ShallowCopy(vertexFilt->GetOutput());

            inputMapper->SetInputData(polyData);
            inputMapper->SetLookupTable(lut);
            
            inputMapper->SetColorModeToMapScalars();
            
            //inputMapper->SetScalarRange(0, numberOfClusters);

            inputMapper->Update();
        }
        else {

            
            
            if (_opacityLoaded) {
                
                vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
                dataArray->SetNumberOfValues(_pointsOpacityData->getNumPoints());
                
                float dataMin = _pointsOpacityData->getValueAt(0);
                float dataMax = _pointsOpacityData->getValueAt(0);
               

                for (int i = 0; i < _pointsOpacityData->getNumPoints(); i++)
                {

                    for (int dim = 0; dim < _pointsOpacityData->getNumDimensions(); dim++)
                    {
                        if (dim == 0)
                        {

                            if (_pointsOpacityData->getValueAt(i * _pointsOpacityData->getNumDimensions()) < dataMin)
                            {
                                dataMin = _pointsOpacityData->getValueAt(i * _pointsOpacityData->getNumDimensions());
                            }
                            else if (_pointsOpacityData->getValueAt(i * _pointsOpacityData->getNumDimensions()) > dataMax) {
                                dataMax = _pointsOpacityData->getValueAt(i * _pointsOpacityData->getNumDimensions());
                            }
                            dataArray->SetValue(i, _pointsOpacityData->getValueAt(i * _pointsOpacityData->getNumDimensions()));
                        }
                    }
                }

               
                lut->SetValueRange(dataMin, dataMax);
                lut->SetRange(0, colorMapImage.width());
                lut->SetNumberOfColors(colorMapImage.width());
                lut->SetNumberOfTableValues(colorMapImage.width());
                lut->SetNanColor(1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());
                
                for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {

                    const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);

                    lut->SetTableValue(pixelX, 0, 1, 0, static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width()));
                }
                pointsPolyData->SetPoints(_pointData);
               
                if (_dataSelected) {

                    pointsPolyData->GetPointData()->SetScalars(_valuesSelected);
                }
                else {
                    pointsPolyData->GetPointData()->SetScalars(dataArray);
                }

            }
            else {
                lut->SetValueRange(0, 1);
                lut->SetNumberOfColors(2);
                lut->Build();
                lut->SetTableValue(0, 1, 1, 1, _VolumeViewerPlugin.getBackgroundAlpha());
                lut->Build();
                lut->SetTableValue(1, 0, 1, 0, 1);
                pointsPolyData->SetPoints(_pointData);
                if (_dataSelected) {

                    pointsPolyData->GetPointData()->SetScalars(_valuesSelected);
                }
                else {
                    pointsPolyData->GetPointData()->SetScalars(_values);
                }
            }
            

            
            
            
            

            
            

            //polyData->SetVerts(_vertices);

            vtkSmartPointer<vtkVertexGlyphFilter> vertexFilt = vtkSmartPointer<vtkVertexGlyphFilter>::New();
            vertexFilt->SetInputData(pointsPolyData);
            vertexFilt->Update();
            vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->ShallowCopy(vertexFilt->GetOutput());            
            inputMapper->SetInputData(polyData);
            inputMapper->SetLookupTable(lut);
            inputMapper->Update();
        }
        vtkNew<vtkActor> inputActor;
        inputActor->SetMapper(inputMapper);

        //inputActor->GetProperty()->SetColor(colors->GetColor3d("Lime").GetData());
        inputActor->GetProperty()->SetPointSize(3);


        // Add the current volume to the renderer.
        mRenderer->AddViewProp(inputActor);
    }
    else {
        mRenderer->AddViewProp(volActor);
    }
    // Center camera.
    mRenderer->ResetCamera();
    // Render.
    mRenderWindow->Render();
}

void ViewerWidget::setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim) {
    bool pointCloud = _VolumeViewerPlugin.getPointCloudEndabled();
    if (pointCloud) {
        // Get x, yand z size from the points dataset.
        QVariant xQSize = points.getProperty("xDim");
        int xSize = xQSize.toInt();
        QVariant yQSize = points.getProperty("yDim");
        int ySize = yQSize.toInt();
        QVariant zQSize = points.getProperty("zDim");
        int zSize = zQSize.toInt();
        int dim;

        // Create bool variable to indicate if data has been selected.
        std::vector<bool> selected;
        points.selectedLocalIndices(selectionIndices, selected);
        // Count the number of selected datapoints.
        int count = std::count(selected.begin(), selected.end(), true);

        //Array to store selected image data.
        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();


        // Set the number of values in the dataArray equal to the number of points in the pointsdataset.
        dataArray->SetNumberOfValues(numPoints);


        // Counter for the amount of values that have been read.
        int j = 0;
        // Counter for the number of selected datapoints to avoid overflowing selectionIndices vector.
        int numSelectedLoaded = 0;



        // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;

            if (chosenDim == dim) {
                // Ensure that numSelectedLoaded does not overflow the slectionIndeces vector to avoid a crash.
                if (numSelectedLoaded != selectionIndices.size()) {
                    // If the index is equal to the current point in the array.
                    if (selected[i / numDimensions]) {

                        // Write value into the dataArray.
                        dataArray->SetValue(j, points.getValueAt(i));

                        numSelectedLoaded++;
                    }
                    else {
                        // All other indices are non-Object.
                        dataArray->SetValue(j, 0);

                    }
                }
                else {
                    // All other indices are non-Object.
                    dataArray->SetValue(j, 0);

                }
                j++;
            }
        }
        _valuesSelected = dataArray;
    }
    else {
        // Get x, y and z size from the points dataset.
        QVariant xQSize = points.getProperty("xDim");
        int xSize = xQSize.toInt();
        QVariant yQSize = points.getProperty("yDim");
        int ySize = yQSize.toInt();
        QVariant zQSize = points.getProperty("zDim");
        int zSize = zQSize.toInt();
        int dim;

        // Create bool variable to indicate if data has been selected.
        std::vector<bool> selected;
        points.selectedLocalIndices(selectionIndices, selected);
        // Count the number of selected datapoints.
        int count = std::count(selected.begin(), selected.end(), true);

        //Array to store selected image data.
        vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();
        //Array to store which part of the image is part of the selection.
        vtkSmartPointer<vtkIntArray> labelArray = vtkSmartPointer<vtkIntArray>::New();

        // Set the number of values in the dataArray equal to the number of points in the pointsdataset.
        dataArray->SetNumberOfValues(numPoints);
        labelArray->SetNumberOfValues(numPoints);

        // Counter for the amount of values that have been read.
        int j = 0;
        // Counter for the number of selected datapoints to avoid overflowing selectionIndices vector.
        int numSelectedLoaded = 0;

        bool firstRead = false;

        auto backgroundValue = points.getValueAt(0);

        // Loop over the number of values in the pointsdata and find minimum values for current dimension to set background color.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;
            if (chosenDim == dim) {
                if (!firstRead || points.getValueAt(i) < backgroundValue) {
                    firstRead = true;
                    backgroundValue = points.getValueAt(i);
                }
            }
        }

        // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices.
        for (int i = 0; i < numPoints * numDimensions; i++) {
            // The remainder of the current value divided by the number of dimensions is the current dimension.
            dim = i % numDimensions;

            if (chosenDim == dim) {
                // Ensure that numSelectedLoaded does not overflow the slectionIndeces vector to avoid a crash.
                if (numSelectedLoaded != selectionIndices.size()) {
                    // If the index is equal to the current point in the array.
                    if (selected[i / numDimensions]) {

                        // Write value into the dataArray.
                        dataArray->SetValue(j, points.getValueAt(i));
                        labelArray->SetValue(j, 1);
                        numSelectedLoaded++;
                    }
                    else {
                        // All other indices are non-Object.
                        dataArray->SetValue(j, backgroundValue);
                        labelArray->SetValue(j, 0);
                    }
                }
                else {
                    // All other indices are non-Object.
                    dataArray->SetValue(j, backgroundValue);
                    labelArray->SetValue(j, 0);
                }
                j++;
            }
        }

        // Add scalarData to the imageData object.

        _labelMap->GetPointData()->SetScalars(labelArray);

         
    }
    if (selectionIndices.size() == 0) {
        _dataSelected = false;
    }
    else {
        _dataSelected = true;
    }
    // Return the selection imagedata object.  

}


vtkSmartPointer<vtkImageData> ViewerWidget::connectedSelection(Points& points, int chosenDim, int* seedpoint, float upperThreshold, float lowerThreshold) {
    double dataMinimum = _imData->GetScalarRange()[0] + 1;

    double dataMaximum = _imData->GetScalarRange()[1];

    auto dataBounds = _imData->GetExtent();

    float onePercent = (abs(dataMaximum - dataMinimum)) / 100;

    _upperThreshold = _imData->GetScalarComponentAsFloat(seedpoint[0], seedpoint[1], seedpoint[2], 0) + upperThreshold * onePercent;
    _lowerThreshold = _imData->GetScalarComponentAsFloat(seedpoint[0], seedpoint[1], seedpoint[2], 0) - lowerThreshold * onePercent;

    _thresholded = true;
    std::vector<unsigned int> selectionIndeces;
    int dim = 0;

    // Loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices.
    for (int i = 0; i < numPoints * numDimensions; i++) {
        // The remainder of the current value divided by the number of dimensions is the current dimension.
        dim = i % numDimensions;

        if (chosenDim == dim) {
            if (points.getValueAt(i) <= _upperThreshold && points.getValueAt(i) >= _lowerThreshold) {
                selectionIndeces.push_back(i / numDimensions);
            }
        }
    }
    points.setSelectionIndices(selectionIndeces);

    if (selectionIndeces.size() == 0) {
        _dataSelected = false;
    }
    else {
        _dataSelected = true;
    }

    return _imData;
}

void ViewerWidget::setSelectedCell(int cellID, int* xyz) {
    _selectedCell = cellID;
    _selectedCellCoordinate = xyz;
    _VolumeViewerPlugin.getRendererSettingsAction().getSelectedPointsAction().getPositionAction().changeValue(xyz);
}

void ViewerWidget::setClusterColor(const Dataset<Clusters>& clusterData, bool loadedOrNot) {
    //auto test = clusterData->getRawDataSize();
    //std::cout << test << std::endl;
    _clusterData = clusterData;
    _clusterLoaded = loadedOrNot;
    for (const auto& cluster : clusterData->getClusters()) {
        cluster.getIndices();
        
    }
}
void ViewerWidget::setPointsColor(const Dataset<Points>& pointsData, bool loadedOrNot) {
    //auto test = clusterData->getRawDataSize();
    //std::cout << test << std::endl;
    _pointsColorData = pointsData;
    _pointsLoaded = loadedOrNot;
    
}
void ViewerWidget::setPointsOpacity(const Dataset<Points>& pointsData, bool loadedOrNot){
    //auto test = clusterData->getRawDataSize();
    //std::cout << test << std::endl;
    _pointsOpacityData = pointsData;
    _opacityLoaded = loadedOrNot;
    
}