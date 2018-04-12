#include <opencv2/opencv.hpp>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#define WHITE 0
#define BLACK 1

using namespace cv;
using namespace std;

struct Vertex {
    int predecessor;
    int color;
};

struct Edge {
    int from;
    int to;
    int capacity;
    int flow;
    int residual_capacity;
};

struct MyNode {
    vector<Edge> edges;
    int label;
};

int cosine_similarity(Mat image, int pixel_a, int pixel_b) {
    int width = image.cols;
    int height = image.rows;

    int x_a, y_a, x_b, y_b;

    x_a = pixel_a % width;
    y_a = int(pixel_a / width);
    Vec3b a = image.at<Vec3b>(y_a, x_a);
    // Vec3b a = image.at<Vec3b>(x_a, y_a);

    x_b = pixel_b % width;
    y_b = int(pixel_b / width);
    Vec3b b = image.at<Vec3b>(y_b, x_b);
    // Vec3b b = image.at<Vec3b>(x_b, y_b);

    int cosine_sim = int((a[0] * b[0] + a[1] * b[1] + a[2] * b[2]) / \
        (sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2)) * \
        sqrt(pow(b[0], 2) + pow(b[1], 2) + pow(b[2], 2))) * 1000);

    if (cosine_sim > 995) {
        return 2000;
    } else {
        return 1;
    }
}

void print_path(vector<int> path) {
    for (int i = 0; i < path.size(); i++) {
        cout<<path[i]<<" ";
    }
}

vector<int> find_min_cut(vector<MyNode> network, int s) {
    vector<int> vertices_of_source_side;
    int size = network.size();
    vector<Vertex> list_of_vertices;

    for (int i = 0; i < size; i++) {
        Vertex new_vertex;
        new_vertex.color = WHITE;
        new_vertex.predecessor = -1;
        list_of_vertices.push_back(new_vertex);
    }

    list_of_vertices[s].color = BLACK;
    list<int> queue_of_vertices;
    queue_of_vertices.push_back(s);

    while (queue_of_vertices.size() != 0) {
        int current_vertex = queue_of_vertices.front();
        queue_of_vertices.pop_front();

        for (int i = 0; i < network[current_vertex].edges.size(); i++) {
            if (network[current_vertex].edges[i].residual_capacity > 0 && \
                list_of_vertices[network[current_vertex].edges[i].to].color == WHITE) {
                queue_of_vertices.push_back(network[current_vertex].edges[i].to);
                list_of_vertices[network[current_vertex].edges[i].to].predecessor = current_vertex;
                list_of_vertices[network[current_vertex].edges[i].to].color = BLACK;
            }
        }
    }

    for (int i = 0; i < size; i++) {
        if (list_of_vertices[i].color == BLACK) {
            vertices_of_source_side.push_back(i);
        }
    }

    return vertices_of_source_side;
}

