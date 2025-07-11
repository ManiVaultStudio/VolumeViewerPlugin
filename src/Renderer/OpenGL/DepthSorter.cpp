#include "DepthSorter.h"

#include <iostream>



Octree::Octree(
    std::vector<float>* pts, 
    const std::vector<GLuint>& indices, 
    const int nRecursMax, 
    const int& maxNumPointsPerCell) : 
    contained(std::vector<GLuint>(0)),
    children(std::vector<Octree*>(8, nullptr))
{
    allPointsCoords = pts;


    // Calculate center of points in indices
    for (int indx : indices) {
        center += QVector3D(allPointsCoords->at(3 * indx), allPointsCoords->at(3 * indx + 1), allPointsCoords->at(3 * indx + 2));
    }
    center = center / float(indices.size());


    if (nRecursMax > 0 && indices.size() > maxNumPointsPerCell) {
        // Distribute the points into child nodes
        std::vector<std::vector<GLuint>> sortedIndices = std::vector<std::vector<GLuint>>(8, std::vector<GLuint>(0));
        for (int i = 0; i < indices.size(); i++) {
            QVector3D point = QVector3D(allPointsCoords->at(3 * indices[i]), allPointsCoords->at(3 * indices[i] + 1), allPointsCoords->at(3 * indices[i] + 2));

            sortedIndices[getChildIndex(point)].push_back(GLuint(indices[i]));

        }


        for (int j = 0; j < 8; j++) {
            if (sortedIndices[j].size() > 0) {
                children[j] = new Octree(pts, sortedIndices[j], nRecursMax - 1);
            }
        }
    }
    else {
        contained = indices;
    }
}

Octree::~Octree() {
    for (Octree* tree : children) {
        delete tree;
    }
}

int Octree::getChildIndex(const QVector3D& position) const {
    QVector3D posRelative = position - center;
    int result = 0;
    for (int i = 0; i < 3; i++) {
        if(posRelative[i] < 0) result += pow(2, i);
    }
    return result;
}

//bool Octree::pointsOnSameCell(const QVector3D& p1, const QVector3D& p2) const {
//    QVector3D dp1 = p1 - center;
//    QVector3D dp2 = p2 - center;
//
//    return (dp1[0] * dp2[0] >= 0 && dp1[1] * dp2[1] >= 0 && dp1[2] * dp2[2] >= 0);
//}


//int Octree::getContainingChildren(const QVector3D& position) const {
//    for (int i = 0; i < children.size(); i++) {
//        if (pointsOnSameCell(position, children[i].getCenter())) {
//            return i;
//        }
//    }
//    return -1;
//}

int Octree::getIndexOpposite(const int& childIndex) const
{
    return 7 - childIndex;
}

std::vector<int> Octree::getIndexNeigbours(const int& childIndex) const
{
    std::vector<int> result;
    for (int j = 0; j < 3; j++) {
        int flip = childIndex / pow(2, j);
        
        result.push_back(childIndex + pow(-1, flip) * pow(2, j));
    }
    return result;
}

//std::vector<int> Octree::getFarNeigbouringChildren(const QVector3D& position) const
//{
//    std::vector<int> result;
//    for (int i = 0; i < 3; i++) {
//        QVector3D posOppositeTemp = 2 * center - position;
//        posOppositeTemp[i] += 2 * (position - center)[i];
//        int child = getContainingChildren(posOppositeTemp);
//        if (child > -1) {
//            result.push_back(child);
//        }
//    }
//    return result;
//}

/// <summary>
/// 
/// </summary>
/// <param name="depth"></param>
/// <returns>Total number of pointsin the whole tree</returns>
int Octree::debugStructure(const int& depth) const
{
    int numPts = 0;
    for (int i = 0; i < depth; i++) {
        std::cout << '\t';
    }
    std::cout << "Node - ";
    std::cout << "Children : " << children.size();
    std::cout << "\t";
    if(contained.size() > 0) std::cout << "NumPoints : " << contained.size();
    for (GLuint indx : contained) {
        if (0 == indx) std::cout << "Has 0 : ";
    }
    std::cout << std::endl;
    numPts += contained.size();
    for (int i = 0; i < children.size(); i++){
        if (children[i] != nullptr) {
            numPts += children[i]->debugStructure(depth + 1);
        }
    }
    return numPts;
}

