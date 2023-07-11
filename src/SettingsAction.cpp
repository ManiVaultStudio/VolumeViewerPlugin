#include "SettingsAction.h"
#include "VolumeViewerPlugin.h"


#include <QMenu>

using namespace hdps::gui;

SettingsAction::SettingsAction(QObject* parent, ViewerWidget* viewerWidget, const QString& title) :
    GroupAction(parent, title),
    _renderSettingsAction(this, viewerWidget,title)

{
    
    GroupsAction::GroupActions groupActions;

    groupActions << &_renderSettingsAction.getColoringAction();
    //addAction(*groupActions);
    _renderSettingsAction.setGroupActions(groupActions);

}

QMenu* SettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu();

    return menu;
}