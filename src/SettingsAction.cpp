#include "SettingsAction.h"
#include "VolumeViewerPlugin.h"


#include <QMenu>

using namespace hdps::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _plugin(dynamic_cast<VolumeViewerPlugin*>(parent)),
    _renderSettingsAction(this, title),
    _pickRendererAction(this, "Pick Renderer Action")
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_renderSettingsAction.getColoringAction();
    //addAction(*groupActions);
    _renderSettingsAction.setGroupActions(groupActions);

    _pickRendererAction.initialize(_plugin);
}

QMenu* SettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu();

    return menu;
}
