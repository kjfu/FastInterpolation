#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <set>
#include <iostream>
#include "aabbox.h"
#include "vector3d.h"

double SixTimesTetrahedronVolume(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3);

class Tetrahedron;
struct Node;
struct TriangleFacet;

struct Node
{

    Vector3D pos;
    int index;
    int label;
    bool fixed = false;
    double sizing;
    std::vector<double> scalarValues;
    std::vector<Vector3D> vectorValues;
    int edit = 0;

    void *tempDate = nullptr;
    
    Node(){

    }

    Node(double x, double y, double z):pos(x,y,z){
    }

    Node(double *xyz): pos(xyz){
    }
    Node(Vector3D &vec):pos(vec){
    }

};

struct TriangleFacet
{
    Tetrahedron *tet = nullptr;
    int localIndex;
    std::array<int, 3> orderedNodeIndices;
    std::vector<Node *> sNodes;
    TriangleFacet() = default;
    TriangleFacet(int i0, int i1, int i2){
        orderedNodeIndices[0] = i0;
        orderedNodeIndices[1] = i1;
        orderedNodeIndices[2] = i2;
        std::sort(orderedNodeIndices.begin(), orderedNodeIndices.end());
    }
};


class Tetrahedron 
{   

public:
    std::array<Node*, 4> nodes;
    std::array<Tetrahedron*, 4> adjacentTetrahedrons;
    AABBox boundingBox;

    int index;
    int label;
    bool fixed = false;
    int edit = 0;
    Tetrahedron(){
        adjacentTetrahedrons.fill(nullptr);
    }
    Tetrahedron(Node *n0, Node *n1, Node *n2, Node *n3){
        nodes[0]=n0; nodes[1]=n1; nodes[2]=n2; nodes[3]=n3;
        adjacentTetrahedrons.fill(nullptr);
    }

    Vector3D center(){
        Vector3D pos;
        for(auto n: nodes){
            pos += n->pos;
        }
        pos /= 4.0;
        return pos;
    }



    TriangleFacet facet(int index, bool withNodes = false){
        static int facetIndices[4][3]=
        { {1, 3, 2}
        , {0, 2, 3}
        , {0, 3, 1}
        , {0, 1, 2}};
        TriangleFacet rst(nodes[facetIndices[index][0]]->index
                        , nodes[facetIndices[index][1]]->index
                        , nodes[facetIndices[index][2]]->index);
        if (withNodes){
            rst.sNodes.push_back(nodes[facetIndices[index][0]]);
            rst.sNodes.push_back(nodes[facetIndices[index][1]]);
            rst.sNodes.push_back(nodes[facetIndices[index][2]]);
        }
        rst.localIndex = index;
        rst.tet = this;
        return rst;
    }

    Node *nodeOnFacet(int facetIndex, int nodeIndex){
        static int facetIndices[4][3]=
        { {1, 3, 2}
        , {0, 2, 3}
        , {0, 3, 1}
        , {0, 1, 2}};
        return nodes[facetIndices[facetIndex][nodeIndex]];
    }

    bool removeAdjacentTetrahedron(Tetrahedron *tet){
        bool rst = false;
        for(int i=0; i<4; i++){
            if(adjacentTetrahedrons[i] == tet){
                adjacentTetrahedrons[i] = nullptr;
                rst = true;
                break;
            }
        }
        return rst;
    }
    bool dualFacet(Node* n, TriangleFacet &goalFacet){
        bool rst = true;
        int goalIndex = -1;
        for(int i=0; i<4; i++){
            if(nodes[i]==n){
                goalIndex = i;
                break;
            }
        }
        if (goalIndex>=0){
            goalFacet = facet(goalIndex);
        }
        else{
            rst = false;
        }
        return rst;
    }

    bool replaceAdjacentTetrahedron(Tetrahedron *origin, Tetrahedron *another){
        bool rst = false;
        for(int i=0; i<4; i++){
            if(adjacentTetrahedrons[i] == origin){
                adjacentTetrahedrons[i] = another;
                rst = true;
                break;
            }
        }
        return rst;        
    }


    bool contain(Vector3D pos){
        static int nodeIndices[4][3]=
        { {1, 3, 2}
        , {0, 2, 3}
        , {0, 3, 1}
        , {0, 1, 2}};
        bool rst = true;
        for(int i=0; i<4; i++){
            if (SixTimesTetrahedronVolume(nodes[nodeIndices[i][0]]->pos, nodes[nodeIndices[i][1]]->pos, nodes[nodeIndices[i][2]]->pos, pos)<0){
                rst = false;
                break;
            }
        }
        return rst;        
    }

