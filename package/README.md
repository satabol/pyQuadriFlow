This is a Python wrapper of quadriflow used in the Blender:

![image](https://github.com/satabol/QuadriFlow/assets/14288520/56dd4baf-284f-4cbb-b866-61b434e44b1b)

See https://github.com/satabol/pyQuadriFlow

## Usage

```
pip install pyQuadriFlow
```

```
import pyQuadriFlow
pyquadriflow(
        faces,  # How many faces you need
        seed,   # random seed
        mesh_vertices, # mesh vertices [ [x0,y0,z0], [x1,y1,z1], ...]
        face_indexes,  # list of face indices as triangles: [[0,1,2],[1,2,3],...]
        flag_preserve_sharp,
        flag_preserve_boundary,
        flag_adaptive_scale,
        flag_aggresive_sat,
        flag_minimum_cost_flow
    )
```