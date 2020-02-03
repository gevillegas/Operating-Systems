/*
Project 1- Implementing a Shell
FSU- COP4610
Spring 2019
Group Members- Franco Marcoccia, Guillermo Villegas, Elizabeth Cobb
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <limits.h>
#include <fcntl.h>
#include <stdbool.h>


/*Struct Instruction*/
typedef struct
{
  char** tokens;
  int numTokens;
} instruction;

/*Struct Alias*/
typedef struct
{
  char ** name;
  int numTokens;
} alias;

/*Function Declarations*/
void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void Prompt(void);
int founddir(const char*path);
int foundfile(const char *path);
void execute(char **cmd);
char * PathResolution(instruction* instr_ptr, char* token);
int IORedirection(instruction * instr_ptr);
char * removeDollar (instruction* instr_ptr);
int Piping(instruction * instr_ptr);
void addAlias(alias* instr_ptr, char* tok);
bool Delete_Alias(instruction* instr_ptr, alias *alias);
char * Get_Alias(instruction* instr_ptr, alias *alias);
int BackgroundProcessing(instruction*instr_ptr);
bool Find_Alias(instruction *instr_ptr, alias *alias);

//global variable for amount of commands ran by the user even if they are invalid
int commandsran = 0;

/*
 @brief Main entry point.
 @return 0
*/
int main() {
  char* token = NULL;
  char* temp = NULL;

  instruction instr;
  instr.tokens = NULL;
  instr.numTokens = 0;

  alias alias;
  alias.name = NULL;
  alias.numTokens=0;

  while (1) {
    Prompt();

	// loop reads character sequences separated by whitespace
    do {
      scanf( "%ms", &token); 
	//scans for next token and allocates token var to size of scanned token
      temp = (char*)calloc((strlen(token)+1) , sizeof(char)); 
	//allocate temp variable with same size as token

      int i;
      int start;

      start = 0;

      for (i = 0; i < strlen(token); i++)
	{
	  //pull out special characters and make them into a separate token in the instruction
	  if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&')
	    {
	      if (i-start > 0)
		{
		  memcpy(temp, token + start, i - start);
		  temp[i-start] = '\0';
		  addToken(&instr, temp);
		}

	      char specialChar[2];
	      specialChar[0] = token[i];
	      specialChar[1] = '\0';

	      addToken(&instr,specialChar);

	      start = i + 1;
	    }
	}
      	  if (start < strlen(token))
	  {
	  memcpy(temp, token + start, strlen(token) - start);
	  temp[i-start] = '\0';
	  addToken(&instr, temp);
	  }

      	//free and reset variables
      	free(token);
      	free(temp);

      	token = NULL;
      	temp = NULL;

        } while ('\n' != getchar());    //until end of line is reached

    //if input is alias get the alias command and execute that
    if (instr.numTokens==1 && (strcmp(instr.tokens[0],"exit")!=0) && 
	(strcmp(instr.tokens[0],"cd")!=0) && (strcmp(instr.tokens[0],"echo")!=0) && 
	(strcmp(instr.tokens[0],"alias")!=0) && (strcmp(instr.tokens[0],"unalias")!=0))
    {
      
	if (Find_Alias(&instr, &alias)==true) //if command is found 
	{ 

        char *commands=NULL;
        commands=Get_Alias(&instr, &alias);
	int i=0;
	int position=0;
                
		//delete first token because it is still in token array
                for (i = position - 1; i < instr.numTokens - 1; i++)
                {   instr.tokens[i] = instr.tokens[i+1];}
                instr.numTokens--;

                //parse commands into tokens
		while (commands !='\0')
		{

		char *newtemp =NULL;
        	newtemp = (char*)calloc((strlen(commands)+1) , sizeof(char)); 
		//allocate temp variable with same size as token

                        int i;
                        int start;

                        start = 0;

                        for (i = 0; i < strlen(commands); i++) 
                        {
                                //pull out special characters and make them into a separate token in the instruction
                                if (commands[i] == '|' || commands[i] == '>' || commands[i] == '<' || commands[i] == '&')
                                {
                                        if (i-start > 0) //store token before the special char
                                        {
                                                memcpy(newtemp, commands + start, i - start);
                                                newtemp[i-start] = '\0';
                                                addToken(&instr, newtemp);
                                        }

                                        char specialChar[2];
                                        specialChar[0] = commands[i];
                                        specialChar[1] = '\0';
                                        //store the special char
                                        addToken(&instr,specialChar);

                                        start = i + 1;
                                }

                                //if white space, skip over it
                                if (commands[i] == ' ')
                                {
                                        if (i-start > 0)
                                        {
                                        memcpy(newtemp, commands + start, strlen(commands) - start);
                                newtemp[i-start] = '\0';
                                addToken(&instr, newtemp);

                                        }
                                        start= i+ 1;

                                }
                        }
			
                        //if start is still less than stringlength store the cmd after
                        if (start < strlen(commands))
                        {
                                memcpy(newtemp, commands + start, strlen(commands) - start);
                                newtemp[i-start] = '\0';
                                addToken(&instr, newtemp);
                        }
			
			//free and reset variables
                        free(commands);
                        free(newtemp);

                        commands = NULL;
                        newtemp = NULL;
            	}
     	 }
	        
    }    
           

    /*If input is a built-in do command, else call execv */
	if((strcmp(instr.tokens[0],"exit")==0)|| (strcmp(instr.tokens[0],"cd")==0) 
	|| (strcmp(instr.tokens[0],"echo")==0)||(strcmp(instr.tokens[0],"alias")==0)
	||(strcmp(instr.tokens[0],"unalias")==0))
	{
	// ********************** Exit COMMAND *************************
	if((strcmp(instr.tokens[0],"exit")==0) && instr.numTokens==1)
	{
	commandsran++; 
	printf("Exiting...\n");
	printf("The number of commands executed was %d\n",commandsran);
	//also terminate the running shell process + wait for background processes
	break;
	}
	else if((strcmp(instr.tokens[0],"exit")==0) && instr.numTokens!=1)
	{
	printf("exit: Expression Syntax.\n");
	}
	
	// ********************** Echo COMMAND *************************
	else if((strcmp(instr.tokens[0],"echo")==0))
	{
	commandsran++;
		if(instr.numTokens==1)
		{
			printf("error, too few arguments\n");
	  	}
		else
		{
	      		printTokens(&instr);
	      		printf("\n");
		}
	}
	
	// ********************** CD COMMAND *************************
	else if(strcmp(instr.tokens[0],"cd")==0)
	{
	char cd[PATH_MAX];
	getcwd(cd, sizeof(cd));
	commandsran++;

	// error for more than 2 arguments
	if(instr.numTokens>=3)
	{
	printf("Error, more than one argument is present.\n");
	}
	//error if .. is tried to be used in root directory
	else if(instr.numTokens == 2)
	{
		if ((strcmp(instr.tokens[1], "..") == 0) && (strcmp(cd, "/") == 0))
		{
		printf("Error, command can't be used in root directory\n");
		}
	}
	// command for cd with no arguments as well as ~ shortcut to both go to $HOME
	if(instr.numTokens==1 || (instr.numTokens==2 && 
	(strcmp(instr.tokens[1],"~")==0)) || (instr.numTokens==2 && 
	strcmp(instr.tokens[1],"$HOME")==0))
	{  
	chdir(getenv("HOME"));
	getcwd(cd,sizeof(cd));
	setenv("PWD",cd,1);
	}
	// error for when the directory isn't found
	else if(founddir(instr.tokens[1])==0)
	{
	char * t = removeDollar (&instr);
		if(getenv(t) != NULL && founddir(getenv(t)) !=0)
		{
		chdir(getenv(t));
		getcwd(cd,sizeof(cd));
		setenv("PWD",cd,1);
		}
		else
		{
		printf("cd: %s: No such directory\n",instr.tokens[1]);
		}
	}
	// command for pathing into another directory as long as it exists
	else if(instr.numTokens==2 && founddir(instr.tokens[1])!=0)
	{
	chdir(instr.tokens[1]);
	getcwd(cd,sizeof(cd));
	setenv("PWD",cd,1);
	}
	}

	// ********************** Alias COMMAND *************************
	if(strcmp(instr.tokens[0],"alias")==0)
	{
	commandsran++;
	//error if already 10 aliases
	//equal to 20 because my alias name and alias 
	//commands are stored in same array
		if (alias.numTokens==20)
		{printf("error, already 10 aliases\n");}
		//if alias command passes 2 or more tokens, else print error
		if(instr.numTokens>=2){
	      	char* t= NULL;
	
	t = (char*)calloc((strlen(instr.tokens[1])+100) , sizeof(char));
	//combine the tokens to one string
	int i;
	for (i = 1; i < instr.numTokens; i++) 
	//token[1] up until the end of last token
	{
	strcat (t,instr.tokens[i]);
	strcat (t," ");
	}

	//error check the format of alias, ex. alias ll='ls -a'
	char c= '\'';
	if (t[strlen(t) - 2]!= c){
	printf("error, command not found\n");}
	      
	char *aliasC= NULL;
	aliasC = (char*)calloc((strlen(t)+100) , sizeof(char));

	int start=0;
	t[strlen(t) - 2] = '\0'; //delete last char in string because it is '
		    
	//parse string t and store in struct alias	    
	for (i = 0; i < strlen(t); i++)
	{
	//stop the search and get everything before = and store in struct
	if (t[i] == '=')
	{
		if (i-start > 0)
		{
		memcpy(aliasC, t + start, i - start);
		aliasC[i-start] = '\0';
		addAlias(&alias, aliasC);    
		}
		//skip over = and also '
		start = i + 2;
	}    
        }

	if (start < strlen(t))
	{
	memcpy(aliasC, t + start, strlen(t) - start);
	aliasC[i-start] = '\0';
	addAlias(&alias, aliasC);
        }
	//free and reset variables	    
	free(aliasC);
	free(t);

	aliasC=NULL;
	t=NULL;

	}
	else
	printf("error, too few arguments\n");
          
	}
	
	// ********************** Unalias COMMAND *************************
	if(strcmp(instr.tokens[0],"unalias")==0)
	{
	commandsran++;
	//delete alias from list
		if (Delete_Alias(&instr, &alias)==false){
		printf("alias not found\n");
	}
		else
		//deleted in function 
		printf("deleting alias from list ");
		printf("... done\n");
	}
    
	}
    	//if not a built-in 
	else if(strcmp(instr.tokens[0],"exit")!=0 && strcmp(instr.tokens[0],"echo")!=0 &&
	    strcmp(instr.tokens[0],"cd")!=0 && strcmp(instr.tokens[0],"alias")!=0 && 
	    strcmp(instr.tokens[0],"unalias")!=0)
	{
	commandsran++;
	int works = 0;
	if(strcmp(instr.tokens[0],">")==0)
	{
		if(instr.numTokens==1)
			printf("Invalid Syntax Error\n");
		else
	      		IORedirection(&instr);

	works=1;
        }
	if(works==0)
	{
	char* token0 = instr.tokens[0];
	instr.tokens[0] = PathResolution(&instr, token0);
	char* ch= NULL;
		if(foundfile(instr.tokens[0])!=0)
		{
		addToken(&instr, ch);
		fflush(0);
		int maybe;
		int possible;
		int questionable = BackgroundProcessing(&instr); 

		//the following functions only work if the return value 
		//is 0 so that they don't repeat
		if(questionable==0)
			maybe = IORedirection(&instr);
		if(maybe==0 && questionable==0)
	      		possible = Piping(&instr);
	    	if(maybe==0 && possible==0 && questionable==0)
			execute(instr.tokens);
          	}
		else
			printf("Command or file not found.\n");
        
		}  
	}
              clearInstruction(&instr);
      
	}
    return 0;
}

