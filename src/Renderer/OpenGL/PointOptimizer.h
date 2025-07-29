#pragma once

#include <QOpenGLWidget>
#include <vector>
#include <QThread>
#include <QVector3D>
#include <QMatrix4x4>
#include <optional>

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


    void getDistanceApproximations(
        const QVector3D& cursor, 
        std::vector<float>& distances, 
        const int& maxDepth = INT_MAX
    );
    template <typename A> void writeVectorRecursive(const A& value, std::vector<A>& distances);

};






class OptimiserWorker : public QThread
{
public:
    explicit OptimiserWorker(
        QObject* parent,
        std::vector<float> pts,
        std::mutex* mtex
    );
    ~OptimiserWorker();
protected:
    Octree* pointTree = nullptr;
    const int numSlices = 100;
    std::vector<float> points;

    std::mutex* mtx = nullptr;
    
    
    virtual void run() override = 0;
    //void shallowSliceSort();
    void generateOctree();
    /*float distance(const QVector3D& a, const QVector3D& b) {
        float dx = a[0] - b[0];
        float dy = a[1] - b[1];
        float dz = a[2] - b[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }*/
};




class DepthWorker : public OptimiserWorker {
    Q_OBJECT
public:
    DepthWorker(
        QObject* parent,
        std::vector<std::vector<GLuint>>* inds,
        std::vector<float> pts,
        std::vector<QMatrix4x4>* camMatrices,
        std::mutex* mtex
    );
protected:
    void run() override;

    void generateSlices();

    void coalesceOrder(const int& cam);

    QVector3D getCamPos(const int& eye);
    QVector3D getCamDir(const int& eye);

    void sliceSort(const bool& force);

    uint8_t getSliceNumber(const GLuint& pointId, const int& axis);

protected:
    std::vector<int> previousCameraMainSliceDir; // Represents the main axis and direction for slicing (Dir based)
    std::vector<std::vector<bool>> previousAxisSides; // On which side of each axis (position based)

    std::vector<uint8_t> pointSlices; // The slice id for each coordinate for each point. Between 0 and 255

    std::vector<std::vector<GLuint>> sliceSorts; // Contains the pre 3 slice-based sortings, computed once
    std::vector<std::vector<GLuint>> slicedIndexes; // Contains the list of indexes split into each slice, computed when the slices changes. For inner slice sorting

    std::vector<std::vector<GLuint>>* indices;
    std::vector<QMatrix4x4>* cams = nullptr;

signals:
    void resultReady(const int& i);

};


class FlashlightWorker : public OptimiserWorker {
    Q_OBJECT
public:
    FlashlightWorker(
        QObject* parent,
        std::vector<float>* distancesPtr,
        QVector3D* cursorPtr,
        std::vector<float> pts,
        std::mutex* mtex
    );
protected:
    void run() override;

    void updatePointDistancesBruteForce() const;
    void updatePointDistancesOctree() const;


protected:
    QVector3D* cursor;
    std::vector<float>* distances;

signals:
    void resultReady();
};

