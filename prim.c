// Дерево процессов 13. 1->(2,3,4,5) 5->(6,7)   7->8   
// Последовательность обмена сигналами 11.  1->(8,7) SIGUSR1   8->(6,5) SIGUSR1  5->(4,3,2) SIGUSR2   2->1 SIGUSR2

#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <libgen.h> 

pid_t pid_number;
char* scriptName;
int counter_1_proc = 0;
int counter = 0;
int counter_USR1 = 0, counter_USR2 = 0;

int get_time_in_millis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (int) (currentTime.tv_usec % 1000L);
}

void pid_to_file(pid_t pid, char* numb)
{
    FILE* file;
    char filename[8] = "pid";
    strcat(filename,numb);
    strcat(filename, ".txt");
    file = fopen(filename, "w");
    if(file == NULL)
    {
        fprintf(stderr, "%d : %s : %s :%s\n", getpid(), scriptName, strerror(errno), filename);
        errno = 0;
        exit(0);
    }
    fprintf(file,"%d", pid);
    if (fclose(file) == EOF)
    {
        fprintf(stderr, "%d : %s : %s : %s\n", getpid(), scriptName, strerror(errno), filename);
        errno = 0;
        exit(0);
    }	
}  

pid_t pid_from_file(const char *path) {
    FILE *file = fopen(path, "r+");
    if (!file) 
    {
        fprintf(stderr, "%d %s: %s: %s\n", getpid(), scriptName, strerror(errno), path); 
        errno = 0;
        exit(0);
    }

    char* buf = malloc(sizeof(long)*8);
    if (!fgets(buf, sizeof(long)*8, file))
    {
        fprintf(stderr, "%d %s: %s: %s\n", getpid(), scriptName, strerror(errno), path); 
        errno = 0;
        exit(0);
    }

    pid_t pid = (pid_t) strtol(buf, NULL, 10);

    if (fclose(file)) 
    {
        fprintf(stderr, "%d %s: %s: %s\n", getpid(), scriptName, strerror(errno), path); 
        errno = 0;
        exit(0);
    }
    return pid;
}


void remove_files()
{
    char filename[14] = "pid0.txt";
    for (int i = 1; i < 9; i++) {
        errno = 0;
        filename[3] = i + '0';
        if (remove(filename) == -1) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), scriptName, strerror(errno), filename);
        }
    }
}

