import ctypes
import numpy as np
import sys
import traceback

from pyQuadriFlow.clib import load_library
QuadriFlow_clib = load_library.load_library()

class MESH_DATA(ctypes.Structure):
    _fields_ = [
        ("nn_verts", ctypes.c_int),
        ("nn_faces", ctypes.c_int),
        ("vertices", ctypes.POINTER((ctypes.c_float)*3), ),
        ("faces", ctypes.POINTER( (ctypes.c_int)*4), ),
        ("has_error", ctypes.c_bool, ),
        ("str_error", ctypes.c_char_p, ),
    ]

def pyquadriflow(
        faces,
        seed,
        mesh_vertices,
        face_indexes,
        flag_preserve_sharp,
        flag_preserve_boundary,
        flag_adaptive_scale,
        flag_aggresive_sat,
        flag_minimum_cost_flow
    ):

    """
    Documentation
    """   

    remesh_quadriflow = QuadriFlow_clib.remesh_quadriflow
    remesh_quadriflow.argtypes = [
        ctypes.c_int, # request number of faces
        ctypes.c_int, # seed
        ctypes.c_int, # n_verts
        ctypes.c_int, # n_faces
        ctypes.POINTER((ctypes.c_float)*3), # vertices 
        ctypes.POINTER((ctypes.c_int)*3  ), # faceVerts
        ctypes.c_bool, # flag_preserve_sharp
        ctypes.c_bool, # flag_preserve_boundary
        ctypes.c_bool, # flag_adaptive_scale
        ctypes.c_bool, # flag_aggresive_sat
        ctypes.c_bool, # flag_minimum_cost_flow
    ]
    remesh_quadriflow.restype = ctypes.POINTER(MESH_DATA)

    free_mem = QuadriFlow_clib.free_mem
    free_mem.argtypes = [
        ctypes.POINTER(MESH_DATA)
    ]
    
    n_verts = len(mesh_vertices)
    n_faces = len(face_indexes)
    vertices_array = ctypes.ARRAY(n_verts,ctypes.c_float*3)(*[(ctypes.c_float*3)(*vert) for vert in mesh_vertices])
    faces_array    = ctypes.ARRAY(n_faces,ctypes.c_int  *3)(*[(ctypes.c_int  *3)(*vert) for vert in face_indexes])

    md = remesh_quadriflow(
        faces,
        seed,
        n_verts,
        n_faces,
        vertices_array, # vertices (as c array)
        faces_array, # vertices (as c array)
        flag_preserve_sharp,
        flag_preserve_boundary,
        flag_adaptive_scale,
        flag_aggresive_sat,
        flag_minimum_cost_flow
    )
    
    if(faces > 0):
        ################ Get Results ################
        #### Gather New Mesh Information ####
        #QuadriFlow_clib.nn_verts.restypes = ctypes.c_int 
        mdc = md.contents
        if mdc.has_error==False:
            #### Extract New Vertices #### 
            new_vertices = [ mdc.vertices[i][:] for i in range(mdc.nn_verts)]

            #### Extract New Faces #### 
            new_faces = [ mdc.faces[i][:] for i in range(mdc.nn_faces)]
            free_mem(mdc)

            # tolist() is quite slow but it seems necessary for blender. 
            # Er, well, maybe it's not that bad idk. 
            new_mesh = {
                'vertices' : np.ctypeslib.as_array(new_vertices).tolist(),
                'faces' : np.ctypeslib.as_array(new_faces).tolist()
            }
        else:
            str_error = mdc.str_error.decode("ascii")
            free_mem(mdc)            
            raise Exception(f"QuadriFlow internal error: {str_error}. Recommendation - change some settings")

        return new_mesh 

if __name__ == "__main__":
    pass