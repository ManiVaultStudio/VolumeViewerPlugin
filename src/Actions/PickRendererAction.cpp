#include "PickRendererAction.h"

#include "VolumeViewerPlugin.h"

using namespace hdps::gui;

PickRendererAction::PickRendererAction(QObject* parent, const QString& title) :
    OptionAction(parent, title, { "VTK", "OpenGL" }),
    _plugin(nullptr),
    _vtkAction(this, "VTK"),
    _openGLAction(this, "OpenGL")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("image"));
    setDefaultWidgetFlags(OptionAction::ComboBox);
    setEnabled(true);

    _vtkAction.setConnectionPermissionsToForceNone(true);
    _openGLAction.setConnectionPermissionsToForceNone(true);

    _vtkAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _openGLAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);

    _vtkAction.setToolTip("Render the volume with VTK");
    _openGLAction.setToolTip("Render the volume with pure OpenGL");
}

void PickRendererAction::initialize(VolumeViewerPlugin* plugin)
{
    Q_ASSERT(plugin != nullptr);

    if (plugin == nullptr)
        return;

    _plugin = plugin;

    plugin->getWidget().addAction(&_vtkAction);
    plugin->getWidget().addAction(&_openGLAction);

    const auto currentIndexChanged = [this]() {
        const auto rendererBackend = static_cast<RendererBackend>(getCurrentIndex());

        _vtkAction.setChecked(rendererBackend == RendererBackend::VTK);
        _openGLAction.setChecked(rendererBackend == RendererBackend::OpenGL);

        _plugin->setRendererBackend(static_cast<VolumeViewerPlugin::RendererBackend>(getCurrentIndex()));
    };

    currentIndexChanged();

    connect(this, &OptionAction::currentIndexChanged, this, currentIndexChanged);

    setCurrentIndex(static_cast<std::int32_t>(RendererBackend::VTK));

    const auto updateReadOnly = [this]() -> void {
        setEnabled(!_plugin->getDataset().isValid());
    };

    connect(&_plugin->getDataset(), &Dataset<Points>::changed, this, updateReadOnly);
}

void PickRendererAction::fromVariantMap(const QVariantMap& variantMap)
{
    OptionAction::fromVariantMap(variantMap);

    _vtkAction.fromParentVariantMap(variantMap);
    _openGLAction.fromParentVariantMap(variantMap);
}

QVariantMap PickRendererAction::toVariantMap() const
{
    auto variantMap = OptionAction::toVariantMap();

    _vtkAction.insertIntoVariantMap(variantMap);
    _openGLAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
