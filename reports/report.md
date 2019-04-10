# Think GridWorld : A Shell 
## Allison Basore, Anusha Datar, and Prava Dhulipalla
# Project Vision
The goal of our project was to develop a functional UNIX shell in C.
The minimal viable product to achieve was to:
Create a shell that could run basic commands (ls, cd, rm, cp, mv, mkdir, man).
Have an understanding of how a UNIX shell works.
We also had stretch goals to pursue, such as to:
Implement more fun linux commands, like cal and sl.
Have support for piping, redirection, and globbing.
Include exciting easter eggs associated with GridWorld and other interests. 
Create a basic text editor that works in the shell (like Vim).
# Learning Goals
We all shared similar learning goals for this project. 
Overall, we wanted to learn more about operating system level design and practice our C programming in general. 
More specifically, we wanted to explore the intersections and relationships among different layers of abstraction associated with the UNIX environment and practice working on programs that span across those layers. And, of course, we wanted to understand more about what a UNIX shell implementation entails.
# Results
Over the course of this project we were able to develop a functional UNIX shell that follows most user commands (including ls, cd, mkdir, man, and nano), implement piping, redirection, and globbing, and create some easter eggs, including a LISP interpreter and a text editor.
Our main source code file invokes the GridWorldShell prompt. When the user types a command, our parsing functions process the input and the execution functions carry out the command instructions. After the command has been executed, the user can enter another input and that command will be executed.
 
Below is an example of a command and output:

Running ls, would list all the files and directories in the current file path.
Our main file is GridWorldShell.c, which contains the prompt, parsing, and execution functions.
We implemented the ability to pipe and redirect output. For piping, we first had to determine if piping was necessary (i.e. if command words were separated by a “|”). From there, we could pipe the output of the left file to the right by using the built-in `pipe` function in C. It receives an array of the output and input location - which we set beforehand to whatever the input specified. Afterwards we reset back to the standard input and output. Redirectioning is a very similar process, except it uses the `fork` command in C instead. This will redirect the output to a file (specified by the input). Overall implementing piping and redirection was fairly trivial with the usage of the built-in C commands.
Globbing is the ability to expand a non-specific file name (a wildcard pattern) into all the associated existing file names that exist on that computer, network, server, etc. 
There are several standard wildcards used by most commands, but ours implemented three of them:
? (question mark): This represents any single character. For instance, “a?” could expand into “aa”, “ab”, “ac”, “a0”, “a1”, “a2”, etc.
* (asterisk): This represents any number of any characters. For instance, “ab*” could expand into “ab”, “aba”, “abc”, “abac”, “abdac”, etc. Essentially, just anything that starts with the letters “ab”. The example “a*b” could expand into “ab”, “acb”, “aab”, “adacb”, etc. Essentially, just anything that starts with the letter “a” and ends with the letter “b”.
. (period): This does the same thing as the question mark.
The wildcard characters are first converted into regex expressions. Each expression begins with a ‘^’ and ends with a ‘$’. The entire path is initially just passed into the function that takes in two parameters, prefix and suffix, as the suffix variables. As the path and wildcards get expanded, the prefix will represent the expanded path (and should hold no wildcard characters), while the suffix will be the path of the path yet to be expanded. The function is set up recursively to allow for this behavior.

Some commands will list out directories in a sorted order, and so the globbing functionality also has the ability to sort paths as well.

The eggs directory contains files with functions that execute our creatively crafted easter eggs. Our eggs include a LISP interpreter, a text editor, and a few fun surprises.
Our LISP interpreter allows users to evaluate simple LISP expressions.

Quark is an extremely minimalist in-terminal text editing experience. Quark uses a grid of addressable rows and columns to allow users to open, edit, and save changes to files - it tracks the location of the cursor and supports syntax highlighting. While it does not quite provide as seamless an experience as vim, it is a fun feature - we even wrote some of Quark in Quark!

The other two easter eggs, title and teach me, provide fun novelty functionality requested by the community. We will leave determining the functionality of them as a design exercise left to the user.

## Design Decisions
One major design decision we made (which turned out to contribute to a lot of debugging time over the course of the project) involved how we parsed the user input for this terminal.

Initially, we used a rudimentary method of parsing. We read user input using the C getline function. We would then pass the line through a function that would separate each whitespace-separated word/character and place them into an array of character pointers (using an ever-so-useful double character pointer). We would then pass these arguments to the execute command.

However, we later decided to pivot from this approach - even though it provided the basic functionality we needed, we wanted our shell to use a more sophisticated and realistic framework. So, we explored using a lexical analyzer and a syntactic analyzer - specially, we worked with lex and yacc to separate commands and place them within SimpleCommand structures. This required a substantial exploration of the tools and their use cases and the design of a program to meet our needs. While these tools were fun to learn how to use and provided interesting use cases, we also found the system to be error-prone and cryptic, which was especially concerning because parsing the user input is a very small component of our shell as a whole. We spent a long time trying to make it work reliably, but continued to face difficulties.

Because of this constraint, we decided return to a simpler framework for parsing user input, but we augmented the original model by manually tokenizing the strings to create individual command structures. While this might not have been as exciting, it was definitely more reliable and allowed us to focus on the rest of the project.