vector<int> find_augmenting_path_in_network(vector<MyNode> network, int s, int t) {
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

        for (int i = 0; i < network[current_vertex].edges.size(); i++) {
            if (network[current_vertex].edges[i].residual_capacity > 0 && \
                struct_path[network[current_vertex].edges[i].to].color == WHITE) {
                queue_of_vertices.push_back(network[current_vertex].edges[i].to);
                struct_path[network[current_vertex].edges[i].to].predecessor = current_vertex;
                struct_path[network[current_vertex].edges[i].to].color = BLACK;
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

int max_flow_FF(Mat &in_image, vector<MyNode> &network, int s, int t) {
// int max_flow_FF(vector<MyNode> &network, int s, int t) {
    Vec3b pixel;
    pixel[0] = 255;
    pixel[1] = 0;
    pixel[2] = 0;
    // line(in_image, Point(s % 400, int(s / 640)), Point(t % 400, int(t / 640)), Scalar( 110, 220, 0 ),  2, 8 );
    int size = network.size();

    vector<int> path = find_augmenting_path_in_network(network, s, t);
    // for (int i = path.size() - 1; i >= 1; i--) {
    //     if (path[i] != s && path[i] != t) {
    //         pixel[0] = 0;
    //         pixel[1] = 0;
    //         pixel[2] = 0;
    //         in_image.at<Vec3b>(int(path[i] / 640), path[i] % 640) = pixel;
    //     }
    // }
    int count = 0;
    while (path.size() != 0) {
        // print_path(path);
        int augmenting_amount = INT_MAX;

        for (int i = path.size() - 1; i >= 1; i--) {
            int residual_capacity_of_edge;
            for (int j = 0; j < network[path[i]].edges.size(); j++) {
                if (network[path[i]].edges[j].to == path[i - 1]) {
                    residual_capacity_of_edge = network[path[i]].edges[j].residual_capacity;
                    break;
                }
            }
            if (augmenting_amount > residual_capacity_of_edge) {
                augmenting_amount = residual_capacity_of_edge;
                if (path[i] != s && path[i] != t) {
                    pixel[0] = 0;
                    pixel[1] = 0;
                    pixel[2] = 0;
                    in_image.at<Vec3b>(int(path[i] / 640), path[i] % 640) = pixel;
                }
            }
        }
        count++;
        cout<<"count: "<<count<<endl;
        cout<<"augmenting_amount: "<<augmenting_amount<<endl;

        for (int i = path.size() - 1; i >= 1; i--) {
            for (int j = 0; j < network[path[i]].edges.size(); j++) {
                if (network[path[i]].edges[j].to == path[i - 1]) {
                    network[path[i]].edges[j].flow = network[path[i]].edges[j].flow + augmenting_amount;

                    network[path[i]].edges[j].residual_capacity = \
                        network[path[i]].edges[j].capacity - network[path[i]].edges[j].flow;

                    for (int k = 0; k < network[path[i - 1]].edges.size(); k++) {
                        if (network[path[i - 1]].edges[k].to == path[i]) {
                            // network[path[i - 1]].edges[k].flow = \
                            //     network[path[i - 1]].edges[k].flow - augmenting_amount;

                            network[path[i - 1]].edges[k].residual_capacity = \
                                network[path[i - 1]].edges[k].capacity + network[path[i]].edges[j].flow;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        path = find_augmenting_path_in_network(network, s, t);
        cout<<"path: "<<path.size()<<endl;
    }

    int max_flow = 0;
    for (int j = 0; j < network[s].edges.size(); j++) {
        if (network[s].edges[j].capacity > 0) {
            max_flow += network[s].edges[j].flow;
        }
    }

    return max_flow;
}

int main( int argc, char** argv )
{
    if(argc!=4){
        cout<<"Usage: ../seg input_image initialization_file output_mask"<<endl;
        return -1;
    }
    
    // Load the input image
    // the image should be a 3 channel image by default but we will double check that in teh seam_carving
    Mat in_image;
    in_image = imread(argv[1]/*, CV_LOAD_IMAGE_COLOR*/);
   
    if(!in_image.data)
    {
        cout<<"Could not load input image!!!"<<endl;
        return -1;
    }

    if(in_image.channels()!=3){
        cout<<"Image does not have 3 channels!!! "<<in_image.depth()<<endl;
        return -1;
    }
    
    // the output image
    Mat out_image = in_image.clone();
    Mat test = in_image.clone();

    
    ifstream f(argv[2]);
    if(!f){
        cout<<"Could not load initial mask file!!!"<<endl;
        return -1;
    }
    
    int width = in_image.cols;
    int height = in_image.rows;
    
    int n;
    f>>n;

    vector<int> initial_fore_set;
    vector<int> initial_back_set;
    
    // get the initil pixels
    for (int i = 0; i < n; i++) {
        int x, y, t;
        f>>x;
        f>>y;
        f>>t;

        if (t == 1) {
            // fore
            // circle(out_image, Point(x, y), 4, Scalar(255, 0, 0), 1, 8 );
            initial_fore_set.push_back(y * width + x);
        } else {
            // back
            // circle(out_image, Point(x, y), 4, Scalar(0, 0, 255), 1, 8 );
            initial_back_set.push_back(y * width + x);
        }
    }

    // initialize for adjacency list
    vector<MyNode> network;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            MyNode new_node;
            new_node.label = i * width + j;

            // left
            if (j != 0) {
                int similarity = cosine_similarity(in_image, i * width + j, i * width + j - 1);
                Edge new_edge;
                new_edge.from = i * width + j;
                new_edge.to = i * width + j - 1;
                new_edge.capacity = similarity;
                new_edge.residual_capacity = similarity;
                new_edge.flow = 0;
                new_node.edges.push_back(new_edge);
            }

            // right
            if (j != width - 1) {
                int similarity = cosine_similarity(in_image, i * width + j, i * width + j + 1);
                Edge new_edge;
                new_edge.from = i * width + j;
                new_edge.to = i * width + j + 1;
                new_edge.capacity = similarity;
                new_edge.residual_capacity = similarity;
                new_edge.flow = 0;
                new_node.edges.push_back(new_edge);
            }

            // top
            if (i != 0) {
                int similarity = cosine_similarity(in_image, i * width + j, (i - 1) * width + j);
                Edge new_edge;
                new_edge.from = i * width + j;
                new_edge.to = (i - 1) * width + j;
                new_edge.capacity = similarity;
                new_edge.residual_capacity = similarity;
                new_edge.flow = 0;
                new_node.edges.push_back(new_edge);
            }

            // bottom
            if (i != height - 1) {
                int similarity = cosine_similarity(in_image, i * width + j, (i + 1) * width + j);
                Edge new_edge;
                new_edge.from = i * width + j;
                new_edge.to = (i + 1) * width + j;
                new_edge.capacity = similarity;
                new_edge.residual_capacity = similarity;
                new_edge.flow = 0;
                new_node.edges.push_back(new_edge);
            }

            network.push_back(new_node);
        }
    }

    MyNode virtual_source;
    virtual_source.label = network.size();
    for (int i = 0; i < initial_fore_set.size(); i++) {
        Edge new_edge;
        new_edge.from = network.size();
        new_edge.to = initial_fore_set[i];
        new_edge.capacity = INT_MAX;
        new_edge.residual_capacity = INT_MAX;
        new_edge.flow = 0;
        virtual_source.edges.push_back(new_edge);
    }
    network.push_back(virtual_source);

    for (int i = 0; i < initial_fore_set.size(); i++) {
        for (int j = 0; j < network[initial_fore_set[i]].edges.size(); j++) {
            network[initial_fore_set[i]].edges[j].capacity = INT_MAX;
            network[initial_fore_set[i]].edges[j].residual_capacity = INT_MAX;
        }
    }

    MyNode virtual_sink;
    virtual_sink.label = network.size();
    for (int i = 0; i < initial_back_set.size(); i++) {
        Edge new_edge;
        new_edge.from = initial_back_set[i];
        new_edge.to = network.size();
        new_edge.capacity = INT_MAX;
        new_edge.residual_capacity = INT_MAX;
        new_edge.flow = 0;
        network[initial_back_set[i]].edges.push_back(new_edge);
    }

    network.push_back(virtual_sink);

    cout<<max_flow_FF(out_image, network, network.size() - 2, network.size() - 1)<<endl;
    vector<int> vertices_belong_to_source = find_min_cut(network, initial_fore_set[0]);
    cout<<"size: "<<vertices_belong_to_source.size()<<endl;

    for (int i = 0; i < network.size() - 2; i++) {
        int x = int(i / 640);
        int y = i % 640;
        Vec3b pixel;
        pixel[0] = 255;
        pixel[1] = 0;
        pixel[2] = 0;
        out_image.at<Vec3b>(x, y) = pixel;
    }

    for (int i = 0; i < vertices_belong_to_source.size(); i++) {
        int x = int(vertices_belong_to_source[i] / 640);
        int y = vertices_belong_to_source[i] % 640;
        Vec3b pixel;
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 255;
        out_image.at<Vec3b>(x, y) = pixel;
    }
    
    // write it on disk
    imwrite( argv[3], out_image);
    
    // also display them both
    
    namedWindow( "Original image", WINDOW_AUTOSIZE );
    namedWindow( "Show Marked Pixels", WINDOW_AUTOSIZE );
    imshow( "Original image", in_image );
    imshow( "Show Marked Pixels", out_image );
    waitKey(0);
    return 0;
}
