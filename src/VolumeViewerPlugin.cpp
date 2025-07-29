/** General headers*/
#include <vector>
/** QT headers*/
#include <QDebug>
#include <QMimeData>
#include <QLayout>
#include <unordered_set>
/** Plugin headers*/
#include "VolumeViewerPlugin.h"
#include <widgets/DropWidget.h>

#include <actions/PluginTriggerAction.h>
#include <DatasetsMimeData.h>

/** mv headers*/
#include "PointData/PointData.h"
#include <ClusterData/Cluster.h>
#include <ClusterData/ClusterData.h>

#include "ColorData/ColorData.h"

#include "cmath"

using namespace mv;
using namespace mv::gui;
using namespace mv::util;

namespace
{
    void normalizeVector(std::vector<float>& v)
    {
        float scalarMin = std::numeric_limits<float>::max();
        float scalarMax = -std::numeric_limits<float>::max();

        // Compute min and max of scalars
        for (int i = 0; i < v.size(); i++)
        {
            if (v[i] < scalarMin) scalarMin = v[i];
            if (v[i] > scalarMax) scalarMax = v[i];
        }
        float scalarRange = scalarMax - scalarMin;

        if (scalarRange != 0)
        {
            float invScalarRange = 1.0f / (scalarMax - scalarMin);
            // Normalize the scalars
            #pragma omp parallel for
            for (int i = 0; i < v.size(); i++)
            {
                v[i] = (v[i] - scalarMin) * invScalarRange;
            }
        }
    }
}

VolumeViewerPlugin::VolumeViewerPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _primaryToolbarAction(this, "PrimaryToolbar"),
    _secondaryToolbarAction(this, "SecondaryToolbar"),
    _settingsAction(),
    _volumeViewerWidget(nullptr),
    // initiate a planeCollection for the SlicingAction
    _points(),
    _pointsParent(),
    _pointsColorCluster(),
    _pointsColorPoints(),
    _selectionDisabled(false),
    _pointsOpacityPoints(),
    _dropWidget(nullptr),
    // initiate a vector containing the current state and index of the x,y and z slicingplanes. 0 means no plane initiated, 1,2 or 3 indicate the index+1 of the x,y,z slicingplane in the planeCollection
    _planeArray(std::vector<int>(3, 0)),
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
    // Add the viewerwidget and dropwidget to the layout.
    _volumeViewerWidget = new VolumeViewerWidget(this, "Volume Viewer Widget");
    // Add the dropwidget to the layout.
    _dropWidget = new DropWidget(_volumeViewerWidget);
    _settingsAction = new SettingsAction(this, "SettingsAction");

    _primaryToolbarAction.addAction(&_settingsAction->getPickRendererAction(), 4, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction->getToggleFullScreen());


    _secondaryToolbarAction.addAction(&_settingsAction->getStartCalibAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getConnectToTrackerAction());
    /*_secondaryToolbarAction.addAction(&_settingsAction->getFocusSelectionAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusSelectionNormAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusFloodfillAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusFloodfillNormAction());*/
    _secondaryToolbarAction.addAction(&_settingsAction->getEyeOffsetAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getCamDistAction());
    //_secondaryToolbarAction.addAction(&_settingsAction->getFlipInterlacingAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getSelectionColorPicker());
    _secondaryToolbarAction.addAction(&_settingsAction->getSelectModeAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getClearSelectionAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getPointOpacityAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFlashlightAction());

    getVolumeRenderer().setSelectionColor(_settingsAction->getSelectionColorPicker().getColor());
}