/*
 @brief addToken reallocates instruction array to hold another token and 
 allocates for new token within instruction array
*/
void addToken(instruction* instr_ptr, char* tok)
{
//extend token array to accomodate an additional token
if (instr_ptr->numTokens == 0)
	instr_ptr->tokens = (char**)malloc(sizeof(char*));
else
	instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, 
(instr_ptr->numTokens+1) * sizeof(char*));

//allocate char array for new token in new slot
if (tok != NULL)
{
instr_ptr->tokens[instr_ptr->numTokens] = 
(char *)malloc((strlen(tok)+1) * sizeof(char));
strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);
}
else
{
instr_ptr->tokens[instr_ptr->numTokens] = NULL;
}

instr_ptr->numTokens++;
}

/*
@brief printTokens prints tokens if the echo command is called, 
along with enviromental variables
*/
void printTokens(instruction* instr_ptr)
{
int i;
char * result;
int chardel = 0;

for (i = 1; i < instr_ptr->numTokens; i++)//for each arg passed to echo
{
if(instr_ptr->tokens[i] != NULL)
{
        if ((instr_ptr->tokens)[i][0]== '$') 
	//if the first character in the token is $
	{
	//need token[i][1] till end of that string
	//delete first char ($) in string
	char *word;
	word=(instr_ptr->tokens)[i];
	memmove(&word[chardel], &word[chardel + 1], strlen(word) - chardel);
	result = getenv(word);

	if (result==NULL) //if enviromental variable is null print error
	{
		printf("error enviromental variable not found ");
	}
	else
		printf("%s ",result); //prints enviromental variable
	}

else
//output token without modification
printf("%s ", (instr_ptr->tokens)[i]);
          
}
        
}
    
}

