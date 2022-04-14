// Tools for the Network/Matrix domain

#pragma once
#include "network_tools.cpp"

// Generate a row of zeros
Row zeroRow(int size);

// Generate a row with a single 1 entry
Row oneRow(int size, int one_position);

// Add two rows together
Row rowAdd(Row new_row, Row input_node_row);

// Multiply a row by a double
Row rowMultiply(Row row, double multiplier);

// Generate an identity matrix matrix
Matrix identityMatrix(int size);

// Extract a column from a matrix
Row getColumn(Matrix matrix, int column_position);

// Transpose a matrix
Matrix transpose(Matrix matrix);

// Switch two rows of a matrix
Matrix switch_rows(Matrix matrix, int row1_ind, int row2_ind);

// Switch two columns of a matrix
Matrix switch_cols(Matrix matrix, int col1_ind, int col2_ind);

// Put a flow matrix into a "normal form" for easier comparison of flows
// Assumes the matrix has positive values (which is always the case for a flow matrix)
Matrix normalize(Matrix matrix);

// Generate an empty network
Network emptyNetwork(int size);

// Link two nodes in a network
void link(Network& nodes, int source, int target);

// Find a node's index in a network
int nodeNum(Network nodes, Node* node);
