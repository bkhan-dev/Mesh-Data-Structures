#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Becomes true if the shape is manifold
bool manifold;

// Function to find the next edge after a given edge
int next(int edge);

 // Defining a 3D vector as 3 doubles
struct vector3 {
    double x,y,z;
};

int main(int argc, char *argv[])
{
    // Exit program if no model is given, or if more than 1 model is given
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " ../face_models/<modelname>.face\n";
        return 0;
    };

    // Will hold data to to write to .diredge file
    std::vector<vector3> vertices;                  // Simple point in 3D
    std::vector<unsigned int> firstDirectedEdge;    // Any directed edge touching each vertex
    std::vector<unsigned int> faceVertices;         // Every 2 index is a "directed edge to"
    std::vector<unsigned int> otherHalf;            // Pair of directed edges

    // Open given .face file
    std::ifstream model(argv[1]);

    // Check if given .diredge file successfully opened
    if(!model.is_open()){
        std::cout << "Could not open file\n";
    }else{
        std::cout << "|----- Computing Directed Edges -----|\n";
        // Get file name
        std::string stringParameter(argv[1]);
        int firstIndex = stringParameter.find_last_of("/") + 1;
        int lastIndex = stringParameter.find_last_of(".");
        int nameSize = lastIndex - firstIndex;
        std::string modelName = stringParameter.substr(firstIndex, nameSize);

        // Creates .diredge file
        std::ofstream diredgeFile;
        std::string filePath = "../diredge_models/" + modelName + ".diredge";
        diredgeFile.open(filePath.c_str());

        // For storing data read from the file
        std::string line;

        // Print standard information to the .diredge file
        for(unsigned int i = 0; i < 8; i++){
            std::getline(model,line);
            diredgeFile << line << "\n";
        }

        // Loop over vertices and faces to extract them
        while(!model.eof()){
            // Check the first word to see if it is a vertex or face
            model >> line;

            // Make sure we are not at the end of the file
            if(model.eof()){
                break;
            }

            // Holds vertex/face numbers (so they can be skipped)
            int number;

            if(line == "Vertex"){
                // To hold the x,y,z values of the vertex
                vector3 vec;

                // Add vertex number to number integer, and x,y,z values to the vector3
                model >> number >> vec.x >> vec.y >> vec.z;

                vertices.push_back(vec);
            }

            if(line == "Face"){
                model >> number;
                for(unsigned int i = 0; i < 3; i++){
                    // To hold each face index
                    unsigned int a;

                    model >> a;

                    // Add face to the faceVertices vector
                    faceVertices.push_back(a);
                }
            }
        }

        // Compute directed edges
        for(int i = 0; i < faceVertices.size(); i++){
            // We know the first half of the edge but not the other half
            int firstHalf = i;
            int secondHalf = -1;

            // Get the face index values for the first half
            int firstHalfV1 = faceVertices[i];
            int firstHalfV2 = faceVertices[next(i)];

            // Go through the edges to find the other half
            for(int j = 0; j < faceVertices.size(); j++){
                if(faceVertices[j] == firstHalfV2 && faceVertices[next(j)] == firstHalfV1){
                    secondHalf = j;
                }
            }

            // Add first half to the other half array
            otherHalf.push_back(firstHalf);

            // Check if second half was found
            if(secondHalf != -1){
                // If it was found then add it to the array
                otherHalf.push_back(secondHalf);
            }else {
                // If it was not found then terminate the program
                std::cout << "Failed to find other half at edge: " << firstHalf << std::endl;
                return 0;
            }
        }

        // Find all the directed edges
        for(int i = 0; i < vertices.size(); i++){
            int directedEdge;
            // Find the first instance of an outgoing edge in the faces list
            for(int j = 0; j < faceVertices.size(); j++){
                if(i==faceVertices[j]){
                    directedEdge = j;
                    break;
                }
            }

            firstDirectedEdge.push_back(i);
            firstDirectedEdge.push_back(directedEdge);
        }

        // Check for less than or more than 2 faces at each edge
        for(int i = 0; i < faceVertices.size(); i++){
            int count = 0;

            // Get the two indices for edge A
            int edgeI = faceVertices[i];
            int edgeInext = faceVertices[next(i)];

            // Go through the edges and count how many times the edge A shows up
            for(int j = 0; j < faceVertices.size(); j++){
                // The edge we are comparing edge A to
                int edgeJ = faceVertices[j];
                int edgeJnext = faceVertices[next(j)];

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

        // Check each vertex for pinch points
        for(int i = 0; i < vertices.size(); i++){

            // becomes true when a single loop through the vertices faces has been complete
            bool backAtOrigin = false;

            // initial values for the first directed edge, the other half of that edge, and the next edge of that other half.
            int FDE = firstDirectedEdge[(i * 2)+1];
            int OH = otherHalf[(FDE * 2) + 1];
            int nxt = next(OH);

            // Counts how many faces have been traversed.
            int facesTraversed = 1;

            while(!backAtOrigin){

                // issue is here
                OH = otherHalf[(nxt*2)+1];
                nxt = next(OH);

                facesTraversed++;
                // If we are back at the original edge, then the loop is complete
                if(nxt == FDE || OH == FDE)
                    backAtOrigin = true;
            }

            // Find the number of faces the vertex is a part of
            int overallNumFaces = 0;
            for(int j = 0; j < faceVertices.size(); j+=3){
                if(faceVertices[j] == i || faceVertices[j+1] == i || faceVertices[j+2] == i ){
                        overallNumFaces++;
                }
            }

            // If the faces we have looped over are not the same as the number of faces that the vertex is touching, then its at a pinch point
            if(overallNumFaces != facesTraversed){
                std::cout << "This mesh is not manifold.\n";
                std::cout << "Pinch point at vertex: " << i << std::endl;
                manifold = false;
                break;
            }
        }

        // Print all vertices to the file
        for(long vertex = 0; vertex < vertices.size(); vertex++){
            diredgeFile << "Vertex " << vertex << " " << vertices[vertex].x << " " << vertices[vertex].y << " " << vertices[vertex].z << "\n";
        }

        // Print all first directed edge values to the file
        for(int i = 0; i < firstDirectedEdge.size(); i+=2){
            diredgeFile << "FirstDirectedEdge " << firstDirectedEdge[i] << " " << firstDirectedEdge[i+1] << std::endl;
        }

        // Print all faces to the file
        int index = 0;
        for (int face = 0; face < faceVertices.size(); face+=3){
            diredgeFile << "Face " << index << " " << faceVertices[face] << " " << faceVertices[face+1] << " " << faceVertices[face+2] << "\n";
            index++;
        }

        // Print other halves to the file
        for (int i = 0; i < otherHalf.size(); i+=2){
            diredgeFile << "OtherHalf " << otherHalf[i] << " " << otherHalf[i+1] << std::endl;
        }


        diredgeFile.close();

        // Calculate the genus if this mesh is a manifold
        if(manifold){
            // Num of vertices
            int v = vertices.size();
            // Num of faces
            int f = faceVertices.size() / 3;
            // Num of edges: 3f = 2e
            int e = (3*f) / 2;
            // Genus: v - e + f = 2 - 2g
            double g = (2 - f + e - v) / 2;

            std::cout << "This mesh is manifold.\n";
            std::cout << "Genus : " << g << std::endl;
        }

        std::cout << "|--------------Finished--------------|\n";
    }

    return 0;
}

int next(int edge){
    int face = edge / 3;
    int which = edge % 3;
    int nextWhich = (which+1) % 3;

    int edgeID = face * 3 + nextWhich;

    return edgeID;
}
