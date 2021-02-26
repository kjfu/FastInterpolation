#include "io.h"
#include<fstream>
#include<sstream>
void loadXYZ(std::string filePath, std::vector<Vector3D> &pos){
    std::ifstream file(filePath);
    if (file.is_open()){
        while (file){
            std::string line;
            std::getline(file, line);
            if(line.size()>3){
                std::stringstream lineStream(line);
                double x, y, z;
                lineStream >> x >> y >>z;
                pos.emplace_back(x, y, z);
            }
        }
        file.close();
    }




}
void exportNodeValues(std::vector<std::vector<double>> &scalars
, std::vector<std::vector<Vector3D>> &vectors
, std::vector<std::string> &scalarValueNames
, std::vector<std::string> &vectorValueNames
, std::string filePath){
    std::ofstream file(filePath);
    for(int i=0; i<scalarValueNames.size(); i++){
        file << "scalar  "<< scalarValueNames[i] <<std::endl;
        for(auto c:scalars[i]){
            file << c << std::endl;
        }        
    }

    for(int i=0; i<vectorValueNames.size(); i++){
        file << "vector  "<< vectorValueNames[i] <<std::endl;
        for(auto c:scalars[i]){
            file << c << std::endl;
        }        
    }
    file.close();
}