/** General headers */
#include <math.h>
/** Plugin headers */
#include <ViewerWidget.h>
#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
/** HDPS headers */
#include <util/DatasetRef.h>
#include <PointData.h>
/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
/** VTK headers */
#include <QVTKOpenGLWidget.h>
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
	// Initiate the QVTKOpenGLWidget
	_openGLWidget = new QVTKOpenGLWidget(this);

	// Setup the Renderwindow
	mRenderWindow->AddRenderer(mRenderer);
	mInteractor->SetRenderWindow(mRenderWindow);
	_openGLWidget->SetRenderWindow(mRenderWindow);
	mInteractor->Initialize();

	// Set the background color 
	mRenderer->SetBackground(0, 0, 0);
	numDimensions = 0;
	numPoints = 0;
}

ViewerWidget::~ViewerWidget()
{

}

vtkSmartPointer<vtkImageData> ViewerWidget::setData(Points points, int chosenDim, std::string interpolationOption, std::string colorMap)
{
	// get number of points from points dataset
	numPoints = points.getNumPoints();

	// get number of dimensions from points dataset
	numDimensions = points.getNumDimensions();

	// get x, y and z size from the points dataset
	QVariant xQSize = points.getProperty("xDim");
	int xSize = xQSize.toInt();
	QVariant yQSize = points.getProperty("yDim");
	int ySize = yQSize.toInt();
	QVariant zQSize = points.getProperty("zDim");
	int zSize = zQSize.toInt();
	
	int dim;

	// Create empty floatarray for reading from pointsdata 
	vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();

	// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	imData->SetDimensions(xSize, ySize, zSize);
	imData->AllocateScalars(VTK_FLOAT, 1);

	// Set the number of values in the dataArray equal to the number of points in the pointsdataset
	dataArray->SetNumberOfValues(numPoints);
	
	// counter for the amount of values that have been read
	int j = 0;

	// loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension
	for (int i = 0; i < numPoints * numDimensions; i++) {
		// The remainder of the current value divided by the number of dimensions is the current dimension
		dim = i % numDimensions;
		if (chosenDim == dim) {
			// write value into the dataArray
			dataArray->SetValue(j, points.getValueAt(i));
			j++;
		}
	}

	// Give the Points in the ImageData object the dataArray values
	imData->GetPointData()->SetScalars(dataArray);

	// Create an empty planeCollection in order to conform to the requirements off callinf renderData (due to implementation of slicing action)
	vtkSmartPointer<vtkPlaneCollection> planeCollection = vtkSmartPointer<vtkPlaneCollection>::New();

	/** Create a vtkimagedatavector to store the current imagedataand selected data(if present).
	*   Vector is needed due to the possibility of having data selected in a scatterplot wich
	*   changes the colormapping of renderdata and creates an aditional actor to visualize the selected data.
	*/
	std::vector<vtkSmartPointer<vtkImageData>> imageData;
	imageData.push_back(imData);

	// Call renderData
	ViewerWidget::renderData( planeCollection,  imageData, interpolationOption, colorMap);

	// Return the imData object for later use in VolumeViewerPlugin
	return imData;
}
	
