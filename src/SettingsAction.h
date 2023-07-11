#pragma once


#include <actions/GroupAction.h>
#include "RendererSettingsAction.h"


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

protected:
    
    RendererSettingsAction    _renderSettingsAction;
    
};