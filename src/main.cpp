#include <opencv2/opencv.hpp>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <stdlib.h>

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

    if (cosine_sim > 950) {
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

vector<int> find_augmenting_path_in_network(vector<MyNode> network, int s, int t, vector<Vertex> struct_path, vector< vector<int> > tmp_mat) {
    int size = network.size();
    vector<int> path;
    list<int> queue_of_vertices;
    queue_of_vertices.push_back(s);

    int count = 0;
    bool time_to_stop = false;

    while (queue_of_vertices.size() != 0 && !time_to_stop) {
        int current_vertex = queue_of_vertices.front();
        queue_of_vertices.pop_front();

        count++;

        for (int i = 0; i < network[current_vertex].edges.size(); i++) {
            if (network[current_vertex].edges[i].residual_capacity > 0 && \
                struct_path[network[current_vertex].edges[i].to].color == WHITE) {
                if (count == 1 || network[current_vertex].edges[i].to > network.size() - 2) {
                    queue_of_vertices.push_back(network[current_vertex].edges[i].to);
                    struct_path[network[current_vertex].edges[i].to].predecessor = current_vertex;
                    struct_path[network[current_vertex].edges[i].to].color = BLACK;
                } else {
                    int x = int(network[current_vertex].edges[i].to / tmp_mat[0].size());
                    int y = network[current_vertex].edges[i].to % tmp_mat[0].size();
                    if (tmp_mat[x][y] != 1) {
                        queue_of_vertices.push_back(network[current_vertex].edges[i].to);
                        struct_path[network[current_vertex].edges[i].to].predecessor = current_vertex;
                        struct_path[network[current_vertex].edges[i].to].color = BLACK;
                    }
                }
                if (network[current_vertex].edges[i].to == t) {
                    time_to_stop = true;
                    break;
                }
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

int max_flow_FF(Mat &in_image, vector<MyNode> &network, int s, int t, vector< vector<int> > tmp_mat) {
    Vec3b pixel;
    pixel[0] = 255;
    pixel[1] = 0;
    pixel[2] = 0;
    int size = network.size();
    vector<Vertex> struct_path;

    for (int i = 0; i < size; i++) {
        Vertex new_vertex;
        new_vertex.color = WHITE;
        new_vertex.predecessor = -1;
        struct_path.push_back(new_vertex);
    }

    struct_path[s].color = BLACK;

    vector<int> path = find_augmenting_path_in_network(network, s, t, struct_path, tmp_mat);
    int count = 0;
    while (path.size() != 0) {
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
            }
        }
        // for (int i = path.size() - 1; i >= 0; i--) {
        //     pixel[0] = 0;
        //     pixel[1] = 0;
        //     pixel[2] = 0;
        //     in_image.at<Vec3b>(int(path[i] / 640), path[i] % 640) = pixel;
        // }
        count++;
        // cout<<"count: "<<count<<endl;
        // cout<<"augmenting_amount: "<<augmenting_amount<<endl;

        for (int i = path.size() - 1; i >= 1; i--) {
            for (int j = 0; j < network[path[i]].edges.size(); j++) {
                if (network[path[i]].edges[j].to == path[i - 1]) {
                    network[path[i]].edges[j].flow = network[path[i]].edges[j].flow + augmenting_amount;

                    network[path[i]].edges[j].residual_capacity = \
                        network[path[i]].edges[j].capacity - network[path[i]].edges[j].flow;

                    for (int k = 0; k < network[path[i - 1]].edges.size(); k++) {
                        if (network[path[i - 1]].edges[k].to == path[i]) {

                            network[path[i - 1]].edges[k].residual_capacity = \
                                network[path[i - 1]].edges[k].capacity + network[path[i]].edges[j].flow;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        path = find_augmenting_path_in_network(network, s, t, struct_path, tmp_mat);
    }

    int max_flow = 0;
    for (int j = 0; j < network[s].edges.size(); j++) {
        if (network[s].edges[j].capacity > 0) {
            max_flow += network[s].edges[j].flow;
        }
    }

    return max_flow;
}

vector<int> init_tmp_mat_fore(Mat &in_image, vector< vector<int> > &tmp_mat, vector<int> initial_fore_set, int min_fore) {
    vector<int> border_pixels;
    int width = in_image.cols;
    int height = in_image.rows;
    list<int> queue_of_vertices;
    for (int i = 0; i < initial_fore_set.size(); i++) {
        queue_of_vertices.push_back(initial_fore_set[i]);
        int x = int(initial_fore_set[i] / width);
        int y = initial_fore_set[i] % width;
        cout<<"x: "<<x<<", y: "<<y<<endl;
        tmp_mat[x][y] = 1;
    }

    while (queue_of_vertices.size() != 0) {
        int current_vertex = queue_of_vertices.front();
        queue_of_vertices.pop_front();
        int x = int(current_vertex / width);
        int y = current_vertex % width;

        int current_top, current_bottom, current_left, current_right;

        if (x > 0) {
            current_top = in_image.at<uchar>(x - 1, y);
            double difference = (double)current_top / (double)min_fore;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x - 1][y] != 1) {
                queue_of_vertices.push_back((x - 1) * width + y);
                tmp_mat[x - 1][y] = 1;
            }
        }
        if (x < height - 1) {
            current_bottom = in_image.at<uchar>(x + 1, y);
            double difference = (double)current_bottom / (double)min_fore;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x + 1][y] != 1) {
                queue_of_vertices.push_back((x + 1) * width + y);
                tmp_mat[x + 1][y] = 1;
            }
        }
        if (y > 0) {
            current_left = in_image.at<uchar>(x, y - 1);
            double difference = (double)current_left / (double)min_fore;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x][y - 1] != 1) {
                queue_of_vertices.push_back(x * width + y - 1);
                tmp_mat[x][y - 1] = 1;
            }
        }
        if (y < width - 1) {
            current_right = in_image.at<uchar>(x, y + 1);
            double difference = (double)current_right / (double)min_fore;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x][y + 1] != 1) {
                queue_of_vertices.push_back(x * width + y + 1);
                tmp_mat[x][y + 1] = 1;
            }
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (tmp_mat[i][j] == 1) {
                if (i > 0 && tmp_mat[i - 1][j] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (i < height - 1 && tmp_mat[i + 1][j] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (j > 0 && tmp_mat[i][j - 1] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (j < width - 1 && tmp_mat[i][j + 1] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
            }
        }
    }

    return border_pixels;
}

vector<int> init_tmp_mat_back(Mat &in_image, vector< vector<int> > &tmp_mat, vector<int> initial_back_set, int min_back) {
    vector<int> border_pixels;
    int width = in_image.cols;
    int height = in_image.rows;
    list<int> queue_of_vertices;
    for (int i = 0; i < initial_back_set.size(); i++) {
        queue_of_vertices.push_back(initial_back_set[i]);
        int x = int(initial_back_set[i] / width);
        int y = initial_back_set[i] % width;
        cout<<"x: "<<x<<", y: "<<y<<endl;
        tmp_mat[x][y] = 2;
    }

    while (queue_of_vertices.size() != 0) {
        int current_vertex = queue_of_vertices.front();
        queue_of_vertices.pop_front();
        int x = int(current_vertex / width);
        int y = current_vertex % width;

        int current_top, current_bottom, current_left, current_right;

        if (x > 0) {
            current_top = in_image.at<uchar>(x - 1, y);
            double difference = (double)current_top / (double)min_back;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x - 1][y] != 1 && tmp_mat[x - 1][y] != 2) {
                queue_of_vertices.push_back((x - 1) * width + y);
                tmp_mat[x - 1][y] = 2;
            }
        }
        if (x < height - 1) {
            current_bottom = in_image.at<uchar>(x + 1, y);
            double difference = (double)current_bottom / (double)min_back;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x + 1][y] != 1 && tmp_mat[x + 1][y] != 2) {
                queue_of_vertices.push_back((x + 1) * width + y);
                tmp_mat[x + 1][y] = 2;
            }
        }
        if (y > 0) {
            current_left = in_image.at<uchar>(x, y - 1);
            double difference = (double)current_left / (double)min_back;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x][y - 1] != 1 && tmp_mat[x][y - 1] != 2) {
                queue_of_vertices.push_back(x * width + y - 1);
                tmp_mat[x][y - 1] = 2;
            }
        }
        if (y < width - 1) {
            current_right = in_image.at<uchar>(x, y + 1);
            double difference = (double)current_right / (double)min_back;
            if (0.95 < difference && difference < 1.05 && tmp_mat[x][y + 1] != 1 && tmp_mat[x][y + 1] != 2) {
                queue_of_vertices.push_back(x * width + y + 1);
                tmp_mat[x][y + 1] = 2;
            }
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (tmp_mat[i][j] == 2) {
                if (i > 0 && tmp_mat[i - 1][j] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (i < height - 1 && tmp_mat[i + 1][j] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (j > 0 && tmp_mat[i][j - 1] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
                if (j < width - 1 && tmp_mat[i][j + 1] == 0) {
                    border_pixels.push_back(i * width + j);
                    continue;
                }
            }
        }
    }

    return border_pixels;
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
    // in_image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
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
    cvtColor(in_image, test, CV_BGR2GRAY);

    
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

    int min_fore = INT_MAX;
    int min_back = INT_MAX;
    
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
            cout<<int(test.at<uchar>(y, x))<<endl;
            if (min_fore > test.at<uchar>(y, x)) {
                min_fore = test.at<uchar>(y, x);
            }
        } else {
            // back
            circle(out_image, Point(x, y), 4, Scalar(0, 0, 255), 1, 8 );
            initial_back_set.push_back(y * width + x);
            cout<<int(test.at<uchar>(y, x))<<endl;
            if (min_back > test.at<uchar>(y, x)) {
                min_back = test.at<uchar>(y, x);
            }
        }
    }

    vector< vector<int> > tmp_mat;

    for (int h = 0; h < height; h++) {
        vector<int> new_row;
        for (int w = 0; w < width; w++) {
            new_row.push_back(0);
        }
        tmp_mat.push_back(new_row);
    }

    vector<int> border_pixels_fore = init_tmp_mat_fore(test, tmp_mat, initial_fore_set, min_fore);
    vector<int> border_pixels_back = init_tmp_mat_back(test, tmp_mat, initial_back_set, min_back);

    // cout<<"border size fore: "<<border_pixels_fore.size()<<endl;
    // cout<<"border size back: "<<border_pixels_back.size()<<endl;

    // for (int i = 0; i < border_pixels_fore.size(); i++) {
    //     int x = int(border_pixels_fore[i] / width);
    //     int y = border_pixels_fore[i] % width;
    //     Vec3b pixel;
    //     pixel[0] = 0;
    //     pixel[1] = 0;
    //     pixel[2] = 255;
    //     in_image.at<Vec3b>(x, y) = pixel;
    // }
    // for (int i = 0; i < border_pixels_back.size(); i++) {
    //     int x = int(border_pixels_back[i] / width);
    //     int y = border_pixels_back[i] % width;
    //     Vec3b pixel;
    //     pixel[0] = 0;
    //     pixel[1] = 0;
    //     pixel[2] = 0;
    //     in_image.at<Vec3b>(x, y) = pixel;
    // }

    // initialize for adjacency list
    vector<MyNode> network;
    int count_same = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            MyNode new_node;
            new_node.label = i * width + j;
            int main_point = tmp_mat[i][j];

            // left
            if (j != 0) {
                int branch_point = tmp_mat[i][j - 1];
                if ((main_point == branch_point) || (main_point == 1 && branch_point != 1)) {
                    int similarity = abs(test.at<uchar>(i, j) - test.at<uchar>(i, j - 1));
                    double difference = (double) similarity / (double) test.at<uchar>(i, j);
                    if (0.05 >= difference) {
                        similarity = 2000;
                    } else {
                        similarity = 1;
                    }
                    Edge new_edge;
                    new_edge.from = i * width + j;
                    new_edge.to = i * width + j - 1;
                    new_edge.capacity = similarity;
                    new_edge.residual_capacity = similarity;
                    new_edge.flow = 0;
                    new_node.edges.push_back(new_edge);
                }
                // int similarity = cosine_similarity(in_image, i * width + j, i * width + j - 1);
            }

            // right
            if (j != width - 1) {
                int branch_point = tmp_mat[i][j + 1];
                if ((main_point == branch_point) || (main_point == 1 && branch_point != 1)) {
                    int similarity = abs(test.at<uchar>(i, j) - test.at<uchar>(i, j + 1));
                    double difference = (double) similarity / (double) test.at<uchar>(i, j);
                    if (0.05 >= difference) {
                        similarity = 2000;
                    } else {
                        similarity = 1;
                    }
                    Edge new_edge;
                    new_edge.from = i * width + j;
                    new_edge.to = i * width + j + 1;
                    new_edge.capacity = similarity;
                    new_edge.residual_capacity = similarity;
                    new_edge.flow = 0;
                    new_node.edges.push_back(new_edge);
                }
                // int similarity = cosine_similarity(in_image, i * width + j, i * width + j + 1);
            }

            // top
            if (i != 0) {
                int branch_point = tmp_mat[i - 1][j];
                if ((main_point == branch_point) || (main_point == 1 && branch_point != 1)) {
                    int similarity = abs(test.at<uchar>(i, j) - test.at<uchar>(i - 1, j));
                    double difference = (double) similarity / (double) test.at<uchar>(i, j);
                    if (0.05 >= difference) {
                        similarity = 2000;
                    } else {
                        similarity = 1;
                    }
                    Edge new_edge;
                    new_edge.from = i * width + j;
                    new_edge.to = (i - 1) * width + j;
                    new_edge.capacity = similarity;
                    new_edge.residual_capacity = similarity;
                    new_edge.flow = 0;
                    new_node.edges.push_back(new_edge);
                }
                // int similarity = cosine_similarity(in_image, i * width + j, (i - 1) * width + j);
            }

            // bottom
            if (i != height - 1) {
                int branch_point = tmp_mat[i + 1][j];
                if ((main_point == branch_point) || (main_point == 1 && branch_point != 1)) {
                    int similarity = abs(test.at<uchar>(i, j) - test.at<uchar>(i + 1, j));
                    double difference = (double) similarity / (double) test.at<uchar>(i, j);
                    if (0.05 >= difference) {
                        similarity = 2000;
                    } else {
                        similarity = 1;
                    }
                    Edge new_edge;
                    new_edge.from = i * width + j;
                    new_edge.to = (i + 1) * width + j;
                    new_edge.capacity = similarity;
                    new_edge.residual_capacity = similarity;
                    new_edge.flow = 0;
                    new_node.edges.push_back(new_edge);
                }
                // int similarity = cosine_similarity(in_image, i * width + j, (i + 1) * width + j);
            }

            network.push_back(new_node);
        }
    }

    MyNode virtual_source;
    virtual_source.label = network.size();
    for (int i = 0; i < border_pixels_fore.size(); i++) {
        Edge new_edge;
        new_edge.from = network.size();
        new_edge.to = border_pixels_fore[i];
        new_edge.capacity = INT_MAX / 2;
        new_edge.residual_capacity = INT_MAX / 2;
        new_edge.flow = 0;
        virtual_source.edges.push_back(new_edge);
    }
    network.push_back(virtual_source);

    // for (int i = 0; i < initial_fore_set.size(); i++) {
    //     for (int j = 0; j < network[initial_fore_set[i]].edges.size(); j++) {
    //         network[initial_fore_set[i]].edges[j].capacity = INT_MAX;
    //         network[initial_fore_set[i]].edges[j].residual_capacity = INT_MAX;
    //     }
    // }

    MyNode virtual_sink;
    virtual_sink.label = network.size();
    for (int i = 0; i < border_pixels_back.size(); i++) {
        Edge new_edge;
        new_edge.from = border_pixels_back[i];
        new_edge.to = network.size();
        new_edge.capacity = INT_MAX / 2;
        new_edge.residual_capacity = INT_MAX / 2;
        new_edge.flow = 0;
        network[border_pixels_back[i]].edges.push_back(new_edge);
    }
    network.push_back(virtual_sink);

    cout<<max_flow_FF(out_image, network, network.size() - 2, network.size() - 1, tmp_mat)<<endl;
    vector<int> vertices_belong_to_source = find_min_cut(network, network.size() - 2);
    cout<<"size: "<<vertices_belong_to_source.size()<<endl;
    // // cout<<"count_same: "<<count_same<<endl;

    for (int i = 0; i < network.size() - 2; i++) {
        int x = int(i / width);
        int y = i % width;
        Vec3b pixel;
        pixel[0] = 255;
        pixel[1] = 0;
        pixel[2] = 0;
        out_image.at<Vec3b>(x, y) = pixel;
    }

    for (int i = 0; i < vertices_belong_to_source.size(); i++) {
        int x = int(vertices_belong_to_source[i] / width);
        int y = vertices_belong_to_source[i] % width;
        Vec3b pixel;
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 255;
        out_image.at<Vec3b>(x, y) = pixel;
    }
    
    // // // write it on disk
    imwrite( argv[3], out_image);
    
    // also display them both
    
    namedWindow( "Original image", WINDOW_AUTOSIZE );
    namedWindow( "Show Marked image", WINDOW_AUTOSIZE );
    imshow( "Original image", in_image );
    imshow( "Show Marked image", out_image );
    // imshow( "Show Marked Pixels", test );
    waitKey(0);
    return 0;
}