void ViewerWidget::renderData(vtkSmartPointer<vtkPlaneCollection> planeCollection, std::vector<vtkSmartPointer<vtkImageData>> imData, std::string interpolationOption, std::string colorMap){
	// Empty the renderer to avoid overlapping visualizations
	mRenderer->RemoveAllViewProps();

	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
	color->AddRGBPoint(imData[0]->GetScalarRange()[0], 0, 0, 0);

    
	auto& colorMapAction = _VolumeViewerPlugin.getRendererSettingsAction().getColoringAction().getColorMapAction();

	auto colorMapImage = colorMapAction.getColorMapImage();

	for (int pixelX = 0; pixelX < colorMapImage.width(); pixelX++) {
		const auto normalizedPixelX = static_cast<float>(pixelX) / static_cast<float>(colorMapImage.width());
		const auto pixelColor = colorMapImage.pixelColor(pixelX, 0);

		color->AddRGBPoint(normalizedPixelX * imData[0]->GetScalarRange()[1], pixelColor.redF(), pixelColor.greenF(), pixelColor.blueF());

		
	}

	// Loop through the imData vector, can contain 1 or 2 objects, the second one is always the selected data
	for (int i = 0; i < imData.size(); i++) {

		// Creates a volumeMapper with its input being the current imageData object in the vector
		vtkSmartPointer<vtkSmartVolumeMapper> volMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		volMapper->SetBlendModeToComposite();
		volMapper->SetInputData(imData[i]);

		// Create volumeProperty for collormapping and opacitymapping (This is value based, in the loader plugin the values are mapped to [1,100] for Object voxels)
		vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
		// Set interpolation type (currently nearerst neighbor, awaiting awnser of Boudewijn Lelieveldt for preference or possible selection option)
		
		if (interpolationOption == "NN") {
			volumeProperty->SetInterpolationType(VTK_NEAREST_INTERPOLATION);
		}
		else if (interpolationOption == "LIN") {
			volumeProperty->SetInterpolationType(VTK_CUBIC_INTERPOLATION);
		}
		else if (interpolationOption == "CUBE") {
			volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
		}
		else {
			qDebug() << "Interpolation option invalid, using default Nearest Neighbor interpolation";
		}


		
		// if funtion to indicate whether we are in the fulldata (i==0) or in the selected data (i!=0) for opacity mapping purposes
		if (i == 0) {
			// Clipping planes are only applied in the fullData not in the selected data
			volMapper->SetClippingPlanes(planeCollection);
			volMapper->Update();

			// Create piecewise function for opacity table
			vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
			// Set the opacity of the non-object voxels to 0
			compositeOpacity->AddPoint(imData[0]->GetScalarRange()[0], 0);

			// Checks if there is 1 or 2 objects in the imdata vector
			if (imData.size() < 2) {
				// if only 1 object is present then the opacity of all data is set to opague
				compositeOpacity->AddSegment(imData[0]->GetScalarRange()[0]+1, 1 , imData[0]->GetScalarRange()[1],1);
			}
			else {
				// if there are 2 objects (so also dataSelected) then the fulldata opacity is set to be semi-translucent
				compositeOpacity->AddSegment(imData[0]->GetScalarRange()[0]+1, 0.02, imData[0]->GetScalarRange()[1], 0.02);
			}
			// add the Opacity options to volumeproperty
			volumeProperty->SetScalarOpacity(compositeOpacity);
			
			//if (colorMap == "BuYlRd") {
			//	// Set BuYlRd color segment
			//	color->AddRGBSegment(1, 0, 0, 1, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);
			//}else if (colorMap == "Gray to White") {
			//	//Set gray to white rgb segment
			//	color->AddRGBSegment(1, 0.1, 0.1, 0.1, 100, 1, 1, 1);		
			//}else if(colorMap == "Qualitative"){
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 0, 0, 1, 10.9, 0, 0, 1);
			//	color->AddRGBSegment(11, 255, 165, 0, 20.8, 255, 165, 0);
			//	color->AddRGBSegment(20.9, 0,1,0, 30.7, 0,1,0);
			//	color->AddRGBSegment(30.8, 1, 0, 0, 40.6, 1, 0, 0);
			//	color->AddRGBSegment(40.7, 153, 50, 204, 50.5, 153, 50, 204);
			//	color->AddRGBSegment(50.6, 139, 69, 19, 60.4, 139, 69, 19);
			//	color->AddRGBSegment(60.5, 219, 112, 147, 70.3, 219, 112, 147);
			//	color->AddRGBSegment(70.4, 0.5,0.5,0.5, 80.2, 0.5,0.5,0.5);
			//	color->AddRGBSegment(80.3, 1, 1, 1, 90.1, 1, 1, 1);
			//	color->AddRGBSegment(90.2, 0, 1, 0, 100, 0, 1, 0);
			//}else if(colorMap == "GnYlRd"){
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 0, 1, 0, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);
			//	
			//}else if (colorMap == "Spectral"){
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 1, 0, 1, 25.75, 0, 1, 0);
			//	color->AddRGBSegment(25.75, 0, 0, 1, 50.5, 0, 1, 0);
			//	color->AddRGBSegment(50.5, 0, 1, 0, 76.25, 1, 1, 0);
			//	color->AddRGBSegment(76.25, 1, 1, 0, 100, 1, 0, 0);
			//	
			//}
			//else {
			//	qDebug() << "Invalid colormap, using default BuYlRd";
			//	color->AddRGBSegment(1, 0, 0, 1, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);
			//}
			// add colortransferfunction to volumeproperty
			volumeProperty->SetColor(color);
			
		}
		else {
			// Selected Data Section
			volMapper->Update();

			// Create piecewise function for opacity table
			vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
			// Set object values as opague
			compositeOpacity->AddSegment(imData[0]->GetScalarRange()[0]+1, 1, imData[0]->GetScalarRange()[1], 1);
			// Set non-object values as seethrough
			compositeOpacity->AddPoint(imData[0]->GetScalarRange()[0], 0);

			// add opacity table to volumeproperty
			volumeProperty->SetScalarOpacity(compositeOpacity);

			// create color transfer function
			
			/*vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
			color->AddRGBPoint(0, 0, 0, 0);
			color->AddRGBSegment(1, 0, 0, 1, 101 / 2, 1, 1, 0);
			*/
			//if (colorMap == "BuYlRd") {
			//	// Set BuYlRd color segment
			//	color->AddRGBSegment(1, 0, 0, 1, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);
			//}
			//else if (colorMap == "Gray to White") {
			//	//Set gray to white rgb segment
			//	color->AddRGBSegment(1, 0.1, 0.1, 0.1, 100, 1, 1, 1);
			//}
			//else if (colorMap == "Qualitative") {
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 0, 0, 1, 10.9, 0, 0, 1);
			//	color->AddRGBSegment(11, 255, 165, 0, 20.8, 255, 165, 0);
			//	color->AddRGBSegment(20.9, 0, 1, 0, 30.7, 0, 1, 0);
			//	color->AddRGBSegment(30.8, 1, 0, 0, 40.6, 1, 0, 0);
			//	color->AddRGBSegment(40.7, 153, 50, 204, 50.5, 153, 50, 204);
			//	color->AddRGBSegment(50.6, 139, 69, 19, 60.4, 139, 69, 19);
			//	color->AddRGBSegment(60.5, 219, 112, 147, 70.3, 219, 112, 147);
			//	color->AddRGBSegment(70.4, 0.5, 0.5, 0.5, 80.2, 0.5, 0.5, 0.5);
			//	color->AddRGBSegment(80.3, 1, 1, 1, 90.1, 1, 1, 1);
			//	color->AddRGBSegment(90.2, 0, 1, 0, 100, 0, 1, 0);
			//}
			//else if (colorMap == "GnYlRd") {
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 0, 1, 0, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);

			//}
			//else if (colorMap == "Spectral") {
			//	// Set transfer funtion for values
			//	color->AddRGBSegment(1, 1, 0, 1, 25.75, 0, 1, 0);
			//	color->AddRGBSegment(25.75, 0, 0, 1, 50.5, 0, 1, 0);
			//	color->AddRGBSegment(50.5, 0, 1, 0, 76.25, 1, 1, 0);
			//	color->AddRGBSegment(76.25, 1, 1, 0, 100, 1, 0, 0);

			//}
			//else {
			//	qDebug() << "Invalid colormap, using default BuYlRd";
			//	color->AddRGBSegment(1, 0, 0, 1, 101 / 2, 1, 1, 0);
			//	color->AddRGBSegment(101 / 2, 1, 1, 0, 100, 1, 0, 0);
			//}
			// add colortransferfunction to volumeproperty
			volumeProperty->SetColor(color);

			// add color transfer function to volumeproperty
			volumeProperty->SetColor(color);

		}

		// Create volumeActor
		vtkSmartPointer<vtkVolume> volActor = vtkSmartPointer<vtkVolume>::New();
		// Set volumeMapper 
		volActor->SetMapper(volMapper);
		// Set opacity and color table
		volActor->SetProperty(volumeProperty);

		// add the current volume to the renderer
		mRenderer->AddVolume(volActor);
	}
	// center camera
	mRenderer->ResetCamera();
	// Render
	mRenderWindow->Render();

}

