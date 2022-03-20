#include "TransferFunctionControlAction.h"
#include <ViewerWidget.h>

using namespace hdps;

TransferFunctionControlAction::TransferFunctionControlAction(QObject* parent, TransferWidget* transferWidget) :
    GroupsAction(parent),
    _nodeControlAction(*this, transferWidget)
    
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_nodeControlAction ;

    set(groupActions);

    emit changed(groupActions);
}