void VolumeViewerPlugin::init()
{    


    connect(getOpenGLRendererWidget(), &OpenGLRendererWidget::ready, this, [this](const OpenGLRendererWidget* emitter) {
        //_secondaryToolbarAction.removeAction(&_settingsAction->getConnectToTrackerAction());
    });

    

	// Detect other instances of the plugin
	const std::vector<Plugin*> instances = plugins().getPluginsByFactory(getFactory());
	getOpenGLRendererWidget()->setInstanceIndex(instances.size());

	// Get the tracker object from the first instance.
    qDebug() << "Number of previous instances :" << instances.size();
	if (instances.size() > 0) {
		VolumeViewerPlugin* firstInstance = static_cast<VolumeViewerPlugin*>(instances[0]);

		connect(firstInstance->getOpenGLRendererWidget(), &OpenGLRendererWidget::ready, this, [this](const OpenGLRendererWidget* emitter) {
			getOpenGLRendererWidget()->setTracker(emitter->getTracker());
            getOpenGLRendererWidget()->setFullScreenWidget(emitter->getFullScreenWidget());
            getOpenGLRendererWidget()->setUpdateTimer(emitter->getUpdateTimer());
            getOpenGLRendererWidget()->setPedalManager(emitter->getPedalManager());
            getOpenGLRendererWidget()->initiateFlashlightWidget(emitter->getFlashlightWidget());

			//_secondaryToolbarAction.removeAction(&_settingsAction->getConnectToTrackerAction());
			});

    }
    else {
        // Create a new full screen widget
        qDebug() << "Creatig FSW";
        FullScreenWidget* fsWidget = new FullScreenWidget(getVolumeViewerWidget());
     
        // Create a new update Timer
        qDebug() << "Creatig Update Timer";
        QTimer* updateTimer = new QTimer(getVolumeViewerWidget());
        updateTimer->start(16);

        // Create a new pedalManager
        qDebug() << "Creatig Pedal Manager";
        PedalManager* pedals = new PedalManager(getVolumeViewerWidget(), updateTimer);

        getOpenGLRendererWidget()->setFullScreenWidget(fsWidget);
        getOpenGLRendererWidget()->setUpdateTimer(updateTimer);
        getOpenGLRendererWidget()->setPedalManager(pedals);
        getOpenGLRendererWidget()->initiateFlashlightWidget();

    }
	// Request tracker from the first instance
	requestTracker();


    //// Create of search for shared variables
    //FullScreenWidget* fsWidget = nullptr;
    //PedalManager* pedals = nullptr;
    //QTimer* updateTimer = nullptr;

    //for (Plugin* plugin : instances) {
    //    VolumeViewerPlugin* instance = static_cast<VolumeViewerPlugin*>(plugin);
    //    if (instance->getOpenGLRendererWidget()->getFullScreenWidget() != nullptr) {
    //        fsWidget = instance->getOpenGLRendererWidget()->getFullScreenWidget();
    //        break;
    //    }
    //    if (instance->getOpenGLRendererWidget()->getPedalManager() != nullptr) {
    //        pedals = instance->getOpenGLRendererWidget()->getPedalManager();
    //        break;
    //    }
    //    if (instance->getOpenGLRendererWidget()->getUpdateTimer() != nullptr) {
    //        updateTimer = instance->getOpenGLRendererWidget()->getUpdateTimer();
    //        break;
    //    }
    //}

    //if (fsWidget == nullptr) {
    //    // Create a new full screen widget
    //    qDebug() << "Creatig FSW";
    //    fsWidget = new FullScreenWidget(getVolumeViewerWidget());
    //}
    //getOpenGLRendererWidget()->setFullScreenWidget(fsWidget);

    //if (updateTimer == nullptr) {
    //    // Create a new update Timer
    //    qDebug() << "Creatig Update Timer";
    //    updateTimer = new QTimer(getVolumeViewerWidget());
    //    updateTimer->start(16);
    //}
    //getOpenGLRendererWidget()->setUpdateTimer(updateTimer);

    //if (pedals == nullptr) {
    //    // Create a new pedalManager
    //    qDebug() << "Creatig Pedal Manager";
    //    pedals = new PedalManager(getVolumeViewerWidget(), updateTimer);
    //}
    //getOpenGLRendererWidget()->setPedalManager(pedals);

    // Old way : update flashlight on every frame : connect(updateTimer, &QTimer::timeout, this, &VolumeViewerPlugin::updateFlashlight);
    connect(getOpenGLRendererWidget(), &OpenGLRendererWidget::flashlightReady, this, &VolumeViewerPlugin::updateFlashlight);



    // Create the layout.
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    layout->addWidget(_volumeViewerWidget, 1);
    layout->addWidget(_secondaryToolbarAction.createWidget(&getWidget()));

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
                           
                        });
                        dropRegions << new DropWidget::DropRegion(this, "Colors and Point Opacity", "Color and Opacity points by scalars", "palette", true, [this, candidateDataset]() {
                          
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsColorPoints = candidateDataset;
                                _pointsOpacityPoints = candidateDataset;
                            }
                        });

                        dropRegions << new DropWidget::DropRegion(this, "Colors", "Color points by scalars", "palette", true, [this, candidateDataset]() {
                          
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsColorPoints = candidateDataset;
                            }
                        });
                        
                        dropRegions << new DropWidget::DropRegion(this, "Point Opacity", "Opacity by scalars", "brush", true, [this, candidateDataset]() {
                        
                            if (_points->getDataHierarchyItem().hasParent()) {
                                _pointsOpacityPoints = candidateDataset;
                            }
                        });

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

    addDockingAction(_settingsAction, nullptr, DockAreaFlag::Left, true, AutoHideLocation::Right, QSize(300, 300));

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_points, &Dataset<Points>::changed, this, [this]() {
        // Get current dimension index
        unsigned int chosenDimension = _settingsAction->getRendererSettingsAction().getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex(); // get the currently selected chosen dimension as indicated by the dimensionchooser in the options menu

        // Update the dimensionpicker action.
        _settingsAction->getRendererSettingsAction().getDimensionAction().getDimensionPickerAction().setPointsDataset(Dataset<Points>(_points));
        // hide dropwidget
        _dropWidget->setShowDropIndicator(false);

        _volumeViewerWidget->setData(_points);

        // notify that data is indeed loaded into the widget
        _dataLoaded = true;
    });

    connect(&_pointsColorPoints, &Dataset<Points>::dataChanged, this, [this]() {
        if (_rendererBackend == RendererBackend::OpenGL)
        {
            updatePointColors();
        }
    });

    connect(&_pointsOpacityPoints, &Dataset<Points>::dataChanged, this, [this]() {
        _pointOpacityLoaded = true;

        if (_rendererBackend == RendererBackend::OpenGL)
        {

            updatePointOpacity();

        }

        });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorCluster, &Dataset<Clusters>::changed, this, [this]() {
        _clusterLoaded = true;
        // Only proceed with valid clusters and position dataset
        if (!_pointsColorCluster.isValid() || !_points.isValid())
            return;

        // Create a color map with all the colors from the clusters.
        // The number of pixels is the number of clusters
        const QVector<Cluster>& clusterVec = _pointsColorCluster->getClusters();
        const int bytesPerPixel = 4; // For RGBA format

        // Allocate memory for the image data
        uchar* rgbadata = new uchar[clusterVec.size() * bytesPerPixel];


        for (int i = 0; i < clusterVec.size(); ++i) {

            /*const auto color = clusterVec[i].getColor();
            rgbadata[bytesPerPixel * i] = static_cast<uint32_t>(color.redF()*255.0f);
            rgbadata[bytesPerPixel * i + 1] = static_cast<uint32_t>(color.greenF()*255.0f);
            rgbadata[bytesPerPixel * i + 2] = static_cast<uint32_t>(color.blueF() * 255.0f);
            rgbadata[bytesPerPixel * i + 3] = static_cast<uint32_t>(255.0f);*/
            const auto color = clusterVec[i].getColor();
            rgbadata[i * bytesPerPixel + 0] = static_cast<uchar>(color.blueF() * 255.0f);   // Blue
            rgbadata[i * bytesPerPixel + 1] = static_cast<uchar>(color.greenF() * 255.0f);  // Green
            rgbadata[i * bytesPerPixel + 2] = static_cast<uchar>(color.redF() * 255.0f);    // Red
            rgbadata[i * bytesPerPixel + 3] = 255;

        }

        QImage image(rgbadata, clusterVec.size(), 1, clusterVec.size()* bytesPerPixel, QImage::Format_ARGB32);
        _volumeViewerWidget->getOpenGLWidget()->setColormap(image);
        delete[] rgbadata;
        


        // Mapping from local to global indices
        std::vector<std::uint32_t> globalIndices;

        // Get global indices from the position dataset
        int totalNumPoints = _points->getNumPoints();

        _points->getGlobalIndices(globalIndices);

        // Generate color buffer for global and local colors
        std::vector<float> globalUV(totalNumPoints);
        std::vector<float> localUV(totalNumPoints);

        // Loop over all clusters and populate global colors
        for (int i = 0; i < clusterVec.size(); i++)
        {
            for (const auto& index : clusterVec[i].getIndices())
                globalUV[index] = float(i) / float(clusterVec.size() - 1);

        }

        // Loop over all global indices and find the corresponding local color
        int localColorIndex = 0;
        for (const auto& globalIndex : globalIndices)
            localUV[localColorIndex++] = globalUV[globalIndex];
        

        // Apply colors to scatter plot widget without modification
        _volumeViewerWidget->getOpenGLWidget()->setColors(localUV);

    });
    
    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorPoints, &Dataset<Points>::changed, this, [this]() {
        if (_clusterLoaded) {
            _clusterLoaded = false;
        }
        _pointColorLoaded = true;

        if (_rendererBackend == RendererBackend::OpenGL)
        {
            auto& colorMapAction = getRendererSettingsAction().getColoringAction().getColorMapAction();
            auto colorMapImage = colorMapAction.getColorMapImage();
            _volumeViewerWidget->getOpenGLWidget()->setColormap(colorMapImage);

            updatePointColors();
 
        }
    });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsOpacityPoints, &Dataset<Points>::changed, this, [this]() {
        if (_clusterLoaded) {
            _clusterLoaded = false;
        }
        _pointOpacityLoaded = true;

        if (_rendererBackend == RendererBackend::OpenGL)
        {

            updatePointOpacity();

        }

    });

    // Dropdown menu for chosen dimension.
    connect(&this->getRendererSettingsAction().getDimensionAction().getDimensionPickerAction(), &DimensionPickerAction::currentDimensionIndexChanged, this, [this](const int& value) {
        // check if there is a dataset loaded in
        if (_dataLoaded) {
            // get the value of the chosenDimension
            int chosenDimension = value;

            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();
        }
    });


    // Shading enabled/disabled.
    connect(&this->getRendererSettingsAction().getColoringAction().getShadingAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        // Check if te slicing is turned on or off
        _shadingEnabled = toggled;
        this->getRendererSettingsAction().getColoringAction().getAmbientAction().setDisabled(!toggled);
        this->getRendererSettingsAction().getColoringAction().getDiffuseAction().setDisabled(!toggled);
        this->getRendererSettingsAction().getColoringAction().getSpecularAction().setDisabled(!toggled);
    });
    
    connect(&this->getRendererSettingsAction().getColoringAction().getdisableSelectionAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        _selectionDisabled = toggled;
    });


    // Shading parameter change.
    // Ambient parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getAmbientAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[0] = value;

        // Check if shading is enbabled.
        if (_shadingEnabled) {
            qDebug() << "Shading not implemented in volumeViewerPlugin";
        }
    });
    // Ambient parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getDiffuseAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[1] = value;
    });
    // Specular parameter.
    connect(&this->getRendererSettingsAction().getColoringAction().getSpecularAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        // get the current value of the xSlicing tickbox
        _shadingParameters[2] = value;

        // Check if shading is enbabled.
        if (_shadingEnabled) {
            qDebug() << "Shading not implemented in volumeViewerPlugin";
        }
    });

    // Selection changed connection.
    connect(&_points, &Dataset<Points>::dataSelectionChanged, this, [this] {
        // if data is loaded
        if (_dataLoaded && !_selectionDisabled) {
            // Get the selection set that changed
            const auto& selectionSet = _points->getSelection<Points>();

            // Get ChosenDimension
            int chosenDimension = _settingsAction->getRendererSettingsAction().getDimensionAction().getDimensionPickerAction().getCurrentDimensionIndex();

            // if the selection is not empty add the selection to the vector 
            if (selectionSet->indices.size() != 0) {
                _dataSelected = true;
            }
            else {
                _dataSelected = false;
            }



            std::vector<int> indices;
            indices.assign(selectionSet->indices.begin(), selectionSet->indices.end());

            if (!_focusSelection && !_focusSelectionNorm) {


                std::vector<bool> selected;

                _points->selectedLocalIndices(selectionSet->indices, selected);


                highlightSelection(selected, static_cast<std::int32_t>(selectionSet->indices.size()));
            }
            else {

                // Focus selection
                if (selectionSet->indices.size() >= 1)
                {

                    if (_focusSelection)
                        applyMaskToColors(indices, false);
                    else if (_focusSelectionNorm)
                        applyMaskToColors(indices, true);
                }
            }
        }
    });// Selection changed connection.


    connect(getOpenGLRendererWidget(), &OpenGLRendererWidget::newSelection, this, [this](const SelectionMode& type, const bool& replace) {
        clock_t start, end;
        // Perform selection of closest point
        const QVector3D cursor = getVolumeRenderer().getCursor();
        if (_points.isValid()) {
            std::vector<uint32_t> selection;

            switch (type) {
            case SelectionMode::Nearest: {
                selection.push_back(_volumeViewerWidget->getClosestPoint(cursor));
                break;
            }
            case SelectionMode::Sphere: {
                selection = _volumeViewerWidget->getPointsInSphere(cursor, getVolumeRenderer().getSelectRadius());
                
                break;
            }
            }

            // If shift is down, we add to the new selection the previous selected points
            if (!replace) {
                std::vector<uint32_t> previousSelection = _points->getSelectionIndices();
                // Concatenate the old selection to the new
                selection.insert(selection.end(), previousSelection.begin(), previousSelection.end());
                // Remove duplicates with an efficient method : Convert to unordered_set manually
                std::unordered_set<int> s;
                for (int i : selection)
                    s.insert(i);
                selection.assign(s.begin(), s.end());
                std::sort(selection.begin(), selection.end());
            }

            _points->setSelectionIndices(selection);
            events().notifyDatasetDataSelectionChanged(_points->getSourceDataset<Points>());
        }

    });

}


