# Fast Interpolation
Fast interpolation of scalars and vectors based on the original tetrahedral mesh

##  1. To interpolate for vetices
Suggest you have test.mesh and test.value as original tetrahedral mesh with values, the vertices you want to interpolate is in vertices.xyz, you can input as below:

```
>> FastInterpolation ./test -iv ./vertices -o ./rst
```
choose -iv for to interpolating for vetices.   
All input should with no postfix!  
After running you can get rst.value for all the values interpolating from the original tetrahedral mesh. 
##  2. To interpolate for mesh
Suggest you have test3d.mesh and test3d.value as original tetrahedral mesh with values, the other mesh you want to interpolate is in otherMesh.mesh, you can input as below:

```
>> FastInterpolation ./test3d -im ./otherMesh -o ./rst
```
choose -im for to interpolating for other mesh.  
All input should with no postfix!  
After running you can get rst.value for all the values interpolating from the original tetrahedral mesh. 
