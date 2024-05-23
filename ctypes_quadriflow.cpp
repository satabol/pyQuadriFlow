//#include <iostream>
//#include <vector>
//#include <string>
//#include <map>
//#include <algorithm>
#include <unordered_map>

// This actually works? 
// https://stackoverflow.com/a/25155315/2391876
#ifdef _WIN32 // This is what Visual Studio defines 
#define DLLEXPORT __declspec(dllexport)
// This resolves the "M_PI (pi) is not defined properly" error. 
// (https://stackoverflow.com/questions/6563810/m-pi-works-with-math-h-but-not-with-cmath-in-visual-studio)
// There may be better approaches to this, but I'm not sure what. 
// I also really feel like this should not be necessary, see OpenSubDiv -> CMakeLists.txt -> /D_USE_MATH_DEFINES. 
#define _USE_MATH_DEFINES
#include <math.h>
#elif __linux__
#define DLLEXPORT 
#elif __APPLE__
// [ ] Implement 
#endif


//---------------- Vertex container implementation. ----------------
struct Vertex {
    // Minimal required interface ----------------------
    Vertex() { }

    Vertex(Vertex const& src) {
        _position[0] = src._position[0];
        _position[1] = src._position[1];
        _position[2] = src._position[2];
    }

    void Clear(void* = 0) {
        _position[0] = _position[1] = _position[2] = 0.0f;
    }

    void AddWithWeight(Vertex const& src, float weight) {
        _position[0] += weight * src._position[0];
        _position[1] += weight * src._position[1];
        _position[2] += weight * src._position[2];
    }

    void SetPosition(float x, float y, float z) {
        _position[0] = x;
        _position[1] = y;
        _position[2] = z;
    }

    const float* GetPosition() const {
        return _position;
    }

private:
    float _position[3];
};

//---------------- QUADRIFLOW ----------------
//#include "MEM_guardedalloc.h"
#include "config.hpp"
#include "field-math.hpp"
#include "loader.hpp"
#include "optimizer.hpp"
#include "parametrizer.hpp"
//#include <quadriflow_capi.hpp>
using namespace qflow;

struct ObjVertex {
    uint32_t p = (uint32_t)-1;
    uint32_t n = (uint32_t)-1;
    uint32_t uv = (uint32_t)-1;

    ObjVertex()
    {
    }

    ObjVertex(uint32_t pi)
    {
        p = pi;
    }

    bool operator==(const ObjVertex& v) const
    {
        return v.p == p && v.n == n && v.uv == uv;
    }
};

struct ObjVertexHash {
    std::size_t operator()(const ObjVertex& v) const
    {
        size_t hash = std::hash<uint32_t>()(v.p);
        hash = hash * 37 + std::hash<uint32_t>()(v.uv);
        hash = hash * 37 + std::hash<uint32_t>()(v.n);
        return hash;
    }
};

typedef std::unordered_map<ObjVertex, uint32_t, ObjVertexHash> VertexMap;

static int check_if_canceled(float progress,
    void (*update_cb)(void*, float progress, int* cancel),
    void* update_cb_data)
{
    int cancel = 0;
    update_cb(update_cb_data, progress, &cancel);
    return cancel;
}

class Parametrizer2 : public Parametrizer {

public:
    Parametrizer2():Parametrizer(){
    }

