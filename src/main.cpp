// Author: Luís Eduardo Rozante de Freitas Preita <luis.eduardo.rozante@usp.br>

# include <fstream>
# include <iostream>

# include <chrono>
# include <random>

# include <vector>
# include <algorithm>

# include <string>
# include <cstring>

# include <cstdint>

// How many times the program should try generating a random edge before giving up.
# define MAX_ITERATIONS 256

class Graph {

    public:

        Graph(uint32_t num_vertices) {

            // Alocates space for an adjacency matrix.
            this->matrix = new bool[num_vertices*num_vertices];

            // Initializes the matrix.
            for(uint32_t i = 0; i < num_vertices; i++)
                matrix[i] = false;

            // Initializes graph info.
            this->num_vertices = num_vertices;
            this->num_edges = 0;

        }

        // Adds a directed edge to the graph.
        void add_edge(uint32_t from, uint32_t to) {

            this->matrix[from + to * num_vertices] = true;
            this->num_edges++;

        }

        // Checks if a directed edge exists.
        bool has_edge(uint32_t from, uint32_t to) { return this->matrix[from + to * num_vertices]; }

        uint32_t get_num_vertices() { return this->num_vertices; }

        uint32_t get_num_edges() { return this->num_edges; }

        ~Graph() { delete[] this->matrix; }

    private:

        uint32_t num_vertices;
        uint32_t num_edges;
        bool *matrix;

};

int main(int argc, char *argv[]) {

    // Shows the correct usage if the number of parameters doesn't match.
    if(argc != 7) {
        std::cout << "Usage: ./$(name) $(num_vertices) $(min_graph_depth) $(additional_edges) $(has_cicles) $(shuffle) $(output_name)" << std::endl;
        return 0;
    }

    // Gets the number of vertices.
    uint32_t num_vertices = atoi(argv[1]);

    // Gets the max depth of the graph.
    uint32_t min_graph_depth = atoi(argv[2]);
    // Checks if the depth is valid.
    if(min_graph_depth > num_vertices) {
        std::cout << "$(min_graph_depth) must be lower than $(num_vertices)!" << std::endl;
        return 1;
    }

    // Gets the number of additional edges.
    uint32_t additional_edges = atoi(argv[3]);

    // Gets if the graph has cicles.
    bool has_cicles = strcmp(argv[4], "true") == 0;

    // If the graph has cycles, checks that additional_edges is valid.
    if(has_cicles && additional_edges < 1) {
        std::cout << "$(has_cicles) must be false if $(additional_edges) is less than 1!" << std::endl;
        return 1;
    }

    // Gets if the graph vertice numbers should be shuffled.
    bool shuffle = strcmp(argv[5], "true") == 0;

    // Creates the graph.
    Graph graph = Graph(num_vertices);

    // Adds edges until the graph has the desired depth.
    for(int i = 1; i < min_graph_depth; i++)
        graph.add_edge(i - 1, i);


    // Random number generator for the edges.
    uint32_t seed = std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::default_random_engine rng(seed);

    // Adds the additional edges.
    uint32_t current_additional_edges = 0; // Current number of added edges.
    uint8_t current_iterations = 0; // Number of iterations at trying to add an edge.
    while (current_additional_edges < additional_edges)
    {
        
        // Selects a random origin vertex.
        uint32_t rand_to = rng() % graph.get_num_vertices(); 

        // Destination vertex is more complicated:
        uint32_t rand_from;
        if(has_cicles) { // If there are cycles allow numbers that can generate them to be picked.

            // Ensures at least one cycle.
            if(current_additional_edges == 0) {


                // Clamps the value of rand_to to ensure a cycle is possible.
                rand_to = rand_to % min_graph_depth;

                // Selects a random destination vertex.
                rand_from = rng() % min_graph_depth;
                if(rand_from != rand_to && !graph.has_edge(rand_from, rand_to)) { // Adds the vertex, ensuring it doesn't already exists, and it isn't a self cycle.
                    graph.add_edge(rand_from, rand_to);
                    current_additional_edges++;
                    current_iterations = 0;
                    continue;
                }

            } else {

                // Selects a random destination vertex.
                rand_from = rng() % graph.get_num_vertices();
                if(rand_from != rand_to && !graph.has_edge(rand_from, rand_to)) { // Adds the vertex, ensuring it doesn't already exists, and it isn't a self cycle.
                    graph.add_edge(rand_from, rand_to);
                    current_additional_edges++;
                    current_iterations = 0;
                    continue;
                }

            }

        } else { // If there are no cycles, more care is needed. 

            // Selects a random destination vertex ensuring the origin is before the destination so no cycles are created.
            if(rand_to > 0) { // If rand_to is 0, this edge is already impossible.
                rand_from = rng() % rand_to;
                // Adds the vertex, ensuring it doesn't already exists.
                if(!graph.has_edge(rand_from, rand_to)) { 
                    graph.add_edge(rand_from, rand_to);
                    current_additional_edges++;
                    current_iterations = 0;
                    continue;
                }
            }

        }

        // If this fails, tries again, but if this keeps failing, maybe the parameters make the graph impossible (there probably is a way of predicting this, but this will do for now).
        // (also you could be extremely unlucky and it keeps generating only invalid edges).
        if(current_iterations > MAX_ITERATIONS) {
            std::cout << "ERROR: too many iterations trying to generate an edge! Are you sure a graph with your parameters is possible?" << std::endl;
            return 1;
        }
        current_iterations++;

    }
    

    // Outputs the graph on the correct format.
    std::ofstream out(argv[6]);
    std::cout.rdbuf(out.rdbuf());
    
    // Write the number of vertices and the number of edges to the file.
    std::cout << graph.get_num_vertices() << std::endl;
    std::cout << graph.get_num_edges() << std::endl;

    // Used to make it possible for the vertex indexes to be shuffled on the output.
    // (this is used because the way the graph is created it would be possible to partially order it topologically by using the indexes)
    std::vector<uint32_t> vertex_numbers;
    for(int i = 0; i < graph.get_num_vertices(); i++)
        vertex_numbers.push_back(i);

    // Shuffles the vector if necessary.
    if(shuffle)
         std::random_shuffle ( vertex_numbers.begin(), vertex_numbers.end());

    // Writes the graph edges to the exit (using the regular indexes or the shufled indexes).
    for(int i = 0; i < graph.get_num_vertices(); i++) {
        for(int j = 0; j < graph.get_num_vertices(); j++) {

            if(graph.has_edge(i, j))
                std::cout << std::to_string(vertex_numbers[i]) << " " << std::to_string(vertex_numbers[j]) << std::endl;

        }
    }

    return 0;

}