void hdl_proc_8(int signal)
{
    if(signal == SIGUSR1)
    {
        fprintf(stdout,"8 %d %d получил SIGUSR1 %u\n", getpid(), getppid(), get_time_in_millis());
        fprintf(stdout,"8 %d %d послал SIGUSR1 %u\n", getpid(), getppid(), get_time_in_millis());
        counter_USR1++;
        fflush(stdout);
        killpg(pid_from_file("pid5.txt"), SIGUSR1);
    }
    else
    {
        fprintf(stdout,"8 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
        killpg(pid_from_file("pid5.txt"), SIGTERM);
        fflush(stdout);
        exit(0);
    }
}

void hdl_proc_7(int signal)
{
    if(signal == SIGUSR1)
    {
        fprintf(stdout,"7 %d %d получил SIGUSR1 %u\n", getpid(), getppid(), get_time_in_millis());
        fflush(stdout);
    }
    else
    {
        kill(pid_from_file("pid8.txt"), SIGTERM);
        wait(0);
        fprintf(stdout,"7 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
        fflush(stdout);
        exit(0);
    }
}

void hdl_proc_6(int signal)
{
    if(signal == SIGUSR1)
    {
        fprintf(stdout,"6 %d %d получил SIGUSR1 %u\n", getpid(), getppid(), get_time_in_millis());
        fflush(stdout);
    }
    else
    {
        fprintf(stdout,"6 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
        fflush(stdout);
        exit(0);
    }
}

void hdl_proc_5(int signal)
{
    if(signal == SIGUSR1)
    {
        fprintf(stdout,"5 %d %d получил SIGUSR1 %u\n", getpid(), getppid(), get_time_in_millis());
        fprintf(stdout,"5 %d %d послал SIGUSR2 %u\n", getpid(), getppid(), get_time_in_millis());
        counter_USR2++;
        fflush(stdout);
        killpg(pid_from_file("pid2.txt"), SIGUSR2);
    }
    else
    {
        kill(pid_from_file("pid6.txt"), SIGTERM);
        kill(pid_from_file("pid7.txt"), SIGTERM);
        for(int i = 0; i < 2; i++)
        {
            wait(0);
        }
        fprintf(stdout,"5 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
        fflush(stdout);
        exit(0);
    }
}

void hdl_proc_4(int signal)
{
    if(signal == SIGUSR2)
    {
        fprintf(stdout,"4 %d %d получил SIGUSR2 %u\n", getpid(), getppid(), get_time_in_millis());
        fflush(stdout);
    }
    else
    {
        fprintf(stdout,"4 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
        fflush(stdout);
        exit(0);
    }
}

void hdl_proc_3(int signal)
{
    if(signal == SIGUSR2)
    {
        fprintf(stdout,"3 %d %d получил SIGUSR2 %u\n", getpid(), getppid(), get_time_in_millis());
        fflush(stdout);
    }
    else
    {
        if(signal == SIGTERM)
        {
            fprintf(stdout,"3 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
            fflush(stdout);
            exit(0);
        }
    }
}

void hdl_proc_2(int signal)
{
    if(signal == SIGUSR2)
    {
        fprintf(stdout,"2 %d %d получил SIGUSR2 %u\n", getpid(), getppid(), get_time_in_millis());
        fprintf(stdout,"2 %d %d послал SIGUSR2 %u\n", getpid(), getppid(), get_time_in_millis());
        counter_USR2++;
        fflush(stdout);
        kill(pid_from_file("pid1.txt"),SIGUSR2);
    }
    else
    {
        if(signal == SIGTERM)
        {
            fprintf(stdout,"2 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
            fflush(stdout);
            exit(0);
        }
    }
}

void hdl_proc_1(int signal)
{
    static int counter = 0;
    if(signal == SIGUSR2)
    {   
        fprintf(stdout,"1 %d %d получил SIGUSR2 %d\n", getpid(), getppid(), get_time_in_millis());
        counter++;
        if(counter == 101)
        {
            raise(SIGTERM);
        }
        fprintf(stdout,"1 %d %d послал SIGUSR1 %d\n", getpid(), getppid(), get_time_in_millis());
        counter_USR1++;
        fflush(stdout);
        killpg(pid_from_file("pid7.txt"),SIGUSR1);
    }
    else
    {
        if(signal == SIGTERM)
        {
            kill(pid_from_file("pid2.txt"), SIGTERM);
            kill(pid_from_file("pid3.txt"), SIGTERM);
            kill(pid_from_file("pid4.txt"), SIGTERM);
            kill(pid_from_file("pid5.txt"), SIGTERM);
            for(int i = 0; i < 4; i++)
            {
                wait(0);
            }
            fprintf(stdout,"1 %d %d завершил работу после %d сигнала SIGUSR1 и %d сигнала SIGUSR2 %d\n", getpid(), getppid(),counter_USR1,counter_USR2, get_time_in_millis());
            fflush(stdout);
            exit(0);
        }
    }
}

int main(int argc, char* argv[])
{
    scriptName = basename(argv[0]);
    errno = 0;
    pid_t pid1 = fork();
    if(pid1 == -1)
    {
        fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
        errno = 0;
    }
    if(pid1 == 0)
    {
        printf("1 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
        pid_number = 1;
        pid_t pid2 = fork();
        if(pid2 == -1)
        {
            fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
            errno = 0;
        }
        if(pid2 == 0)
        {
            setpgid(getpid(),getpid());
            printf("2 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
            pid_number = 2;
        }
        else
        {
            pid_t pid3 = fork();
            if(pid3 == -1)
            {
                fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                errno = 0;
            }
            if(pid3 == 0)
            {
                printf("3 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                pid_number = 3;
            }
            else
            {
                pid_t pid4 = fork();
                if(pid4 == -1)
                {
                    fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                    errno = 0;
                }
                if(pid4 == 0)
                {
                    printf("4 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                    pid_number = 4;
                }
                else
                {
                    pid_t pid5 = fork();
                    
                    if(pid5 == -1)
                    {
                        fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                        errno = 0;
                    }
                    if(pid5 == 0)
                    {
                        setpgid(getpid(),getpid());
                        printf("5 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                        pid_number = 5;

                        pid_t pid6 = fork();
                        if(pid6 == -1)
                        {
                            fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                            errno = 0;
                        }
                        if(pid6 == 0)
                        {
                            printf("6 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                            pid_number = 6;
                        }
                        else
                        {
                            pid_t pid7 = fork();
                            
                            if(pid7 == -1)
                            {
                                fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                                errno = 0;
                            }
                            if(pid7 == 0)
                            {
                                setpgid(getpid(),getpid());
                                printf("7 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                                pid_number = 7;

                                pid_t pid8 = fork();
                                
                                if(pid8 == -1)
                                {
                                    fprintf(stderr, "%d : %s : %s\n", getpid(), scriptName, strerror(errno));
                                    errno = 0;
                                }
                                if(pid8 == 0)
                                {
                                    printf("8 %d %d %d\n", getpid(), getppid(), get_time_in_millis());
                                    pid_number = 8;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        wait(0);
        remove_files();
        return 0;
    }

    if(pid_number == 1)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_1;
        sigaction(SIGUSR2, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 2)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_2;
        sigaction(SIGUSR2, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 3)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_3;
        sigaction(SIGUSR2, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 4)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_4;
        sigaction(SIGUSR2, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 5)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_5;
        sigaction(SIGUSR1, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 6)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_6;
        sigaction(SIGUSR1, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 7)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_7;
        sigaction(SIGUSR1, action, 0);
        sigaction(SIGTERM, action, 0);
    }
    if(pid_number == 8)
    {
        struct sigaction* action;
        action = malloc(sizeof(struct sigaction));
        action->sa_handler = hdl_proc_8;
        sigaction(SIGUSR1, action, 0);
        sigaction(SIGTERM, action, 0);
    }


    if(pid_number == 8)
    {
        pid_to_file(getpid(), "8");
        setpgid(getpid(), pid_from_file("pid7.txt"));
    }
    if(pid_number == 7)
    {
        pid_to_file(getpid(), "7");
    }
    if(pid_number == 6)
    {
        pid_to_file(getpid(), "6");
        struct stat buf;
        while (stat("pid5.txt", &buf) == -1);
        setpgid(getpid(), pid_from_file("pid5.txt"));
    }

    if(pid_number == 5)
    {
        pid_to_file(getpid(), "5");
        setpgid(getpid(),getpid());
    }

    if(pid_number == 4)
    {
        pid_to_file(getpid(), "4");
        setpgid(getpid(), pid_from_file("pid2.txt"));
    }

    if(pid_number == 3)
    {
        pid_to_file(getpid(),"3");
        struct stat buf;
        while (stat("pid2.txt", &buf) == -1);
        setpgid(getpid(), pid_from_file("pid2.txt"));
    }
 
    if(pid_number == 2)
    {
        pid_to_file(getpid(), "2");
    }

    if(pid_number == 1)
    {
        pid_to_file(getpid(), "1");
        struct stat buf;
        while (stat("pid8.txt", &buf) == -1);
        fprintf(stdout,"1 %d %d послал SIGUSR1 %d\n", getpid(), getppid(), get_time_in_millis());
        killpg(pid_from_file("pid7.txt"), SIGUSR1);
        
    }

    while(1)
    {
        pause();
    }
 
    return 0;
}