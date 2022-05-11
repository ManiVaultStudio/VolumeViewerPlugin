#ifndef ViewerWidget_H
#define ViewerWidget_H
/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
/** VTk headers*/
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <QVTKInteractor.h>
#include <vtkInteractorStyle.h>
#include <vtkImageData.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartVolumeMapper.h>   

class VolumeViewerPlugin;
using namespace hdps;

class ViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewerWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent = nullptr);
    ~ViewerWidget();

    /** Set inital data in the viewerWidget  
    *   The chosenDim input is an integer indicating which dimension is to be visualized, starting from 0.
    */
    vtkSmartPointer<vtkImageData> setData(Points& data, int chosenDim, std::string interpolationOption, std::string colorMap);

    /** Renders the data
    *   This function requires a planecollection to indicate where slicing needs to take place
    *   vtkImageData needs to be in a vector for purposes of adding selected data to the renderer in order to display the selected data inside the full dataset. The first index in this vector needs to be the full dataset, the second the selectedData.
    
    *  An interpolation option string is also required due to the 3 different type of colormap interpolations that are available, namely NN, LIN and CUBE 
    */
    void renderData(vtkSmartPointer<vtkPlaneCollection> planeCollection, std::vector<vtkSmartPointer<vtkImageData>> imData, std::string interpolationOption, std::string colorMap, bool shadingEnabled, std::vector<double> shadingParameters);

    /** This function returns a vtkImageData object containing the selected data points.
    *   Next to the points data, an array containing the selected indices is also needed.
    *   The chosenDim input is an integer indicating which dimension is to be visualized, starting from 0.
    */
    vtkSmartPointer<vtkImageData> setSelectedData(Points& points, std::vector<unsigned int, std::allocator<unsigned int>> selectionIndices, int chosenDim);

    void resizeEvent(QResizeEvent* e) override {
        _openGLWidget->setFixedSize(e->size());
    }
    
private:
    QVTKOpenGLNativeWidget* _openGLWidget;                          /** OpenGl Widget for rendering*/
    vtkSmartPointer<vtkRenderer> mRenderer;                         /** vtk Renderer*/
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> mRenderWindow;    /** vtk RenderWindow*/
    vtkSmartPointer<QVTKInteractor> mInteractor;                    /** qvtk Interactor*/
    vtkSmartPointer<vtkInteractorStyle> mInteractorStyle;           /** interactorStyle*/
    int numPoints;                                                  /** Number of points in current dataset*/
    int numDimensions;                                              /** Number of dimensions in current dataset*/

protected:
    VolumeViewerPlugin& _VolumeViewerPlugin;
};

#endif // ViewerWidget_H
