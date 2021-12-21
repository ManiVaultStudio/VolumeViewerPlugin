# VolumeViewerPlugin
View plugin for volumetric image data

This plugin is made using vtk 8.2 and requires vtk 8.2.  Other versions of vtk could work but are not tested.

While running cmake for VTK, make sure to enable VTK_group_QT

If the plugin is to be run in debug, make sure the vtk library is also build in debug, otherwise hdps will crash the moment the plugin is initiated due to a mismatch. (same for the other way around)
