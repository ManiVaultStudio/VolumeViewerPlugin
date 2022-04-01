/** General headers */
#include <math.h>
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */
#include <ViewerWidget.h>
#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
/** HDPS headers */
#include <Dataset.h>
#include <PointData.h>
/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
/** VTK headers */
#include <QVTKOpenGLNativeWidget.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>

using namespace hdps;
using namespace hdps::gui;


ViewerWidget::ViewerWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent) :
	QWidget(parent),
	mRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()),
	mRenderer(vtkSmartPointer<vtkRenderer>::New()),
	mInteractor(vtkSmartPointer<QVTKInteractor>::New()),
	mInteractorStyle(vtkSmartPointer<vtkInteractorStyle>::New()),
	numDimensions(),
	numPoints(),
	_VolumeViewerPlugin(VolumeViewerPlugin),
	_openGLWidget()

{
	setAcceptDrops(true);
	// Initiate the QVTKOpenGLWidget.
	_openGLWidget = new QVTKOpenGLNativeWidget(this);

	// Setup the Renderwindow.
	mRenderWindow->AddRenderer(mRenderer);
	mInteractor->SetRenderWindow(mRenderWindow);
	_openGLWidget->setRenderWindow(mRenderWindow);
	mInteractor->Initialize();

	// Set the background color.
	mRenderer->SetBackground(0, 0, 0);
	numDimensions = 0;
	numPoints = 0;
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

	// Create empty floatarray for reading from pointsdata.
	vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();

	// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors.
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	imData->SetDimensions(xSize, ySize, zSize);
	imData->AllocateScalars(VTK_FLOAT, 1);

	// Set the number of values in the dataArray equal to the number of points in the pointsdataset.
	dataArray->SetNumberOfValues(numPoints);
	
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

	// Create an empty planeCollection in order to conform to the requirements off callinf renderData. (due to implementation of slicing action)
	vtkSmartPointer<vtkPlaneCollection> planeCollection = vtkSmartPointer<vtkPlaneCollection>::New();

	// Return the imData object for later use in VolumeViewerPlugin.
	return imData;
}
	