/// <summary>
/// Sorts indices of points from further to closest. If one leaf contains more than 
/// one index, the points in the leaf will not be ordered between themselves
/// </summary>
/// <param name="position"></param>
/// <param name="indices">Vector to which the sorted indexes will be appened.</param>
void Octree::getSortedIndicesUsingCubes(const QVector3D& position, std::vector<GLuint>& indices)
{
    if (contained.size() > 0) {
        /*for (GLuint index : contained) {
            indices.push_back(index);
        }*/
        copy(contained.begin(), contained.end(), back_inserter(indices));
    }
    else {
        std::vector<int> orderedChildren;

        // Distance based (semi-bruteforce)
        /*for (int i = 0; i < children.size(); i++) {
            if(children[i]!= nullptr)
                orderedChildren.push_back(i);
        }
        std::sort(orderedChildren.begin(), orderedChildren.end(), [&](const int& a, const int& b) {
            return (
                distance(position, children[a]->getCenter())
                    > distance(position, children[b]->getCenter())
                );
            });*/
        // Distance Basd - END

        // Geometry based
        int containing = getChildIndex(position);
        int opposite = getIndexOpposite(containing);

        orderedChildren.push_back(opposite);

        for (const int& index : getIndexNeigbours(opposite)) {
            orderedChildren.push_back(index);
        }

        for (const int& index : getIndexNeigbours(containing)) {
            orderedChildren.push_back(index);
        }

        orderedChildren.push_back(containing);
        // Geometry based - END


        for (const int& index : orderedChildren) {
            if (children[index] != nullptr) {
                children[index]->getSortedIndicesUsingCubes(position, indices);
            }
        }

    }


}







WorkerThread::WorkerThread(
    QObject* parent = nullptr,
    std::vector<std::vector<GLuint>>* inds = nullptr,
    std::vector<float> pts = std::vector<float>(0),
    std::vector<QMatrix4x4>* camMatrices = nullptr,
    std::mutex* mtex = nullptr
) : QThread(parent), 
    previousCameraMainSliceDir(std::vector<int>(camMatrices->size(), -1)),
    sliceSorts(std::vector<std::vector<GLuint>>(3, std::vector<GLuint>(0))),
    previousAxisSides(std::vector<std::vector<bool>>(camMatrices->size()))
{
    indices = inds;
    points = pts;
    pointSlices = std::vector<uint8_t>(points.size(), 0);
    // Camera position relative to the cloud of points
    cams = camMatrices;
    mtx = mtex;

    std::vector<GLuint> allIndices = std::vector<GLuint>(points.size() / 3);

    for (GLuint i = 0; i < allIndices.size(); i++) {
        allIndices[i] = i;
    }

    // Generate octree for later fast sorting
    qDebug() << "Generating Octree";
    clock_t start = clock();
    pointTree = new Octree(&points, allIndices, 5);
    qDebug() << "Octree Gen finished in " << clock() - start;
    qDebug() << "Clocks per second" << CLOCKS_PER_SEC;

    //int treeSize = pointTree->debugStructure();
    //qDebug() << "Tree size " << treeSize;
    //qDebug() << "Indices size " << indices->size();

    //// Create the slice sorts
    //for (int side = 0; side < 3; side++) {
    //    qDebug() << "Cretating slices for side : " << side;
    //    clock_t start = clock();
    //    int nSlices = 100;
    //    float coordmin = -1.f;
    //    float coordmax = 1.f;

    //    for (int k = 0; k < points.size() / 3; k++) {
    //        if (points[3 * k + side] < coordmin) {
    //            coordmin = points[3 * k + side] - 0.1f;
    //        }
    //        if (points[3 * k + side] > coordmax) {
    //            coordmax = points[3 * k + side] + 0.1f;
    //        }
    //    }
    //    for (int slice = 0; slice < nSlices; slice++) {
    //        for (int k = 0; k < points.size() / 3; k++) {
    //            
    //            if (
    //                points[3 * k + side] >= coordmin + (coordmax - coordmin) * slice / nSlices
    //                && points[3 * k + side] < coordmin + (coordmax - coordmin) * (slice + 1) / nSlices
    //                ) {
    //                sliceSorts[side].push_back(k);
    //            }
    //            
    //        }
    //    }
    //    qDebug() << "Cretated slices for side : " << side << ", in " << clock() - start;
    //}

    // Advanced slice sorting
    start = clock();
    for (int side = 0; side < 3; side++) {
        float coordmin = FLT_MAX;
        float coordmax = FLT_MIN;

        for (int k = 0; k < points.size() / 3; k++) {
            if (points[3 * k + side] < coordmin) {
                coordmin = points[3 * k + side] - 0.01f;
            }
            if (points[3 * k + side] > coordmax) {
                coordmax = points[3 * k + side] + 0.01f;
            }
        }

        for (int i = 0; i < points.size() / 3; i++) {
            pointSlices[3 * i + side] = static_cast<uint8_t>(numSlices*(points[3 * i + side] - coordmin) / (coordmax - coordmin));
        }
    }
    qDebug() << "Prepared slices in " << clock() - start;

}

