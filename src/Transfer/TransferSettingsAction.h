#pragma once

#include "actions/GroupsAction.h"
#include "Transfer/TransferColoringAction.h"



using namespace hdps::gui;

/**
 * Renderer settings action class
 *
 * Action class for rendering settings
 *
 * @author Thomas Kroes
 */
class TransferSettingsAction : public GroupsAction
{
    Q_OBJECT

public:

    /** 
     * Constructor
     * @param parent Pointer to parent object
     */
    TransferSettingsAction(QObject* parent);

public: /** Action getters */
    TransferColoringAction& getColoringAction() { return _transferColoringAction; }

protected:
    TransferColoringAction _transferColoringAction;
    
};