#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// Defining a vector as 3 doubles
struct vector3 {
    double x, y, z;
};

// Function to find the next edge after a given edge
int next(int edge);

// Becomes true if the shape is manifold
bool manifold;

// Returns true if the two vectors are the same.
bool compareTwoVectors(vector3 first, vector3 second);

int main(int argc, char *argv[])
{
    // Exit program if no model is given, or if more than 1 model is given
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " ../models/<modelname>.tri\n";
        return 0;
    };

    // Will hold vertices from the given model
    std::vector<vector3> raw_vertices;

    // Open given model
    std::ifstream model(argv[1]);

    // Check if given model successfully opened
    if(!model.is_open()){
        std::cout << "Could not open file\n";
    }else{
        std::cout << "|----- Computing Face Index Format -----|\n";
        // First read number of triangles from the first line
        int numTriangles;
        model >> numTriangles;

        // Read all the vertices and place them into the raw_vertices vector
        for (int i = 0; i < numTriangles * 3; i++){
            // Check if end of file is reached before loop ends. This means vertices missing.
            if(model.eof()){
                std::cout << "The file seems to be missing one or more vertices\n";
                return 0;
            }

            vector3 vec;
            model >> vec.x >> vec.y >> vec.z;

            raw_vertices.push_back(vec);
        }

        // Check if there is still more data left in the file. This means too many vertices.
        double x; model >> x;
        if(!model.eof()){
                std::cout << "The file seems to have 1 or more extra vertices\n";
                return 0;
        }

        // Will hold vertex ID's for all of the triangles
        std::vector<long> vertexID(raw_vertices.size(), -1);

        // Will hold non-repeating vertices
        std::vector<vector3> new_vertices;

        // Set first ID to 0
        long nextVertexID = 0;

        // Gives an ID to all vertices, and matching vertices have the same ID
        // Loop through vertices
        for(long vertex = 0; vertex < raw_vertices.size(); vertex++){
            // Check if the vertex already exists. If it exists then set its ID as the same.
            for (long other = 0; other < vertex; other++){
                if(compareTwoVectors(raw_vertices[vertex], raw_vertices[other])){
                    vertexID[vertex] = vertexID[other];
                }
            }

            // If the vertex is not already set, set it to the next id available. Also add the vertex to new_vertices
            if(vertexID[vertex] == -1){
                vertexID[vertex] = nextVertexID++;
                new_vertices.push_back(raw_vertices[vertex]);
            }
        }

        // Check for less than or more than 2 faces at each edge
        for(int i = 0; i < vertexID.size(); i++){
            int count = 0;

            // Get the two indices for edge A
            int edgeI = vertexID[i];
            int edgeInext = vertexID[next(i)];

            // Go through the edges and count how many times the edge A shows up
            for(int j = 0; j < vertexID.size(); j++){
                // The edge we are comparing edge A to
                int edgeJ = vertexID[j];
                int edgeJnext = vertexID[next(j)];

                // Increment the counter if edge A is found in the list
                if((edgeI == edgeJ && edgeInext == edgeJnext) || (edgeInext == edgeJ && edgeI == edgeJnext)){
                    count++;
                }
            }

            // If the count is not 2, then that means it is used for more than 2 faces or less than 2 faces
            if(count != 2){
                manifold = false;
                // Print out the edge that has failed
                std::cout << "This mesh is not manifold.\n";
                std::cout << "Edge " << i << " has failed as it has " << count << " faces.\n";
                break;
            }else{
                manifold = true;
            }
        }

        // Get model name
        std::string stringParameter(argv[1]);
        int firstIndex = stringParameter.find_last_of("/") + 1;
        int lastIndex = stringParameter.find_last_of(".");
        int nameSize = lastIndex - firstIndex;
        std::string modelName = stringParameter.substr(firstIndex, nameSize);

        // Create face file
        std::ofstream facefile;
        std::string filePath = "../face_models/" + modelName + ".face";
        facefile.open(filePath.c_str());

        // Printing standard information to the face file
        facefile << "# University of Leeds 2020-2021\n";
        facefile << "# COMP 5821M Assignment 1\n";
        facefile << "# Bilal Khan\n";
        facefile << "# 201051660\n";
        facefile << "#\n";
        facefile << "# Object name: " << modelName << "\n";
        facefile << "# Vertices=" << new_vertices.size() << " Faces=" << numTriangles << "\n";
        facefile << "#\n";

        // Print all vertices to the file
        for(long vertex = 0; vertex < new_vertices.size(); vertex++){
            facefile << "Vertex " << vertex << " " << new_vertices[vertex].x << " " << new_vertices[vertex].y << " " << new_vertices[vertex].z << "\n";
        }

        // Print all faces to the file
        int index = 0;
        for (int face = 0; face < vertexID.size(); face+=3){
            facefile << "Face " << index << " " << vertexID[face] << " " << vertexID[face+1] << " " << vertexID[face+2] << "\n";
            index++;
        }

        facefile.close();

        if(manifold){
            // Checking if it is manifold or not. We require these three values.
            // Num of vertices
            int v = new_vertices.size();
            // Num of faces
            int f = vertexID.size() / 3;
            // Num of edges: 3f = 2e
            int e = (3 * f) / 2;
            // Genus: v - e + f = 2 - 2g
            double g = (2 - f + e - v) / 2;

            std::cout << "This mesh is manifold.\n";
            std::cout << "Genus : " << g << std::endl;
        }

        std::cout << "\nWorst Case Algorithm Complexity: O(n^2). Where n is the number of vertices." << std::endl;
        std::cout << "Best Case Algorithm Complexity: O(n). Where n is the number of vertices." << std::endl;

        std::cout << "|----------------Finished---------------|\n";
    }
    return 0;
}

// Returns true if the two vectors are the same
bool compareTwoVectors(vector3 first, vector3 second){
    if(first.x == second.x && first.y == second.y && first.z == second.z){
        return true;
    }else{
        return false;
    }
}

int next(int edge){
    int face = edge / 3;
    int which = edge % 3;
    int nextWhich = (which+1) % 3;

    int edgeID = face * 3 + nextWhich;

    return edgeID;
}
