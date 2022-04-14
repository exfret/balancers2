// Computes the list of all flows possible with a certain number of splitters

#include <algorithm>
#include <set>
#include <vector>

#include "exists_balancer.h"
#include "types.h"
#include "network_tools.h"

using namespace std;

bool existsBalancer(int input_size, int output_size, int max_num_splitters) {
    set<Matrix> possible_networks;
    possible_networks.insert({{1}});
    
    // Note: I assume out1 and out2 aren't both looped back to inputs; check to see if this is valid later
    for (int i = 0; i < max_num_splitters; ++i) {
        set<Matrix> new_possible_networks;
        
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
    Row balanced_output;
    for (int i = 0; i < input_size; ++i) {
        balanced_output.push_back(1.0 / output_size);
    }
    Matrix balancer;
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
