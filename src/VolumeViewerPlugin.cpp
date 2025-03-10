/** General headers*/
#include <vector>
/** QT headers*/
#include <QDebug>
#include <QMimeData>
#include <QLayout>
/** Plugin headers*/
#include "VolumeViewerPlugin.h"
#include "ViewerWidget.h"
//#include "Transfer/CustomColorMapEditor.h"
#include <widgets/DropWidget.h>
#include <QMenu>
#include <actions/PluginTriggerAction.h>
#include <DatasetsMimeData.h>

/** HDPS headers*/
#include "PointData/PointData.h"
#include <ClusterData/Cluster.h>
#include <ClusterData/ClusterData.h>


#include "ColorData/ColorData.h"

/** VTK headers*/
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>




using namespace mv;
using namespace mv::gui;
using namespace mv::util;

VolumeViewerPlugin::VolumeViewerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _viewerWidget(nullptr),
    //_transferWidget(nullptr),
    //_selectionData(vtkSmartPointer<vtkImageData>::New()),
    _imageData(vtkSmartPointer<vtkImageData>::New()),
    // initiate a planeCollection for the SlicingAction
    _planeCollection(vtkSmartPointer<vtkPlaneCollection>::New()),
    _points(),
    _pointsParent(),
    _pointsColorCluster(),
    _pointsColorPoints(),
    _selectionDisabled(false),
    _pointsOpacityPoints(),
    _rendererSettingsAction(),
    _dropWidget(nullptr),
    // initiate a vector containing the current state and index of the x,y and z slicingplanes. 0 means no plane initiated, 1,2 or 3 indicate the index+1 of the x,y,z slicingplane in the planeCollection
    _planeArray(std::vector<int>(3, 0)),
    _position(std::vector<double>(3, 0)),
    // boolian to indicate if data is loaded for selection visualization purposes
    _dataLoaded(false),
    // boolian to indicate if data has been selected in a scatterplot
    _dataSelected(false),
    // Boolian to indicate if shading has been enabled;
    _shadingEnabled(false),
    // Boolian to indicate if non-selected data should be shown
    _backgroundEnabled(true),
    _pointCloudEnabled(true),
    // float to indicate alpha value of background when data is selected
    _backgroundAlpha(0.02),
    // Boolian to indicate wheter selected data should be opaque or use the transfer function
    _selectionOpaque(true),
    // Shading parameter vector.
    _shadingParameters(std::vector<double>{0.9, 0.2, 0.1}),
    // string variable to keep track of the interpolation option with default being Nearest Neighbour
    _interpolationOption("NN"),
    _pointColorLoaded(false),
    _pointOpacityLoaded(false),
    _clusterLoaded(false)
{
    getWidget().setContextMenuPolicy(Qt::CustomContextMenu);
    getWidget().setFocusPolicy(Qt::ClickFocus);
}