vtkSmartPointer<vtkImageData> ViewerWidget::setSelectedData(Points points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim) {
	// get x, y and z size from the points dataset
	QVariant xQSize = points.getProperty("xDim");
	int xSize = xQSize.toInt();
	QVariant yQSize = points.getProperty("yDim");
	int ySize = yQSize.toInt();
	QVariant zQSize = points.getProperty("zDim");
	int zSize = zQSize.toInt();

	int dim;
	
	// Create empty floatarray for reading from pointsdata 
	vtkSmartPointer<vtkFloatArray> dataArray = vtkSmartPointer<vtkFloatArray>::New();

	// Create a vtkimagedata object of size xSize, ySize and zSize with vtk_float type vectors
	vtkSmartPointer<vtkImageData> imData = vtkSmartPointer<vtkImageData>::New();
	imData->SetDimensions(xSize, ySize, zSize);
	imData->AllocateScalars(VTK_FLOAT, 1);

  
	
	// Set the number of values in the dataArray equal to the number of points in the pointsdataset
	dataArray->SetNumberOfValues(numPoints);
	
	// counter for the amount of values that have been read
	int j = 0;
	// Counter for the number of selected datapoints to avoid overflowing selectionIndices vector
	int numSelectedLoaded = 0;

    bool firstRead = false;

    auto backgroundValue = points.getValueAt(0);

    // loop over the number of values in the pointsdata and find minimum values for current dimension to set background color
    for (int i = 0; i < numPoints * numDimensions; i++) {
        // The remainder of the current value divided by the number of dimensions is the current dimension
        dim = i % numDimensions;

        if (chosenDim == dim) {
            if (!firstRead || points.getValueAt(i)<backgroundValue) {

                firstRead = true;

                backgroundValue = points.getValueAt(i);
            }
        }
    }
	
	// loop over the number of values in the pointsdata and write values into the dataArray if the current dimension  equals the chosen dimension and the selected indices
	for (int i = 0; i < numPoints * numDimensions; i++) {
		// The remainder of the current value divided by the number of dimensions is the current dimension
		dim = i % numDimensions;

		if (chosenDim == dim) {
			// ensure that numSelectedLoaded does not overflow the slectionIndeces vector to avoid a crash 
			if (numSelectedLoaded != selectionIndices.size() ) {
				// if the index is equal to the current point in the array
				if (selectionIndices[numSelectedLoaded] * numDimensions + chosenDim == i) {
					// write value into the dataArray
					dataArray->SetValue(j, points.getValueAt(i));
					numSelectedLoaded++;

				}
				else {
					// all other indices are non-Object
					dataArray->SetValue(j, backgroundValue);
				}
			}
			else {
				// all other indices are non-Object
				dataArray->SetValue(j, backgroundValue);
			}

			
			j++;

		}



	}
	
	// add scalarData to the imageData object
	imData->GetPointData()->SetScalars(dataArray);
	
	// here i have chosen not to call renderdata in order to perserve the clipping in the new renderer
	
	// return the selection imagedata object
	return imData;
}