WorkerThread::~WorkerThread() {
    delete pointTree;
}

uint8_t WorkerThread::getSliceNumber(const GLuint& pointId, const int& axis) {
    return pointSlices[3 * pointId + axis];
}
QVector3D WorkerThread::getCamPos(const int& eye) {
    std::lock_guard<std::mutex> lock(*mtx);
    return (cams->at(eye) * QVector4D(0, 0, 0, 1.0)).toVector3DAffine();
}
QVector3D WorkerThread::getCamDir(const int& eye) {
    std::lock_guard<std::mutex> lock(*mtx);
    return (cams->at(eye) * QVector4D(0, 0, 1.0, 1.0)).toVector3DAffine();
}


void WorkerThread::run() {
    QString result;
    std::vector<QVector3D> previousCameras = std::vector<QVector3D>(2, QVector3D(0,0,0));
    std::vector<std::vector<GLuint>> localIndices = std::vector<std::vector<GLuint>>(2);
    std::vector<clock_t> start = std::vector<clock_t>(2,0);
    std::vector<bool> forceFullRender = std::vector<bool>(2, false);
    while (true) {
        for (int cam = 0; cam < 2; cam++) {
            QVector3D camPos = getCamPos(cam);
            if (
                (camPos - previousCameras[cam]).length() > 0.01f
                && clock() - start[cam] > 0.05f * CLOCKS_PER_SEC
            ) {

                start[cam] = clock();
                // Sort indices based on distance to camera
            
                // Bruteforce method
                /*localIndices.assign(indices->begin(), indices->end());
                std::sort(localIndices.begin(), localIndices.end(), [&](const int& a, const int& b) {
                    return (
                        distance(*camPos, QVector3D(points[a * 3], points[a * 3 + 1], points[a * 3 + 2]))
                        > distance(*camPos, QVector3D(points[b * 3], points[b * 3 + 1], points[b * 3 + 2]))
                    );
                });*/

                //std::unique_lock<std::mutex> lock(mtx);
                //indices->at(i).assign(localIndices[i].begin(), localIndices[i].end());
                //lock.unlock();
                //emit resultReady(i);
                // Bruteforce method - END


                if ((camPos - previousCameras[cam]).length() > 0.7f) {
                    // Octree based method
                    localIndices[cam].clear();
                    pointTree->getSortedIndicesUsingCubes(getCamPos(cam), localIndices[cam]);


                    std::unique_lock<std::mutex> lock(*mtx);
                    indices->at(cam).assign(localIndices[cam].begin(), localIndices[cam].end());
                    lock.unlock();
                    emit resultReady(cam);
                    forceFullRender[cam] = true;
                    // Octree based method - END
                }
                else {
                    qDebug() << "slice sorting";
                    // Do slicesort if the movement is not too fast
                    sliceSort(forceFullRender[cam]);
                    forceFullRender[cam] = false;
                    qDebug() << "End Slice sorting, took : " << clock() - start[cam];
                }
                



                previousCameras[cam] = camPos;

            }
       
            //qDebug() << "Depth sorting in " << clock() - start;

        }
    }
}