void VolumeViewerPlugin::setFlashlightState(const bool& state) {
    getVolumeRenderer().setIsFlashlightSource(state);

    if (state)
    {
        if (!flashlightScalars.isValid()) {
            getOpenGLRendererWidget()->getFlashlightWidget()->show();

            flashlightScalars = mv::data().createDataset<Points>("Points", "Flashlight");

            events().notifyDatasetAdded(flashlightScalars);

            getOpenGLRendererWidget()->startFlashlightWorker();
        }

    }
    else {
        events().notifyDatasetAboutToBeRemoved(flashlightScalars);
        mv::data().removeDataset(flashlightScalars);
        getOpenGLRendererWidget()->stopFlashlightWorker();


        // Hide flashlight widget if no plugin uses flashlight
        const std::vector<Plugin*> instances = plugins().getPluginsByFactory(getFactory());
        bool doHide = true;
        for (Plugin* plugin : instances) {
            VolumeViewerPlugin* instance = static_cast<VolumeViewerPlugin*>(plugin);
            if (instance->getFlashlightState()) {
                doHide = false;
                break;
            }
        }
        if(doHide) getOpenGLRendererWidget()->getFlashlightWidget()->hide();
    }
}


void VolumeViewerPlugin::updateFlashlight() {
    if (flashlightScalars.isValid() && _points.isValid()) {
        // Blocking bruteforce way
        // std::vector<float> values = getVolumeViewerWidget()->getPointDistances(getVolumeRenderer().getCursor());

        // Non blocking smart way
        std::vector<float> values = getOpenGLRendererWidget()->getPointDistances();
     
        /*for (int i = 0; i < values.size(); i++) {
            values[i] = (1.f - std::min(1.f, 5*values[i]));
        }*/

        flashlightScalars->setData<float>(values.data(), values.size(), 1);
        events().notifyDatasetDataChanged(flashlightScalars);
    }
}

