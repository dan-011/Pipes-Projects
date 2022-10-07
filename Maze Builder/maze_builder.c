#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>
#include "maze.h"
#define IN_CHILD(pid)((pid) == 0)
#include <stdlib.h>

/*
 * Read the info on each maze cell from "maze_file", complete the setup of
 * the pipes used by the parent process to send and receive data to/from
 * the standard input/output of cell "start_cell", setup pipes for communication
 * between connected cells, start a process executing the included "maze_cell"
 * program for each maze cell, passing "treasure_amount" as command line argument
 * to the cell process corresponding to cell "treasure_cell", and save the process
 * IDs of the maze_cell processes in the "pid" array
 */
int build_maze( FILE* maze_file,
                char* cell_executable,
                int num_cells,
                int start_cell,
                int treasure_cell,
                char* treasure_amount,
                int to_start[2],
                int from_start[2],
                pid_t* pid)
{
   FILE* f = maze_file;
   int** cells = (int**)calloc(num_cells, sizeof(int*));
   //int cells[num_cells][4];
   char line[128];
   char countdown = 0;
   while(!feof(f)){
      if(countdown == 0){
         int index;
         int N;
         int S;
         int E;
         int W;
         fscanf(f, "%d %d %d %d %d\n", &index, &N, &S, &E, &W);
         cells[index] = (int*)calloc(4, sizeof(int));
         cells[index][NORTH] = N;
         cells[index][SOUTH] = S;
         cells[index][EAST] = E;
         cells[index][WEST] = W;
      }
      else{
         fgets(line, 128, f);
         countdown--;
      }
   }
   
   int** input_pipes = (int**)calloc(num_cells, sizeof(int**));
   //int input_pipes[num_cells][4][2];
   int** output_pipes = (int**)calloc(num_cells, sizeof(int**));
   //int output_pipes[num_cells][4][2];
   for(int i = 0; i < num_cells; i++){
      input_pipes[i] = (int*)calloc(4, sizeof(int*));
      output_pipes[i] = (int*)calloc(4, sizeof(int*));
      for(int j = 0; j < 4; j++){
         input_pipes[i][j] = -1;
         output_pipes[i][j] = -1;
      }
   }
   for(int i = 0; i < num_cells; i++){ // for each cell
      for(int j = 0; j < 4; j++){ // for each direction in the cell
         if(cells[i][j] >= 0){ // if there is a connection
            if(input_pipes[i][j] == -1){ // if there is no pipe previously established
               int p[2];
               int check_in = pipe(p);
               if(check_in < 0){
                  printf("something went wrong\n");
                  return -1;
               }
               int comm_cell = cells[i][j];
               input_pipes[i][j] = p[0];
               output_pipes[comm_cell][OPPOSITE_DIR(j)] = p[1];           
            }
         }
      }
   }
   /*for(int i = 0; i < num_cells; i++){
      for(int j = 0; j < 4; j++){
         //printf("%d ", cells[i][j]);
         //printf("(%d, %d) ", input_pipes[i][j][0], input_pipes[i][j][1]);
         //printf("(%d, %d) ", output_pipes[i][j][0], output_pipes[i][j][1]);
      }
      printf("\n");
   }*/
   for(int i = 0; i < num_cells; i++){ // for each cell
      pid_t cpid = fork(); // fork
      if(cpid == 0){ // in the child
         for(int k = 0; k < num_cells; k++){ // for each pipe
            if(k == i){ // if it is a pipe for this cell
               for(int r = 0; r < 4; r++){ // close the writing side of the output pi
                  close(INPIPE_FD(r));
                  if(input_pipes[k][r] != -1){
                     dup2(input_pipes[k][r], INPIPE_FD(r));
                     close(input_pipes[k][r]);
                  }
                  close(OUTPIPE_FD(r));
                  if(output_pipes[k][r] != -1){
                     dup2(output_pipes[k][r], OUTPIPE_FD(r));
                     close(output_pipes[k][r]);
                  }
               }
            }
            else{ // if it is not a part of the cell, close it
                  for(int r = 0; r < 4; r++){
                        if(input_pipes[k][r] != -1){
                           close(input_pipes[k][r]);
                        }
                        if(output_pipes[k][r] != -1){
                           close(output_pipes[k][r]);
                        }
                     }
                  }
         }
         // if start cell, dup stdin and stdout with to start and from start, otherwise close stdin and stdout
         // close both ends of to_start and from_start
         if(i == start_cell){
            dup2(to_start[0], 0);
            dup2(from_start[1], 1);
         }
         else{
            close(0);
            close(1);
         }
         close(to_start[0]);
         close(to_start[1]);

         close(from_start[0]);
         close(from_start[1]);
         if(i == treasure_cell){
            execl(cell_executable, cell_executable, treasure_amount, NULL); // idk if this is correct
         }
         else{
            execl(cell_executable, cell_executable, NULL); // idk if this is correct
         }
      }
      else{ // in parent
         pid[i] = cpid;
      }
      // in parent, close each pipe if it exists, close one end of the to_start[0] and from_start[1] (the end that we don't need)
   }

   for(int i = 0; i < num_cells; i++){
      for(int dir = 0; dir < 4; dir++){
         if(input_pipes[i][dir] != -1){
            close(input_pipes[i][dir]);
            input_pipes[i][dir] = -1;
         }
         if(output_pipes[i][dir] != -1){
            close(output_pipes[i][dir]);
            output_pipes[i][dir] = -1;
         }
      }
   }

   for(int i = 0; i < num_cells; i++){
      free(cells[i]);
      free(input_pipes[i]);
      free(output_pipes[i]);
   }
   free(cells);
   free(input_pipes);
   free(output_pipes);
   close(to_start[0]);
   close(from_start[1]);
   return 0;
}


