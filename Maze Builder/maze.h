// STUDY BUT DO NOT MODIFY THE CODE IN THIS FILE

/*
 * Enum type for directions
 */
typedef enum {
    NORTH, SOUTH, EAST, WEST
} direction;

// In the enum type NORTH is 0, SOUTH is 1, EAST is 2, and WEST is 3
// The opposite direction is obtained by flipping the least significant bit
#define OPPOSITE_DIR(d) ((d)^1)

// Fixed file descriptors for pipe ends used for communication between cells
// Both maze_builder and cell processes must use the INPIPE_FD and OUTPIPE_FD macros to ensure consistency
// The FDs must be "reserved" in maze_buider by calling the FD_RESERVE macro before opening files or creating pipes

// Use FDs 10-13 for reading pipe ends from N/S/E/W and 20-23 for writing pipe ends to N/S/E/W
#define INPIPE_FD(d)  ((d)+10)
#define OUTPIPE_FD(d) ((d)+20)
#define MAX_PIPE_FD 23

#define INPIPE_FD_RESERVE() {assert(dup2(0,10)!=-1); assert(dup2(0,11)!=-1); assert(dup2(0,12)!=-1); assert(dup2(0,13)!=-1);}
#define OUTPIPE_FD_RESERVE() {assert(dup2(1,20)!=-1); assert(dup2(1,21)!=-1); assert(dup2(1,22)!=-1); assert(dup2(1,23)!=-1);}


#define MAX_PATH_LEN 1000

