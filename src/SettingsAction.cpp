#include "SettingsAction.h"
#include "VolumeViewerPlugin.h"


#include <QMenu>

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<VolumeViewerPlugin*>(parent)),
    _renderSettingsAction(this, title),
    _pickRendererAction(this, "Pick Renderer Action"),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "ColorPoints"),
    _focusSelectionAction(this, "Focus Selection"),
    _focusFloodfillAction(this, "Focus Floodfill"),
    _focusSelectionNormAction(this, "SNorm"),
    _focusFloodfillNormAction(this, "FNorm"),
    _connectToTrackerAction(this, "Connect Tracker")
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_renderSettingsAction.getColoringAction();
    //addAction(*groupActions);
    _renderSettingsAction.setGroupActions(groupActions);

    _pickRendererAction.initialize(_plugin);

    //connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
    //    _plugin->getDataset() = pickedDataset;
    //});

    connect(&_plugin->getDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });

    //connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
    //    _plugin->getColorDataset() = pickedDataset;
    //});

    connect(&_plugin->getColorDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(dataset);
    });

    connect(&_focusSelectionAction, &ToggleAction::toggled, this, [this](const bool& toggled) {
            _plugin->setFocusSelection(toggled);
    });

    connect(&_focusFloodfillAction, &ToggleAction::toggled, this, [this](const bool& toggled) {
            _plugin->setFocusFloodfill(toggled);
    });

    connect(&_focusSelectionNormAction, &ToggleAction::toggled, this, [this](const bool& toggled) {
            _plugin->setFocusSelectionNorm(toggled);
    });

    connect(&_focusFloodfillNormAction, &ToggleAction::toggled, this, [this](const bool& toggled) {
            _plugin->setFocusFloodfillNorm(toggled);
    });

    connect(&_connectToTrackerAction, &TriggerAction::triggered, this, [this]() { _plugin->connectToTracker(); });
}

QMenu* SettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu();

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _pickRendererAction.fromParentVariantMap(variantMap);

    _positionDatasetPickerAction.fromParentVariantMap(variantMap);
    _colorDatasetPickerAction.fromParentVariantMap(variantMap);

    auto positionDataset = _positionDatasetPickerAction.getCurrentDataset();
    if (positionDataset.isValid())
    {
        Dataset pickedDataset = mv::data().getDataset(positionDataset.getDatasetId());
        _plugin->getDataset() = pickedDataset;
    }

    auto colorDataset = _colorDatasetPickerAction.getCurrentDataset();
    if (colorDataset.isValid())
    {
        Dataset pickedDataset = mv::data().getDataset(colorDataset.getDatasetId());
        _plugin->getColorDataset() = pickedDataset;
    }
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _pickRendererAction.insertIntoVariantMap(variantMap);

    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
