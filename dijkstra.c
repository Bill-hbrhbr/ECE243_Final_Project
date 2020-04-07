#define DIJKSTRA_SIZE ((NUM_ROWS + 2) * (NUM_COLS + 2))
typedef struct node {
    int index;
    unsigned distance;
} Node;
Node priority_queue[DIJKSTRA_SIZE];
//bool processed[DIJKSTRA_SIZE];
int backtrack[DIJKSTRA_SIZE];
unsigned int best_distances[DIJKSTRA_SIZE];
bool vacant[DIJKSTRA_SIZE];
int queue_size;

// Find path between two tiles. Return true if found, false if not
bool find_path(int src_row, int src_col, int dest_row, int dest_col) {
    // Clear the processed array: set all to false
    //memset(processed, false, DIJKSTRA_SIZE * sizeof(bool));
    // Set the backtrack array to -1
    memset(backtrack, 0xFF, DIJKSTRA_SIZE * sizeof(int));
    // Set the best distances to INT_MAX
    memset(best_distances, 0xFF, DIJKSTRA_SIZE * sizeof(unsigned int));
    
    // Set the queue size to 0
    queue_size = 0;
    // Get the dijkstra ids of the src and dest positions
    int best_node_id = get_dijkstra_id(src_row, src_col);
    int dest_node_id = get_dijkstra_id(dest_row, dest_col);
    
    // The best distance so far is 0
    unsigned int best_dist = 0;
    best_distances[best_node_id] = 0;
    //processed[best_node_id] = true;
    
    while (true) {
        int left = best_node_id - 1, right = best_node_id + 1;
        int up = best_node_id - (NUM_COLS + 2), down = best_node_id + (NUM_COLS + 2);
        
        // Extend the best_node_id in all 4 directions
        // up or down
        if (up == dest_node_id || down == dest_node_id) {
            backtrack[dest_node_id] = best_node_id;
            return true;
        }
        pq_insert(up, best_node_id, best_dist + 1); // up
        pq_insert(down, best_node_id, best_dist + 1); // down
        
        // left
        if (best_node_id % (NUM_COLS + 2)) {
            if (left == dest_node_id) {
                backtrack[dest_node_id] = best_node_id;
                return true;
            }
            pq_insert(left, best_node_id, best_dist + 1);
        }
        
        // right
        if (right % (NUM_COLS + 2)) {
            if (right == dest_node_id) {
                backtrack[dest_node_id] = best_node_id;
                return true;
            }
            pq_insert(right, best_node_id, best_dist + 1);
        }
        
        // Check the best element in the priority queue
        while (queue_size > 0) {
            // If the node has already been processed, skip
            if (priority_queue[queue_size - 1].distance > best_distances[priority_queue[queue_size - 1].index]) {
                --queue_size;
            } else {
                break;
            }
        }
        
        // If empty, then there's no valid path
        if (queue_size == 0) {
            return false;
        }
        
        // Update the best node
        --queue_size;
        best_node_id = priority_queue[queue_size].index;
        best_dist = priority_queue[queue_size].distance;
    }
    return true;
}

// Return the dijkstra queue index given a row and a col
inline int get_dijkstra_id(int r, int c) {
    return (r + 1) * (NUM_COLS + 2) + c + 1;
}

// Insert a node into the priority queue
void pq_insert(int node_id, int src_id, unsigned int new_dist) {
    // Check if out of bounds
    if (node_id < 0 || node_id >= DIJKSTRA_SIZE) {
        return;
    }
    // Check if vacant
    if (!vacant[node_id]) {
        return;
    }
    // Check if the time is longer than already recorded
    if (best_distances[node_id] < new_dist) {
        return;
    }

    // Update the new best distance
    best_distances[node_id] = new_dist;
    // Update backtrack list
    backtrack[node_id] = src_id;
    
    // Push into the priority queue
    priority_queue[queue_size].index = node_id;
    priority_queue[queue_size].distance = new_dist;
    ++queue_size;
    
    // Correct the order using insertion sort
    for (int i = queue_size - 1; i > 0; --i) {
        if (priority_queue[i].distance > priority_queue[i - 1].distance) {
            Node temp = priority_queue[i];
            priority_queue[i] = priority_queue[i - 1];
            priority_queue[i - 1] = temp;
        } else {
            break;
        }
    }
}
