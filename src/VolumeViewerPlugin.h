#pragma once

/** QT headers*/
#include <QItemSelectionModel>
#include <QWidget>
#include <QLayout>
/** Plugin headers*/
#include <ViewPlugin.h>
//#include <Transfer/CustomColorMapEditor.h>
/** mv headers*/
#include <Dataset.h>
#include <widgets/DropWidget.h>
#include <RendererSettingsAction.h>
//#include <Transfer/TransferFunctionControlAction.h>
#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include "SettingsAction.h"
#include <actions/HorizontalToolbarAction.h>
#include <actions/VerticalGroupAction.h>
//#include <ClusterData/Cluster.h>

#include <common.h>

#include "Widgets/VolumeViewerWidget.h"
#include "Renderer/OpenGL/OpenGLRendererWidget.h"

using mv::plugin::ViewPluginFactory;
using mv::plugin::ViewPlugin;
using namespace mv::plugin;
using namespace mv::util;
using namespace mv;

class Images;
class SettingsWidget;
class ViewerWidget;
class Points;

namespace mv {
    class CoreInterface;
    namespace gui {
        class DropWidget;
    }
}

/**
 * 3D viewer plugin class
 * This mv view plugin class provides functionality to view high-dimensional Points Data loaded by the HDVOL loader plugin
 *
 * @author Mitchell M. de Boer
 */
class VolumeViewerPlugin : public ViewPlugin
{
    Q_OBJECT

public:
    enum class RendererBackend {
        OpenGL
    };

public:
    /** Constructor */
    VolumeViewerPlugin(const mv::plugin::PluginFactory* factory);

public: // Inherited from ViewPlugin

    ~VolumeViewerPlugin() override = default;

    /** Initializes the plugin */
    void init() override;
    mv::CoreInterface* getCore();

    void reInitializeLayout(QHBoxLayout layout);

    void setSelectionPosition(double x, double y, double z);

    /** Returns a pointer to the core interface */
    mv::CoreInterface* core() { return _core; }

public: // Miscellaneous
    void setRendererBackend(RendererBackend backend)
    {
        _rendererBackend = backend;
    }
    
    RendererBackend getRendererBackend()
    {
        return _rendererBackend;
    }

    /** Returns the render settings action*/
    RendererSettingsAction& getRendererSettingsAction() {
        return _settingsAction->getRendererSettingsAction();
    }


    /** Returns the names of the points datasets in mv */
    QStringList getPointsDatasets() const {
        return _pointsDatasets;
    }

    //Dataset<Cluster> getColorCluster() {
    //    return _pointsColorCluster;
    //}

    bool& getBackgroundEndabled() {
        return _backgroundEnabled;
    }

    bool& getPointCloudEndabled() {
        return _pointCloudEnabled;
    }

    float& getBackgroundAlpha() {
        return _backgroundAlpha;
    }

    bool& getSelectionOpaque() {
        return _selectionOpaque;
    }

    Dataset<Points>& getDataset() {
        return _points;
    }

    Dataset<Points>& getColorDataset() {
        return _pointsColorPoints;
    }

public: // Focus selection
    void setFocusSelection(bool focusSelection);
    void setFocusFloodfill(bool focusFloodfill);
    void setFocusSelectionNorm(bool focusSelectionNorm);
    void setFocusFloodfillNorm(bool focusFloodfillNorm);

public: // Serialization
    /**
    * Load plugin from variant map
    * @param Variant map representation of the plugin
    */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
    * Save plugin to variant map
    * @return Variant map representation of the plugin
    */
    QVariantMap toVariantMap() const override;

signals:
    /** Signals that list of point datasets in mv has changed */
    void pointsDatasetsChanged(QStringList pointsDatasets);

private: // Focus section or focus floodfill
    void loadFloodfillDataset();
    void getFloodfillIndices(std::vector<int>& indices);
    void applyMaskToColors(const std::vector<int>& indices, bool norm);
    void updateFocusMode();

private:
    RendererBackend                     _rendererBackend;

    SettingsAction*                     _settingsAction;            /** The options menu on the side of the viewer*/
    VolumeViewerWidget*                 _volumeViewerWidget;        /** Widget for the volume viewer */

    Dataset<Points>                     _points;                    /** Declare a points dataset reference */
    Dataset<Points>                     _pointsParent;              /** Declare a points dataset reference */
    Dataset<Clusters>                   _pointsColorCluster;        /** Declare a points dataset reference */
    Dataset<Points>                     _pointsColorPoints;         /** Declare a points dataset reference */
    Dataset<Points>                     _pointsOpacityPoints;       /** Declare a points dataset reference */
    Dataset<Points>                     _floodFillDataset;          /** For focusing on floodfill */

    QStringList                         _pointsDatasets;            /** Point datasets loaded in mv */
    mv::gui::DropWidget*              _dropWidget;                /** Widget for dropping data */
    QString                             _currentDatasetName;        /** Name of the current dataset */
    std::vector<int>                    _planeArray;                /** Array indicating the index+1 of the x,y and z clipping planes in the plane collection*/
    std::vector<double>                 _shadingParameters;         /** Shading parameter save vector index 0 = ambient, index 1 = diffuse and index 2 = specular*/
    std::vector<double>                 _position;
    std::string                         _interpolationOption;       /** String for storing the current color interpolation option*/
    std::string                         _colorMap;                  /** String for storing the current color map*/

    bool                                _dataLoaded;                /** Booling indicating if data has been loaded in*/
    bool                                _dataSelected;              /** Boolian indicating if data has been selected in a scatterplot*/
    bool                                _shadingEnabled;            /** Boolian for inicating if shading should be enabled*/
    bool                                _backgroundEnabled;         /** Boolian for indicating if the non-selected datapoints should be shown*/
    bool                                _pointCloudEnabled;         /** Boolian for indicating the render option*/
    bool                                _pointColorLoaded;          /** Boolian for indicating if the non-selected datapoints should be shown*/
    bool                                _clusterLoaded;             /** Boolian for indicating the render option*/
    bool                                _selectionOpaque;           /** Boolian indicating wether the selected datapoints should be opaque or use the transfer function*/
    float                               _backgroundAlpha;           /** Float indcating the alpha value of the background during selection.*/
    bool                                _selectionDisabled;
    bool                                _pointOpacityLoaded;
    bool                                _focusSelection = false;    /** Boolian indicating whether focus on selection*/
    bool                                _focusFloodfill = false;    /** Boolian indicating whether focus on floodfill*/
    bool                                _focusSelectionNorm = false;/** Boolian indicating whether focus on selection, normalized on selection*/
    bool                                _focusFloodfillNorm = false;/** Boolian indicating whether focus on floodfill, normalized on floodfill*/
    bool                                _floodFillDatasetFound = false;/** Boolian indicating whether floodfill dataset is found*/

    HorizontalToolbarAction             _primaryToolbarAction;      /** Horizontal toolbar for primary content */
    HorizontalToolbarAction             _secondaryToolbarAction;    /** Secondary toolbar for secondary content */
};

/**
 * Image viewer plugin factory class
 * A factory for creating image viewer plugin instances
 */
class VolumeViewerPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "nl.tudelft.VolumeViewerPlugin" FILE "VolumeViewerPlugin.json")

public:
    /** Default constructor */
    VolumeViewerPluginFactory() {}

    /** Destructor */
    ~VolumeViewerPluginFactory() override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    /** Creates an image viewer plugin instance */
    VolumeViewerPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
