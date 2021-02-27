#pragma once
#include<vector>
#include"vector3d.h"
#include<string>


void loadXYZ(std::string filePath, std::vector<Vector3D> &pos);
void exportNodeValues(std::vector<std::vector<double>> &scalars
, std::vector<std::vector<Vector3D>> &vectors
, std::vector<std::string> &scalarValueNames
, std::vector<std::string> &vectorValueNames
, std::string filePath);