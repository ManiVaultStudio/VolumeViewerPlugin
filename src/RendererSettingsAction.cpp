#include "RendererSettingsAction.h"

using namespace mv;
using namespace mv::gui;

RendererSettingsAction::RendererSettingsAction(QObject* parent, const QString& title) :
    GroupsAction(parent, title),
    _dimensionAction(*this, title),
    _slicingAction(*this, title),
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
