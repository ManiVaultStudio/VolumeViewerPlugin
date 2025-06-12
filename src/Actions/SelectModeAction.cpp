#include "Actions/SelectModeAction.h"

#include "VolumeViewerPlugin.h"

using namespace mv::gui;

SelectModeAction::SelectModeAction(QObject* parent, const QString& title) :
    OptionAction(parent, title, { "Closest", "Sphere" }),
    _plugin(nullptr),
    _nearestAction(this, "Closest"),
    _sphericalAction(this, "Sphere")
{
    //setIcon(mv::Application::getIconFont("FontAwesome").getIcon("image"));
    setDefaultWidgetFlags(OptionAction::ComboBox);
    setEnabled(true);

    _nearestAction.setConnectionPermissionsToForceNone(true);
    _nearestAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _nearestAction.setToolTip("Select closest point to cursor.");

    _sphericalAction.setConnectionPermissionsToForceNone(true);
    _sphericalAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _sphericalAction.setToolTip("Select with a sphere around the cursor.");
}

void SelectModeAction::initialize(VolumeViewerPlugin* plugin)
{
    Q_ASSERT(plugin != nullptr);

    if (plugin == nullptr)
        return;

    _plugin = plugin;

    plugin->getWidget().addAction(&_nearestAction);
    plugin->getWidget().addAction(&_sphericalAction);

    const auto currentIndexChanged = [this]() {
        const auto selectMode = static_cast<SelectionMode>(getCurrentIndex());

        _nearestAction.setChecked(selectMode == SelectionMode::Nearest);
        _sphericalAction.setChecked(selectMode == SelectionMode::Sphere);

        _plugin->getOpenGLRendererWidget().setSelectionMode(getCurrentIndex());
        _plugin->getOpenGLRendererWidget().setFocus();
        };

    currentIndexChanged();

    connect(this, &OptionAction::currentIndexChanged, this, currentIndexChanged);

    setCurrentIndex(static_cast<std::int32_t>(SelectionMode::Nearest));

    connect(&_plugin->getDataset(), &Dataset<Points>::changed, this, [this]() {
        /*setEnabled(!_plugin->getDataset().isValid());
        qDebug() << "Valid: " << _plugin->getDataset().isValid();*/
        setEnabled(true);
    });
}

void SelectModeAction::fromVariantMap(const QVariantMap& variantMap)
{
    OptionAction::fromVariantMap(variantMap);

    _nearestAction.fromParentVariantMap(variantMap);
    _sphericalAction.fromParentVariantMap(variantMap);
}

QVariantMap SelectModeAction::toVariantMap() const
{
    auto variantMap = OptionAction::toVariantMap();

    _nearestAction.insertIntoVariantMap(variantMap);
    _sphericalAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