void VolumeViewerPlugin::init()
{
    // Add the viewerwidget and dropwidget to the layout.
    _viewerWidget = new ViewerWidget(*this);
    // Add the dropwidget to the layout.
    _dropWidget = new DropWidget(_viewerWidget);
    _rendererSettingsAction = new RendererSettingsAction(this, _viewerWidget, "Primary Toolbar");
    //_rendererSettingsAction->addAction(&_rendererSettingsAction.getRendererSettingsAction().getDimensionAction());
    
    
    // Create the layout.
    auto layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_viewerWidget, 1);


    //auto settingsLayout = new QVBoxLayout();

    
    //_rendererSettingsAction->setShowLabels(true);
    //_rendererSettingsAction->addAction(&_rendererSettingsAction->getColoringAction());
    //_rendererSettingsAction->
    //settingsLayout->setContentsMargins(6, 6, 6, 6);

    //_primaryGroupSettings.addAction(&_rendererSettingsAction->getRenderSettingsAction());
    //layout->addWidget(_rendererSettingsAction->createWidget(&getWidget()));
    

    //layout->addLayout(settingsLayout, 1);

    //getWidget().setAutoFillBackground(true);
    getWidget().setLayout(layout);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    // Initialize the drop regions
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {

        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->text();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ PointType , ColorType, ClusterType });

        // Visually indicate if the dataset is of the wrong data type and thus cannot be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "", "This type of data is not supported", false);
        }
        else {
            // Accept points datasets drag and drop
            if (dataType == PointType) {
                const auto candidateDataset = mv::data().getDataset<Points>(datasetId);
                //const auto candidateDatasetName = candidateDataset.getName();
                const auto description = QString("Visualize %1 as voxels").arg(candidateDataset->getGuiName());

                if (!_points.isValid()) {
                    dropRegions << new DropWidget::DropRegion(this, "Position", description, "cube", true, [this, candidateDataset]() {
                        _points = candidateDataset;
                        if (_points->getDataHierarchyItem().hasParent()) {
                            _pointsParent = _points->getParent();
                        }
                        
                        //_rendererSettingsAction.reset();
                        
                        //GroupsAction::GroupActions groupActionsPointcloudData;
                        //groupActionsPointcloudData << &_rendererSettingsAction.getColoringAction() << &_rendererSettingsAction.getSelectedPointsAction();
                        //_rendererSettingsAction.setActionGroup()
                        //_rendererSettingsAction.setGroupActions(groupActionsPointcloudData);

                    });
                    
                }
                else {
                    if (candidateDataset == _points) {
                        dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                    }
                    else {
                        dropRegions << new DropWidget::DropRegion(this, "Voxels", description, "cube", true, [this, candidateDataset]() {
                            _points = candidateDataset;
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsParent = _points->getParent();
                            }
                            // Add the actions.
                            //_rendererSettingsAction.resetGroupActions();
                            //GroupsAction::GroupActions groupActions;
                            //groupActions << &_rendererSettingsAction.getDimensionAction() << &_rendererSettingsAction.getSlicingAction() << &_rendererSettingsAction.getColoringAction() << &_rendererSettingsAction.getSelectedPointsAction();
                            //_rendererSettingsAction.setGroupActions(groupActions);
                        });
                        dropRegions << new DropWidget::DropRegion(this, "Colors and Point Opacity", "Color and Opacity points by scalars", "palette", true, [this, candidateDataset]() {
                            //_points = candidateDataset;
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsColorPoints = candidateDataset;
                                _pointsOpacityPoints = candidateDataset;
                            }
                        });

                        dropRegions << new DropWidget::DropRegion(this, "Colors", "Color points by scalars", "palette", true, [this, candidateDataset]() {
                            //_points = candidateDataset;
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsColorPoints = candidateDataset;
                            }
                        });
                        
                        dropRegions << new DropWidget::DropRegion(this, "Point Opacity", "Opacity by scalars", "brush", true, [this, candidateDataset]() {
                            //_points = candidateDataset;
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsOpacityPoints = candidateDataset;
                            }
                        });
                        //if (candidateDataset->getNumPoints() == _points->getNumPoints()) {

                        //    // The number of points is equal, so offer the option to use the points dataset as source for points colors
                        //    dropRegions << new DropWidget::DropRegion(this, "Point color", QString("Colorize %1 points with %2"), "palette", true, [this, candidateDataset]() {
                        //        //_rendererSettingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //        //_rendererSettingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                        //        _pointsColorCluster = candidateDataset;
                        //    });

                    }
                }
            }
        }
        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {


            // Get clusters dataset from the core
            auto candidateDataset = mv::data().getDataset<Clusters>(datasetId);
            

            // Establish drop region description
            const auto description = QString("Color points by %1").arg(candidateDataset->getGuiName());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_points.isValid()) {

                if (true) {

                    // The clusters dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        
                        _pointsColorCluster = candidateDataset;
                    });
                }
                else {

                    // Use the clusters set for points color
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        //_rendererSettingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //_rendererSettingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                    });
                }
            }
            else {

                // Only allow user to color by clusters when there is a positions dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", "exclamation-circle", false);
            }
        }
        return dropRegions;
    });

    connect(_viewerWidget, &ViewerWidget::customContextMenuRequested, this, [this](const QPoint& point) {
        
        std::cout << "i made it" << std::endl;
        //_rendererSettingsAction->getContextMenu()->exec(getWidget().mapToGlobal(point));
    });

    
    addDockingAction(_rendererSettingsAction, nullptr, DockAreaFlag::Left, true, AutoHideLocation::Right, QSize(300, 300));

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::changed, this, [this]() {
        // Get current dimension index
        unsigned int chosenDimension = _rendererSettingsAction->getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex(); // get the currently selected chosen dimension as indicated by the dimensionchooser in the options menu

        // Update the dimensionpicker action.
        _rendererSettingsAction->getDimensionAction().getDimensionPickerAction().setPointsDataset(Dataset<Points>(_points));
        // hide dropwidget
        _dropWidget->setShowDropIndicator(false);

        // Check if chosen dimension does not exeed the amount of dimensions, otherwise use chosenDimension=0.
        if (chosenDimension > _points->getNumDimensions() - 1) {
            // Pass the dataset and dimension 0 to the viewerwidget which initiates the data and renders it. returns the imagedata object for future operations.
            _imageData = _viewerWidget->setData(*_points, 0, _interpolationOption, _colorMap);
        }
        else {
            // Pass the dataset and chosen dimension to the viewerwidget which initiates the data and renders it. returns the imagedata object for future operations.
            _imageData = _viewerWidget->setData(*_points, chosenDimension, _interpolationOption, _colorMap);
        }

        //Initial render.
        runRenderData();

        // set the maximum x, y and z values for the slicing options
        _rendererSettingsAction->getSlicingAction().getXAxisPositionAction().setMaximum(_imageData->GetDimensions()[0]);
        _rendererSettingsAction->getSlicingAction().getYAxisPositionAction().setMaximum(_imageData->GetDimensions()[1]);
        _rendererSettingsAction->getSlicingAction().getZAxisPositionAction().setMaximum(_imageData->GetDimensions()[2]);

        // notify that data is indeed loaded into the widget
        _dataLoaded = true;
    });


    connect(&_pointsColorPoints, &Dataset<Points>::dataChanged, this, [this]() {
        

        if (!_pointOpacityLoaded && _pointColorLoaded) {
            _viewerWidget->setPointsColor(*_pointsColorPoints, true);
            runRenderData();
        }
        
    });

    connect(&_pointsOpacityPoints, &Dataset<Points>::dataChanged, this, [this]() {
        _viewerWidget->setPointsOpacity(*_pointsOpacityPoints, true);

        if (!_pointOpacityLoaded && _pointColorLoaded) {
            runRenderData();
        }
        else {
            _viewerWidget->setPointsColor(*_pointsColorPoints, true);
            runRenderData();
        }
    });


    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorCluster, &Dataset<Clusters>::changed, this, [this]() {
        
        _viewerWidget->setClusterColor(*_pointsColorCluster, true);
        _clusterLoaded = true;
        //Initial render.
        runRenderData();

    });
    
    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorPoints, &Dataset<Points>::changed, this, [this]() {
        
        if (_clusterLoaded) {
            _clusterLoaded = false;
            _viewerWidget->setClusterColor(*_pointsColorCluster, false);
        }
        _viewerWidget->setPointsColor(*_pointsColorPoints, true);
        _pointColorLoaded = true;
        //Initial render.
        runRenderData();

    });
    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsOpacityPoints, &Dataset<Points>::changed, this, [this]() {
        if (_clusterLoaded) {
            _clusterLoaded = false;
            _viewerWidget->setClusterColor(*_pointsColorCluster, false);
        }
        _viewerWidget->setPointsOpacity(*_pointsOpacityPoints, true);
        _pointOpacityLoaded = true;
        //Initial render.
        runRenderData();

    });
    // Respond when the name of the dataset in the dataset reference changes// Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorCluster, &Dataset<Clusters>::dataChanged, this, [this]() {
        
        _viewerWidget->setClusterColor(*_pointsColorCluster, true);

        //Initial render.
        runRenderData();

    });// Respond when the name of the dataset in the dataset reference changes
    

    // Dropdown menu for chosen dimension.
    connect(&this->getRendererSettingsAction().getDimensionAction().getDimensionPickerAction(), &DimensionPickerAction::currentDimensionIndexChanged, this, [this](const int& value) {
        // check if there is a dataset loaded in
        if (_dataLoaded) {
            // get the value of the chosenDimension
            int chosenDimension = value;

            // pass the dataset and chosen dimension to the viewerwidget which initiates the data and renders it. returns the imagedata object for future operations
            _imageData = _viewerWidget->setData(*_points, chosenDimension, _interpolationOption, _colorMap);

            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();

            // get selectionData
            _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension);
            runRenderData();

        }
    });


    // Shading enabled/disabled.
    connect(&this->getRendererSettingsAction().getColoringAction().getShadingAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        // Check if te slicing is turned on or off
        _shadingEnabled = toggled;
        this->getRendererSettingsAction().getColoringAction().getAmbientAction().setDisabled(!toggled);
        this->getRendererSettingsAction().getColoringAction().getDiffuseAction().setDisabled(!toggled);
        this->getRendererSettingsAction().getColoringAction().getSpecularAction().setDisabled(!toggled);

        if (_dataLoaded) {
            runRenderData();
        }

    });
    
    connect(&this->getRendererSettingsAction().getColoringAction().getdisableSelectionAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        _viewerWidget->disableSelection(toggled);
        _selectionDisabled = toggled;
        
        
        if (_dataLoaded) {
            runRenderData();
        }

    });


    // Shading parameter change.
    // Ambient parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getAmbientAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[0] = value;

        // Check if shading is enbabled.
        if (_shadingEnabled) {
            runRenderData();
        }
    });
    // Ambient parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getDiffuseAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[1] = value;

        runRenderData();
    });
    // Specular parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getSpecularAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[2] = value;

        // Check if shading is enbabled.
        if (_shadingEnabled) {
            runRenderData();
        }
    });

    // When the enable x, y and z Slicing are changed add or remove that slicing 

    // xSlicing tickbox
    connect(&this->getRendererSettingsAction().getSlicingAction().getXAxisEnabledAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            // Enable the x-Position slider when the slice is enabled.
            this->getRendererSettingsAction().getSlicingAction().getXAxisPositionAction().setDisabled(false);
        }
        else {
            // Disable the x-Position slider when the slice is disabled.
            this->getRendererSettingsAction().getSlicingAction().getXAxisPositionAction().setDisabled(true);

        }

        // Check if data is loaded to prevent errors.
        if (_dataLoaded) {
            // Check if te slicing is turned on or off
            if (toggled) {
                // check if ther is already a x slicing plane active, if so remove it
                if (_planeArray[0] != 0) {
                    _planeCollection->RemoveItem(_planeArray[0] - 1);
                }

                // get the x value for which the slice needs to be performed
                int value = _rendererSettingsAction->getSlicingAction().getXAxisPositionAction().getValue();

                // Create a clipping plane for the xvalue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(value, 0.0, 0.0);
                clipPlane->SetNormal(1, 0.0, 0.0);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the x slicing plane
                _planeArray[0] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
            else {
                // if the toggle is unclicked remove the xclipping plane from the collection
                _planeCollection->RemoveItem(_planeArray[0] - 1);

                // due to the removal of the xPlane, if the was not the last item in the plane collection the others will slide back,
                // to compensate this in the index tracker the following 2 if functions are created.
                // This is probably not the most elegant way to solve the issue however it is the only one i could come up with

                // if the xPlane index was smaller than the yplane index and the yplaneindex is present, slide the yindex back 1 spot
                if (_planeArray[0] < _planeArray[1] && _planeArray[1] != 0) {
                    _planeArray[1] = _planeArray[1] - 1;
                }
                // if the xPlane index was smaller than the yPlane index and the yplaneindex is present, slide the yindex back 1 spot
                if (_planeArray[0] < _planeArray[2] && _planeArray[2] != 0) {
                    _planeArray[2] = _planeArray[2] - 1;
                }

                // Set the xPlane index to 0  (not active)
                _planeArray[0] = 0;


                runRenderData();
            }
        }
    });
    // ySlicing tickbox
    connect(&this->getRendererSettingsAction().getSlicingAction().getYAxisEnabledAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            // Enable the y-Position slider when the slice is enabled.
            this->getRendererSettingsAction().getSlicingAction().getYAxisPositionAction().setDisabled(false);
        }
        else {
            // Disable the y-Position slider when the slice is disabled.
            this->getRendererSettingsAction().getSlicingAction().getYAxisPositionAction().setDisabled(true);

        }

        // Dataloaded check to prevent errors
        if (_dataLoaded) {
            // Check if te slicing is turned on or off
            if (toggled) {
                // check if ther is already a y slicing plane active, if so remove it

                if (_planeArray[1] != 0) {
                    _planeCollection->RemoveItem(_planeArray[1] - 1);
                }

                // get the y value for which the slice needs to be performed
                int value = _rendererSettingsAction->getSlicingAction().getYAxisPositionAction().getValue();

                // Create a clipping plane for the yValue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(0.0, value, 0.0);
                clipPlane->SetNormal(0, 1, 0.0);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the y slicing plane
                _planeArray[1] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
            else {
                // if the toggle is unclicked remove the yClipping plane from the collection
                _planeCollection->RemoveItem(_planeArray[1] - 1);

                // due to the removal of the yPlane, if the was not the last item in the plane collection the others will slide back,
                // to compensate this in the index tracker the following 2 if functions are created.
                // This is probably not the most elegant way to solve the issue however it is the only one i could come up with

                // if the yPlane index was smaller than the xPlane index and the yplaneindex is present, slide the xIndex back 1 spot
                if (_planeArray[1] < _planeArray[0] && _planeArray[0] != 0) {
                    _planeArray[0] = _planeArray[0] - 1;
                }
                // if the yPlane index was smaller than the zPlane index and the zplaneindex is present, slide the zIndex back 1 spot
                if (_planeArray[1] < _planeArray[2] && _planeArray[2] != 0) {
                    _planeArray[2] = _planeArray[2] - 1;
                }

                // Set the yPlane index to 0  (not active)
                _planeArray[1] = 0;

                runRenderData();
            }
        }
    });
    // zSlicing tickbox
    connect(&this->getRendererSettingsAction().getSlicingAction().getZAxisEnabledAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            // Enable the z-Position slider when the slice is enabled.
            this->getRendererSettingsAction().getSlicingAction().getZAxisPositionAction().setDisabled(false);
        }
        else {
            // Disable the z-Position slider when the slice is disabled.
            this->getRendererSettingsAction().getSlicingAction().getZAxisPositionAction().setDisabled(true);

        }

        // Dataloaded check to prevent errors
        if (_dataLoaded) {
            // Check if te slicing is turned on or off
            if (toggled) {
                // check if ther is already a z slicing plane active, if so remove it
                if (_planeArray[2] != 0) {
                    _planeCollection->RemoveItem(_planeArray[2] - 1);
                }

                // get the z value for which the slice needs to be performed
                int value = _rendererSettingsAction->getSlicingAction().getZAxisPositionAction().getValue();

                // Create a clipping plane for the zValue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(0.0, 0.0, value);
                clipPlane->SetNormal(0, 0.0, 1);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the z slicing plane
                _planeArray[2] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
            else {
                // if the toggle is unclicked remove the yClipping plane from the collection
                _planeCollection->RemoveItem(_planeArray[2] - 1);

                // due to the removal of the yPlane, if the was not the last item in the plane collection the others will slide back,
               // to compensate this in the index tracker the following 2 if functions are created.
               // This is probably not the most elegant way to solve the issue however it is the only one i could come up with

                // if the zPlane index was smaller than the xPlane index and the yplaneindex is present, slide the xIndex back 1 spot
                if (_planeArray[2] < _planeArray[0] && _planeArray[0] != 0) {
                    _planeArray[0] = _planeArray[0] - 1;
                }
                // if the zPlane index was smaller than the yPlane index and the zplaneindex is present, slide the yIndex back 1 spot
                if (_planeArray[2] < _planeArray[1] && _planeArray[1] != 0) {
                    _planeArray[1] = _planeArray[1] - 1;
                }

                // Set the zPlane index to 0  (not active)
                _planeArray[2] = 0;

                runRenderData();
            }
        }
    });

    // When the value of the x,y and z slicing sliders are changed change the slicing index if the tickbox is ticked

    // xSlicing slider
    connect(&this->getRendererSettingsAction().getSlicingAction().getXAxisPositionAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        //Check if data is loaded to prevent errors.
        if (_dataLoaded) {
            // get the current value of the xSlicing tickbox
            bool toggled = _rendererSettingsAction->getSlicingAction().getXToggled();

            // if the tickbox is enabled perform the slicing change
            if (toggled) {
                // check if ther is already a x slicing plane active, if so remove it
                if (_planeArray[0] != 0) {
                    _planeCollection->RemoveItem(_planeArray[0] - 1);
                }

                // convert float value to integer
                int intValue = value;

                // Create a clipping plane for the xValue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(intValue, 0.0, 0.0);
                clipPlane->SetNormal(1, 0.0, 0.0);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the x slicing plane
                _planeArray[0] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
        }
    });
    // xSlicing slider
    connect(&this->getRendererSettingsAction().getSlicingAction().getYAxisPositionAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        //Check if data is loaded to prevent errors.
        if (_dataLoaded) {
            // get the current value of the ySlicing tickbox
            bool toggled = _rendererSettingsAction->getSlicingAction().getYToggled();

            // if the tickbox is enabled perform the slicing change
            if (toggled) {
                // check if ther is already a y slicing plane active, if so remove it
                if (_planeArray[1] != 0) {
                    _planeCollection->RemoveItem(_planeArray[1] - 1);
                }

                // convert float value to integer
                int intValue = value;

                // Create a clipping plane for the yValue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(0.0, intValue, 0.0);
                clipPlane->SetNormal(0.0, 1, 0.0);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the y slicing plane
                _planeArray[1] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
        }
    });
    // xSlicing slider
    connect(&this->getRendererSettingsAction().getSlicingAction().getZAxisPositionAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        //Check if data is loaded to prevent errors.
        if (_dataLoaded) {
            // get the current value of the zSlicing tickbox
            bool toggled = _rendererSettingsAction->getSlicingAction().getZToggled();

            // if the tickbox is enabled perform the slicing change
            if (toggled) {
                // check if ther is already a z slicing plane active, if so remove it
                if (_planeArray[2] != 0) {
                    _planeCollection->RemoveItem(_planeArray[2] - 1);
                }

                // convert float value to integer
                int intValue = value;

                // Create a clipping plane for the zValue
                vtkSmartPointer<vtkPlane> clipPlane = vtkSmartPointer<vtkPlane>::New();
                clipPlane->SetOrigin(0.0, 0.0, intValue);
                clipPlane->SetNormal(0.0, 0.0, 1);

                // add the plane to the to the collection
                _planeCollection->AddItem(clipPlane);

                // store the index+1 of the y slicing plane
                _planeArray[2] = _planeCollection->GetNumberOfItems();

                runRenderData();
            }
        }
    });

    // Interpolation Selector
    connect(&this->getRendererSettingsAction().getColoringAction().getInterpolationAction(), &OptionAction::currentTextChanged, this, [this](const QString& interpolType) {
        // change the interpolation type for the colormap, nearest neighbor is best for inspecting transition from object to nonobject due to the transition artifact that appears in linear and cubic
        // however linear and cubic give a better looking representation of a sliced object
        std::string type = interpolType.toStdString();
        if (type == "Nearest Neighbor") {
            qDebug() << "Changed interpolation type to: NEAREST NEIGHBOR";
            _interpolationOption = "NN";
        }
        else if (type == "Linear") {

            qDebug() << "Changed interpolation type to: LINEAR";
            _interpolationOption = "LIN";
        }
        else if (type == "Cubic") {
            qDebug() << "Changed interpolation type to: CUBIC";
            _interpolationOption = "CUBE";
        }
        else {
            qDebug() << "Invalid interpolationtype, using default Nearest Neighbor";
            _interpolationOption = "NN";
        }
        if (_dataLoaded) {

            runRenderData();
        }
    });

    // Surrounding data enabled selector
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundShowAction(), &OptionAction::currentTextChanged, this, [this](const QString& show) {
        // Selector option handeling
        if (show == "Show background") {
            _backgroundEnabled = true;
            // Enable the alpha slider for the background
            this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction().setDisabled(false);
        }
        else {
            _backgroundEnabled = false;
            // Disable the alpha slider for the background
            this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction().setDisabled(true);
        }

        if (_dataSelected) {
            runRenderData();
        }
    });

    // full render or point cloud
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getPointCloudAction(), &OptionAction::currentTextChanged, this, [this](const QString& option) {
        // Selector option handeling
        if (option == "Point Cloud") {
            _pointCloudEnabled = true;

        }
        else {
            _pointCloudEnabled = false;

        }

        //if (_dataSelected) {
        //    runRenderData();
        //}
    });
    // Background alpha slider
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getBackgroundAlphaAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        if (_backgroundEnabled) {
            _backgroundAlpha = value;
            if (_dataSelected) {
                runRenderData();
            }
        }
    });
    // Selection alpha setting selector
    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectionAlphaAction(), &OptionAction::currentTextChanged, this, [this](const QString& opaqueOrTf) {
        if (opaqueOrTf == "Opaque") {
            _selectionOpaque = true;
        }
        else {
            _selectionOpaque = false;
        }
        if (_dataSelected) {
            runRenderData();
        }
    });

    connect(&this->getRendererSettingsAction().getSelectedPointsAction().getSelectPointAction(), &TriggerAction::triggered, this, [this]() {
        float lowerThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getThresholdAction().getLowerThresholdAction().getValue();
        float upperThreshold = this->getRendererSettingsAction().getSelectedPointsAction().getThresholdAction().getUpperThresholdAction().getValue();

        //_points->setSelectionIndices();
        int chosenDimension = _rendererSettingsAction->getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex();
        _viewerWidget->connectedSelection(*_points, chosenDimension, _viewerWidget->getSelectedCellCoordinate(), upperThreshold, lowerThreshold);
        const auto& selectionSet = _points->getSelection<Points>();
        //auto test = selectionSet->indices;
        _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension);
        runRenderData();
        
        events().notifyDatasetDataSelectionChanged(_points);
    });

    connect(&this->getRendererSettingsAction().getColoringAction().getUnloadColorMap(), &TriggerAction::triggered, this, [this]() {

        if (_clusterLoaded) {
            _viewerWidget->setClusterColor(*_pointsColorCluster, false);
            
            _pointsColorCluster = NULL;
        }
        else if (_pointColorLoaded) {
            _viewerWidget->setPointsColor(*_pointsColorPoints, false);
            
            _pointsColorPoints = NULL;
        }
       
        

        //Initial render.
        runRenderData();
    });
    connect(&this->getRendererSettingsAction().getColoringAction().getUnloadOpacityData(), &TriggerAction::triggered, this, [this]() {
        _viewerWidget->setPointsOpacity(*_pointsOpacityPoints, false);
        _pointsOpacityPoints = NULL;

        //Initial render.
        runRenderData();
    });

    // Colormap selector
    connect(&getRendererSettingsAction().getColoringAction().getColorMapAction(), &ColorMapAction::imageChanged, this, [this](const QImage& colorMapImage) {
        if (_dataLoaded) {
            runRenderData();
        }
    });

    // Selection changed connection.
    connect(&_points, &Dataset<Points>::dataSelectionChanged, this, [this] {
        // if data is loaded

        if (_dataLoaded && !_selectionDisabled) {

            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();

            // Get ChosenDimension
            int chosenDimension = _rendererSettingsAction->getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex();

            const auto backGroundValue = _imageData->GetScalarRange()[0];

            // create a selectiondata imagedata object with 0 values for all non selected items
            _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension);


            // if the selection is not empty add the selection to the vector 
            if (selectionSet->indices.size() != 0) {

                _dataSelected = true;
            }
            else {
                _dataSelected = false;
            }

            // Render the data with the current slicing planes and selections
            runRenderData();


        }
    });// Selection changed connection.
    connect(&_pointsParent, &Dataset<Points>::dataSelectionChanged, this, [this] {
        // if data is loaded

        if (_dataLoaded && !_selectionDisabled) {

            // Get the selection set that changed
            const auto& selectionSet = _pointsParent->getSelection<Points>();

            // Get ChosenDimension
            int chosenDimension = _rendererSettingsAction->getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex();

            const auto backGroundValue = _imageData->GetScalarRange()[0];

            // create a selectiondata imagedata object with 0 values for all non selected items
            _viewerWidget->setSelectedData(*_points, selectionSet->indices, chosenDimension);


            // if the selection is not empty add the selection to the vector 
            if (selectionSet->indices.size() != 0) {

                _dataSelected = true;
            }
            else {
                _dataSelected = false;
            }

            // Render the data with the current slicing planes and selections
            runRenderData();


        }
    });


}

