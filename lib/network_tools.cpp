// Tools for the Network/Matrix domain

#include "types.hpp"
#include "utils.hpp"
#include "network_tools.hpp"

Row zeroRow(int size) {
  Row zero_row;
  for (int i = 0; i < size; ++i) {
    zero_row.push_back(0);
  }
  return zero_row;
}

Row oneRow(int size, int one_position) {
  Row one_row = zeroRow(size);
  vectorGuard(one_row, one_position);
  one_row[one_position] = 1;
  return one_row;
}

Row rowAdd(Row A, Row B) {
  if (A.size() != B.size()) {
    throw "Row sizes mismatch";
  }

  Row C = zeroRow(A.size());
  for (int i = 0; i < A.size(); i++) {
    C[i] = A[i] + B[i];
  }
  return C;
}

Row rowMultiply(Row row, double multiplier) {
  for (int i = 0; i < row.size(); ++i) {
    row[i] *= multiplier;
  }
  return row;
}

Matrix identityMatrix(int size) {
  Matrix identity_matrix;
  for (int i = 0; i < size; ++i) {
    identity_matrix.push_back(oneRow(size, i));
  }
  return identity_matrix;
}

Row getColumn(Matrix matrix, int column_position) {
  vectorGuard(matrix[0], column_position);
  Row column;
  int m = matrix.size();
  for (int j = 0; j < m; j++) {
    column.push_back(matrix[j][column_position]);
  }
  return column;
}

Matrix transpose(Matrix matrix) {
    Matrix transpose_matrix;
    
    int m = matrix.size();

    if (m == 0) {
        return matrix;
    }
    
    int n = matrix[0].size();
    
    for (int i = 0; i < n; ++i) {
        Row column = getColumn(matrix, i);
        transpose_matrix.push_back(column);
    }
    
    return transpose_matrix;
}

Network emptyNetwork(int size) {
  Network nodes;
  for (int i = 0; i < size; ++i) {
    Node* node = new Node;
    nodes.push_back(node);
  }
  return nodes;
}

void link(Network& nodes, int source, int target) {
  vectorGuard(nodes, source);
  vectorGuard(nodes, target);
  Node* target_node = nodes[target];
  Node* source_node = nodes[source];
  source_node->outputs.push_back(target_node);
  target_node->inputs.push_back(source_node);
}

int nodeNum(Network nodes, Node* node) {
  for (int node_num = 0; node_num < nodes.size(); ++node_num) {
    if (node == nodes[node_num]) {
      return node_num;
    }
  }
  throw "Node not found";
}

using Wiring = vector<int>;
using Config = vector<Wiring>;
using Configs = vector<Config>;

Matrix addSplitterToFlow(Matrix flow, const Wiring splitter_inputs, const Wiring splitter_outputs) {
    const int num_flow_outputs = flow.size(); // Previously N
    const int num_flow_inputs = flow[0].size(); // Previously M
    
    const int num_splitter_outputs = splitter_outputs.size(); // Previously m
    const int num_splitter_inputs = splitter_inputs.size(); // Previously n

    Row splitter_flow = zeroRow(num_flow_inputs);
    for (int i = 0; i < num_splitter_inputs; ++i) {
        splitter_flow.push_back(1.0 / num_splitter_outputs);
    }
    
    // Find this splitter's output in terms of its input flows
    for (int i = 0; i < num_splitter_inputs; ++i) {
        if (splitter_inputs[i] != -1) {
            for (int j = 0; j < num_flow_inputs; ++j) {
                splitter_flow[j] += flow[splitter_inputs[i]][j] / num_splitter_outputs;
            }
        }
    }
    
    // Remove circular dependencies of the new outputs on any inputs that they lead to
    Row new_splitter_flow = splitter_flow;
    for (int i = 0; i < num_splitter_outputs; ++i) {
        if (splitter_outputs[i] != -1) {
            for (int j = 0; j < num_flow_inputs + num_splitter_inputs; ++j) {
                new_splitter_flow[j] *= 1 / (1 - splitter_flow[splitter_outputs[i]]);
                
                if (splitter_flow[splitter_outputs[i]] == 1) {
                    splitter_flow = {};
                }
            }
            
            new_splitter_flow[splitter_outputs[i]] = 0;
        }
    }
    splitter_flow = new_splitter_flow;
    
    // Remove other dependencies on the input (if any) that was just removed
    // Might be wonky when there are two self-loops added at once
    for (int i = 0; i < num_flow_outputs; ++i) {
        bool added_inputs = false;
        
        for (int j = 0; j < num_splitter_outputs; ++j) {
            if (splitter_outputs[j] != -1) {
                for (int k = 0; k < num_flow_inputs; ++k) {
                    flow[i][k] += flow[i][splitter_outputs[j]] * splitter_flow[k];
                }
                // Add dependencies on inputs
                for (int k = 0; k < num_splitter_inputs; ++k) {
                    flow[i].push_back(flow[i][splitter_outputs[j]] * splitter_flow[num_flow_inputs + k]);
                    
                    added_inputs = true;
                }
                
                flow[i][splitter_outputs[j]] = 0;
            }
        }
        
        if (!added_inputs) {
            for (int j = 0; j < num_splitter_inputs; ++j) {
                flow[i].push_back(0);
            }
        }
    }
    
    // Add new outputs
    for (int i = 0; i < num_splitter_outputs; ++i) {
        flow.push_back(splitter_flow);
    }
    
    // Now trim the new unused inputs/outputs; start from the back so that erasing can work properly
    for (int i = num_flow_outputs + num_splitter_outputs - 1; i >= 0; --i) {
        // Erase this row if it's an output that is now used
        bool is_used = false;
        // Check if it's an output of the previous network that is now used in the splitter
        for (int j = 0; j < num_splitter_inputs; ++j) {
            if (splitter_inputs[j] == i) {
                is_used = true;
            }
        }
        // Check if it's an output of the splitter that's used
        int output_index = i - num_flow_outputs;
        bool in_output_segment = output_index >= 0;
        bool output_wired = splitter_outputs[output_index] != -1;
        if (in_output_segment && output_wired) {
            is_used = true;
        }
        if (is_used) {
            flow.erase(flow.begin() + i);
            continue;
        }
        
        // Now trim columns (inputs)
        int old_row_size = flow[i].size();
        for (int j = old_row_size - 1; j >= 0; --j) {
            bool is_used = false;
            // Check if it's an input of the previous network that's now an output of the splitter
            for (int k = 0; k < num_splitter_outputs; ++k) {
                if (splitter_outputs[k] == j) {
                    is_used = true;
                }
            }
            // Check if it's a used input of the splitter that's used
            int input_index = num_splitter_inputs + j - old_row_size;
            bool in_input_segment = 0 <= input_index;
            bool input_wired = splitter_inputs[input_index] != -1;
            if (in_input_segment && input_wired) {
                is_used = true;
            }
            if (is_used) {
                flow[i].erase(flow[i].begin() + j);
            }
        }
    }
    
    // Keep sorting rows and columns until nothing further happens (to transform into normal form)
    bool sorted = false;
    while (!sorted) {
        Matrix old_flow = flow;
        
        sort(flow.begin(), flow.end());
        flow = transpose(flow);
        sort(flow.begin(), flow.end());
        flow = transpose(flow);
        
        if (flow == old_flow) {
            sorted = true;
        }
    }
    
    return flow;
}