void VolumeViewerPlugin::setFocusSelection(bool focusSelection) {
    _focusSelection = focusSelection;
    qDebug() << "Focus selection: " << _focusSelection;

    updateFocusMode();
}

void VolumeViewerPlugin::setFocusFloodfill(bool focusFloodfill) {
    _focusFloodfill = focusFloodfill;
    qDebug() << "Focus floodfill: " << _focusFloodfill;

    if (!_floodFillDatasetFound)
        loadFloodfillDataset();

    updateFocusMode();
}

void VolumeViewerPlugin::setFocusSelectionNorm(bool focusSelectionNorm) {
    _focusSelectionNorm = focusSelectionNorm;
    qDebug() << "Focus selection norm: " << _focusSelectionNorm;

    updateFocusMode();
}

void VolumeViewerPlugin::setFocusFloodfillNorm(bool focusFloodfillNorm) {
    _focusFloodfillNorm = focusFloodfillNorm;
    qDebug() << "Focus floodfill norm: " << _focusFloodfillNorm;

    if (!_floodFillDatasetFound)
        loadFloodfillDataset();

    updateFocusMode();
}

void VolumeViewerPlugin::updatePointColors() {
    if (!_pointsColorPoints.isValid()) {
        // TODO :: Tell volumeRenderer to stop displaying colors (hascolors = fasle)
        return;
    }
    std::vector<float> colors;
    for (int i = 0; i < _pointsColorPoints->getNumPoints(); i++) {
        colors.push_back(_pointsColorPoints->getValueAt(i));
    }
    _volumeViewerWidget->getOpenGLWidget()->setColors(colors);
}