int main(int argc, char* argv[])
{
   // reserve the FDs used for communication between connected cells
   // before they get used for other purposes
   INPIPE_FD_RESERVE();
   OUTPIPE_FD_RESERVE();

   // open maze file
   FILE* maze_file;
   if(argc == 1) {
      fprintf(stderr, "Usage: %s <maze_file> [<cell_executable>]\n", argv[0]);
      return -1;
   }

   maze_file = fopen(argv[1], "r");
   if(maze_file == NULL) {
      fprintf(stderr, "could not open %s\n", argv[1]);
      return -1;
   }

   char* cell_executable = "./maze_cell";
   if(argc > 2)
      cell_executable = argv[2];

   // read info from first three lines of the maze file
   int num_cells;
   fscanf(maze_file, "size: %d\n", &num_cells);

   int start_cell;
   fscanf(maze_file, "start: %d\n", &start_cell);

   int  treasure_cell;
   char treasure_amount[20]; // enough to hold a long
   fscanf(maze_file, "treasure: %d %s\n", &treasure_cell, treasure_amount);


   pid_t pid[num_cells];  // array to hold process ids for cells

   // create pipes for communication with start cell
   int to_start[2];
   int from_start[2];
   if( pipe(to_start)!=0 || pipe(from_start)!=0 ) {
      fprintf(stderr, "failed to create pipes to/from start cell\n");
         return -1;
   }

   // setup pipes and start a child process for each maze cell
   int ret = build_maze( maze_file, cell_executable, num_cells, start_cell,
                         treasure_cell, treasure_amount, to_start, from_start, pid);
   if( ret != 0 ) {
      fprintf(stderr, "failed to build the maze\n");
      return -1;
   }


   // Send lines from stdin to the start cell using the to_start pipe,
   // get back replies using the from_start pipe, and print the replies to stdout
   char buffer[MAX_PATH_LEN+21];  // extra space for treasure amount and a newline

   while(1) {

      // TODO: Update the code of the loop to perform multiplexed input and handle invalid paths
      // select waits for input from its parameters
      // initialize the FD set, (done for each iteration)
      fd_set set; // make a file descriptor set
      FD_ZERO(&set); // make the set 0
      FD_SET(0, &set); // give it the file descriptor you want to add (stdin)
      FD_SET(from_start[0], &set); // give it the file descriptor of from_start
      // select
      ssize_t bytes_read;
      select(MAX_PIPE_FD, &set, NULL, NULL, NULL);
      if(FD_ISSET(0, &set)){ // if the set is the standard input
          bytes_read = read(0, buffer, MAX_PATH_LEN+1); // read from the standard input

          if(bytes_read == 0){
             break;
          }

          if(buffer[bytes_read-1] != '\n'){
             buffer[bytes_read] = '\n';
             bytes_read++;
          }

          write(to_start[1], buffer, bytes_read );  // write only bytes we got from fgets
      }
      else{
         // read bytes from from_start
         bytes_read = read(from_start[0], buffer, sizeof(buffer)-1 );
         // get input from stdin
         //if( fgets(buffer, sizeof(buffer), stdin) == NULL)
         //   break;   // stdin has ended, exit loop and finish execution

         // add NUL terminator for printf
         buffer[bytes_read] = '\0';
         printf("%s", buffer);
         fflush(stdout);
      }
   }

   // done with the maze
   // close remaining open files and pipe ends and terminate all cell processes
   fclose(maze_file);
   close(to_start[1]);
   close(from_start[0]);


   for(int i=0; i<num_cells; i++){
      kill(pid[i], SIGINT);
   }

   return 0;
}
