#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stddef.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
int  parse_commands(char* div,char* in, char* com[50]);
void parse_args(int count,char* com[50],char* arg[20][20]);
int execute_comm(char* div,int count,char* arg[20][20]);
int read_file(char* buffer,char* argv[],int line);
void redirection(char* args[20][20],int j,int i,char dir);
void execpipe(char* arg[20][20],int line);
int numofargs[20];
int main(int argc,char* argv[])
{
    int i,j,z,exit_val=0;
    int counter1;
    char* commands[50];
    for(j=0; j<50; j++)
        commands[i]=NULL;
    char* divider=NULL;
    char* args[20][20];
    for(i=0; i<20; i++)
    {
        for(j=0; j<20; j++)
            args[i][j]=NULL;
    }
    for(z=0; z<20; z++)
        numofargs[z]=0;
    if(argc==2) //batch mode
    {
        int eof=1;
        int line=0;
        for(;;)
        {
            char* buf=malloc(sizeof(char)*2000);
            eof=read_file(buf,argv,line);
            line++;
            if(buf[0]==0)continue;
            if(strstr(buf,"&&")!=NULL)
                divider="&&";
            else if(strstr(buf,";")!=NULL)
                divider=";";
            else divider=" ";
            counter1=parse_commands(divider,buf,commands);
            parse_args(counter1,commands,args);
            exit_val=execute_comm(divider,counter1,args);
            if((exit_val==2)||(eof==0))
            {
                printf("\n");
                printf("Exiting program.....\n\n");
                exit(0);
            }
            for(i=0; i<20; i++)
            {
                for(j=0; j<20; j++)
                    args[i][j]=NULL;
            }
            for(j=0; j<50; j++)
                commands[i]=NULL;
        }
    }
    else if(argc<2) //interactive mode
    {
        char input[512];
        for(i=0; i<512; i++)
            input[i]='\0';
        for(;;)
        {
            printf("dami_8606> ");
            fgets(input,512,stdin);
            if(input[0]==' ')continue;
            if(strstr(input,"&&")!=NULL)
                divider="&&";
            else if(strstr(input,";")!=NULL)
                divider=";";
            else divider=" ";
            counter1=parse_commands(divider,input,commands);
            parse_args(counter1,commands,args);
            exit_val=execute_comm(divider,counter1,args);
            if(exit_val==2)
            {
                printf("\n");
                printf("exiting program.....\n\n");
                exit(0);
            }
            for(i=0; i<20; i++)
            {
                for(j=0; j<20; j++)
                    args[i][j]=NULL;
            }
            for(j=0; j<50; j++)
                commands[i]=NULL;
            for(i=0; i<512; i++)
                input[i]='\0';
        }
    }
    else
    {
        printf("not acceptable number of args...exiting program..\n");
        exit(0);
    }

    return 0;
}
int read_file(char* buffer,char* argv[],int line)
{
    FILE *batchfile=fopen(argv[1],"r");
    int file_out=0,pre_out=0;
    int executed=0;
    int file_count=0;
    int count=0;
    if(batchfile==0)
    {
        printf("\n");
        printf("error in opening batchfile...\n");
        exit(0);
    }
    else
    {
        while((file_out=getc(batchfile))!=EOF)
        {
            if(executed<line)
            {
                if(file_out=='\n')
                    executed++;
            }
            else
            {
                if((pre_out=='\n')&&(file_out!='\n'))
                {
                    buffer[file_count-1]='\0';
                    break;
                }
                else
                {

                    buffer[file_count]=file_out;
                    file_count++;
                    pre_out=file_out;
                }
            }

        }
        fclose(batchfile);
        buffer[file_count-1]='\0';
        if((file_count==0)&&(executed==0))
        {
            printf("\n");
            printf("file was empty...try again with other file");
        }
        if((file_out==EOF))
        {
            file_out='\0';
            buffer[file_count]=file_out;
            return 0;
        }
        else return 1;
    }
}
int parse_commands(char* div,char* in, char* com[50])
{
    char* token=NULL;
    int counter;
    if(div==" ")
    {
        com[0]=in;
        return 1;
    }
    token=strtok(in,div);
    com[0]=token;
    counter=1;

    while(token!=NULL)
    {
        token=strtok(NULL,div);
        if(token==NULL)
        {
            com[counter]=NULL;
            break;
        }
        com[counter]=token;
        counter++;
    }
    return counter;
}
void parse_args(int count,char* com[50],char* arg[20][20])
{
    int i;
    char *token;
    for(i=0; i<20; i++)
        numofargs[i]=0;
    if(((strstr(com[0]," ")==NULL)&&(count==1)))
    {
        token=strtok(com[0],"\n");
        numofargs[0]++;
        arg[0][0]=token;
        arg[0][1]=NULL;
        return;
    }
    for( i=0; i<count; i++)
    {
        token=strtok(com[i]," \n");
        arg[i][0]=token;
        int counter=1;
        while(token!=NULL)
        {
            token=strtok(NULL," \n");
            arg[i][counter]=token;
            numofargs[i]++;
            counter++;
            if(token==NULL)
            {
                arg[i][counter]=NULL;
            }
        }
    }

}
int execute_comm(char* div,int count,char* arg[20][20])
{
    int i,j,z,exitv=0;
    char dir;
    int check=0;
    int status[count],pid[count];
    if((div==";")||(div=="&&"))
    {
        for(i=0; i<count; i++)
        {
            check=0;
            if(strcmp(arg[i][0],"quit")==0) exit(1);
            if(numofargs[i]!=1)
            {
                for(z=1; z<numofargs[i]; z++)
                {
                    if((strstr(arg[i][z],">")!=NULL)||(strstr(arg[i][z],"<")!=NULL))
                    {
                        check=1;
                        if(strstr(arg[i][z],">")!=NULL) dir='>';
                        else if(strstr(arg[i][z],">")!=NULL) dir='<';
                        redirection(arg,i,z,dir);

                    }
                }
            }
            if(check==0)
            {
                pid[i]=fork();
                if(pid[i]==-1)perror("fork");
                if(pid[i]==0)
                {
                    if(execvp(arg[i][0],arg[i])==-1)
                    {
                        printf("\n");
                        printf("unknown command....please try again\n\n");
                        exit(1);
                    }
                }
                if(pid[i]>0)
                {
                    wait(&status[i]);
                    if(status[i]!=0)
                    {
                        perror("wait");
                        if(div=="&&")return exitv;
                    }
                }
            }
        }
    }

    else
    {
        check=0;
        if(strcmp(arg[0][0],"quit")==0)
        {
            exitv=2;
            return exitv;
        }
        int status,pid;
        if(numofargs[0]!=1)
        {
            for(z=0; z<numofargs[0]; z++)
            {
                if((strstr(arg[0][z],">")!=NULL)||(strstr(arg[0][z],"<")!=NULL))
                {
                    if(strstr(arg[0][z],">")!=NULL) dir='>';
                    else if(strstr(arg[0][z],">")!=NULL) dir='<';
                    redirection(arg,0,z,dir);
                    check=1;
                }
            }
        }
        if(check==0)
        {
            pid=fork();
            if(pid==0)
            {
                if(execvp(arg[0][0],arg[0])==-1)
                {
                    printf("\n");
                    printf("unknown command...please try again\n\n");
                    exit(1);
                }
            }
            if(pid>0)
            {
                wait(&status);
                if(status!=0)
                {
                    perror("wait");
                }
            }
            return exitv;
        }
    }
    return exitv;
}
void redirection(char* args[20][20],int j,int i,char dir)
{
    int status;
    int pid;
    if(dir=='>')
    {
        args[j][i]=strtok(args[j][i],">\n");
        pid=fork();
        if(pid==0)
        {
            FILE *ptr;
            ptr=fopen(args[j][i+1],"w");
            dup2(fileno(ptr), STDOUT_FILENO);
            if(execvp(args[j][0],args[j])==-1)
            {
                printf("\n");
                printf("unknown command...please try again\n\n");
                dup2(STDOUT_FILENO,fileno(ptr));
                fclose(ptr);
                exit(1);
            }
            dup2(STDOUT_FILENO,fileno(ptr));
            fclose(ptr);
        }
        if(pid>0)
        {
            wait(&status);
            if(status<0)
            {
                perror("wait");
            }
            return ;
        }
        return;
    }

    else
    {
        args[j][i]=strtok(args[j][i],"<\n");
        pid=fork();
        if(pid==0)
        {
            FILE *ptr;
            ptr=fopen(args[j][i+1],"r");
            dup2(fileno(ptr), STDIN_FILENO);
            if(execvp(args[j][0],args[j])==-1)
            {
                printf("\n");
                printf("unknown command,please try again...\n\n");
                dup2(STDIN_FILENO,fileno(ptr));
                fclose(ptr);
            }
            dup2(STDIN_FILENO,fileno(ptr));
            fclose(ptr);
        }

        if(pid>0)
        {
            wait(&status);
            if(status<0)
            {
                perror("wait");

            }

        }
    }

}
void execpipe(char* arg[20][20],int line)
{
    char* arg1[20];
    char* arg2[20];
    int status;
    int z,j;
    for(z=0; z<20; z++)
    {
        arg1[z]=NULL;
        arg2[z]=NULL;
    }

    for(int z=0; z<numofargs[line]; z++)
    {

        if(strstr(arg[line][z],"|")!=NULL)
        {
            arg[line][z]=strtok(arg[line][z],"|\n");
            arg1[z]=arg[line][z];
            arg1[z+1]=NULL;
            for(j=z+1; j<numofargs[line]; j++)
            {
                arg2[j-(z+1)]=arg[line][j];
            }
            arg2[j-z]=NULL;
            break;
        }
        arg1[z]=arg[line][z];
    }
    int pd[2];
    pipe(pd);
    int i;
    pid_t pid =fork();
    if (pid ==-1)
    {
        printf("error in forking...");
        return;
    }
    if(pid==0)
    {
        close(pd[1]);
        dup2(pd[0],0);
        if(execvp(arg2[0],arg2)==-1)
        {
            printf("\n");
            printf("unknown command,please try again...\n\n");
        }
        return;
    }
    else
    {
        close(pd[0]);
        dup2(pd[1],1);
        if(execvp(arg1[0],arg2)==-1)
        {
            printf("\n");
            printf("unknown command,please try again...\n\n");
        }
        return;
    }
}