void VolumeViewerPlugin::updatePointOpacity() {
    if (!_pointsOpacityPoints.isValid()) {
        // TODO :: Tell volumeRenderer to stop displaying alphas (hasalphas = fasle)

        return;
    }

    
    getVolumeRenderer().setFlashlightState(_pointsOpacityPoints.getDataset()->getGuiName() == "Flashlight");

    std::vector<float> alphas;
    for (int i = 0; i < _pointsOpacityPoints->getNumPoints(); i++) {
        alphas.push_back(_pointsOpacityPoints->getValueAt(i));
    }
    _volumeViewerWidget->getOpenGLWidget()->setAlphas(alphas);
}

void VolumeViewerPlugin::updateFocusMode() {
    qDebug() << "Update focus mode";
    if (!_focusSelection && !_focusFloodfill && !_focusSelectionNorm && !_focusFloodfillNorm) {/*
        std::vector<float> colors;
        _pointsColorPoints->extractDataForDimension(colors, 0);
        _volumeViewerWidget->getOpenGLWidget()->setColors(colors);
        _volumeViewerWidget->getOpenGLWidget()->update();*/
    }
    else if (_focusSelection) {
        std::vector<int> indices;
        const auto& selectionSet = _points->getSelection<Points>();
        indices.assign(selectionSet->indices.begin(), selectionSet->indices.end());
        applyMaskToColors(indices, false);
    }
    else if (_focusFloodfill) {
        std::vector<int> indices;
        getFloodfillIndices(indices);
        applyMaskToColors(indices, false);
    }
    else if (_focusSelectionNorm) {
        std::vector<int> indices;
        const auto& selectionSet = _points->getSelection<Points>();
        indices.assign(selectionSet->indices.begin(), selectionSet->indices.end());
        applyMaskToColors(indices, true);
    }
    else if (_focusFloodfillNorm) {
        std::vector<int> indices;
        getFloodfillIndices(indices);
        applyMaskToColors(indices, true);
    }
}

