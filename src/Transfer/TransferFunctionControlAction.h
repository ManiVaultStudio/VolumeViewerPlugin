#pragma once

#include "actions/GroupsAction.h"

#include <Transfer/TransferWidget.h>

#include "NodeControlAction.h"


using namespace hdps::gui;

/**
 * Renderer settings action class
 *
 * Action class for rendering settings
 *
 * @author Thomas Kroes
 */
class TransferFunctionControlAction : public GroupsAction
{
    Q_OBJECT

public:

    /** 
     * Constructor
     * @param parent Pointer to parent object
     */
    TransferFunctionControlAction(QObject* parent, TransferWidget* transferWidget);

public: /** Action getters */
    NodeControlAction& getNodeControlAction() { return _nodeControlAction; }


protected:
    
    NodeControlAction   _nodeControlAction;     /** Slicing action */

    
};