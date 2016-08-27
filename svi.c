#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LENGTH 256
#define MAX_NUM_COMMAND 100
#define MAX_TEXT_LENGTH 80

//String is for each line of i/o and TextString is for the text with the edits
typedef char String[MAX_LENGTH];
typedef char TextString[MAX_TEXT_LENGTH];

//Type of line specifier
typedef enum 
{
  none,
  text,
  lineNum
} LineSpecType;

//Line range
typedef union 
{
  TextString text;
  int lineRange[2];
} LineRange;

//Edit operations
typedef enum 
{
  append,
  prepend,
  newLine,
  delete,
  replace,
  error
} Operation;

//Struct of edit commands
typedef struct 
{
  LineSpecType rangeType;
  LineRange range;
  Operation op;
  TextString data;
} EditCommand;

/* @param C2 a char pointer to a line of command
   returns the LineSpecType of the line
*/
LineSpecType GetLineRangeType(char *C2) 
{
  int C1 = *C2;
  if (C1 == '/')
    return text;
  if (isdigit(C1))
    return lineNum;
  return none;
}

/* @param C2 a char pointer to a line of command                                             
   returns the Operation of the line                                                       
*/
Operation GetOperation(char *C2)
{
  int C1 = *C2;
  switch (C1)
    {
    case 'A':
      return append;
      break;
    case 'I':
      return prepend;
      break;
    case 'O': 
      return newLine;
      break;
    case 'd':
      return delete;
      break;
    case 's':
      return replace;
      break;
    default:
      break;
    }
  return error;
}

/* @param lineNumber the line number of the input
          command the command from the EditCommand array
	  Line the line of input text
    returns 0 if the line is not in the range and 1 if it is
*/
int IsLineInRange(int lineNumber, EditCommand command, String Line)
{
  switch(command.rangeType)
    {
    case text:
      if(strstr(Line, command.range.text) != NULL) //if the range text is a part of Line ret 1
	return 1;
      break;
    case lineNum:
      if ((lineNumber >= command.range.lineRange[0]) && (lineNumber <= command.range.lineRange[1]))
	return 1;
      break;
    case none:
      return 1;
      break;
    default:
      break;
    }

  return 0;
}

/*
  @param command is the command from the command array
         outputLine is the outputLine passed in
	 ret is a return string 
  returns the outputLine after performing the edit
 */
char* EditLine(EditCommand command, String outputLine, String ret) 
{
  switch (command.op)
    {
    case append: 
      strcat(outputLine, command.data); //concatenate the data to the output
      return outputLine;
      break;
    case prepend:
      strcpy(ret, command.data); //copy the data before concatenating the output
      strcat(ret, outputLine); 
      return ret;
      break;
    case newLine:
      printf("%s\n", command.data); //print the data and return the original output
      strcpy(ret, outputLine);
      return ret;
    case delete:
      return "deleted"; //if the output is "deleted" it is not printed
      break;
    case replace:
      {
	String data = ""; //holds command.data to keep the original intact
	String newStr = ""; //holds data for ret during edit

	char* token; //used to split the data                                                             
	char* ptrToFirstToken = ""; //points to first occurrence of token in the outputLine
	int sizeOfFirstToken = 0;

	strcpy(data, command.data);
	data[strlen(data)] = '\0';
	
	token = strtok(data, "/"); //gets "find string" from data
	token[strlen(token)] = '\0';
  
	sizeOfFirstToken = strlen(token);

	ptrToFirstToken = strstr(outputLine, token);
	
	//if there's no range specifier AND the find string is not in the original
	if (ptrToFirstToken == NULL) 
	  return outputLine;

	ptrToFirstToken[strlen(ptrToFirstToken)] = '\0';
		
	//copies the outputLine until the first occurrence of the find string                                             
	strncpy(newStr, outputLine, strlen(outputLine) - strlen(ptrToFirstToken)); 
		
	token = strtok(NULL, "/"); //gets replace string
	strcat(newStr, token); //concatenates the replace string
	strcat(newStr, ptrToFirstToken + sizeOfFirstToken); //concatenates the rest of the line after the first token
        newStr[strlen(newStr)] = '\0';
	
	strcpy(ret, newStr);
	return ret;
      }
    default:
      break;
    }
  return NULL;
}


