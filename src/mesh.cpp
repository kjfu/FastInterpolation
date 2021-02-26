#include "mesh.h"
#include "hashFacet.h"
#include "kdtree.h"
#include <unordered_map>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>

double SixTimesTetrahedronVolume(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3){
    Vector3D a = v0 - v2;
    Vector3D b = v1 - v0;
    Vector3D c = v3 - v0;
    double rst = c[0]*(a[1]*b[2]-b[1]*a[2]) - c[1]*(a[0]*b[2]-b[0]*a[2]) + c[2]*(a[0]*b[1]-b[0]*a[1]);
    return rst;
}





void Mesh::rebuildIndices(){
    for(int i=0; i<nodes.size(); i++){
        nodes[i]->index = i;
    }

    for(int i=0; i<tetrahedrons.size(); i++){
        tetrahedrons[i]->index = i;
    }
}


void Mesh::rebuildTetrahedronsAdjacency(){
    HashFacetTable table;
    for(auto e: tetrahedrons){
        for(int i=0; i<4; i++){
            if(e->adjacentTetrahedrons[i] == nullptr){
                TriangleFacet keyTri = e->facet(i);
                TriangleFacet goalTri;
                if (table.searchAnother(keyTri, goalTri)){
                    Tetrahedron *goalTet = goalTri.tet;
                    e->adjacentTetrahedrons[i] = goalTet;
                    goalTet->adjacentTetrahedrons[goalTri.localIndex] = e;
                    table.remove(goalTri);
                }
                else{
                    table.insert(keyTri);
                }
            }
        }
    }
}






void Mesh::readyForSpatialSearch(bool toBuildTetKDTree, bool toBuildNodeKDTree, bool toEstimateSizing){
    if (toBuildNodeKDTree){
        if(nodeKDTree!=nullptr){
            free(nodeKDTree);
        }

        nodeKDTree = kd_create(3);
        for(auto n: nodes){
            kd_insert(nodeKDTree, n->pos.data(), static_cast<void*>(n));
        }
    }


    // estimateSizing();
    // tetRTree.RemoveAll();
    // for(auto e:tetrahedrons){
    //     e->generateBoundingBox();
    //     tetRTree.Insert(e->boundingBox.minimum.data(), e->boundingBox.minimum.data(), e);
    // }
    if(toEstimateSizing){
        estimateSizing();
    }


    if (toBuildTetKDTree){
        if(tetKDTree!=nullptr){
            free(tetKDTree);    
        }

        tetKDTree = kd_create(3);
        for(auto e: tetrahedrons){
            e->generateBoundingBox();
            kd_insert(tetKDTree, e->center().data(), static_cast<void*>(e));

        }        
    }

}



bool Mesh::searchTetrahedronContain(Vector3D pos, Tetrahedron* &goalTet){
    bool rst = false;
    kdres *set = kd_nearest_range(tetKDTree, pos.data(), maxSizing);
    while (!kd_res_end(set)){
        Tetrahedron *tet = static_cast<Tetrahedron *>(kd_res_item_data(set));
        if (tet->boundingBox.contain(pos) && tet->contain(pos)){
            goalTet = tet;
            rst = true;
            break;
        }
        kd_res_next(set);
    }
    kd_res_free(set);
    return rst;

    // bool findGoal = false;
    // auto rtreeCallBack =
    // [&goalTet, &pos, &findGoal]
    // (Tetrahedron* const &tet)
    // {
    //     bool rst = true;

    //     if (tet->contain(pos)){
    //         goalTet = tet;
    //         findGoal = true;
    //         rst = false;
    //     }
    //     return rst;
    // };

    // Vector3D max = pos + maxSizing;
    // Vector3D min = pos - maxSizing;
    // tetRTree.Search(min.data(), max.data(), rtreeCallBack);
    // return findGoal;
}

