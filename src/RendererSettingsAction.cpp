#include "RendererSettingsAction.h"
#include <ViewerWidget.h>
#include <QMenu>

using namespace mv;
using namespace mv::gui;

RendererSettingsAction::RendererSettingsAction(QObject* parent, ViewerWidget* viewerWidget, const QString& title) :
    GroupsAction(parent, title),
    _dimensionAction(*this, viewerWidget, title),
    _slicingAction(*this, viewerWidget, title),
    _coloringAction(*this, title),
    _selectedPointsAction(*this, title)

{
    //GroupsAction::GroupActions groupActions;

    //groupActions << &_coloringAction;
    //addAction(*groupActions);
    //setGroupActions(groupActions);
    addGroupAction(&_dimensionAction);
    addGroupAction(&_slicingAction);
    addGroupAction(&_coloringAction);
    
    addGroupAction(&_selectedPointsAction);

    //auto groupAction = new GroupAction(this, "testGroup");
    //addGroupAction(groupAction);
}
