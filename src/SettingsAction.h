#pragma once

#include "RendererSettingsAction.h"
#include "Actions/PickRendererAction.h"
#include "actions/DatasetPickerAction.h"

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
    ToggleAction& getFocusSelectionAction() { return _focusSelectionAction; }
    ToggleAction& getFocusFloodfillAction() { return _focusFloodfillAction; }
    ToggleAction& getFocusSelectionNormAction() { return _focusSelectionNormAction; }
    ToggleAction& getFocusFloodfillNormAction() { return _focusFloodfillNormAction; }
    TriggerAction& getConnectToTrackerAction() { return _connectToTrackerAction; }
    DecimalAction& getEyeOffsetAction() { return _eyeOffsetAction; }
    DecimalAction& getCamDistAction() { return _camDistAction; }
    ToggleAction& getFlipInterlacingAction() { return _flipInterlacingAction; }

protected:
    VolumeViewerPlugin*     _plugin;                /** Pointer to volume viewer plugin */
    RendererSettingsAction  _renderSettingsAction;
    PickRendererAction      _pickRendererAction;
    DatasetPickerAction     _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;
    ToggleAction            _focusSelectionAction;
    ToggleAction            _focusFloodfillAction;
    ToggleAction            _focusSelectionNormAction;
    ToggleAction            _focusFloodfillNormAction;
    TriggerAction           _connectToTrackerAction;
    DecimalAction           _eyeOffsetAction;
    DecimalAction           _camDistAction;
    ToggleAction            _flipInterlacingAction;
};
