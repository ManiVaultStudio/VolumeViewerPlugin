/** General headers*/
#include <vector>
/** QT headers*/
#include <QDebug>
#include <QMimeData>
#include <QLayout>
/** Plugin headers*/
#include "VolumeViewerPlugin.h"
//#include "Transfer/CustomColorMapEditor.h"
#include <widgets/DropWidget.h>

#include <actions/PluginTriggerAction.h>
#include <DatasetsMimeData.h>

/** mv headers*/
#include "PointData/PointData.h"
#include <ClusterData/Cluster.h>
#include <ClusterData/ClusterData.h>

#include "ColorData/ColorData.h"

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
    //_viewerWidget(nullptr),
    //_volumeRenderer(new OpenGLRendererWidget()),
    //_transferWidget(nullptr),
    //_selectionData(vtkSmartPointer<vtkImageData>::New()),
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
    // Add the viewerwidget and dropwidget to the layout.
    _volumeViewerWidget = new VolumeViewerWidget(this, "Volume Viewer Widget");
    //_viewerWidget = new ViewerWidget(*this);
    // Add the dropwidget to the layout.
    _dropWidget = new DropWidget(_volumeViewerWidget);
    _settingsAction = new SettingsAction(this, "SettingsAction");

    _primaryToolbarAction.addAction(&_settingsAction->getPickRendererAction(), 4, GroupAction::Horizontal);

    _secondaryToolbarAction.addAction(&_settingsAction->getFocusSelectionAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusSelectionNormAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusFloodfillAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getFocusFloodfillNormAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getConnectToTrackerAction());
    _secondaryToolbarAction.addAction(&_settingsAction->getEyeOffsetAction());
}

void VolumeViewerPlugin::init()
{    
    // Create the layout.
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    //layout->addWidget(_viewerWidget, 1);
    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    layout->addWidget(_volumeViewerWidget, 1);
    layout->addWidget(_secondaryToolbarAction.createWidget(&getWidget()));

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
                        //        //_settingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //        //_settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
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
                        //_settingsAction.getColoringAction().addColorDataset(candidateDataset);
                        //_settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
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
            updateFocusMode();
        }
    });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsColorCluster, &Dataset<Clusters>::changed, this, [this]() {
        _clusterLoaded = true;
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

            updateFocusMode();
        }
    });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_pointsOpacityPoints, &Dataset<Points>::changed, this, [this]() {
        if (_clusterLoaded) {
            _clusterLoaded = false;
        }
        _pointOpacityLoaded = true;

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

            if (selectionSet->indices.size() == 1)
            {
                int selectedPoint = selectionSet->indices[0];

                float x = _points->getValueAt(_points->getNumDimensions() * selectedPoint + 0);
                float y = _points->getValueAt(_points->getNumDimensions() * selectedPoint + 1);
                float z = _points->getValueAt(_points->getNumDimensions() * selectedPoint + 2);
                _volumeViewerWidget->setCursorPoint(Vector3f(x, y, z));
            }

            // Focus selection
            if (selectionSet->indices.size() >=1)
            {
                std::vector<int> indices;
                indices.assign(selectionSet->indices.begin(), selectionSet->indices.end());
                if (_focusSelection)
                    applyMaskToColors(indices, false);
                else if (_focusSelectionNorm)
                    applyMaskToColors(indices, true);
            }
        }
    });// Selection changed connection.

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

void VolumeViewerPlugin::updateFocusMode() {
    qDebug() << "Update focus mode";
    if (!_focusSelection && !_focusFloodfill && !_focusSelectionNorm && !_focusFloodfillNorm) {
        std::vector<float> colors;
        _pointsColorPoints->extractDataForDimension(colors, 0);
        _volumeViewerWidget->getOpenGLWidget()->setColors(colors);
        _volumeViewerWidget->getOpenGLWidget()->update();
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

QIcon VolumeViewerPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return mv::Application::getIconFont("FontAwesome").getIcon("cube", color);
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
                auto pluginTriggerAction = new PluginTriggerAction(const_cast<VolumeViewerPluginFactory*>(this), this, "Volume viewer", "Load dataset in volume viewer", getIcon(), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                    for (auto dataset : datasets)
                        getInstance()->loadData(Datasets({ dataset }));
                });

                pluginTriggerActions << pluginTriggerAction;
            }
        }
    }

    return pluginTriggerActions;
}

void VolumeViewerPlugin::setSelectionPosition(double x, double y, double z) {
    _position[0] = x;
    _position[1] = y;
    _position[2] = z;

}

/******************************************************************************
 * Serialization
 ******************************************************************************/

void VolumeViewerPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    //_loadingFromProject = true;

    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "SettingsAction");

    _settingsAction->fromVariantMap(variantMap["SettingsAction"].toMap());

    //_loadingFromProject = false;
}

QVariantMap VolumeViewerPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    _settingsAction->insertIntoVariantMap(variantMap);

    return variantMap;
}
