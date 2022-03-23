//
//  main.cpp
//  beltbalancers3
//
//  Created by Kyle Hess on 2/23/22.
//
// Note: This could be made a lot better if these things were stored as some sort of matrix object

#include <iostream>
#include <set>
#include <vector>
using namespace std;

vector<vector<double>> transpose(vector<vector<double>> matrix) {
    vector<vector<double>> new_matrix;
    
    if (matrix.size() == 0) {
        return matrix;
    }
    
    for (int i = 0; i < matrix[0].size(); ++i) {
        vector<double> temp;
        
        for (int j = 0; j < matrix.size(); ++j) {
            temp.push_back(matrix[j][i]);
        }
        
        new_matrix.push_back(temp);
    }
    
    return new_matrix;
}

// network[i] is flow from inputs to i-th output
// splitter_inputs has an entry of -1 for a new input that's not already part of the network
// splitter_outputs has an entry of -1 for a new output
// There must be at least one output
// Network must have at least one input
vector<vector<double>> addSplitter(vector<vector<double>> network, vector<int> splitter_inputs, vector<int> splitter_outputs) {
    vector<double> splitter_flow;
    for (int i = 0; i < network[0].size(); ++i) {
        splitter_flow.push_back(0);
    }
    for (int i = 0; i < splitter_inputs.size(); ++i) {
        splitter_flow.push_back(1.0 / splitter_outputs.size());
    }
    
    // Find this splitter's output in terms of its input flows
    for (int i = 0; i < splitter_inputs.size(); ++i) {
        if (splitter_inputs[i] != -1) {
            for (int j = 0; j < network[splitter_inputs[i]].size(); ++j) {
                splitter_flow[j] += network[splitter_inputs[i]][j] / splitter_outputs.size();
            }
        }
    }
    
    // Remove circular dependencies of the new outputs on any inputs that they lead to
    vector<double> new_splitter_flow = splitter_flow;
    for (int i = 0; i < splitter_outputs.size(); ++i) {
        if (splitter_outputs[i] != -1) {
            for (int j = 0; j < splitter_flow.size(); ++j) {
                new_splitter_flow[j] *= 1 / (1 - splitter_flow[splitter_outputs[i]]);
            }
            
            new_splitter_flow[splitter_outputs[i]] = 0;
        }
    }
    splitter_flow = new_splitter_flow;
    
    // Remove other dependencies on the input (if any) that was just removed
    // Might be wonky when there are two self-loops added at once
    for (int i = 0; i < network.size(); ++i) {
        bool added_inputs = false;
        
        for (int j = 0; j < splitter_outputs.size(); ++j) {
            if (splitter_outputs[j] != -1) {
                for (int k = 0; k < network[i].size(); ++k) {
                    network[i][k] += network[i][splitter_outputs[j]] * splitter_flow[k];
                }
                // Add dependencies on inputs
                int old_network_size = network[i].size();
                for (int k = 0; k < splitter_inputs.size(); ++k) {
                    // splitter_flow[network[i].size() + l should be the same as just 1.0 / splitter_outputs.size()
                    network[i].push_back(network[i][splitter_outputs[j]] * splitter_flow[old_network_size + k]);
                    
                    added_inputs = true;
                }
                
                network[i][splitter_outputs[j]] = 0;
            }
        }
        
        if (!added_inputs) {
            for (int j = 0; j < splitter_inputs.size(); ++j) {
                network[i].push_back(0);
            }
        }
    }
    
    
    // Add new outputs
    for (int i = 0; i < splitter_outputs.size(); ++i) {
        network.push_back(splitter_flow);
    }
    
    // Update the inputs and outputs of this network
    /*for (int i = 0; i < network.size(); ++i) {
        // For each previous input node i depends on that is now an output of the splitter, add the splitter's flow
        for (int j = 0; j < network[i].size(); ++j) {
            // Check if j is an output node of the splitter
            bool isOutputNode = false;
            for (int k = 0; k < splitter_outputs.size(); ++k) {
                if (splitter_outputs[k] == j) {
                    isOutputNode = true;
                }
            }
            
            // If so, update the output to not depend on it any longer
            if (isOutputNode) {
                for (int k = 0; k < splitter_flow.size(); ++k) {
                    network[i][k] += network[i][j] * splitter_flow[k];
                }
            }
        }
    }*/
    
    // Now trim the new unused inputs/outputs; start from the back so that erasing can work properly
    int old_network_size = network.size();
    for (int i = network.size() - 1; i >= 0; --i) {
        // Erase this row if it's an output that is now used
        bool is_used = false;
        // Check if it's an output of the previous network that is now used in the splitter
        for (int j = 0; j < splitter_inputs.size(); ++j) {
            if (splitter_inputs[j] == i) {
                is_used = true;
            }
        }
        // Check if it's an output of the splitter that's used
        int output_index = splitter_outputs.size() + i - old_network_size;
        if (0 <= output_index && splitter_outputs[output_index] != -1) {
            is_used = true;
        }
        if (is_used) {
            network.erase(network.begin() + i);
            continue;
        }
        
        // Now trim columns (inputs)
        int old_row_size = network[i].size();
        for (int j = network[i].size() - 1; j >= 0; --j) {
            bool is_used = false;
            // Check if it's an input of the previous network that's now an output of the splitter
            for (int k = 0; k < splitter_outputs.size(); ++k) {
                if (splitter_outputs[k] == j) {
                    is_used = true;
                }
            }
            // Check if it's a used input of the splitter that's used
            int input_index = splitter_inputs.size() + j - old_row_size;
            if (0 <= input_index && splitter_inputs[input_index] != -1) {
                is_used = true;
            }
            if (is_used) {
                network[i].erase(network[i].begin() + j);
            }
        }
    }
    
    // Keep sorting rows and columns until nothing further happens (to transform into normal form)
    bool sorted = false;
    while (!sorted) {
        vector<vector<double>> old_network = network;
        
        sort(network.begin(), network.end());
        network = transpose(network);
        sort(network.begin(), network.end());
        network = transpose(network);
        
        if (network == old_network) {
            sorted = true;
        }
    }
    
    /*
    // Note: can't sort individually because we have a matrix, not a simple bunch of vectors
    // Sort the network by putting the biggest entry in the upper left, sorting that first column, then sort the first row
    double max_element = 0.0;
    int max_row_ind = 0;
    int max_col_ind = 0;
    for (int i = 0; i < network.size(); ++i) {
        for (int j = 0; j < network[i].size(); ++j) {
            if (network[i][j] > max_element) {
                max_element = network[i][j];
                max_row_ind = i;
                max_col_ind = j;
            }
        }
    }
    // Now permute so that the max element is in the upper left corner
    // Switch rows
    vector<double> first_row = network[0];
    vector<double> max_row = network[max_row_ind];
    network[0] = max_row;
    network[max_row_ind] = first_row;
    // Switch columns
    vector<double> first_col;
    for (int i = 0; i < network[0].size(); ++i) {
        first_col.push_back(network[0][i]);
    }
    vector<double> max_col;
    for (int i = 0; i < network[max_col_ind].size(); ++i) {
        max_col.push_back(network[max_col_ind][i]);
    }
    for (int i = 0; i < network[0].size(); ++i) {
        network[0][i] = max_col[i];
    }
    for (int i = 0; i < network[0].size(); ++i) {
        
    }*/
    
    return network;
}

