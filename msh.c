/*
  Name: Cole Montgomery
  ID: 1001301470
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // White space as the delimeter

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11 // The maximum number of parameters including the command

int main()
{

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

  // Array of PIDs of processes created by shell
  int pid_history[15] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  int num_pids = 0;

  // Array of previous commands run by the shell and their arguments
  char *com_history[15][MAX_NUM_ARGUMENTS];
  int num_coms = 0;

  // Initialize the command history to be blank
  int i;
  int j;
  for (i = 0; i < 15; i++)
  {
    for (j = 0; j < MAX_NUM_ARGUMENTS; j++)
    {
      com_history[i][j] = (char *)malloc(sizeof(char) * MAX_COMMAND_SIZE);
      com_history[i][j] = "*";
    }
  }

  int skip = 0;

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the parsed token
    char *argument_ptr;

    char *working_str = strdup(cmd_str);

    if (strlen(working_str) > 1) // Make sure that input isn't blank or input is skipped from !n
    {
      int bad_com = 0;

      // Move the working_str pointer to keep track of the original value
      char *working_root = working_str;

      // Tokenize the input strings with whitespace used as the delimiter
      while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
             (token_count < MAX_NUM_ARGUMENTS))
      {
        token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
        if (strlen(token[token_count]) == 0)
        {
          token[token_count] = NULL;
        }
        token_count++;
      }

      free(working_root);

      // Write the most recent command to com_history
      com_history[num_coms % 15][0] = token[0];

      // Write the arguments of the most recent command to com_history
      int i;
      for (i = 1; i < token_count; i++)
      {
        com_history[num_coms % 15][i] = token[i];
      }
      num_coms++;

      // Handle the special !n command
      // If the command is !n, overwrite all values in the token array with the correct values from com_history
      if (token[0][0] == '!' && token_count == 2)
      {
        char *temp_num = (char *)malloc(MAX_COMMAND_SIZE);

        int i;
        for (i = 1; i < MAX_COMMAND_SIZE; i++)
        {
          temp_num[i - 1] = token[0][i];
        }

        // Convert the characters other than ! to an integer
        int command_num = (int)strtol(temp_num, (char **)NULL, 10);

        // Ensure that the command exists in history and overwrite the values in token[]
        if (command_num < num_coms && command_num < 15 && command_num >= 0)
        {
          int arg_num = 0;
          while (com_history[command_num][arg_num] != NULL)
          {
            token[arg_num] = com_history[command_num][arg_num];
            arg_num++;
          }
        }
        else
        {
          printf("Command not in history.\n");
          bad_com = 1;
        }

        free(temp_num);
      }

      if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0) // Handle the case of "quit" or "exit" by ending the shell
      {
        return 0;
      }
      else if (strcmp(token[0], "cd") == 0) // Handle the case of "cd" by calling the chdir function on the first argument after the command
      {
        chdir(token[1]);
      }
      else if (strcmp(token[0], "showpids") == 0) // Handle the case of "showpids" by reading previous values from pid_history
      {
        int i;
        for (i = 0; i < 15; i++)
        {
          if (pid_history[i] != -1)
          {
            printf("Process %d:\t%d\n", i, pid_history[i]);
          }
        }
      }
      else if (strcmp(token[0], "history") == 0) // Handle the case of "history" by reading from com_history
      {
        int i;
        for (i = 0; i < 15; i++)
        {
          if (strcmp(com_history[i][0], "*") != 0) // Only print commands that exist
          {
            printf("%d: %s ", i, com_history[i][0]);
            int arg_num = 1;
            while (com_history[i][arg_num] != NULL)
            {
              printf("%s ", com_history[i][arg_num]);
              arg_num++;
            }
            printf("\n");
          }
        }
      }
      else if (bad_com == 0) // Only execute the command if there were no errors from reading from history
      {
        pid_t pid = fork();

        if (pid == 0)
        {
          int ret = execvp(token[0], &token[0]);

          if (ret == -1)
          {
            perror(token[0]);
          }
        }
        else
        {
          // Copy the PID to pid_history of the newly created process
          pid_history[num_pids % 15] = pid;
          num_pids++;

          int status;
          wait(&status);
        }
      }
    }
  }

  // Free the command history array
  for (i = 0; i < 15; i++)
  {
    for (j = 0; j < MAX_NUM_ARGUMENTS; j++)
    {
      free(com_history[i][j]);
    }
    free(com_history[i]);
  }
}