void VolumeViewerPlugin::loadFloodfillDataset() {
    for (const auto& data : mv::data().getAllDatasets())
    {
        if (data->getGuiName() == "allFloodNodesIndices") {
            _floodFillDataset = data;
            _floodFillDatasetFound = true;
            break;
        }
    }
    if (!_floodFillDatasetFound) {
        qDebug() << "VolumeViewer Warning: No floodFillDataset named allFloodNodesIndices found!";
        return;
    }
    // only connect if the dataset is found
    connect(&_floodFillDataset, &Dataset<Points>::dataChanged, this, [this]() {
        if (_focusFloodfill) {
            std::vector<int> indices;
            getFloodfillIndices(indices);
            applyMaskToColors(indices, false);
        } else if (_focusFloodfillNorm) {
            std::vector<int> indices;
            getFloodfillIndices(indices);
            applyMaskToColors(indices, true);
        }
     });
}

void VolumeViewerPlugin::getFloodfillIndices(std::vector<int>& indices) {
    std::vector<float> floodNodesWave(_floodFillDataset->getNumPoints());
    _floodFillDataset->populateDataForDimensions < std::vector<float>, std::vector<float>>(floodNodesWave, { 0 });
    for (int i = 0; i < floodNodesWave.size(); ++i) {
        float node = floodNodesWave[i];
        if (node != -1.0f) {
            indices.push_back(static_cast<int>(node));
        }
    }
}

