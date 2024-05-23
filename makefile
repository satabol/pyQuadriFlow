default:
	g++ ctypes_quadriflow.cpp -L/opt/github.com/quadriflow/build -l:libquadriflow.a -I /opt/github.com/quadriflow/src -I /opt/github.com/eigen/build/include/include/eigen3 -o ctypes_quadriflow.so -fPIC -shared

executable:
	g++ ctypes_quadriflow.cpp -L/opt/github.com/quadriflow/build -l:libquadriflow.a -I /opt/github.com/quadriflow/src -I /opt/github.com/eigen/build/include/include/eigen3 -o ctypes_quadriflow