    void load(int n_verts, int n_faces, float mesh_vertices[][3], int faceVerts[][3], MatrixXd& V, MatrixXi& F) {
        /// Vertex indices used by the OBJ format
        struct obj_vertex {
            uint32_t p = (uint32_t)-1;
            uint32_t n = (uint32_t)-1;
            uint32_t uv = (uint32_t)-1;

            inline obj_vertex() { }
            inline obj_vertex(uint32_t _p) {
                p = _p;
            }

            inline bool operator==(const obj_vertex& v) const {
                return v.p == p && v.n == n && v.uv == uv;
            }
        };

        /// Hash function for obj_vertex
        struct obj_vertexHash : std::unary_function<obj_vertex, size_t> {
            std::size_t operator()(const obj_vertex& v) const {
                size_t hash = std::hash<uint32_t>()(v.p);
                hash = hash * 37 + std::hash<uint32_t>()(v.uv);
                hash = hash * 37 + std::hash<uint32_t>()(v.n);
                return hash;
            }
        };

        typedef std::unordered_map<obj_vertex, uint32_t, obj_vertexHash> VertexMap;

        std::vector<Vector3d>  positions;
        std::vector<uint32_t>   indices;
        std::vector<obj_vertex> vertices;
        VertexMap vertexMap;

        int I = 0;
        for (I = 0; I <= n_verts - 1; I++) {
            Vector3d p(mesh_vertices[I][0], mesh_vertices[I][1], mesh_vertices[I][2]);
            positions.push_back(p);
        }


        for (I = 0; I <= n_faces - 1; I++) {
            obj_vertex tri[3];
            int nVertices = 3;

            tri[0] = obj_vertex(faceVerts[I][0]);
            tri[1] = obj_vertex(faceVerts[I][1]);
            tri[2] = obj_vertex(faceVerts[I][2]);

            /* Convert to an indexed vertex list */
            for (int i = 0; i < nVertices; ++i) {
                const obj_vertex& v = tri[i];
                VertexMap::const_iterator it = vertexMap.find(v);
                if (it == vertexMap.end()) {
                    vertexMap[v] = (uint32_t)vertices.size();
                    indices.push_back((uint32_t)vertices.size());
                    vertices.push_back(v);
                }
                else {
                    indices.push_back(it->second);
                }
            }
        }

        F.resize(3, indices.size() / 3);
        memcpy(F.data(), indices.data(), sizeof(uint32_t) * indices.size());

        V.resize(3, vertices.size());
        for (uint32_t i = 0; i <= vertices.size()-1; ++i)
            V.col(i) = positions.at(vertices[i].p);
    }
    void Load(int n_verts, int n_faces, float mesh_vertices[][3], int faceVerts[][3]) {
        load(n_verts, n_faces, mesh_vertices, faceVerts, V, F);
        NormalizeMesh();
    }
};

class MESH_DATA {
public:
    int nn_verts = 0;
    int nn_faces = 0;
    float* vertices = NULL;
    int* faces = NULL;
    bool has_error = false;
    char* str_error = NULL;
};

class quadriflow {
public:
    Parametrizer2 field;
    quadriflow() {
        nn_verts = 0;
        nn_edges = 0;
        nn_faces = 0;
        new_vertices.clear();
        new_edges.clear();
        edge_list.clear();
        new_faces.clear();
    }

    void reset() {
        // Need to do this otherwise these values end up growing as you do subdivisions on top of each other
        new_vertices.clear();
        new_edges.clear();
        edge_list.clear();
        new_faces.clear();
    }

    // outgoing topology 
    int nn_verts;
    int nn_edges;
    int nn_faces;
    std::vector<std::vector<float>> new_vertices;
    std::vector<std::vector<int>> new_edges;
    std::vector<std::vector<int>> edge_list;
    std::vector<std::vector<int>> new_faces;

    // ---------------- Return New Vertices ----------------
    void return_new_vertices(float py_new_vertices[][3]) {
        for (int i = 0; i < nn_verts; i++) {
            py_new_vertices[i][0] = new_vertices[i][0];
            py_new_vertices[i][1] = new_vertices[i][1];
            py_new_vertices[i][2] = new_vertices[i][2];
        }
    }

    // ---------------- Return New Edges ----------------
    void return_new_edges(int py_new_edges[][2]) {
        for (int i = 0; i < nn_edges; i++) {
            py_new_edges[i][0] = edge_list[i][0];
            py_new_edges[i][1] = edge_list[i][1];
        }
    }
    // ---------------- Return New Faces ----------------
    // void return_new_faces(int **py_new_faces) {
    void return_new_faces(int py_new_faces[][4]) {
        for (int i = 0; i < nn_faces; i++) {
            py_new_faces[i][0] = new_faces[i][0];
            py_new_faces[i][1] = new_faces[i][1];
            py_new_faces[i][2] = new_faces[i][2];
            py_new_faces[i][3] = new_faces[i][3];
        }
    }

