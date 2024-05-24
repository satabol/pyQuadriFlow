## Description

This project is a wrapper for Python of https://github.com/hjwdzh/QuadriFlow
This wrapper is used for a Blender addon [Sverchok](https://github.com/nortikin/sverchok)
for a node "quadriflow" to remesh.

## Dependency

This project has dependency of 
https://github.com/satabol/quadriflow.git. You have to build quadriflow library first.

## Build for Windows

Clone this repository to some folder:

```
git clone https://github.com/satabol/pyQuadriFlow.git
```

![image](https://github.com/satabol/QuadriFlow/assets/14288520/6c7953c6-06c0-4356-9694-2f61a912179c)

Now open this project with Visual Studio 2022 and check some dependencies of the project:

![image](https://github.com/satabol/QuadriFlow/assets/14288520/6bb47366-bdb9-4b2c-a43f-f43f9eda8835)

Now you can build the project:

![image](https://github.com/satabol/QuadriFlow/assets/14288520/5f635fd2-e8e5-4264-8957-6f9f89938d55)

If all good you get a file with a name **ctypes_QuadriFlow.dll**

It is a first file for package. Now you have to build library file for Linux...

## Build for Linux

Clone this repository to some linux folder:

```
git clone https://github.com/satabol/pyQuadriFlow.git
```

![image](https://github.com/satabol/QuadriFlow/assets/14288520/c73975ec-f9db-4bf0-809b-25c947772dc1)

Run command to build library:

```
g++ ctypes_QuadriFlow.cpp -L/opt/github.com/quadriflow/build -l:libquadriflow.a -I /opt/github.com/quadriflow/src -I /opt/github.com/eigen/build/include/include/eigen3 -o ctypes_QuadriFlow.so -fPIC -shared
```

![image](https://github.com/satabol/QuadriFlow/assets/14288520/405c5763-2f13-4c27-b592-9eeebf880c6f)

And now you have two libraries for the Python package pyQuadriFlow.
Copy them into the package clib folder:

![image](https://github.com/satabol/QuadriFlow/assets/14288520/eca1f40c-b4ab-41a7-bb6c-589f0adace1d)

Your python package is ready.

## Test in Sverchok

To install this package into the Blender python package repository:

```
"E:\install\Blender\blender-3.6.12-windows-x64\3.6\python\Scripts\pip.exe" install .
```

Not start the Blender and try to import this package:

```
import pyQuadriFlow
```

![image](https://github.com/satabol/QuadriFlow/assets/14288520/4cea4879-4dc9-4b92-85ab-a4f5e528bf66)

Now lets try use a node 'quadriflow' in [Sverchok](https://github.com/nortikin/sverchok):

![image](https://github.com/satabol/QuadriFlow/assets/14288520/1eac54bb-5a6f-4583-b73a-7295cd452860)

One yet example:

![image](https://github.com/satabol/QuadriFlow/assets/14288520/1cf41183-a7d0-4556-bf8a-03fdc47a5656)