//
//void WorkerThread::shallowSliceSort() {
//    std::vector<int> cameraDirMainAxis = std::vector<int>(2, 0); // Reflects new version of previousCameraMainSliceDir
//    for (int cam = 0; cam < cameraDirMainAxis.size(); cam++) {
//
//        //Determine the main axis from the camera direction + on which side of each axis we are
//        float biggestComponentValue = 0;
//        for (int i = 0; i < 3; i++) {
//            QVector3D unitVector = QVector3D(0,0,0);
//            unitVector[i] = 1.0f;
//            float componentDir = QVector3D::dotProduct(getCamDir(cam), unitVector);
//            if (abs(componentDir) >= biggestComponentValue) {
//                cameraDirMainAxis[cam] = i+1;
//                if (componentDir < 0) cameraDirMainAxis[cam] *= -1;
//                biggestComponentValue = abs(componentDir);
//            }
//        }
//
//        if (previousCameraMainSliceDir[cam] > -1 && cameraDirMainAxis[cam] != previousCameraMainSliceDir[cam]) {
//            qDebug() << "Cam dir" << getCamDir(cam);
//            qDebug() << "Detected change in face on camera : " << cam << ", setting new face : " << cameraDirMainAxis[cam];

//            std::unique_lock<std::mutex> lock(mtx);
//            qDebug() << abs(cameraDirMainAxis[cam]) - 1;
//            indices->at(cam).clear();
//            indices->at(cam).assign(sliceSorts[abs(cameraDirMainAxis[cam]) - 1].begin(), sliceSorts[abs(cameraDirMainAxis[cam]) - 1].end());
//            if (cameraDirMainAxis[cam] < 0) {
//                std::reverse(indices->at(cam).begin(), indices->at(cam).end());
//            } 
//            lock.unlock();
//
//            emit resultReady(cam);
//            previousCameraMainSliceDir[cam] = cameraDirMainAxis[cam];
//        }
//    }
//
//}


