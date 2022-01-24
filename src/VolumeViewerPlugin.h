#pragma once
/** QT headers*/
#include <QItemSelectionModel>
#include <QWidget>
#include <QLayout>
/** Plugin headers*/
#include <ViewPlugin.h>
#include <ViewerWidget.h>
#include <TransferWidget.h>
/** HDPS headers*/
#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <RendererSettingsAction.h>
#include <PointData.h>
/** VTK headers*/
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>

using hdps::plugin::ViewPluginFactory;
using hdps::plugin::ViewPlugin;
using namespace hdps::util;
using namespace hdps;

class Images;
class ViewerWidget;
class SettingsWidget;
class ViewerWidget;
class Points;

namespace hdps {
    class CoreInterface;
    namespace gui {
        class DropWidget;
    }
}

/**
 * 3D viewer plugin class
 * This HDPS view plugin class provides functionality to view high-dimensional Points Data loaded by the HDVOL loader plugin
 *
 * @author Mitchell M. de Boer
 */
class VolumeViewerPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    /** Constructor */
    VolumeViewerPlugin(const hdps::plugin::PluginFactory* factory);

public: // Inherited from ViewPlugin

    ~VolumeViewerPlugin() override = default;

    /** Initializes the plugin */
    void init() override;
    hdps::CoreInterface* getCore();

    void reInitializeLayout(QHBoxLayout layout);

    /** Returns a pointer to the core interface */
    hdps::CoreInterface* core() { return _core; }


public: // Miscellaneous
    /** Returns the image viewer widget */
    ViewerWidget& getViewerWidget() {
        return *_viewerWidget;
    }

    TransferWidget& getTransfertWidget() {
        return *_transferWidget;
    }

    /** Returns the render settings action*/
    RendererSettingsAction& getRendererSettingsAction() {
        return _rendererSettingsAction;
    }

    /** Returns the names of the points datasets in HDPS */
    QStringList getPointsDatasets() const {
        return _pointsDatasets;
    }

    /** Returns the imageData */
    vtkSmartPointer<vtkImageData> getImageData() {
        return _imageData;
    }

signals:
    /** Signals that list of point datasets in HDPS has changed */
    void pointsDatasetsChanged(QStringList pointsDatasets);

private:
    RendererSettingsAction              _rendererSettingsAction;    /** The options menu on the side of the viewer*/
    ViewerWidget*                       _viewerWidget;              /** The image viewer widget */
    TransferWidget*                     _transferWidget;
    vtkSmartPointer<vtkImageData>       _imageData;                 /** The full data loaded into the viewer */
    vtkSmartPointer<vtkPlaneCollection> _planeCollection;           /** The collection of clipping planes used for the slicing action*/
    vtkSmartPointer<vtkImageData>       _selectionData;             /** The selected data*/
    Dataset<Points>                     _points;                    /** Declare a points dataset reference */
    QStringList                         _pointsDatasets;            /** Point datasets loaded in HDPS */
    hdps::gui::DropWidget*              _dropWidget;                /** Widget for dropping data */
    QString                             _currentDatasetName;        /** Name of the current dataset */
    std::vector<int>                    _planeArray;                /** Array indicating the index+1 of the x,y and z clipping planes in the plane collection*/
    std::vector<double>                  _shadingParameters;         /** Shading parameter save vector index 0 = ambient, index 1 = diffuse and index 2 = specular*/
    std::string                         _interpolationOption;       /** String for storing the current color interpolation option*/
    std::string                         _colorMap;                  /** String for storing the current color map*/
    bool                                _dataLoaded;                /** Booling indicating if data has been loaded in*/
    bool                                _dataSelected;              /** Boolian indicating if data has been selected in a scatterplot*/
    bool                                _shadingEnabled;            /** Boolian for inicating if shading should be enabled*/
};

/**
 * Image viewer plugin factory class
 * A factory for creating image viewer plugin instances
 */
class VolumeViewerPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(hdps::plugin::ViewPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "nl.tudelft.VolumeViewerPlugin" FILE "VolumeViewerPlugin.json")

public:
    /** Default constructor */
    VolumeViewerPluginFactory() {}

    /** Destructor */
    ~VolumeViewerPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    /** Creates an image viewer plugin instance */
    VolumeViewerPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};