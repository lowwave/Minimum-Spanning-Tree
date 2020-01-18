#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>

#include "graph.h"

template<typename TimePoint>
std::chrono::milliseconds to_ms(TimePoint tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp);
}

unsigned long get_random_number_in_range(unsigned long start, unsigned long end) {
    static std::mt19937 mt{std::random_device{}()};
    std::uniform_int_distribution<unsigned long> dist(start, end);
    return dist(mt);
}

void print_help() {
    std::cout << "\n\n<Command> may be one of the following:\n" << std::endl
              << "--generate\t Generate random graph and save to graph_data.txt" << std::endl
              << "--single\t Run Borůvka's algorithm using single thread using data from graph_data.txt" << std::endl
              << "\t\t result will be saved to result.txt" << std::endl
              << "--parallel\t Run Borůvka's algorithm in multiple threads using data from graph_data.txt" << std::endl
              << "\t\t result will be saved to result.txt" << std::endl
              << "--compare\t Compares single and multi-threaded execution and prints the result"
              << "\n\nMade by antosand" << std::endl;
}

void print_sum_of_edge_weights(graph const &g) {
    unsigned long sum = 0;

    for (auto edge : g.edges()) {
        sum += edge.weight;
    }

    std::cout << "The sum of edge weights is " << sum << "." << std::endl;
}

void print_comparison(std::chrono::milliseconds single_thread_ms, std::chrono::milliseconds multi_thread_ms) {
    std::cout << std::setprecision(2);

    if (multi_thread_ms < single_thread_ms) {
        double coefficient = single_thread_ms.count() / (double) multi_thread_ms.count();
        std::cout << "\nThe multi-threaded simulation was approximately " << coefficient << "x faster!" << std::endl;
    } else {
        double coefficient = multi_thread_ms.count() / (double) single_thread_ms.count();
        std::cout << "\nThe single-threaded simulation was approximately " << coefficient << "x faster!" << std::endl;
    }
}

graph generate_random_graph(unsigned long vertex_count = get_random_number_in_range(300, 500),
                            unsigned long edge_count = get_random_number_in_range(4000, 6000)) {

    std::cout << "Generating a random graph..." << std::endl;
    std::cout << "Generated graph has " << vertex_count << " vertices." << std::endl;

    graph g{};

    for (unsigned long i = 0; i < vertex_count; i++)
        g.add_vertex(i);

    std::cout << "Generated graph has " << edge_count << " edges." << std::endl;

    for (unsigned long i = 0; i < edge_count; i++) {
        while (true) {
            auto edge_start{g.get_vertex_at(get_random_number_in_range(0, vertex_count - 1))};
            auto edge_end{g.get_vertex_at(get_random_number_in_range(0, vertex_count - 1))};

            auto e = edge(edge_start, edge_end, get_random_number_in_range(1, 100));

            if (!g.has_edge(e)) {
                g.add_edge(e);
                break;
            }
        }
    }

    std::cout << "Checking if the generated graph is connected..." << std::endl;
    if (g.is_connected()) {
        std::cout << "Random graph was generated!" << std::endl << std::endl;
        return g;
    } else {
        std::cout << "Randomly generated graph is not connected :c\nGenerating new graph." << std::endl;
        return generate_random_graph(vertex_count, edge_count);
    }
}

bool fexists(std::string const &filename) {
    std::ifstream ifile(filename.c_str());
    return (bool)ifile;
}

graph const load_from_file(std::string const &filename) {

    if (!fexists(filename)) {
        std::cout << "Looks like file with the data doesn't exist!\nYou should run --generate first!" << std::endl;
        throw;
    }

    std::cout << "Loading graph from " << filename << "." << std::endl;

    std::ifstream file;

    file.open(filename, std::ios::in);

    int vertex_count, edge_count;
    file >> vertex_count >> edge_count;

    graph g{};

    for (int i = 0; i < vertex_count; i++) g.add_vertex(i); // we already know what vertices are in a graph
    for (int i = 0; i < vertex_count; i++) {
        int start, vertex_edge_count; // start is basically the same as i
        file >> start >> vertex_edge_count;

        for (int j = 0; j < vertex_edge_count; j++) {
            int destination;
            unsigned long weight;
            file >> destination >> weight; // read edge

            if (!g.has_edge(edge(start, destination, weight))) g.add_edge(edge(start, destination, weight));
        }
    }

    file.close();

    std::cout << "Finished loading graph from file." << std::endl;

    return g;
}

void save_to_file(graph const &g, std::string const &filename) {
    auto vertex_count = g.vertices().size();
    auto edge_count = g.edges().size();
    auto edges_by_vertex = g.get_edges_by_vertex();


    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);

    file << vertex_count << " " << edge_count << std::endl;

    std::cout << "Saving the graph to " << filename << "." << std::endl;

    for (auto &entry : edges_by_vertex) {
        file << entry.first << " " << entry.second.size();

        for (auto &edge : entry.second) {
            file << " ";
            if (edge->start == entry.first) {
                file << edge->end;
            } else {
                file << edge->start;
            }
            file << ' ' << edge->weight;
        }
        file << std::endl;
    }

    file.close();

    std::cout << "Graph is saved to " << filename << "." << std::endl;
}

std::chrono::milliseconds run_single() {
    auto g = load_from_file("graph_data.txt");

    try {
        std::cout << "Starting the single-threaded MST calculation." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        auto mst = g.calculate_minimum_spanning_tree();
        auto end = std::chrono::high_resolution_clock::now();

        auto total = to_ms(end - start);

        std::cout << "The single-threaded MST calculation took " << total.count() << " ms." << std::endl;

        print_sum_of_edge_weights(mst);
        save_to_file(mst, "result.txt");

        return total;
    } catch (std::domain_error const &error) {
        std::cout << error.what();
    }

    return {};
}

std::chrono::milliseconds run_parallel() {
    auto g = load_from_file("graph_data.txt");

    try {
        std::cout << "Starting the multi-threaded MST calculation." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        auto mst = g.calculate_minimum_spanning_tree_parallel();
        auto end = std::chrono::high_resolution_clock::now();

        auto total = to_ms(end - start);

        std::cout << "The multi-threaded MST calculation took " << total.count() << " ms." << std::endl;

        print_sum_of_edge_weights(*mst);
        save_to_file(*mst, "result.txt");

        return total;
    } catch (std::domain_error const &error) {
        std::cout << error.what();
    }

    return {};
}

void compare() {
    auto single_thread_ms = run_single();

    std::cout << std::endl << "================================================" << std::endl << std::endl;

    auto multi_thread_ms = run_parallel();

    print_comparison(single_thread_ms, multi_thread_ms);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "\n\nUsage: ./main <command>\n\n";
        std::cout << "For more info: ./main --help\n\n";
        return 1;
    }

    auto arg = std::string(argv[1]);

    if (arg == "--help") {
        print_help();
        return 0;
    }

    if (arg == "--generate") {
        save_to_file(generate_random_graph(), "graph_data.txt");
        return 0;
    }

    if (arg == "--single") {
        run_single();
        return 0;
    }

    if (arg == "--parallel") {
        run_parallel();
        return 0;
    }

    if (arg == "--compare") {
        compare();
        return 0;
    }

    std::cout << "This command is unknown\nFor more info run --help" << std::endl;

    return 1;
}