void VolumeViewerPlugin::reInitializeLayout(QHBoxLayout layout) {

}

mv::CoreInterface* VolumeViewerPlugin::getCore()
{
    return _core;
}

VolumeViewerPluginFactory::VolumeViewerPluginFactory()
{
    setIconByName("cube");
}

VolumeViewerPlugin* VolumeViewerPluginFactory::produce()
{
    return new VolumeViewerPlugin(this);
}

mv::DataTypes VolumeViewerPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

mv::gui::PluginTriggerActions VolumeViewerPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> VolumeViewerPlugin* {
        return dynamic_cast<VolumeViewerPlugin*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (numberOfDatasets >= 1) {
            if (datasets.first()->getDataType() == PointType) {
                auto pluginTriggerAction = new PluginTriggerAction(const_cast<VolumeViewerPluginFactory*>(this), this, "Volume viewer", "Load dataset in volume viewer", icon(), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                    for (auto dataset : datasets)
                        getInstance()->loadData(Datasets({ dataset }));
                });

                pluginTriggerActions << pluginTriggerAction;
            }
        }
    }

    return pluginTriggerActions;
}

/** Create a vtkimagedatavector to store the current imagedataand selected data(if present).
*   Vector is needed due to the possibility of having data selected in a scatterplot wich
*   changes the colormapping of renderdata and creates an aditional actor to visualize the selected data.
*/
void VolumeViewerPlugin::runRenderData() {
    _viewerWidget->renderData(_planeCollection, _imageData, _interpolationOption, _colorMap, _shadingEnabled, _shadingParameters);
}

void VolumeViewerPlugin::setSelectionPosition(double x, double y, double z) {
    _position[0] = x;
    _position[1] = y;
    _position[2] = z;

}