/*
 * @Author: Kejie Fu
 * @Date: 2021-09-27 19:41:31
 * @LastEditTime: 2021-09-27 22:06:13
 * @LastEditors: Kejie Fu
 * @Description: 
 * @FilePath: /FastInterpolation/src/io.cpp
 */
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
        file.precision(10);
        file << "scalar  "<< scalarValueNames[i] <<std::endl;
        for(auto s: scalars){
            for(auto c: s){
                file << c << std::endl;
            }              
        }
      
    }

    for(int i=0; i<vectorValueNames.size(); i++){
        file << "vector  "<< vectorValueNames[i] <<std::endl;
        for (auto v: vectors){
            for(auto c: v){
                file.precision(10);
                file << c[0] << "  " << c[1] << "  " << c[2] << std::endl;
            }
        }        
    }
    file.close();
}