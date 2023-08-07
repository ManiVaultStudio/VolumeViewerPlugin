#pragma once

#include <actions/OptionAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

class VolumeViewerPlugin;

class PickRendererAction : public OptionAction
{
    Q_OBJECT

    enum class RendererBackend {
        VTK,
        OpenGL
    };

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE PickRendererAction(QObject* parent, const QString& title);

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

    ToggleAction& getVTKAction() { return _vtkAction; }
    ToggleAction& getOpenGLAction() { return _openGLAction; }

private:
    VolumeViewerPlugin* _plugin;                /** Pointer to plugin */
    ToggleAction        _vtkAction;             /** Trigger action for activating the VTK renderer */
    ToggleAction        _openGLAction;          /** Trigger action for activating the OpenGL renderer */

    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(PickRendererAction)

inline const auto pickRendererActionMetaTypeId = qRegisterMetaType<PickRendererAction*>("PickRendererAction");
