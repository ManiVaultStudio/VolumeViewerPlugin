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

        for (int index : getIndexNeigbours(opposite)) {
            orderedChildren.push_back(index);
        }

        for (int index : getIndexNeigbours(containing)) {
            orderedChildren.push_back(index);
        }

        orderedChildren.push_back(containing);
        // Geometry based - END


        for (int index : orderedChildren) {
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
    std::vector<QMatrix4x4>* camMatrices = nullptr
) : QThread(parent), 
    previousCameraSide(std::vector<int>(camMatrices->size(), 0)),
    sliceSorts(std::vector<std::vector<GLuint>>(3, std::vector<GLuint>(0)))
{
    indices = inds;
    points = pts;
    // Camera position relative to the cloud of points
    cams = camMatrices;

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

    // Create the slice sorts
    for (int side = 0; side < 3; side++) {
        qDebug() << "Cretating slices for side : " << side;
        clock_t start = clock();
        int nSlices = 100;
        float coordmin = -1.f;
        float coordmax = 1.f;

        for (int k = 0; k < points.size() / 3; k++) {
            if (points[3 * k + side] < coordmin) {
                coordmin = points[3 * k + dataAxesConversion[side]] - 0.1f;
            }
            if (points[3 * k + side] > coordmax) {
                coordmax = points[3 * k + dataAxesConversion[side]] + 0.1f;
            }
        }
        for (int slice = 0; slice < nSlices; slice++) {
            for (int k = 0; k < points.size() / 3; k++) {
                
                if (
                    points[3 * k + dataAxesConversion[side]] >= coordmin + (coordmax - coordmin) * slice / nSlices
                    && points[3 * k + dataAxesConversion[side]] < coordmin + (coordmax - coordmin) * (slice + 1) / nSlices
                    ) {
                    sliceSorts[side].push_back(k);
                }
                
            }
        }
        qDebug() << "Cretated slices for side : " << side << ", in " << clock() - start;
    }
}

WorkerThread::~WorkerThread() {
    delete pointTree;
}



void WorkerThread::run() {
    QString result;
    std::vector<QVector3D> previousCameras = std::vector<QVector3D>(2, QVector3D(0,0,0));
    std::vector<std::vector<GLuint>> localIndices = std::vector<std::vector<GLuint>>(2);
    std::vector<clock_t> start = std::vector<clock_t>(2,0);
    while (true) {
        for (int i = 0; i < 2; i++) {
            if (
                (getCamPos(i) - previousCameras[i]).length() > 0.01f
                && clock() - start[i] > 0.05f * CLOCKS_PER_SEC
            ) {
                start[i] = clock();
            

                // Sort indices based on distance to camera
            
                // Bruteforce method
                /*localIndices.assign(indices->begin(), indices->end());
                std::sort(localIndices.begin(), localIndices.end(), [&](const int& a, const int& b) {
                    return (
                        distance(*camPos, QVector3D(points[a * 3], points[a * 3 + 1], points[a * 3 + 2]))
                        > distance(*camPos, QVector3D(points[b * 3], points[b * 3 + 1], points[b * 3 + 2]))
                    );
                });*/
                // Bruteforce method - END

                //// Octree based method
                //localIndices[i].clear();
                //pointTree->getSortedIndicesUsingCubes(camPos->at(i), localIndices[i]);
                //// Octree based method - END

                //std::mutex mtx;
                //mtx.lock();
                //indices->at(i).assign(localIndices[i].begin(), localIndices[i].end());
                //mtx.unlock();
                //emit resultReady(i);

                sliceSort();


                previousCameras[i] = getCamPos(i);

            }
       
            //qDebug() << "Depth sorting in " << clock() - start;

        }
    }
}



void WorkerThread::sliceSort() {
    std::vector<int> cameraSide = std::vector<int>(2, 0);
    //Determine f the front facing side has changed for each amera
    for (int cam = 0; cam < cameraSide.size(); cam++) {
        float biggestComponentValue = 0;
        for (int i = 0; i < 3; i++) {
            QVector3D unitVector = QVector3D(0,0,0);
            unitVector[i] = 1.0f;
            float component = QVector3D::dotProduct(getCamDir(cam), unitVector);
            if (abs(component) >= biggestComponentValue) {
                cameraSide[cam] = i+1;
                if (component < 0) cameraSide[cam] *= -1;
                biggestComponentValue = abs(component);
            }
        }
        if (cameraSide[cam] != previousCameraSide[cam]) {
            qDebug() << "Cam dir" << getCamDir(cam);
            qDebug() << "Detected change in face on camera : " << cam << ", setting new face : " << cameraSide[cam];
            std::mutex mtx;
            mtx.lock();
            qDebug() << abs(cameraSide[cam]) - 1;
            indices->at(cam).clear();
            indices->at(cam).assign(sliceSorts[abs(cameraSide[cam]) - 1].begin(), sliceSorts[abs(cameraSide[cam]) - 1].end());
            if (cameraSide[cam] < 0) {
                std::reverse(indices->at(cam).begin(), indices->at(cam).end());
            } 
            mtx.unlock();

            emit resultReady(cam);
            previousCameraSide[cam] = cameraSide[cam];
        }
    }

}