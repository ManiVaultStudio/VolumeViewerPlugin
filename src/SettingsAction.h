#pragma once

#include "RendererSettingsAction.h"
#include "actions/PickRendererAction.h"
#include "actions/DatasetPickerAction.h"

#include <actions/GroupAction.h>

using namespace hdps::gui;

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

protected:
    VolumeViewerPlugin*     _plugin;                /** Pointer to volume viewer plugin */
    RendererSettingsAction  _renderSettingsAction;
    PickRendererAction      _pickRendererAction;
    DatasetPickerAction     _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;
};
