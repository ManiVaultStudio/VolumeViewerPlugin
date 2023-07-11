#include "RendererSettingsAction.h"
#include <ViewerWidget.h>

using namespace hdps;
using namespace hdps::gui;

RendererSettingsAction::RendererSettingsAction(QObject* parent, ViewerWidget* viewerWidget, const QString& title) :
    GroupsAction(parent, title),
    _dimensionAction(*this, viewerWidget, title),
    _slicingAction(*this, viewerWidget, title),
    _coloringAction(*this, title),
    _selectedPointsAction(*this, title)
    
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_coloringAction;
    //addAction(*groupActions);
    setGroupActions(groupActions);
    
    /*addAction(&_coloringAction);
    addAction(&_slicingAction);
    addAction(&_dimensionAction);
    addAction(&_selectedPointsAction);*/
}