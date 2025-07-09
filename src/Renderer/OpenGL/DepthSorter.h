#pragma once

#include <QOpenGLWidget>
#include <vector>
#include <QThread>
#include <QVector3D>
#include <QMatrix4x4>

#include <cmath>



/**
* Class representing octree. Only leaves from the octree (not containing other cells) contain the points
*/
class Octree {
    std::vector<float>* allPointsCoords;
    std::vector<GLuint> contained;
    QVector3D center = QVector3D(0, 0, 0); // Placed at the barycenter from all points contained in the tree
    std::vector<Octree*> children;
    //bool pointsOnSameCell(const QVector3D& p1, const QVector3D& p2) const;

    QVector3D getCenter() const { return center; }
    int getChildIndex(const QVector3D& position) const;
    //int getContainingChildren(const QVector3D& position) const;
    int getIndexOpposite(const int& childIndex) const;
    std::vector<int> getIndexNeigbours(const int& childIndex) const;
    //std::vector<int> getFarNeigbouringChildren(const QVector3D& position) const;
    /*float distance(const QVector3D& a, const QVector3D& b) {
        float dx = a[0] - b[0];
        float dy = a[1] - b[1];
        float dz = a[2] - b[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }*/
    

public:
    /// <summary>
    /// </summary>
    /// <param name="pts">Coordinates from all points</param>
    /// <param name="indices">Specific points that belong to this cell and it's children</param>
    Octree(std::vector<float>* pts, const std::vector<GLuint>& indices, const int nRecursMax, const int& maxNumPointsPerCell = 1);
    
    ~Octree();

    int debugStructure(const int& depth = 0) const;
    /// <summary>
    /// Sort indices in elementary cubes, from furthest to closest
    /// </summary>
    /// <param name="position"></param>
    /// <param name="indices"></param>
    void getSortedIndicesUsingCubes(const QVector3D& position, std::vector<GLuint>& indices);
    /*float distance(const QVector3D& a, const QVector3D& b) {
        float dx = a[0] - b[0];
        float dy = a[1] - b[1];
        float dz = a[2] - b[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }*/
    /*int numberOfLeavesHaving(const GLuint& number) {
        int result = 0;
        for (GLuint indx : contained) {
            if (number == indx) result++;
        }
        for (int i = 0; i < children.size(); i++) {
            result += children[i].numberOfLeavesHaving(number);
        }
        return result;
    }*/

};






class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(
        QObject* parent,
        std::vector<std::vector<GLuint>>* inds,
        std::vector<float> pts,
        std::vector<QMatrix4x4>* camMatrices
    );
    ~WorkerThread();
protected:
    Octree* pointTree = nullptr;
    std::vector<float> points;
    std::vector<std::vector<GLuint>>* indices;

    std::vector<std::vector<GLuint>> sliceSorts; // Contains the 3 slice-based sortings
    std::vector<QMatrix4x4>* cams = nullptr;
    std::vector<int> previousCameraSide; // takes valeus -3,..., -1, 0 (means undefined), 1, ...,3. The coordinate 1 means axis x, 2 axis y, 3 axis z
    
    std::vector<int> dataAxesConversion = {0,1,2};//{2,1,0};
    
    void run() override;
    void sliceSort();
    QVector3D getCamPos(const int& eye) { return (cams->at(eye) * QVector4D(0, 0, 0, 1.0)).toVector3DAffine(); }
    QVector3D getCamDir(const int& eye) { return (cams->at(eye) * QVector4D(0, 0, 1.0, 1.0)).toVector3DAffine(); }
    /*float distance(const QVector3D& a, const QVector3D& b) {
        float dx = a[0] - b[0];
        float dy = a[1] - b[1];
        float dz = a[2] - b[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }*/
signals:
    void resultReady(const int& i);
};
