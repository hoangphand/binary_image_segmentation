#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <list>
using namespace std;

#define WHITE 0
#define BLACK 1

struct Vertex {
	int predecessor;
	int color;
};

int min(int a, int b) {
	if (a > b) {
		return b;
	} else {
		return a;
	}
}

void print_graph(vector< vector<int> > matrix) {
	for (int i = 0; i < matrix.size(); i++) {
		for (int j = 0; j < matrix.size(); j++) {
			cout<<matrix[i][j]<<" ";
		}
		cout<<"\n";
	}
}

void print_path(vector<int> path) {
	for (int i = 0; i < path.size(); i++) {
		cout<<path[i]<<" ";
	}
}

vector< vector<int> > init_flow_network(int size) {
	vector< vector<int> > residual_network;
	for (int i = 0; i < size; i++) {
		vector<int> row;
		for (int j = 0; j < size; j++) {
			row.push_back(0);
		}

		residual_network.push_back(row);
	}

	return residual_network;
}

vector<int> find_path_in_network(vector< vector<int> > network, int s, int t) {
	int size = network.size();
	vector<int> path;
	vector<Vertex> struct_path;

	for (int i = 0; i < size; i++) {
		Vertex new_vertex;
		new_vertex.color = WHITE;
		new_vertex.predecessor = -1;
		struct_path.push_back(new_vertex);
	}

	struct_path[s].color = BLACK;

	list<int> queue_of_vertices;
	queue_of_vertices.push_back(s);

	while (queue_of_vertices.size() != 0) {
		int current_vertex = queue_of_vertices.front();
		queue_of_vertices.pop_front();

		for (int i = 0; i < size; i++) {
			if (network[current_vertex][i] != 0 && struct_path[i].color == WHITE) {
				queue_of_vertices.push_back(i);
				struct_path[i].predecessor = current_vertex;
				struct_path[i].color = BLACK;
			}
		}
	}

	if (struct_path[t].predecessor != -1) {
		Vertex current_vertex = struct_path[t];
		path.push_back(t);
		while (current_vertex.predecessor != -1) {
			path.push_back(current_vertex.predecessor);
			current_vertex = struct_path[current_vertex.predecessor];
		}
	}

	return path;
}

int max_flow_FF(vector< vector<int> > network, int s, int t) {
	int size = network.size();
	vector< vector<int> > flow_network = init_flow_network(size);
	vector< vector<int> > residual_network(network);

	vector<int> path = find_path_in_network(network, s, t);

	while (path.size() != 0) {
		cout<<"path";
		print_path(path);
		cout<<endl;
		int augmenting_amount = INT_MAX;

		for (int i = path.size() - 1; i >= 1; i--) {
			augmenting_amount = min(augmenting_amount, residual_network[path[i]][path[i - 1]]);
		}

		for (int i = path.size() - 1; i >= 1; i--) {
			if (network[path[i]][path[i - 1]] > 0) {
				flow_network[path[i]][path[i - 1]] = flow_network[path[i]][path[i - 1]] + augmenting_amount;
				residual_network[path[i]][path[i - 1]] = network[path[i]][path[i - 1]] - flow_network[path[i]][path[i - 1]];
			} else {
				flow_network[path[i - 1]][path[i]] = flow_network[path[i - 1]][path[i]] - augmenting_amount;
				if (network[path[i - 1]][path[i]] > 0) {
					residual_network[path[i]][path[i - 1]] = flow_network[path[i - 1]][path[i]];
				}
			}
		}

		cout<<"flow_network"<<endl;
		print_graph(flow_network);
		cout<<"residual_network"<<endl;
		print_graph(residual_network);

		path = find_path_in_network(residual_network, s, t);
	}

	int max_flow = 0;
	for (int i = 0; i < size; i++) {
		max_flow += flow_network[s][i];
	}

	return max_flow;
}

int main(int argc, char const *argv[]) {
	std::ifstream fi("test_max_flow.txt");
	if (fi) {
		int size;
		fi>>size;

		vector< vector<int> > network;

		for (int i = 0; i < size; i++) {
			vector<int> row;
			for (int j = 0; j < size; j++) {
				int tmp_value;
				fi>>tmp_value;
				row.push_back(tmp_value);
			}

			network.push_back(row);
		}
		print_graph(network);
		cout<<max_flow_FF(network, 0, 5)<<endl;
		// print_path(find_path_in_network(network, 0, 4));
	}
	return 0;
}