void WorkerThread::sliceSort(const bool& force) {
    std::vector<std::vector<int>> cameraSortedAxis = std::vector<std::vector<int>>(2, std::vector<int>(3, -1));
    std::vector<int> cameraDirMainAxis = std::vector<int>(2, 0); // Reflects new version of previousCameraMainSliceDir
    std::vector<std::vector<bool>> axisComponentPositive = std::vector<std::vector<bool>>(2, std::vector<bool>(3)); // Reflects new version of previousAxisSides
    for (int cam = 0; cam < 2; cam++) {
        bool recalculate = false;
        //Determine the main axis from the camera direction + on which side of each axis we are
        float biggestComponentValue = 0;
        float smallestComponentValue = INT_MAX;
        QVector3D unitVector;
        QVector3D camDir = getCamDir(cam);
        for (int i = 0; i < 3; i++) {
            unitVector = QVector3D(0, 0, 0);
            unitVector[i] = 1.0f;
            float componentDir = QVector3D::dotProduct(camDir, unitVector);
            axisComponentPositive[cam][i] = componentDir > 0;
            if (abs(componentDir) >= biggestComponentValue) {
                cameraSortedAxis[cam][0] = i;
                biggestComponentValue = abs(componentDir);
            }
            if (abs(componentDir) <= smallestComponentValue) {
                cameraSortedAxis[cam][2] = i;
                smallestComponentValue = abs(componentDir);
            }

        }
        cameraSortedAxis[cam][1] = 3 - cameraSortedAxis[cam][0] - cameraSortedAxis[cam][2];

        // Perform shallow slice sorting (slice along the main axe with no in slice sorting)
        if (force || cameraSortedAxis[cam][0] != previousCameraMainSliceDir[cam]) {
            recalculate = true;

            slicedIndexes.clear();
            slicedIndexes = std::vector<std::vector<GLuint>>(numSlices);
            uint8_t sliceId;
            for (GLuint i = 0; i < points.size() / 3; i++) {
                sliceId = getSliceNumber(i, cameraSortedAxis[cam][0]); // pointSlices[3 * i + cameraSortedAxis[cam][0]];
                
                slicedIndexes[sliceId].push_back(i);
            }
            if (!axisComponentPositive[cam][cameraSortedAxis[cam][0]]) {
                std::reverse(slicedIndexes.begin(), slicedIndexes.end());
            }


            previousCameraMainSliceDir[cam] = cameraSortedAxis[cam][0];
        }
        
        bool changeOneAxisSide = (
            previousAxisSides[cam].size() == 0 || (
            axisComponentPositive[cam][0] != previousAxisSides[cam][0]
            || axisComponentPositive[cam][1] != previousAxisSides[cam][1]
            || axisComponentPositive[cam][2] != previousAxisSides[cam][2]
                )
        );

        // Perform deep slice sorting (sort inside slices), based on the two other main axis
        if (force || changeOneAxisSide || recalculate) {
            recalculate = true;
            for (uint8_t slice = 0; slice < slicedIndexes.size(); slice++) {
                std::vector<std::vector<GLuint>> subSlicing = std::vector<std::vector<GLuint>>(numSlices);
                uint8_t subSliceId;
                for (const GLuint& index : slicedIndexes[slice]) {
                    subSliceId = getSliceNumber(index, cameraSortedAxis[cam][1]); // pointSlices[3 * i + cameraSortedAxis[cam][0]];

                    subSlicing[subSliceId].push_back(index);
                }
                if (!axisComponentPositive[cam][cameraSortedAxis[cam][1]]) {
                    std::reverse(subSlicing.begin(), subSlicing.end());
                }
                slicedIndexes[slice].clear();
                for (const std::vector<GLuint>& subSlice : subSlicing) {
                    slicedIndexes[slice].insert(slicedIndexes[slice].end(), subSlice.begin(), subSlice.end());
                }
                /*std::sort(slicedIndexes[i].begin(), slicedIndexes[i].end(), [&](const GLuint& a, const GLuint& b) {
                    bool secondaryAxisCond = axisComponentPositive[cam][cameraSortedAxis[cam][1]]?
                        getSliceNumber(a, cameraSortedAxis[cam][1]) < getSliceNumber(b, cameraSortedAxis[cam][1])
                        : getSliceNumber(b, cameraSortedAxis[cam][1]) < getSliceNumber(a, cameraSortedAxis[cam][1]);
                    if (secondaryAxisCond) return true;

                    return (getSliceNumber(a, cameraSortedAxis[cam][1]) == getSliceNumber(b, cameraSortedAxis[cam][1])
                        && (
                            axisComponentPositive[cam][cameraSortedAxis[cam][2]] ?
                            getSliceNumber(a, cameraSortedAxis[cam][2]) < getSliceNumber(b, cameraSortedAxis[cam][2])
                            : getSliceNumber(b, cameraSortedAxis[cam][2]) < getSliceNumber(a, cameraSortedAxis[cam][2])
                        ));
                });*/
            }
            //coalesceOrder(cam);
            previousAxisSides[cam] = axisComponentPositive[cam];
        }
        
        if (recalculate) {
            coalesceOrder(cam);

        }

        //if (cameraSortedAxis[cam][0] != previousCameraMainSliceDir[cam]/* || changeOneAxisSide*/) {

        //    std::unique_lock<std::mutex> lock(mtx);
        //    std::sort(indices->at(cam).begin(), indices->at(cam).end(), [this, &cam, &cameraDirMainAxis](GLuint a, GLuint b) {
        //        return (
        //            getSliceNumber(a, abs(cameraDirMainAxis[cam])-1) < getSliceNumber(b, abs(cameraDirMainAxis[cam])-1)
        //            );
        //        });
        //    /*indices->at(cam).clear();
        //    indices->at(cam).assign(sliceSorts[abs(cameraDirMainAxis[cam]) - 1].begin(), sliceSorts[abs(cameraDirMainAxis[cam]) - 1].end());*/
        //    if (axisSides[cam][cameraDirMainAxis[cam]] < 0) {
        //        std::reverse(indices->at(cam).begin(), indices->at(cam).end());
        //    }
        //    lock.unlock();
        //    emit resultReady(cam);
        //    previousCameraMainSliceDir[cam] = cameraDirMainAxis[cam];
        //}
    }

}

void WorkerThread::coalesceOrder(const int& cam)
{

    std::unique_lock<std::mutex> lock(*mtx);

    indices->at(cam).clear();
    for (const std::vector<GLuint>& slice : slicedIndexes) {
        indices->at(cam).insert(indices->at(cam).end(), slice.begin(), slice.end());
    }

    lock.unlock();
    emit resultReady(cam);
}
