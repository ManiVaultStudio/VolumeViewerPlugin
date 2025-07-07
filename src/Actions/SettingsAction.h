#pragma once

#include "Actions/RendererSettingsAction.h"
#include "Actions/PickRendererAction.h"
#include "Actions/SelectModeAction.h"
#include "Actions/DatasetPickerAction.h"

#include <actions/GroupAction.h>

using namespace mv::gui;

/**
 * Settings action class
 *
 * Action class for image viewer plugin settings (panel on the right of the view)
 *
 * @author Thomas Kroes
 */
class SettingsAction : public GroupAction
{
public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

    
    /**
   * Get the context menu for the action
   * @param parent Parent widget
   * @return Context menu
   */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;
    /** Get reference to the image viewer plugin */
    
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

public: // Action getters

    RendererSettingsAction& getRendererSettingsAction() { return _renderSettingsAction; }
    PickRendererAction& getPickRendererAction() { return _pickRendererAction; }
    SelectModeAction& getSelectModeAction() { return _selectModeAction; }
    ToggleAction& getFocusSelectionAction() { return _focusSelectionAction; }
    ToggleAction& getFocusFloodfillAction() { return _focusFloodfillAction; }
    ToggleAction& getFocusSelectionNormAction() { return _focusSelectionNormAction; }
    ToggleAction& getFocusFloodfillNormAction() { return _focusFloodfillNormAction; }
    TriggerAction& getConnectToTrackerAction() { return _connectToTrackerAction; }
    TriggerAction& getStartCalibAction() { return _startCalibAction; }
    TriggerAction& getToggleFullScreen() { return _toggleFullScreen; }
    TriggerAction& getClearSelectionAction() { return _clearSelectionAction; }
    DecimalAction& getColorAdjustAction() { return _colorAdjust; }
    DecimalAction& getEyeOffsetAction() { return _eyeOffsetAction; }
    DecimalAction& getCamDistAction() { return _camDistAction; }
    ColorAction& getSelectionColorPicker() { return _selectionColorPicker; }

protected:
    VolumeViewerPlugin*     _plugin;                /** Pointer to volume viewer plugin */
    RendererSettingsAction  _renderSettingsAction;
    PickRendererAction      _pickRendererAction;
    SelectModeAction        _selectModeAction;
    DatasetPickerAction     _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;
    ToggleAction            _focusSelectionAction;
    ToggleAction            _focusFloodfillAction;
    ToggleAction            _focusSelectionNormAction;
    ToggleAction            _focusFloodfillNormAction;
    TriggerAction           _connectToTrackerAction;
    TriggerAction           _startCalibAction;
    TriggerAction           _clearSelectionAction;
    TriggerAction           _toggleFullScreen;
    DecimalAction           _colorAdjust;
    DecimalAction           _eyeOffsetAction;
    DecimalAction           _camDistAction;
    ColorAction             _selectionColorPicker;
};