int main (int argc, char* argv[]) 
{
  FILE *CommandFile;
  EditCommand commands[MAX_NUM_COMMAND];
  
  char *C1; //to iterate through Line
  String Line;

  //counter variable within array of EditCommand                                                                          
  int commandCount = 0;

  //inRange and inOperation are used to store edit commands
  int inRange = 1; //whether a char is part of the line range specification
  int inOperation = 1; //whether a char is part of operation

  //These are used for editing and outputting
  String outputLine = "";
  String ret = "";
  int lineCount = 1;
  int numOfCommands = 0;
  
  //Error message if a command file name is not given
  if (argc != 2) 
    {
      printf("ERROR: Command file name is not given.\n");
      return (EXIT_FAILURE);
    }
  
  //Opens command file provided it exists
  if ((CommandFile = fopen(argv[1], "r")) != NULL) 
    {
      //Read in command file line by line and store into commands arrays
      while (fgets(Line, MAX_LENGTH, CommandFile) != NULL)
	{
	  Line[strlen(Line) - 1] = '\0';
	  C1 = &Line[0];
	  
	  commands[commandCount].rangeType = GetLineRangeType(C1);
	  
	  
	  //Necessary to bypass the initial '/' for ranges with texts                         
	  if (commands[commandCount].rangeType == text)
	    C1++;

	  //Loop to go through each char in the line of command                    
	  while (*C1 != '\0')  
	    {                       
	      //This if block stores the range of the edit commands
	      if (inRange) 
		{
		  switch(commands[commandCount].rangeType)
		    {
		    case text:
		      {
			//Append next char to the range text
			if (*C1 != '/')
			  strncat(commands[commandCount].range.text, C1, 1);
			else 
			  inRange = 0; //No longer in range
			break;
		      }
		    case lineNum:
		      {
			//Splits up Line into tokens to store as range
			String tempLine; //hold the line for strtok, since strtok changes original line
			strcpy(tempLine, Line);
			
			char *token;
			//sizeOfTokens is used to find out how much to increment C1 (depends on length of line number)
			int sizeOfTokens = 0;

			token = strtok(tempLine, ",/");
			sizeOfTokens += strlen(token);
			commands[commandCount].range.lineRange[0] = atoi(token); //first number in range
			
			token = strtok(NULL, ",/");
			sizeOfTokens += strlen(token);
			commands[commandCount].range.lineRange[1] = atoi(token); //last number in range
			
			//Increments C1 by amount of space taken up by line range
			C1 += (sizeOfTokens + 1);
			
			inRange = 0;
			break;
		      }
		    case none:
		      inRange = 0;
		      //When no line range specifiers, the operation is found here to keep the C1 incrementing intact
		      commands[commandCount].op = GetOperation(C1);
		      inOperation = 0;
		      break;
		    default:
		      break;
		    } 
		}
	      //If the edit command counter is not part of the range, store the edit operation specifier and data
	      else
		{
		  //If the next char is for the operation get it
		  if (inOperation)
		    {
		      commands[commandCount].op = GetOperation(C1);
		      inOperation = 0;
		    }
		  //Else add the next char to the data
		  else
		    strncat(commands[commandCount].data, C1, 1);
		}
	      C1++;
	    }
	  
	  //Null terminate the two text fields in the command: the text range and data
	  commands[commandCount].range.text[strlen(commands[commandCount].range.text)] = '\0';
	  commands[commandCount].data[strlen(commands[commandCount].data)] = '\0';

	  //The start of a new command resets booleans and increments the command array
	  inRange = 1;
	  inOperation = 1;
	  commandCount++;	  
	}
    }
  
  numOfCommands = commandCount;

  //Do the edits from input
  while (fgets(Line, MAX_LENGTH, stdin) != NULL)
    {
      Line[strlen(Line) - 1] = '\0';
      strcpy(outputLine, Line);
      
      for (commandCount = 0; commandCount < numOfCommands; commandCount++)
	{
	  if (IsLineInRange(lineCount, commands[commandCount], Line))
	    strcpy(outputLine, EditLine(commands[commandCount], outputLine, ret)); //copy edits to the outputLine
	 
	  if (!strcmp(outputLine, "deleted")) //if a line is to be deleted stop going through commands
	    break; 
	}

      if (strcmp(outputLine, "deleted"))
	printf("%s\n", outputLine);
      
      lineCount++;
    }
 
  return (EXIT_SUCCESS);
}
