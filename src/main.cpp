/*
 * @Author: Kejie Fu
 * @Date: 2021-09-27 19:41:31
 * @LastEditTime: 2021-09-27 22:18:43
 * @LastEditors: Kejie Fu
 * @Description: 
 * @FilePath: /FastInterpolation/src/main.cpp
 */
#include<iostream>
#include<fstream>
#include<string>
#include "mesh.h"
#include "io.h"

int main(int argc, char *argv[]){
    std::string inputName;
    std::string outputName;
    std::string backgroundName;
    int choice = -1;
    for(int i=1; i<argc; i++){
        std::string str(argv[i]);
        if (str=="-o"){
            i++;
            outputName = std::string(argv[i]);
        }
        else if(str=="-iv"){
            choice = 1;
            i++;
            inputName = std::string(argv[i]);
        }
        else if(str=="-im"){
            choice = 2;
            i++;
            inputName = std::string(argv[i]);
        }
        else{
            backgroundName = std::string(argv[i]);
        }

    }

    // choice = 1;
    // inputName = "/home/kjfu/research/FastInterpolation/examples/bugCase/test";
    // backgroundName = "/home/kjfu/research/FastInterpolation/examples/bugCase/disloc2dout";
    if(choice<0){
        std::cout << "*Invalid Input*: Choose -iv to interpolating for vetices or -im to interpolating for mesh\n";
        return 0;
    }

    if (inputName.empty()){
        std::cout << "*Invalid Input*: Input the input file path without any postfix\n";
        return 0;
    }
    if (backgroundName.empty()){
        std::cout << "*Invalid Input*: Input the background file path with out any postfix\n";
        return 0;
    }

    if(outputName.empty()){
        outputName = inputName + "_out";
    }

    Mesh backgroundMesh;
    backgroundMesh.loadMESH(backgroundName+".mesh");
    backgroundMesh.loadNodeValues(backgroundName+".value");


    if (choice==1){
        std::vector<Vector3D> pos;
        loadXYZ(inputName+".xyz", pos);
        std::vector<std::vector<double>> scalars(pos.size());
        std::vector<std::vector<Vector3D>> vectors(pos.size());
        
        backgroundMesh.interpolateNodeValues(pos, scalars, vectors);

        exportNodeValues(scalars, vectors, backgroundMesh.scalarValueNames, backgroundMesh.vectorValueNames, outputName+".value");
    }
    else if(choice==2){
        Mesh keyMesh;
        keyMesh.loadMESH(inputName+".mesh");
        backgroundMesh.interpolateNodeValuesForAnotherMesh(keyMesh);
        keyMesh.exportNodeValues(outputName+".value");

    }




    return 0;

}