    void remesh_quadriflow(
        int faces,
        int seed,
        int n_verts, int n_faces, float mesh_vertices[][3], int faceVerts[][3],
        bool flag_preserve_sharp, bool flag_preserve_boundary, bool flag_adaptive_scale, bool flag_aggresive_sat, bool flag_minimum_cost_flow
        /*void (*update_cb)(void*, float progress, int* cancel),
        void* update_cb_data*/
    ) {
        reset();

        if (flag_preserve_sharp == true) {
            field.flag_preserve_sharp = 1;
        }
        if (flag_preserve_boundary == true) {
            field.flag_preserve_boundary = 1;
        }
        if (flag_adaptive_scale == true) {
            field.flag_adaptive_scale == 1;
        }
        if (flag_aggresive_sat == true) {
            field.flag_aggresive_sat == 1;
        }
        if (flag_minimum_cost_flow == true) {
            field.flag_minimum_cost_flow = 1;
        }
        field.hierarchy.rng_seed = seed;

        field.Load(n_verts, n_faces, mesh_vertices, faceVerts);
        field.Initialize(faces);

        if (field.flag_preserve_boundary) {
            Hierarchy& mRes = field.hierarchy;
            mRes.clearConstraints();
            for (uint32_t i = 0; i < 3 * mRes.mF.cols(); ++i) {
                if (mRes.mE2E[i] == -1) {
                    uint32_t i0 = mRes.mF(i % 3, i / 3);
                    uint32_t i1 = mRes.mF((i + 1) % 3, i / 3);
                    Vector3d p0 = mRes.mV[0].col(i0), p1 = mRes.mV[0].col(i1);
                    Vector3d edge = p1 - p0;
                    if (edge.squaredNorm() > 0) {
                        edge.normalize();
                        mRes.mCO[0].col(i0) = p0;
                        mRes.mCO[0].col(i1) = p1;
                        mRes.mCQ[0].col(i0) = mRes.mCQ[0].col(i1) = edge;
                        mRes.mCQw[0][i0] = mRes.mCQw[0][i1] = mRes.mCOw[0][i0] = mRes.mCOw[0][i1] =
                            1.0;
                    }
                }
            }
            mRes.propagateConstraints();
        }
        Optimizer::optimize_orientations(field.hierarchy);
        field.ComputeOrientationSingularities();

        if (field.flag_adaptive_scale == 1) {
            field.EstimateSlope();
        }

        Optimizer::optimize_scale(field.hierarchy, field.rho, field.flag_adaptive_scale);
        field.flag_adaptive_scale = 1; // Confused, but Blender do the same.
        Optimizer::optimize_positions(field.hierarchy, field.flag_adaptive_scale);
        field.ComputePositionSingularities();

        field.ComputeIndexMap();

        nn_verts = field.O_compact.size();  // In very rare circumstances go to the infinite loop. I don't know to do.
        for (int i = 0; i <= nn_verts-1; ++i) {
            auto t = field.O_compact[i] * field.normalize_scale + field.normalize_offset;
            new_vertices.push_back(std::vector<float>{ (float)t.x(), (float)t.y(), (float)t.z()});
        }

        nn_faces = field.F_compact.size();
        for (int i = 0; i <= nn_faces-1; ++i) {
            new_faces.push_back(std::vector<int>({ field.F_compact[i][0], field.F_compact[i][1], field.F_compact[i][2], field.F_compact[i][3] }));
        }
    }

};