bool Mesh::searchTetrahedronContain(Vector3D pos,  Tetrahedron* &goalTet, std::array<double, 4> &weights){

    bool rst = false;
    kdres *set = kd_nearest_range(tetKDTree, pos.data(), maxSizing);
    while (!kd_res_end(set)){
        Tetrahedron *tet = static_cast<Tetrahedron *>(kd_res_item_data(set));
        if (tet->boundingBox.contain(pos) && tet->contain(pos, weights)){
            goalTet = tet;
            rst = true;
            break;
        }
        kd_res_next(set);
    }
    kd_res_free(set);
    return rst;  

    // bool findGoal = false;
    // auto rtreeCallBack =
    // [&goalTet, &pos, &findGoal, &weights]
    // (Tetrahedron* const &tet)
    // {
    //     bool rst = true;

    //     if (tet->contain(pos, weights)){
    //         goalTet = tet;
    //         findGoal = true;
    //         rst = false;
    //     }
    //     return rst;
    // };

    // Vector3D max = pos + maxSizing;
    // Vector3D min = pos - maxSizing;
    // tetRTree.Search(min.data(), max.data(), rtreeCallBack);
    // return findGoal;    
}

void Mesh::clone(const Mesh &aMesh){
    std::unordered_map<Node*, Node *> originNodesMap;
    std::unordered_map<Tetrahedron*, Tetrahedron*> originElementsMap;
    for(auto n: aMesh.nodes){
        Node *aNode = new Node();
        *aNode = *n;
        originNodesMap[n] = aNode;
        this->nodes.push_back(aNode);
    }

    for(auto e: aMesh.tetrahedrons){
        Tetrahedron *aTet = new Tetrahedron();
        *aTet = *e;
        originElementsMap[e] = aTet;
        this->tetrahedrons.push_back(aTet);
        for(auto &n: aTet->nodes){
            n = originNodesMap[n];
        }
    }

    for(auto e: aMesh.tetrahedrons){
        for(auto &adj: e->adjacentTetrahedrons){
            if (adj){
                adj = originElementsMap[adj];
            }
        }
    }
}


void Mesh::loadNodeValues(const std::string &filePath){
    std::ifstream file(filePath);
    if (file.is_open()){
        while(file){
            std::string line;
            std::getline(file, line);
            std::stringstream lineStream(line);
            std::string keyString;
            lineStream >> keyString;
            if(keyString == "scalar"){
                std::string name;
                lineStream >> name;
                scalarValueNames.push_back(name);
                for(int i=0; i<nodes.size(); i++){
                    std::string line;
                    std::getline(file, line);
                    std::stringstream lineStream(line);
                    double value;
                    lineStream >> value;
                    nodes[i]->scalarValues.push_back(value); 
                }                
            }
            else if (keyString == "vector"){
                std::string name;
                lineStream >> name;
                vectorValueNames.push_back(name);
                for(int i=0; i<nodes.size(); i++){
                    std::string line;
                    std::getline(file, line);
                    std::stringstream lineStream(line);
                    double u, v, w;
                    lineStream >> u >> v>> w;
                    Vector3D vec(u, v, w);
                    nodes[i]->vectorValues.push_back(vec); 
                }    
            }
            
        }

        file.close();
    }
}



void Mesh::loadMESH(const std::string &filePath){
    std::ifstream file(filePath);
    if (file.is_open()){
        while (file)
        {
            std::string line;
            std::string keyString;
            std::getline(file, line);
            std::stringstream lineStream(line);
            lineStream >> keyString;

            if (keyString == "Vertices"){
                std::getline(file, line);
                std::stringstream lineStream(line);
                int nv;
                lineStream >> nv;
                for(int i=0; i<nv; i++){
                    std::getline(file, line);
                    std::stringstream vertexLineStream(line);
                    double x, y, z;
                    int label;
                    vertexLineStream >> x >> y >> z >> label;
                    Node *n = new Node(x, y, z);
                    n->label = label;
                    nodes.push_back(n);
                }
            }
            else if (keyString == "Tetrahedra"){
                std::getline(file, line);
                std::stringstream lineStream(line);
                int nt;
                lineStream >> nt;
                for(int i=0; i<nt;  i++){
                    std::getline(file, line);
                    std::stringstream tetLineStream(line);
                    int n0, n1, n2, n3, label;
                    tetLineStream >> n0 >> n1 >> n2 >> n3 >> label;
                    Tetrahedron *tet = new Tetrahedron(nodes[n0-1], nodes[n1-1], nodes[n2-1], nodes[n3-1]);
                    tet->label = label;
                    tetrahedrons.push_back(tet);
                }
            }
            else if (keyString == "End"){
                break;
            }

        }
        rebuildIndices();
        file.close();
    }
}