void ViewerWidget::renderData(vtkSmartPointer<vtkPlaneCollection> planeCollection, std::vector<vtkSmartPointer<vtkImageData>> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters){
    // Store data parameters with clear names.
    double dataMinimum = imData[0]->GetScalarRange()[0] + 1;
    double background = imData[0]->GetScalarRange()[0];
    double dataMaximum = imData[0]->GetScalarRange()[1];
    //std::cout << dataMinimum << std::endl;
    // Empty the renderer to avoid overlapping visualizations.
	mRenderer->RemoveAllViewProps();
    
    // create color transfer function
	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(background, 0, 0, 0, 1, 1);

    // Get the colormap action.
	auto& colorMapAction = _VolumeViewerPlugin.getRendererSettingsAction().getColoringAction().getColorMapAction();

    // Get the colormap image.
	auto colorMapImage = colorMapAction.getColorMapImage();


    //auto colorMapImage = _VolumeViewerPlugin.getTransfertWidget().getTransferFunction().getColorMap();

    // Loop to read in colors from the colormap qimage.
	for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
		const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
		const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);
		color->AddRGBPoint(normalizedPixelX * (dataMaximum - dataMinimum) + dataMinimum, pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());
	}
    
	// Loop through the imData vector, can contain 1 or 2 objects, the second one is always the selected data.
	for (int i = 0; i < imData.size(); i++) {
        
		// Creates a volumeMapper with its input being the current imageData object in the vector.
		vtkSmartPointer<vtkSmartVolumeMapper> volMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		volMapper->SetBlendModeToComposite();
		volMapper->SetInputData(imData[i]);

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
		
		// If funtion to indicate whether we are in the fulldata (i==0) or in the selected data (i!=0) for opacity mapping purposes.
		if (i == 0) {
			// Clipping planes are only applied in the fullData not in the selected data.
			volMapper->SetClippingPlanes(planeCollection);
			volMapper->Update();

			// Create piecewise function for opacity table.
			vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
			// Set the opacity of the non-object voxels to 0.
			compositeOpacity->AddPoint(background, 0, 1, 1);

			// Checks if there is 1 or 2 objects in the imdata vector.
			if (imData.size() < 2) {
				// If only 1 object is present then the opacity of all data is set to opague.
				compositeOpacity->AddSegment(dataMinimum, 1 , dataMaximum, 1);
			}
			else {
				// If there are 2 objects (so also dataSelected) then the fulldata opacity is set to be semi-translucent.
				compositeOpacity->AddSegment(dataMinimum, 0.02, dataMaximum, 0.02);
			}
			// Add the Opacity options to volumeproperty.
			volumeProperty->SetScalarOpacity(compositeOpacity);
			
			// Add colortransferfunction to volumeproperty.
			volumeProperty->SetColor(color);
		}
		else {
			// Selected Data Section.
			volMapper->Update();

			// Create piecewise function for opacity table.
			vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

			// Set object values as opague.
			compositeOpacity->AddSegment(dataMinimum, 1, dataMaximum, 1);

			// Set non-object values as seethrough.
			compositeOpacity->AddPoint(background, 0, 1, 1);

			// Add opacity table to volumeproperty.
			volumeProperty->SetScalarOpacity(compositeOpacity);

			// Add colortransferfunction to volumeproperty.
			volumeProperty->SetColor(color);

			// Add color transfer function to volumeproperty.
			volumeProperty->SetColor(color);
		}
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

		// Create volumeActor.
		vtkSmartPointer<vtkVolume> volActor = vtkSmartPointer<vtkVolume>::New();
		// Set volumeMapper .
		volActor->SetMapper(volMapper);
		// Set opacity and color table.
		volActor->SetProperty(volumeProperty);

		// Add the current volume to the renderer.
		mRenderer->AddVolume(volActor);
	}
	// Center camera.
	mRenderer->ResetCamera();
	// Render.
	mRenderWindow->Render();
}

vtkSmartPointer<vtkImageData> ViewerWidget::setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim) {
	// Get x, y and z size from the points dataset.
	QVariant xQSize = points.getProperty("xDim");
	int xSize = xQSize.toInt();
	QVariant yQSize = points.getProperty("yDim");
	int ySize = yQSize.toInt();
	QVariant zQSize = points.getProperty("zDim");
	int zSize = zQSize.toInt();
	int dim;
   
    
    std::vector<bool> selected;
    points.selectedLocalIndices(selectionIndices, selected);
    int count = std::count(selected.begin(), selected.end(), true);

   std::cout << count << std::endl;
    //std::cout << selectionIndices.size() << std::endl;
	vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();

	// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors.
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	imData->SetDimensions(xSize, ySize, zSize);
	imData->AllocateScalars(VTK_FLOAT, 1);

	// Set the number of values in the dataArray equal to the number of points in the pointsdataset.
	dataArray->SetNumberOfValues(numPoints);
	
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
            if (!firstRead || points.getValueAt(i)<backgroundValue) {
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
			if (numSelectedLoaded != selectionIndices.size() ) {
				// If the index is equal to the current point in the array.
				if (selected[i / numDimensions]) {
                    //std::cout << selectionIndices[numSelectedLoaded] << std::endl;
					// Write value into the dataArray.
					dataArray->SetValue(j, points.getValueAt(i));
					numSelectedLoaded++;
				}
				else {
					// All other indices are non-Object.
					dataArray->SetValue(j, backgroundValue);
				}
			}
			else {
				// All other indices are non-Object.
				dataArray->SetValue(j, backgroundValue);
			}
			j++;
		}
	}

    
    std::cout << numSelectedLoaded << "indices : " << selectionIndices.size() << std::endl;
	// Add scalarData to the imageData object.
	imData->GetPointData()->SetScalars(dataArray);
	
	// Return the selection imagedata object.
	return imData;
}