extern "C"{

    DLLEXPORT MESH_DATA *remesh_quadriflow(
        int faces, int seed, int n_verts, int n_faces, float mesh_vertices[][3], int faceVerts[][3],
        bool flag_preserve_sharp, bool flag_preserve_boundary, bool flag_adaptive_scale, bool flag_aggresive_sat, bool flag_minimum_cost_flow
    ) {
        MESH_DATA *mesh_data = new MESH_DATA();
        quadriflow quadriflow_new;
        try {
            quadriflow_new.remesh_quadriflow(faces, seed, n_verts, n_faces, mesh_vertices, faceVerts, flag_preserve_sharp, flag_preserve_boundary, flag_adaptive_scale, flag_aggresive_sat, flag_minimum_cost_flow);
            mesh_data->has_error = false;
        }catch (const std::exception& ex) {
            mesh_data->has_error = true;
            mesh_data->nn_verts = 0;
            mesh_data->nn_faces = 0;
            const char *str_error = ex.what();
            int ln = strlen(str_error);
            mesh_data->str_error = (char*)malloc(sizeof(char) * (ln + 2));
            strncpy(mesh_data->str_error, str_error, ln+1);
        }

        if (mesh_data->has_error == false) {
            /* Get the output mesh data */
            mesh_data->nn_verts = quadriflow_new.field.O_compact.size();
            mesh_data->nn_faces = quadriflow_new.field.F_compact.size();

            if (mesh_data->nn_verts > 0) {
                mesh_data->vertices = (float*)malloc(sizeof(float[3]) * mesh_data->nn_verts);
            }
            else {
                mesh_data->vertices = NULL;
                mesh_data->nn_verts = 0;
            }

            if (mesh_data->nn_faces > 0) {
                mesh_data->faces = (int*)malloc(sizeof(int[4]) * mesh_data->nn_faces);
            }
            else {
                mesh_data->faces = NULL;
                mesh_data->nn_faces = 0;
            }

            if (mesh_data->vertices != NULL) {
                for (int i = 0; i < mesh_data->nn_verts; i++) {
                    auto t = quadriflow_new.field.O_compact[i] * quadriflow_new.field.normalize_scale + quadriflow_new.field.normalize_offset;
                    mesh_data->vertices[i * 3 + 0] = t[0];
                    mesh_data->vertices[i * 3 + 1] = t[1];
                    mesh_data->vertices[i * 3 + 2] = t[2];
                }
            }

            if (mesh_data->faces != NULL) {
                for (int i = 0; i < mesh_data->nn_faces; i++) {
                    mesh_data->faces[i * 4 + 0] = quadriflow_new.field.F_compact[i][0];
                    mesh_data->faces[i * 4 + 1] = quadriflow_new.field.F_compact[i][1];
                    mesh_data->faces[i * 4 + 2] = quadriflow_new.field.F_compact[i][2];
                    mesh_data->faces[i * 4 + 3] = quadriflow_new.field.F_compact[i][3];
                }
            }
        } else {

        }
        return mesh_data;
    }

    /// <summary>
    /// Free resurces after process data in python. (Call in Python)
    /// </summary>
    /// <param name="md"></param>
    /// <returns></returns>
    DLLEXPORT void free_mem(MESH_DATA md) {
        if (md.str_error != NULL) {
            free( (char*)md.str_error); // https://stackoverflow.com/questions/2819535/unable-to-free-const-pointers-in-c
            md.str_error = NULL;
        }
        if (md.vertices != NULL) {
            free(md.vertices);
            md.vertices = NULL;
            md.nn_verts = 0;
        }
        if (md.faces != NULL) {
            free(md.faces);
            md.faces = NULL;
            md.nn_faces = 0;
        }
    }
}

