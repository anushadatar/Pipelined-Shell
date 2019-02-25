
#include <string.h>

#define INPUT_BUFF_SIZE 1000;
#define TOKEN_BUFF_SIZE 60;
#define TOKEN_SEPERATORS " \t\r\n\a"; // We can change these to be anything that seperatres the user input, ideally we could accpet quotes with spaces in them

int sh_execute(char **args){
  int status;
  pid_t pid, wpid; // ID variables

  pid = fork(); // makes a copy of the current process and returns the id
  // 0 for child, child id for parent
  if(pid == 0){
    if(execvp(args[0]), args == -1){
      perror("shell error");
      exit(EXIT_FAILUTRE);
    }
    //exit(EXIT_FAILUTRE);
  }else if(pid < 0){
    perror("shell error");
  }else{
    // pid = actual parent id
    do{
      wpid = waitpid(pid, &status, WUNTRACED); // waiting for the child process to execute
    }while(!WTFEXITED(status) && !WTFSIGNALED(status));
  }

  return status; // should be 1
}

char** sh_split_line(char *line){
  int position = 0;
  int buffSize = TOKEN_BUFF_SIZE;
  char **tokens = malloc(buffSize * sizeof(char*));
  char *token;

  if(!tokens){
    fprintf(stderr, "Allocation Error\n");
    exit(EXIT_FAILUTRE);
  }

  token = strtok(line, TOKEN_SEPERATORS); // splits the input line by the token seperatres and returns first one
  while(token != NULL){
    tokens[position] = token;
    position++;

    if(position >= buffSize){ //checking if we need more space
      buffSize += TOKEN_BUFF_SIZE;
      tokens = realloc(tokens, buffSize*sizeof(char*));
      if(!tokens){
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILUTRE);
      }
    }
    token = strtok(NULL, TOKEN_SEPERATORS)

  }

}

char* sh_read(){
  // IMPORTANT: could just use getline() from stdio.h instead of this function

  // taking in user input and reallocating space if more is needed
  int buffSize = INPUT_BUFF_SIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char)*buffSize);
  int c; // Since EOF is an int not a char

  if(!buffer){
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILUTRE);
  }

  while(1){
    c = getchar();

    //Checking for end of line character
    if(c == EOF || c == '\n'){
      buffer[position] = '\0';
      return buffer; // We have our command and are ready to execute it
    }else{
      buffer[position] = c; //keep getting characters
    }
    position++;

    //Checking if we need more space
    if(position >= buffSize){
      buffSize += INPUT_BUFF_SIZE;
      buffer = realloc(buffer, buffSize);
        if(!buffer){
          fprintf(stderr, "allocation error\n");
          exit(EXIT_FAILUTRE);
        }
    }
  }
}


void shell_lop(){
  //Read input from the standand input, parse, and execute

  char *line;
  char **args;
  int status;

  do{
    printf(">Shell> ");
    line = sh_read();
    args = sh_split_line(line);
    status = sh_execute(args);

    free(line);
    free(args);

  }while(status)

}

int main(int argc, char **argv){

  shell_lop();

  return 0;
}
