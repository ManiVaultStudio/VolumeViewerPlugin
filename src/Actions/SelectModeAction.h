#pragma once


#include <actions/OptionAction.h>
#include <actions/ToggleAction.h>

using namespace mv::gui;

class VolumeViewerPlugin;

class SelectModeAction : public OptionAction
{
    Q_OBJECT

    enum class SelectionMode {
        Nearest, Sphere
    };
    

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE SelectModeAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p scatterplotPlugin
     * @param scatterplotPlugin Pointer to scatterplot plugin
     */
    void initialize(VolumeViewerPlugin* plugin);


    /**
     * Get action context menu
     * @return Pointer to menu
     */
     //QMenu* getContextMenu();

public: // Serialization

    /**
     * Load widget action from variant map
     * @param Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */

    QVariantMap toVariantMap() const override;

public: // Action getters

    ToggleAction& getNearestAction() { return _nearestAction; }
    ToggleAction& getSphericalAction() { return _sphericalAction; }

private:
    VolumeViewerPlugin* _plugin;                /** Pointer to plugin */
    ToggleAction        _nearestAction;         
    ToggleAction        _sphericalAction;         

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(SelectModeAction)

inline const auto selectModeActionMetaTypeId = qRegisterMetaType<SelectModeAction*>("SelectModeAction");
