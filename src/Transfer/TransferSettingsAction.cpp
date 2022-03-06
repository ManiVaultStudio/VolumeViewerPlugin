#include "Transfer/TransferSettingsAction.h"


using namespace hdps;

TransferSettingsAction::TransferSettingsAction(QObject* parent) :
    GroupsAction(parent),
    _transferColoringAction(*this)
    
{
    GroupsAction::GroupActions groupActions;

    groupActions << &_transferColoringAction;

    set(groupActions);

    emit changed(groupActions);
}