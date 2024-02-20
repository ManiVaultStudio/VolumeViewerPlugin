#include "PickRendererAction.h"

#include "VolumeViewerPlugin.h"

using namespace mv::gui;

PickRendererAction::PickRendererAction(QObject* parent, const QString& title) :
    OptionAction(parent, title, { "OpenGL" }),
    _plugin(nullptr),
    _openGLAction(this, "OpenGL")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("image"));
    setDefaultWidgetFlags(OptionAction::ComboBox);
    setEnabled(true);

    _openGLAction.setConnectionPermissionsToForceNone(true);

    _openGLAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);

    _openGLAction.setToolTip("Render the volume with pure OpenGL");
}

void PickRendererAction::initialize(VolumeViewerPlugin* plugin)
{
    Q_ASSERT(plugin != nullptr);

    if (plugin == nullptr)
        return;

    _plugin = plugin;

    plugin->getWidget().addAction(&_openGLAction);

    const auto currentIndexChanged = [this]() {
        const auto rendererBackend = static_cast<RendererBackend>(getCurrentIndex());

        _openGLAction.setChecked(rendererBackend == RendererBackend::OpenGL);

        _plugin->setRendererBackend(static_cast<VolumeViewerPlugin::RendererBackend>(getCurrentIndex()));
    };

    currentIndexChanged();

    connect(this, &OptionAction::currentIndexChanged, this, currentIndexChanged);

    setCurrentIndex(static_cast<std::int32_t>(RendererBackend::OpenGL));

    const auto updateReadOnly = [this]() -> void {
        setEnabled(!_plugin->getDataset().isValid());
    };

    connect(&_plugin->getDataset(), &Dataset<Points>::changed, this, updateReadOnly);
}

void PickRendererAction::fromVariantMap(const QVariantMap& variantMap)
{
    OptionAction::fromVariantMap(variantMap);

    _openGLAction.fromParentVariantMap(variantMap);
}

QVariantMap PickRendererAction::toVariantMap() const
{
    auto variantMap = OptionAction::toVariantMap();

    _openGLAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