void VolumeViewerPlugin::applyMaskToColors(const std::vector<int>& indices, bool norm) {
    std::vector<float> colors;
    _pointsColorPoints->extractDataForDimension(colors, 0);


    std::vector<float> maskedColors(colors.size(), 0);
    for (int idx : indices) {
         maskedColors[idx] = colors[idx];
    }

    if (norm) {
        normalizeVector(maskedColors);
    }

    _volumeViewerWidget->getOpenGLWidget()->setColors(maskedColors);
    _volumeViewerWidget->getOpenGLWidget()->update();
}

void VolumeViewerPlugin::reInitializeLayout(QHBoxLayout layout) {

}

mv::CoreInterface* VolumeViewerPlugin::getCore()
{
    return _core;
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

void VolumeViewerPlugin::requestTracker()
{
	// Sync tracker object between all instances. It's the job of the first instance to create the tracker
	
	// Detect other instances of the plugin
	const std::vector<Plugin*> instances = plugins().getPluginsByFactory(getFactory());

	if (instances.size() > 0) {
		VolumeViewerPlugin* firstInstance = static_cast<VolumeViewerPlugin*>(instances[0]);
		firstInstance->getOpenGLRendererWidget()->requestTracker();
	}
	else { // I am the first instance
		getOpenGLRendererWidget()->requestTracker();
	}
}


/******************************************************************************
 * Serialization
 ******************************************************************************/

void VolumeViewerPlugin::fromVariantMap(const QVariantMap& variantMap)
{

    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "SettingsAction");

    _settingsAction->fromVariantMap(variantMap["SettingsAction"].toMap());

}

QVariantMap VolumeViewerPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    _settingsAction->insertIntoVariantMap(variantMap);

    return variantMap;
}


void VolumeViewerPlugin::highlightSelection(const std::vector<bool>& highlights, const std::int32_t& numSelectedPoints) {
    std::vector<int> intHighlights(highlights.size(), 0.f);
    for (int i = 0; i < highlights.size(); i++) {
        intHighlights[i] = highlights[i] ? 1 : 0;
    }

    _volumeViewerWidget->getOpenGLWidget()->getVolumeRenderer().setHighlights(intHighlights);
}



