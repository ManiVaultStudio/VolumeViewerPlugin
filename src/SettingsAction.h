#pragma once

#include "RendererSettingsAction.h"
#include "Actions/PickRendererAction.h"

#include <actions/GroupAction.h>

class VolumeViewer;

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
    Q_INVOKABLE SettingsAction(QObject* parent, ViewerWidget* viewerWidget, const QString& title);

    
    /**
   * Get the context menu for the action
   * @param parent Parent widget
   * @return Context menu
   */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;
    /** Get reference to the image viewer plugin */
    

public: // Action getters

    RendererSettingsAction& getRendererSettingsAction() { return _renderSettingsAction; }
    PickRendererAction& getPickRendererAction() { return _pickRendererAction; }

protected:
    VolumeViewerPlugin*     _plugin;                /** Pointer to volume viewer plugin */
    RendererSettingsAction  _renderSettingsAction;
    PickRendererAction      _pickRendererAction;
};