/*
@brief clearInstructions frees instructions
*/
void clearInstruction(instruction* instr_ptr)
{
int i;
for (i = 0; i < instr_ptr->numTokens; i++)
	free(instr_ptr->tokens[i]);
free(instr_ptr->tokens);
instr_ptr->tokens = NULL;
instr_ptr->numTokens = 0;
}

/*
@brief Prompt
*/
void Prompt(void)
{
//prompts user who they are, machine name, and absolute working dir
printf("%s@%s : %s > ",getenv("USER"),getenv("MACHINE"),getenv("PWD"));
}

/*
@brief founddir finds directory 
*/
int founddir(const char*path)
{
struct stat stattime;
stat(path, &stattime);
	return S_ISDIR(stattime.st_mode);
}

/*
@brief foundfile finds file in path
*/
int foundfile(const char *path)
{
struct stat path_stat;
stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

/*
@brief execute
*/
void execute(char **cmd)
{
int status;
pid_t pid =fork();
if (pid == -1)
{
//error
exit(1);
}
else if (pid ==0)
{
//child
execv(cmd[0], cmd);
//fprintf("problem executing %s\n", cmd[0]);
exit(1);
}
else 
{
//Parent 
waitpid(pid, &status, 0);
}
}

/*
@brief PathResolution
*/
char * PathResolution(instruction* instr_ptr, char* token)
{
const char ch = ':';
char* completePath;
char* temp;
char* buffer;
int amountOfPaths = 1;
int i = 0;
int j = 0;
int k = 1;
int adding = 0;

completePath = (char*)malloc(sizeof(char)*(strlen(getenv("PATH"))+1));
temp = (char*)malloc(sizeof(char)*(strlen(getenv("PATH"))+(strlen(token))+1));
buffer = (char*)malloc(sizeof(char)*(strlen(getenv("PATH"))+1));

strcpy(completePath, getenv("PATH"));
for(k; k<strlen(completePath);k++)
{
	if(completePath[k] == ch)
	{
	amountOfPaths++;
	}
}
const char* arr[amountOfPaths];
// arr = (char**)malloc(sizeof(char*)*(strlen(getenv("PATH"))+1));
k = 0;
if(completePath != NULL)
{
amountOfPaths = 1;
while(completePath[i] != '\0')
{
	if(completePath[i] == ch)
	{
	if(amountOfPaths == 1)
	{
	strncpy(temp, completePath, i);
	temp[strlen(temp)] = '/';

	k++;
	j = i;
        }
	else
	{
	memcpy(temp, &completePath[j+1], i-j-1);
	temp[i-j-1] = '/';
	temp[i-j] = '\0';
	strcpy(buffer,temp);
	arr[k] = buffer;
	k++;
	j = i;

	}
	amountOfPaths++;
	strcat(buffer, token);

	char* ch= NULL;
	if(foundfile(buffer)!=0)
	{
	return buffer;
	}
          
	}
	i++;
}
}
  return NULL;
}

/*
@brief removeDollar removes dollar sign on the enviromental variable 
*/
char * removeDollar(instruction* instr_ptr)
{
int sizeOfToken2  = strlen(instr_ptr->tokens[1]);
char env [sizeOfToken2];
char *r = malloc(sizeOfToken2);
int i = 0;
for(i; i<sizeOfToken2; i++)
{
env[i] = instr_ptr->tokens[1][i+1];
}
env[sizeOfToken2] = '\0';
strcpy(r, env);
  
return r;
}

/*
 @brief IORedirection
*/
int IORedirection(instruction * instr_ptr)
{
int i;
int tof = 0;
for(i=0;i<instr_ptr->numTokens-1;i++)
{
	// used for obtaining the size of the CMD
	if(strcmp(instr_ptr->tokens[i],"<")==0)
	{
	tof = 1;
	int scounter = 0;
	int k;
	for(k=0;k<instr_ptr->numTokens-1;k++)
	{
	if(strcmp(instr_ptr->tokens[k],">")!=0)
		scounter++;
	else
		break;
	}

	scounter++;
	char * temp[scounter];
	int f;

	// takes care of null termination
	for (f = 0; f < scounter; f++)
	{
	if(strcmp(instr_ptr->tokens[f],"<")==0)
	{
		temp[f] = '\0';
		break;
	}
	else
	{
	temp[f] = instr_ptr->tokens[f];
		if (f + 1 >= scounter-1)
		{
			temp[f+1] = '\0';
			break;
		}
	}        
}
//error checking <
if((i-1)<0 || (i+1)>=instr_ptr->numTokens-1)
	printf("Invalid Redirection Syntax Error\n");
else if(strcmp(instr_ptr->tokens[i],"<")==0 
&&foundfile(instr_ptr->tokens[i+1])==0)
	printf("Redirection not a file\n");
else
{
int fd = open(instr_ptr->tokens[i+1],O_RDONLY);
int status;
pid_t pid = fork();
      
if (pid == -1)
{
	//error
	exit(1);
}
else if (pid ==0)
{
	//child
	close(0);
	dup2(fd,STDIN_FILENO);
	close(fd);
	execv(temp[0], temp);
	exit(1);
}
else
{                                        
	//Parent
	//close(fd);
	waitpid(pid, &status, 0);
	close(fd);
}
}
        
}
else if(strcmp(instr_ptr->tokens[i],">")==0)
{
tof = 1;
int scounter = 0;
int k;
// gets size of CMD for temp variable
for(k=0;k<instr_ptr->numTokens-1;k++)
{
	if(strcmp(instr_ptr->tokens[k],">")!=0)
		scounter++;
	else
		break;
}

scounter++;
char * temp[scounter];
int f;

//takes care of null termination
for (f = 0; f < scounter; f++)
{
if(strcmp(instr_ptr->tokens[f],">")==0)
{
	temp[f] = '\0';
	break;
}
else
{
temp[f] = instr_ptr->tokens[f];

if (f + 1 >= scounter-1)
{
temp[f+1] = '\0';
break;
}

}
}

// > error checking
if((i-1)<0 && (i+1)>=instr_ptr->numTokens)
{
printf("Invalid Redirection Syntax Error\n");
break;
}
else if((i+1)>=instr_ptr->numTokens)
{
printf("Invalid Redirection Syntax Error\n");
break;
}
else
{
int fd = open(instr_ptr->tokens[i+1],O_RDWR|O_CREAT|O_TRUNC,0666);
int status;
pid_t pid = fork();

if(pid==-1)
{
	//error
	exit(1);
}
else if(pid==0)
{
	//Child
	close(1);
	dup2(fd,STDOUT_FILENO);                
	close(3);
	//Execute process
	execv(temp[0],temp);
	exit(1);                                                                                    
}
else                                                                                                                                                                                                                   
{
//Parent
waitpid(pid,&status,0);
close(fd);
}

}
}
}
	//returns 0 or 1 based off it did redirection
	return tof;
}

/*
@brief Piping
*/
int Piping(instruction * instr_ptr)
{
int i;
int tof = 0;
int pipecounter = 0;
int l;
int startcounter;

//counts the number of pipes
for(l=0;l<instr_ptr->numTokens-1;l++)
{
	if(strcmp(instr_ptr->tokens[l],"|")==0)
		pipecounter++;
      
}
if(pipecounter==1)
{
tof = 1;
for(i=0;i<instr_ptr->numTokens-1;i++)
{
//error checking pipelines
	if(strcmp(instr_ptr->tokens[0],"|")==0)
	{
		printf("Pipeline Syntax Error\n");
		break;
        }
        else if(strcmp(instr_ptr->tokens[i],"|")==0 &&
        (i+1 >= instr_ptr->numTokens-1))
       	{
        	printf("Pipeline Syntax Error\n");
                break;
                
        }

        // stores size of CMD's that need to be executed
        if(strcmp(instr_ptr->tokens[i],"|")==0)
        {
        int scounter = 0;
        int k;
        for(k=0;k<instr_ptr->numTokens-1;k++)
        {
		if(strcmp(instr_ptr->tokens[k],"|")!=0)
                        scounter++;
                else
                {
                	startcounter = ++k;
                	break;
                }
                    
	}

scounter++;
int scounter2 = 0;
char * temp[scounter];
int f;
int q;
int w;      
	// used for null termination and size of second CMD
	for(w=startcounter;w<instr_ptr->numTokens-1;w++)
		scounter2++;

scounter2++;
char *temp2[scounter2];

	for (f = 0; f < scounter; f++)
	{
		if(strcmp(instr_ptr->tokens[f],"|")==0)
		{
		temp[f] = '\0';
		break;
		}
          	else
          	{
		temp[f] = instr_ptr->tokens[f];
              
              	if (f + 1 >= scounter-1)
              	{
			temp[f+1] = '\0';
			break;
		}
              
                }
	}

	// stores into temp2 the second CMD
	for(q=startcounter;q<instr_ptr->numTokens-1;q++)
		temp2[q-startcounter]=instr_ptr->tokens[q];
	      
	temp2[scounter2]='\0';
	temp2[0] = PathResolution(instr_ptr, temp2[0]);
	      
	// more error checking
	if(strcmp(instr_ptr->tokens[0],"|")==0)
	{
		printf("Pipeline Syntax Error\n");
		break;
	}
	else if(strcmp(instr_ptr->tokens[i],"|")==0 && 
	(i+1 >= instr_ptr->numTokens-1))
	{
	printf("Pipeline Syntax Error\n");
	break;
	}
	else
	{
	//int pipe(int pipefd[2]);
	int status;
	pid_t pid = fork();
	int fd[2];

	if(pid==-1)
	{
	//error
	exit(1);
	}
	else if(pid==0)    
	{
	//child (cmd1|cmfd2)
	pipe(fd);
	if(fork()==0)
	{
	//cmd1 (writer)
	close(STDOUT_FILENO);
	dup2(fd[1],1);
	close(fd[0]);
	close(fd[1]);
	//execute command
	execv(temp[0],temp);
        exit(1);
	}
	else
	{
	//cmd2 (reader)
	close(STDIN_FILENO);
	dup2(fd[0], 0);
	close(fd[0]);
	close(fd[1]);
	execv(temp2[0],temp2);
        exit(1);
        }
	}
	else
	{
	//parent shell
	waitpid(pid,&status,0);
	//close(fd[0]);
	//close(fd[1]);
        return;       
        }
            
        }
                
}
            
}
        
}
//returns 0 or 1 based off if it did any piping
	return tof;
}

/*
@brief addAlias
*/
void addAlias(alias* instr_ptr, char* tok)
{
//extend token array to accomodate an additional token
if (instr_ptr->numTokens == 0)
	instr_ptr->name = (char**)malloc(sizeof(char*));
else
	instr_ptr->name = (char**)realloc(instr_ptr->name, 
	(instr_ptr->numTokens+1) * sizeof(char*));

//allocate char array for new token in new slot
if (tok != NULL)
{
	instr_ptr->name[instr_ptr->numTokens] = 
	(char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->name[instr_ptr->numTokens], tok);
}
else
{
	instr_ptr->name[instr_ptr->numTokens] = NULL;
}

instr_ptr->numTokens++;
}


/*
@brief Get_Alias
*/
char * Get_Alias(instruction* instr_ptr, alias *alias)
{
char * commands;
int result;
int i=0;

//for each token in alias list
for (i = 0; i < alias->numTokens; i++)
{
//compare token[0] to each token in alias list
result=strcmp(instr_ptr->tokens[0],(alias->name)[i]);
      
	if (result==0)
	{
	//printf("alias found: "); //for checking

        //if found access the token under and make it equal to commands
        commands = (alias->name)[i+1];
	}
}
	return commands; //return the commands as a char*
        //parse the string again in main
}

/*
@brief Delete_Alias deletes alias from alias struct
*/
bool Delete_Alias(instruction* instr_ptr, alias *alias)
{
char * commands;
int result;
int i=0;

//for each token in alias list
for (i = 0; i < alias->numTokens; i++)
{
//compare token[0] to each token in alias list
result=strcmp(instr_ptr->tokens[1],(alias->name)[i]);

	if (result==0)
	{ 
	//if found delete that token
        int j;
	for(j = i; j < alias->numTokens; j++)
	{
	alias->name[j] = alias->name[j + 1];
	}
	alias->numTokens--;
	i--;
	return true;
	}
}
//if not found return false
if(result!=0)
	return false;
}

/*
 @brief BackgroundProcessing
*/
int BackgroundProcessing(instruction*instr_ptr)
{
	int i;
	int tof = 0;
	
	// purely error checking
	for(i=0;i<instr_ptr->numTokens-1;i++)
	{
	if(instr_ptr->numTokens == 6)
	{
		if(strcmp(instr_ptr->tokens[i],"&")==0 && 
			strcmp(instr_ptr->tokens[i+1],"|")==0 && 
		   	strcmp(instr_ptr->tokens[i+3],"&")==0)
		{
			tof=1;
			printf("Invalid Background Processing Syntax\n");
			break;
		}
	}
	else if(instr_ptr->numTokens == 5)
	{
		if(strcmp(instr_ptr->tokens[i],"&")==0 && 
			strcmp(instr_ptr->tokens[i+1],"|")==0)
		{
			tof=1;
			printf("Invalid Background Processing Syntax\n");
			break;
		}
		else if(strcmp(instr_ptr->tokens[i],"&")==0 && 
			strcmp(instr_ptr->tokens[i-1],">")==0 && 
			foundfile(instr_ptr->tokens[i+1])==0)
		{
			tof=1;
			printf("Invalid Background Processing Syntax\n");
			break;
		}
		else if(strcmp(instr_ptr->tokens[i],"&")==0 && 
			strcmp(instr_ptr->tokens[i-1],"<")==0 && 
			foundfile(instr_ptr->tokens[i+1])==0)
		{	
			tof=1;
			printf("Invalid Background Processing Syntax\n");
			break;
		}
	}
        
    }
//always going to return 0 since it wasn't implemented
    return tof;
}

/*
@brief Find_Alias
@return false if alias not found, true if found
*/
bool Find_Alias(instruction *instr_ptr, alias *alias)
{
char * commands;
int result;
int i=0;
      
//for every token in alias
for(i=0; i<alias->numTokens; i++)
{
result= strcmp(instr_ptr->tokens[0], (alias->name)[i]);
	if (result==0)
	{
	return true;
	}
}

	return false;    
}

