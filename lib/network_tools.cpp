// Tools for the Network/Matrix domain

#include "matrix.h"
#include "types.h"
#include "utils.h"

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

Matrix switch_rows(Matrix matrix, int row1_ind, int row2_ind) {
    Row row1 = matrix[row1_ind];
    Row row2 = matrix[row2_ind];
    
    matrix[row2_ind] = row1;
    matrix[row1_ind] = row2;
    
    return matrix;
}

Matrix switch_cols(Matrix matrix, int col1_ind, int col2_ind) {
    Row col1;
    for (int row = 0; row < matrix.size(); ++row) {
        col1.push_back(matrix[row][col1_ind]);
    }
    Row col2;
    for (int row = 0; row < matrix.size(); ++row) {
        col2.push_back(matrix[row][col2_ind]);
    }
    
    for (int row = 0; row < matrix.size(); ++row) {
        matrix[row][col1_ind] = col2[row];
    }
    for (int row = 0; row < matrix.size(); ++row) {
        matrix[row][col2_ind] = col1[row];
    }
    
    return matrix;
}

Matrix normalize(Matrix matrix) {
    if (matrix.size() == 0 || matrix[0].size() == 0) {
        return {};
    }
    
    for (int i = 0; i < min(matrix.size(), matrix[0].size()); ++i) {
        int max_row_ind = 0;
        int max_col_ind = 0;
        double max_val = 0;
        for (int row = i; row < matrix.size(); ++row) {
            for (int col = i; col < matrix[row].size(); ++col) {
                if (matrix[row][col] > max_val) {
                    max_row_ind = row;
                    max_col_ind = col;
                    max_val = matrix[row][col];
                }
            }
        }
        
        matrix = switch_rows(matrix, i, max_row_ind);
        matrix = switch_cols(matrix, i, max_col_ind);
    }
    
    return matrix;
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