int main(int argc, char** argv) {
    printf("Start testing quadriflow!!!");
    float mesh_vertices[][3] = {
        {1.83936208486557e-08, 7.450580596923828e-09, -1.0}, {0.0, 0.0, 1.0}, {0.0, 1.0, -1.0}, {0.19509032368659973, 0.9807852506637573, -1.0}, {0.3826834559440613, 0.9238795042037964, -1.0}, {0.5555702447891235, 0.8314695954322815, -1.0}, {0.7071067690849304, 0.7071067690849304, -1.0}, {0.8314695954322815, 0.5555702447891235, -1.0}, {0.9238795042037964, 0.3826834559440613, -1.0}, {0.9807852506637573, 0.19509032368659973, -1.0}, {1.0, 0.0, -1.0}, {0.9807852506637573, -0.19509032368659973, -1.0}, {0.9238795042037964, -0.3826834559440613, -1.0}, {0.8314695954322815, -0.5555702447891235, -1.0}, {0.7071067690849304, -0.7071067690849304, -1.0}, {0.5555702447891235, -0.8314695954322815, -1.0}, {0.3826834559440613, -0.9238795042037964, -1.0}, {0.19509032368659973, -0.9807852506637573, -1.0}, {0.0, -1.0, -1.0}, {-0.19509032368659973, -0.9807852506637573, -1.0}, {-0.3826834559440613, -0.9238795042037964, -1.0}, {-0.5555702447891235, -0.8314695954322815, -1.0}, {-0.7071067690849304, -0.7071067690849304, -1.0}, {-0.8314695954322815, -0.5555702447891235, -1.0}, {-0.9238795042037964, -0.3826834559440613, -1.0}, {-0.9807852506637573, -0.19509032368659973, -1.0}, {-1.0, 0.0, -1.0}, {-0.9807852506637573, 0.19509032368659973, -1.0}, {-0.9238795042037964, 0.3826834559440613, -1.0}, {-0.8314695954322815, 0.5555702447891235, -1.0}, {-0.7071067690849304, 0.7071067690849304, -1.0}, {-0.5555702447891235, 0.8314695954322815, -1.0}, {-0.3826834559440613, 0.9238795042037964, -1.0}, {-0.19509032368659973, 0.9807852506637573, -1.0},
    };

    //std::vector<std::vector<float>> verts(mesh_vertices, mesh_vertices + sizeof(mesh_vertices) / sizeof(mesh_vertices[0]));

    int mesh_faces[][3] = {
        {2, 1, 3}, {3, 1, 4}, {4, 1, 5}, {5, 1, 6}, {6, 1, 7}, {7, 1, 8}, {8, 1, 9}, {9, 1, 10}, {10, 1, 11}, {11, 1, 12}, {12, 1, 13}, {13, 1, 14}, {14, 1, 15}, {15, 1, 16}, {16, 1, 17}, {17, 1, 18}, {18, 1, 19}, {19, 1, 20}, {20, 1, 21}, {21, 1, 22}, {22, 1, 23}, {23, 1, 24}, {24, 1, 25}, {25, 1, 26}, {26, 1, 27}, {27, 1, 28}, {28, 1, 29}, {29, 1, 30}, {30, 1, 31}, {31, 1, 32}, {32, 1, 33}, {33, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 6}, {0, 6, 7}, {0, 7, 8}, {0, 8, 9}, {0, 9, 10}, {0, 10, 11}, {0, 11, 12}, {0, 12, 13}, {0, 13, 14}, {0, 14, 15}, {0, 15, 16}, {0, 16, 17}, {0, 17, 18}, {0, 18, 19}, {0, 19, 20}, {0, 20, 21}, {0, 21, 22}, {0, 22, 23}, {0, 23, 24}, {0, 24, 25}, {0, 25, 26}, {0, 26, 27}, {0, 27, 28}, {0, 28, 29}, {0, 29, 30}, {0, 30, 31}, {0, 31, 32}, {0, 32, 33}, {0, 33, 2},
    };
    //std::vector<std::vector<int>> faces(mesh_faces, mesh_faces + sizeof(mesh_faces) / sizeof(mesh_faces[0]));
    quadriflow quadriflow_new;
    try {
        quadriflow_new.remesh_quadriflow(
            1000, 0, 34, 64, mesh_vertices, mesh_faces, 0, 0, 0, 0, 0
        );

        auto nv = quadriflow_new.new_vertices;
        auto nf = quadriflow_new.new_faces;

        std::string nv_str;
        for (int I = 0; I <= nv.size() - 1; I++) {
            //nv_str_vec.push_back("(" + std::to_string(nv.at(I)[0]) + ", " + std::to_string(nv.at(I)[1]) + ", " + std::to_string(nv.at(I)[2]) + ",)");
            nv_str.append("(" + std::to_string(nv.at(I)[0]) + ", " + std::to_string(nv.at(I)[1]) + ", " + std::to_string(nv.at(I)[2]) + ",), ");
        }

        std::string nf_str;
        for (int I = 0; I <= nf.size() - 1; I++) {
            nf_str.append("(" + std::to_string(nf.at(I)[0]) + ", " + std::to_string(nf.at(I)[1]) + ", " + std::to_string(nf.at(I)[2]) + ", " + std::to_string(nf.at(I)[3]) + ",), ");
        }
        printf("Quadriflow finished OK.");
    }
    catch (const std::exception& ex) {
        const char* str_error = ex.what();
        printf("Quadriflow finished with an error: %s", str_error);
    }

    int I = 0;
    I++;
    
}