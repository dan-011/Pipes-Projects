// STUDY BUT DO NOT MODIFY THE CODE IN THIS FILE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <assert.h>
#include "maze.h"

#define IS_OPEN(fd) (fcntl((fd), F_GETFD) != -1)

int main(int argc, char* argv[])
{
   char buffer[MAX_PATH_LEN+21];   // extra space for treasure amount and a newline

   int is_source = IS_OPEN(0);  // only source cell is started with stdin & stdout open
#ifdef DEBUG
   assert(IS_OPEN(0)==IS_OPEN(1) );
#endif
   int num_pipes = 0;
   int has_pipe[4];

   for(direction dir=NORTH; dir<=WEST; dir++) {
      has_pipe[dir] = IS_OPEN( INPIPE_FD(dir) );
      num_pipes += has_pipe[dir];
   }

   // get input coming from stdin or any of the pipes, and, if first letter
   // is a valid direction send what follows to that outgoing pipe, appending
   // the amount of the treasure in the cell, if any
   while(1) {

      // Initialize the file descriptor set; must be done in each iteration since select changes it in place
      fd_set set;
      FD_ZERO(&set);

      // add stdin to the set for the source cell
      if( is_source )
         FD_SET(0, &set);

      // add to the set all file descriptors of open reading pipe ends
      for(direction dir=NORTH; dir<=WEST; dir++) {
         if( has_pipe[dir] )
            FD_SET(INPIPE_FD(dir), &set);
      }

      // wait for input on any of the file descriptors using select
      select(MAX_PIPE_FD+1, &set, NULL, NULL, NULL);

      // got some input, see where is coming from
      int incoming = -1;

      if( is_source && FD_ISSET(0, &set) )
         incoming = 0;

      for( direction dir=NORTH; dir<=WEST; dir++ ) {
         if( has_pipe[dir] && FD_ISSET(INPIPE_FD(dir), &set) )
            incoming = INPIPE_FD(dir);
      }

      // get incoming bytes
      if(incoming != -1) {
         ssize_t bytes_read = read(incoming, buffer, sizeof(buffer)-1);
         // add a newline if input got truncated or sender did not include one
         buffer[bytes_read] = '\n';
      }

      int outgoing = -1;
      int outmsg_start = -1;
      // if this is the source cell and input is coming from a pipe or no outgoing pipe
      // exists, send whole input to stdout
      if( is_source && (incoming>0 || num_pipes==0) ) {
         outgoing = 1;
         outmsg_start = 0;
      }
      else {
         // first character determines the direction
         int dir = -1;
         char ch = toupper(buffer[0]);
         if(ch == 'N') dir = NORTH;
         else if(ch == 'S') dir = SOUTH;
         else if(ch == 'E') dir = EAST;
         else if(ch == 'W') dir = WEST;

         if( dir>=0 && has_pipe[dir] ) {
            outgoing = OUTPIPE_FD(dir);
            outmsg_start = 1;  // first char is not sent out on the pipe
        }
      }

      if(outgoing != -1) {
#ifdef DEBUG
         assert(outmsg_start != -1);
#endif
         // find position of first newline
         int pos = 0;
         while(buffer[pos] != '\n')
            pos++;

         // if the cell has the treasure, append treasure amount to the message
         if(argc > 1) {
            char* treasure = argv[1];
            int len = strlen(treasure);
            for(int i=0; i<len; i++, pos++)
               buffer[pos] = treasure[i];
         }
         else if(outgoing != 1) {
            buffer[pos++] = '.';
         }
         buffer[pos++] = '\n';

#ifdef DEBUG
         assert(pos < sizeof(buffer));
#endif
         write(outgoing, &buffer[outmsg_start], pos - outmsg_start);
      }
   }

   return 0;  // should never get here; cell processes are terminated by maze_builder
}