bool existsBalancer(int input_size, int output_size, int max_num_splitters) {
    set<vector<vector<double>>> possible_networks;
    possible_networks.insert({{1}});
    
    // Note: I assume out1 and out2 aren't both looped back to inputs; check to see if this is valid later
    for (int i = 0; i < max_num_splitters; ++i) {
        set<vector<vector<double>>> new_possible_networks;
        
        // Expand on each possible network
        // Need to do this in a way so that there are no "infinite loops"
        for (auto it = possible_networks.begin(); it != possible_networks.end(); ++it) {
            vector<vector<vector<int>>> valid_configs;
            
            // Assume out2 is -1
            // Add case that in1 and in2 is -1
            valid_configs.push_back({{-1, -1}, {-1}});
            for (int out1 = -1; out1 < (int)(*it)[0].size(); ++out1) {
                valid_configs.push_back({{-1, -1}, {out1, -1}});
            }
            for (int in1 = -1; in1 < (int)it->size(); ++in1) {
                // Case where there is no in2
                valid_configs.push_back({{in1}, {-1}});
                for (int out1 = -1; out1 < (int)(*it)[0].size(); ++out1) {
                    valid_configs.push_back({{in1}, {out1, -1}});
                }
                
                for (int in2 = in1 + 1; in2 < (int)it->size(); ++in2) {
                    valid_configs.push_back({{in1, in2}, {-1}});
                    for (int out1 = -1; out1 < (int)(*it)[0].size(); ++out1) {
                        valid_configs.push_back({{in1, in2}, {out1, -1}});
                    }
                }
            }
            
            // Check that there are no completely circular dependencies that would be added
            for (int j = valid_configs.size() - 1; j >= 0; --j) {
                // If there's a new input, we're fine
                if (valid_configs[j][0][0] == -1) {
                    continue;
                }
                
                bool circular = true;
                for (int k = 0; k < valid_configs[j][0].size(); ++k) {
                    // Check if this input is non-circular
                    for (int l = 0; l < (*it)[0].size(); ++l) {
                        // Check that this isn't a new output of the splitter
                        bool true_input = true;
                        for (int m = 0; m < valid_configs[j][1].size(); ++m) {
                            if (l == valid_configs[j][1][m]) {
                                true_input = false;
                            }
                        }
                        
                        if (true_input == true && (*it)[valid_configs[j][0][k]][l] != 0) {
                            circular = false;
                        }
                    }
                }
                
                if (circular) {
                    valid_configs.erase(valid_configs.begin() + j);
                }
            }
            
            for (int j = 0; j < valid_configs.size(); ++j) {
                new_possible_networks.insert(addSplitter(*it, valid_configs[j][0], valid_configs[j][1]));
            }
        }
        
        for (auto it = new_possible_networks.begin(); it != new_possible_networks.end(); ++it) {
            possible_networks.insert(*it);
        }
    }
    
    // Check if it's a splitter
    vector<double> balanced_output;
    for (int i = 0; i < input_size; ++i) {
        balanced_output.push_back(1.0 / output_size);
    }
    vector<vector<double>> balancer;
    for (int i = 0; i < output_size; ++i) {
        balancer.push_back(balanced_output);
    }
    
    // it_num is for debugging purposes
    int it_num = 0;
    for (auto it = possible_networks.begin(); it != possible_networks.end(); ++it, ++it_num) {
        if (*it == balancer) {
            return true;
        }
    }
    return false;
}

int main() {
    //vector<vector<double>> my_network = {{0.1, 0.2, 0.3, 0.4}, {0.5, 0.6, 0.7, 0.8}};
    
    //vector<vector<double>> new_network = addSplitter(my_network, {0}, {1, -1});
    
    bool balancerExists = existsBalancer(5, 5, 6);
    
    cout << "\n";
}
