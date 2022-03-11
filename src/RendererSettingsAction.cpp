#include "RendererSettingsAction.h"
#include <ViewerWidget.h>

using namespace hdps;

RendererSettingsAction::RendererSettingsAction(QObject* parent, ViewerWidget* viewerWidget) :
    GroupsAction(parent),
    _dimensionAction(*this, viewerWidget),
    _slicingAction(*this, viewerWidget),
    _coloringAction(*this)
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_dimensionAction << &_slicingAction << &_coloringAction;
    
    setGroupActions(groupActions);
}