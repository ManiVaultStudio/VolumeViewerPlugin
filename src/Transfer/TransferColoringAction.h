#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorAction.h"



using namespace hdps::gui;

class TransferSettingsAction;

/**
 * Coloring action class
 *
 * Action class for coloring parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class TransferColoringAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    TransferColoringAction(TransferSettingsAction& transferSettingsAction);

public: /** Action getters */


    ColorAction& getColorAction() { return  _colorAction; }



protected:
    TransferSettingsAction& _transferSettingsAction;        /** Reference to renderer settings action */

    ColorAction              _colorAction;                 /** Color map Action */
};