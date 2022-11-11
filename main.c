#include "shell.h"

void sig_handler(int sig);
int execute(char **args, char **front);

/**
 * sig_handler - Prints a new prompt upon a signal.
 * @sig: The signal.
 */
void sig_handler(int sig)
{
	char *event = "\n$ ";

	(void)sig;
	signal(SIGINT, sig_handler);
	write(STDIN_FILENO, event, 3);
}

/**
 * execute - Executes a command in a child process.
 * @args: An array of arguments.
 * @front: A double pointer to the beginning of args.
 *
 * Return: If an error occurs - a corresponding error code.
 *         O/w - The exit value of the last executed command.
 */
int execute(char **args, char **front)
{
	pid_t child_pid;
	int state, flag = 0, response = 0;
	char *command = args[0];

	if (command[0] != '/' && command[0] != '.')
	{
		flag = 1;
		command = get_location(command);
	}

	if (!command || (access(command, F_OK) == -1))
	{
		if (errno == EACCES)
        {
			response = (create_error(args, 126));
        }    
		else
        {
			response = (create_error(args, 127));
        }
	}
	else
	{
		child_pid = fork();
		if (child_pid == -1)
		{
			if (flag)
				free(command);
			perror("Error child:");
			return (1);
		}
		if (child_pid == 0)
		{
			execve(command, args, environ);
			if (errno == EACCES)
				response = (create_error(args, 126));
			free_env();
			free_args(args, front);
			free_alias_list(aliases);
			_exit(response);
		}
		else
		{
			wait(&state);
			response = WEXITSTATUS(state);
		}
	}
	if (flag)
		free(command);
	return (response);
}

/**
 * main - Runs a simple UNIX command interpreter.
 * @argc: The number of arguments supplied to the program.
 * @argv: An array of pointers to the arguments.
 *
 * Return: The return value of the last executed command.
 */
int main(int argc, char *argv[])
{
	int response = 0, retn;
	int *pointer_ret = &retn;
	char *prompt = "$ ", *new_line = "\n";
    int i;

	name = argv[0];
	hist = 1;
	aliases = NULL;
	signal(SIGINT, sig_handler);

	*pointer_ret = 0;
	environ = _copyenv();
	if (!environ)
		exit(-100);

	if (argc != 1)
	{
		response = proc_file_commands(argv[1], pointer_ret);
		free_env();
		free_alias_list(aliases);
		return (*pointer_ret);
	}

	if (!isatty(STDIN_FILENO))
	{
		for (i = 0; response != END_OF_FILE && response != EXIT; i++)
        {
			response = handle_args(pointer_ret);
        }
		free_env();
		free_alias_list(aliases);
		return (*pointer_ret);
	}

	while (1)
	{
		write(STDOUT_FILENO, prompt, 2);
		response = handle_args(pointer_ret);
		if (response == END_OF_FILE || response == EXIT)
		{
			if (response == END_OF_FILE)
				write(STDOUT_FILENO, new_line, 1);
			free_env();
			free_alias_list(aliases);
			exit(*pointer_ret);
		}
	}

	free_env();
	free_alias_list(aliases);
	return (*pointer_ret);
}