    bool contain(Vector3D pos, std::array<double, 4> &weights, double eps=std::numeric_limits<double>::epsilon()) {
        static int nodeIndices[4][3]=
        { {1, 3, 2}
        , {0, 2, 3}
        , {0, 3, 1}
        , {0, 1, 2}};
        bool rst = true;
        for(int i=0; i<4; i++){
            double weight = SixTimesTetrahedronVolume(nodes[nodeIndices[i][0]]->pos, nodes[nodeIndices[i][1]]->pos, nodes[nodeIndices[i][2]]->pos, pos);
            if (weight<-eps){
                rst = false;
                break;
            }
            else if (weight>=-eps && weight<0) {
                weights[i] = 0;
            }
            else{
                weights[i] = weight;
            }
        }
        return rst;        
    }

    double volume(){
        return SixTimesTetrahedronVolume(nodes[0]->pos, nodes[1]->pos, nodes[2]->pos, nodes[3]->pos)/6.0;
    }

    bool isValid(){
        return (SixTimesTetrahedronVolume(nodes[0]->pos, nodes[1]->pos, nodes[2]->pos, nodes[3]->pos)>0);
    }

    void interpolateNodeValue(Vector3D pos, std::vector<double> &scalars, std::vector<Vector3D> &vectors){

        double v0 = SixTimesTetrahedronVolume(nodes[1]->pos, nodes[3]->pos, nodes[2]->pos, pos);
        double v1 = SixTimesTetrahedronVolume(nodes[0]->pos, nodes[2]->pos, nodes[3]->pos, pos);
        double v2 = SixTimesTetrahedronVolume(nodes[0]->pos, nodes[3]->pos, nodes[1]->pos, pos);
        double v3 = SixTimesTetrahedronVolume(nodes[0]->pos, nodes[1]->pos, nodes[2]->pos, pos);
        double v= v0 + v1 + v2 + v3;
        std::array<double, 4> weights{{v0/v, v1/v, v2/v, v3/v}};
        int numScalars = nodes[0]->scalarValues.size();
        int numVectors = nodes[0]->vectorValues.size();
        for(int i=0; i<numScalars; i++){
            double value =0;
            for(int j=0; j<4; j++){
                value += weights[j] * nodes[i]->scalarValues[j];
            }
            scalars.push_back(value);
        }

        for(int i=0; i<numVectors; i++){
            Vector3D vec(0.0);
            for(int j=0; j<4; j++){
                vec += weights[j] * nodes[i]->vectorValues[j];
            }
            vectors.push_back(vec);
        }

    }

    void interpolateNodeValue(std::array<double, 4> &sWeights, std::vector<double> &scalars, std::vector<Vector3D> &vectors){
        std::array<double, 4> weights = sWeights;
        double sumWeights = 0;
        for(auto w: weights){
            sumWeights += w;
        }
        for(auto &w: weights){
            w /= sumWeights;
        }

        int numScalars = nodes[0]->scalarValues.size();
        int numVectors = nodes[0]->vectorValues.size();
        for(int i=0; i<numScalars; i++){
            double value =0;
            for(int j=0; j<4; j++){
                value += weights[j] * nodes[i]->scalarValues[j];
            }
            scalars.push_back(value);
        }

        for(int i=0; i<numVectors; i++){
            Vector3D vec(0.0);
            for(int j=0; j<4; j++){
                vec += weights[j] * nodes[i]->vectorValues[j];
            }
            vectors.push_back(vec);
        }
    }

    void generateBoundingBox(){
        boundingBox.reset();
        for(auto n: nodes){
            boundingBox.insert(n->pos.XYZ());
        }
    }
};


class Mesh{
public:
    std::vector<std::string> scalarValueNames;
    std::vector<std::string> vectorValueNames;
    std::vector<Node *> nodes;
    std::vector<Tetrahedron *> tetrahedrons;

    struct kdtree *nodeKDTree = nullptr;
    struct kdtree *tetKDTree = nullptr;
    Mesh(){}
    void clone(const Mesh &aMesh);

    void rebuildIndices();
    void rebuildTetrahedronsAdjacency();

    
    //
    double maxSizing = std::numeric_limits<double>::min();
    double minSizing = std::numeric_limits<double>::max();
    void estimateSizing();
    void interpolateNodeValuesForAnotherMesh(Mesh &anotherMesh);
    void interpolateNodeValues(std::vector<Vector3D> &positions, std::vector<std::vector<double>> &scalars, std::vector<std::vector<Vector3D>> &vectors);


    //Spatial search
    double searchRangeSize;
    void readyForSpatialSearch(bool toBuildTetKDTree=true, bool toBuildNodeKDTree = true, bool toEstimateSizing = true);
    bool searchTetrahedronContain(Vector3D pos,  Tetrahedron* &goalTet);
    bool searchTetrahedronContain(Vector3D pos,  Tetrahedron* &goalTet, std::array<double, 4> &weights);

    //IO
    void loadMESH(const std::string &filePath);
    void loadNodeValues(const std::string &filePath);
    void exportNodeValues(const std::string &filePath);
    void exportMESH(const std::string &filePath);
};
