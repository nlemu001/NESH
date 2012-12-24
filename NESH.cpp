#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <map>

using namespace std;

bool bg = false;

void displayJobs(map<int,string> trabajos)
{
  map<int, string>::iterator pos;
	cout << "PID		Process" << endl;
	for(pos = trabajos.begin(); pos != trabajos.end(); pos++)
	{
		cout << pos->first << "		" << pos->second << endl;
	}
}

void addJob(map<int,string>& trabajos, int num, string str)
{
	trabajos[num] = str;
}

void zombieReaper(int Sig)
{
	signal(SIGCHLD,zombieReaper); 
	int status;
	pid_t pid;
	
	while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

void catch_ctrl_c(int signum)
{
	cout << " Process Interrupted" << endl;
}

int command(char ** argv)
{
	pid_t child = fork();
	int status;
	int in, out, id;

	int counter = 0;
	
	if(child >= 0)
	{
		if(child == 0)
		{
			for(int i = 0; argv[i] != '\0'; i++)
			{
				if(!strcmp(argv[i], ">"))
				{	
					argv[i] = '\0';
					if((out = open(argv[i+1], O_RDWR|O_CREAT|O_TRUNC, 0666)) == -1)
						cout << "couldn't open file!" << endl;
					if(dup2(out, STDOUT_FILENO) == -1) cout << "error with dup2()" << endl;
					close(out);
					
				}
				else if(!strcmp(argv[i], "<"))
				{	
					argv[i] = '\0';
					if((in = open(argv[i+1], O_RDWR)) == -1)
						cout << "couldn't open file!" << endl;
					if(dup2(in, STDIN_FILENO) == -1) cout << "error with dup2()" << endl;
					close(in);
					
				}
				else if(!strcmp(argv[i], "&"))
				{
					argv[i] = '\0';
					bg = true;
				}
			}    
			if(bg == true)
			{
				pid_t gchild = fork();
				if(gchild == -1) perror("fork error: ");
				else if(gchild == 0)
				{
					id = getpid();
					
					execvp(argv[0], argv);
					return id;
				}
				return 0;
			}			
			execvp(argv[0], argv);
			_exit(0);
		}
		else
		{
			if(bg == true)
			{
				return id;
			}
			wait(&status);
		}
		
	}
	else
	{perror("fork error: ");}
	
}

void parse(char * buffer, char ** argus)
{
	for(;*buffer;)
	{
		for(; *buffer == ' ' || *buffer == '\t';)
		*buffer++ = '\0';
		*argus++ = buffer;
		for(;*buffer!='\0' && *buffer!=' ' && *buffer!= '\t'; buffer++) continue;
	}
	*argus = 0;
}	

void eraseline(char * buf)
{
	for(;buf;buf++)
	{
		if(*buf == '\n') 
		{
			*buf = '\0';
			return;
		}
	}  
}

int main()
{
	map<int,string> trabajos;
	string prompt = "you@your_machine - % ";
	signal(SIGINT, catch_ctrl_c);
	signal(SIGCHLD,zombieReaper);
	int i;
	while(true)
	{
		
		char buf[1024];
		char * argv[64];
		cout << prompt;
		fgets(buf, sizeof buf, stdin);
		eraseline(buf);
		parse(buf, argv);	
		i = command(argv);
		if(!strcmp(argv[0], "q")) return 0;
		if(!strcmp(argv[0], "jobs"))
		{
			displayJobs(trabajos);
		}
		else if(!strcmp(argv[0], "kill"))
		{
			int status;
			int k = atoi(argv[1]);
			int killReturn = kill(k, SIGKILL);
			if( killReturn == ESRCH)  
			{
				cout << "Group does not exist!" << endl;
			}
			trabajos.erase(k);
		}
		else if(bg == true)
		{
			i = getpid();	
			addJob(trabajos, ++i, argv[0]);
			
		}
		bg = false;
	}
	
	return 0;
}