## Implementation Details
Our shell has three major pieces: the parser that collects the user input, the infrastructure to organize the input and parameters, and the execution of the command.

### Parsing: 
As mention, one of our biggest design decisions was around how robust our parsing function could be. In the end we used a simple parsing function inspired by Stephen Brennan’s Simple Shell tutorial. We first read the line from the user using C’s getline function:
```
char *read_line(void) {
	char *line = NULL;
	size_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}
```
Then we split the user’s input line by predetermined delimiters of a space or tab into tokens (words). Here we use C’s strtok function to break up the lines.
token = strtok(line, LSH_TOK_DELIM);
We then add each token to the simpleCommand data structure that will be used by the execute function. A command data structure is then used to hold the simpleCommand as well as passed file names for piping or redirecting if a user included these in the command call.

### Organization: 

One useful organization plan we found useful was from the Purdue CS252 shell lab assignment. In this configuration, the parse makes an instance of a simple command for every command it encounters in a user input line. Then, it stores all of the simple commands in an overall command structure that holds smaller simple commands within it. The execution function can break the current overall command into simple commands and parse each one.
This is the simple command struct. It holds arguments to a single command.
```
typedef struct SimpleCommand{
  int numberOfAvailableArguments;
  int numberOfArguments;
  char ** arguments;
}simpleCommand;
```
The overall command struct holds simple commands and holds file names for piping and other file functions. 
```
typedef struct Command{
  int numberOfAvailableSimpleCommands;
  int numberOfSimpleCommands;
  char ** commands;
  struct SimpleCommand ** simpleCommands;
  char * outputFile;
  char * inputFile;
  char * errFile;
  int background;
}command;
```
### Execution: 
The actual line of execution is fairly simple as it makes use of C’s in-built command. The next few lines of code show a basic fork of the current process and an execution call for the saved command:
  ret = fork();
  if (!ret) {
      execvp(currentSimpleCommand->arguments[0], &currentSimpleCommand->arguments[0]);
      perror(currentSimpleCommand->arguments[0]);
      exit(1);
    }
Our execution function runs through all the saved commands and executes them.
There are two types of execution, ones that carry on the same process (like ls) and ones that fork into a different process (like cd). The ones that carry on the same process can easily be taken care of by the execvp function. However, we had to code functions and figure out the logic for if a user input one of commands that fork into a new process.
For example, if a user commands to make a new directory using the mkdir command, our execution function would call our mkdir function:
```
void lsh_mkdir(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument \"mkdir\"\n");
  }
  else if ((mkdir(args[1],00777)) == -1) {
    perror("lsh");
  }
}
```
We used the built-in command framework to also implement our Easter Eggs. Much like the normal built-in commands, the Easter Eggs are toggled by an input (and also obviously would not be something built into the execvp function), and so we edited the existing framework in order to add the key words.
While the bulk of the execution function is simple, it also handled piping and error handling. 

## Reflection
We were very successful in achieving our personal learning goals. Over the course of our project, we ended up iterating on our shell by building several complete versions of increased complexity. This workflow, which we completed both individually and as a group, allowed us to clearly understand the system-level functionality of a shell and how different methods of parsing and executing commands interact with the overall system. 

Building the functions around the organization of the system multiple times required the use of many functions and methods fundamental to C programming, so we were able to practice and refine our skills in writing C code. The eggs we implemented encouraged individual understanding of programming in C.

Though a large part of the project did involve massive amounts of debugging or following tutorials / guides we found online, we felt that this was in the spirit of the project - and, we feel that we are walking away with a better knowledge of how the Unix Shell works.

# Resources
Throughout the course of this project, we found the following resources useful:

#### Unix Shell Implementation:

Purdue’s “Write Your Own Shell” Lab Assignment from their CS252 Class: https://www.cs.purdue.edu/homes/grr/SystemsProgrammingBook/Book/Chapter5-WritingYourOwnShell.pdf

Stephen Brennan’s Simple Shell Outline and Tutorial:  https://brennan.io/2015/01/16/write-a-shell-in-c/

#### Lex and Yacc:

Georgetown University’s explanation of Lex and Yacc: http://people.cs.georgetown.edu/~clay/classes/spring2017/os/shell1.html 

The Official Lex and Yacc page: http://dinosaur.compilertools.net 

Writing an interpreter with Lex, Yacc, and Memphis: http://memphis.compilertools.net/interpreter.html 

The compact guide to Lex and Yacc: https://www.epaperpress.com/lexandyacc/download/LexAndYacc.pdf 

#### Piping and Redirection:

Implementation of redirection and pipe operators in shell: http://www.sarathlakshman.com/2012/09/24/implementation-overview-of-redirection-and-pipe-operators-in-shell 

Creating Pipes in C: https://www.tldp.org/LDP/lpg/node11.html 

#### Globbing / Wildcards:

Glob Matching Can Be Simple And Fast Too: https://research.swtch.com/glob 

GNU/Linux Command-Line Wildcards Summary: http://tldp.org/LDP/GNU-Linux-Tools-Summary/html/x11655.htm 

Text Editor:
Kilo text editor guide: https://viewsourcecode.org/snaptoken/kilo/index.html