void Mesh::exportMESH(const std::string &filePath){
    std::ofstream file(filePath);
    file << "MeshVersionFormatted 2\n";
    file << "Dimension\n         3\n";

    file << "Vertices\n";
    file << nodes.size() << "\n";
    for(auto n:nodes){
        file << n->pos[0] 
        << "  " << n->pos[1] 
        << "  " << n->pos[2]
        << "  " << n->label <<"\n";
    }

    file << "Tetrahedra\n";
    file << tetrahedrons.size() << "\n";
    for(auto e: tetrahedrons){
        file << e->nodes[0]->index + 1
        << "  " << e->nodes[1]->index + 1
        << "  " << e->nodes[2]->index + 1
        << "  " << e->nodes[3]->index + 1
        << "  " << e->label << "\n"; 
    }

    file << "End\n";
    file.close();
}


void Mesh::estimateSizing(){
    //TODO accelerate
    maxSizing = std::numeric_limits<double>::min();
    minSizing = std::numeric_limits<double>::max();
    for(auto n: nodes){
        n->sizing = std::numeric_limits<double>::max();
    }
    
    for(auto e: tetrahedrons){
        for(int i=0; i<4; i++){
            for(int j=i+1; j<4; j++){
                Node *n0 = e->nodes[i];
                Node *n1 = e->nodes[j];
                double d = distance(n0->pos, n1->pos);
                if(d<n0->sizing){
                    n0->sizing = d;
                }

                if (d<n1->sizing){
                    n1->sizing = d;
                }

                if (d>maxSizing){
                    maxSizing = d;
                }

                if (d<minSizing){
                    minSizing = d;
                }
            }
        }
    }

}

void Mesh::exportNodeValues(const std::string &filePath){
    std::ofstream file(filePath);
    for(int i=0; i<scalarValueNames.size(); i++){
        file << "scalar " << scalarValueNames[i] << std::endl;    
        for (auto n: nodes){
            file << n->scalarValues[i] << std::endl;
        }
    }

    for(int i=0; i<vectorValueNames.size(); i++){
        file << "vector " << vectorValueNames[i] << std::endl;    
        for (auto n: nodes){
        file << n->vectorValues[i][0] << "  "<<n->vectorValues[i][1] << "  " << n->vectorValues[i][2] << std::endl;
        }
    }
    file.close();
}


void Mesh::interpolateNodeValuesForAnotherMesh(Mesh &anotherMesh){
    estimateSizing();
    readyForSpatialSearch(true);
    anotherMesh.scalarValueNames = this->scalarValueNames;
    anotherMesh.vectorValueNames = this->vectorValueNames;
    for(auto n: anotherMesh.nodes){
        kdres *set = kd_nearest_range(nodeKDTree, n->pos.data(), 1e-10);
        if (kd_res_size(set)){
            double pos[3];
            Node *goalNode = static_cast<Node*>(kd_res_item_data(set));
            n->vectorValues = goalNode->vectorValues;
            n->scalarValues = goalNode->scalarValues;
            // n->edit = 0;
            kd_res_free(set);
            continue;
        }
        kd_res_free(set);


        Tetrahedron *goalTet;
        std::array<double, 4> weights;
        if (searchTetrahedronContain(n->pos, goalTet, weights)){
            std::vector<double> scalars;
            std::vector<Vector3D> vecs;
            goalTet->interpolateNodeValue(weights, scalars, vecs);
            n->scalarValues = scalars;
            n->vectorValues = vecs;
            // n->edit=0;
        }
        // else{
        //     n->edit=-1;
        // }
        
    }


}



void Mesh::interpolateNodeValues(std::vector<Vector3D> &positions, std::vector<std::vector<double>> &scalars, std::vector<std::vector<Vector3D>> &vectors){
    estimateSizing();
    readyForSpatialSearch(true);


    for(int i=0; i<positions.size(); i++){
        kdres *set = kd_nearest_range(nodeKDTree, positions[i].data(), 1e-10);
        if (kd_res_size(set)){
            double pos[3];
            Node *goalNode = static_cast<Node*>(kd_res_item_data(set));
            vectors[i] = goalNode->vectorValues;
            scalars[i] = goalNode->scalarValues;
            kd_res_free(set);
            continue;
        }
        kd_res_free(set);


        Tetrahedron *goalTet;
        std::array<double, 4> weights;
        if (searchTetrahedronContain(positions[i].data(), goalTet, weights)){
            std::vector<double> scals;
            std::vector<Vector3D> vecs;
            goalTet->interpolateNodeValue(weights, scals, vecs);
            scalars[i] = scals;
            vectors[i] = vecs;

        }
        
    }
}
