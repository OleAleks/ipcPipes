// Name:          BSRN Werkstück A - Alternative 1
// Beschreibung:  Interprozesskommunikation zwischen 4 mit fork() erzeugten Prozessen mittels benannter Pipes
// Autor:         Aleksander Olewinski
// Datum:         25.06.2023
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

bool program_running = true;

int fd1 = -1;
int fd2 = -1;
int fd3 = -1;

char *pipe1 = "pipe1";
char *pipe2 = "pipe2";
char *pipe3 = "pipe3";

void signal_handler(int num)
{
    signal(SIGINT, SIG_DFL);
    printf("\nProgramm wurde abgebrochen\n");
    program_running = false;
    
    // Pipes schließen
    if (fd1 != -1) {
        unlink(pipe1);
        close(fd1);
    }
    if (fd2 != -1) {
        unlink(pipe2);
        close(fd2);
    }
    if (fd3 != -1) {
        unlink(pipe3);
        close(fd3);
    }
}

void create_named_pipe(char *pipe_name)
{
    unlink(pipe_name); // Pipe löschen, falls sie bereits existiert

    if (mkfifo(pipe_name, 0666) == -1)
    {
        perror("Fehler beim Erstellen der benannten Pipe");
        exit(1);
    }
}

int open_named_pipe(char *pipe_name, int mode)
{
    int pipe_fd = open(pipe_name, mode);
    if (pipe_fd == -1)
    {
        perror("Fehler beim Öffnen der benannten Pipe");
        exit(1);
    }
    return pipe_fd;
}

//size_t kann die maximale Größe möglichen Objekts eines beliebigen Typs (einschließlich Array) speichern
void write_to_pipe(int fd, void *werte, size_t size)
{
    if (write(fd, werte, size) < 0)
    {
        perror("Fehler beim Schreiben in die Pipe");
        exit(1);
    }
}

void read_from_pipe(int pipe_fd, void *buffer, size_t size)
{
    if (read(pipe_fd, buffer, size) < 0)
    {
        perror("Fehler beim Lesen aus der Pipe");
        exit(1);
    }
}

int main(void)
{
    signal(SIGINT, signal_handler);

    while (program_running)
    {

        create_named_pipe(pipe1);
        create_named_pipe(pipe2);
        create_named_pipe(pipe3);

        int prozess_id = fork();
        int prozess_id2 = fork();

        if (prozess_id > 0 && prozess_id2 > 0)
        {   // CONV
            //printf("CONV PID:%d, PPID:%d\n", getpid(), getppid());
            int konverterWerte[15];
            int arrayLaenge = 15;
            srand(time(NULL));
            
            // Zufallszahlen zwischen 1 und 10 generieren und in konverterWerte[] speichern

            for (int i = 0; i < arrayLaenge; i++)
            {
                konverterWerte[i] = rand() % 100 + 1;
            }

            fd1 = open_named_pipe(pipe1, O_WRONLY);
            fd2 = open_named_pipe(pipe2, O_WRONLY);

            write_to_pipe(fd1, &arrayLaenge, sizeof(int));
            write_to_pipe(fd1, konverterWerte, sizeof(int) * arrayLaenge);
            write_to_pipe(fd2, &arrayLaenge, sizeof(int));
            write_to_pipe(fd2, konverterWerte, sizeof(int) * arrayLaenge);

            close(fd1);
            close(fd2);
        }
        else if (prozess_id == 0 && prozess_id2 > 0)
        { // STAT Prozess
            //printf("STAT PID:%d, PPID:%d\n", getpid(), getppid());
            int array[15];
            int arrayLaenge;
            int summe = 0;
            float mittelwert;
            int minimum = 100;
            int maximum = 0;

            fd1 = open_named_pipe(pipe1, O_RDONLY);

            read_from_pipe(fd1, &arrayLaenge, sizeof(int));
            read_from_pipe(fd1, &array, sizeof(int) * arrayLaenge);

            close(fd1);

            for (int i = 0; i < arrayLaenge; i++)
            {
                summe += array[i];
                if (array[i] > maximum)
                {
                    maximum = array[i];
                }

                if (array[i] < minimum)
                {
                    minimum = array[i];
                }
            }
            mittelwert = (float)summe / arrayLaenge;

            fd3 = open_named_pipe(pipe3, O_WRONLY);

            write_to_pipe(fd3, &summe, sizeof(int));
            write_to_pipe(fd3, &mittelwert, sizeof(float));
            write_to_pipe(fd3, &minimum, sizeof(int));
            write_to_pipe(fd3, &maximum, sizeof(int));

            close(fd3);
            return 0;
        }
        else if (prozess_id == 0 && prozess_id2 == 0)
        { // REPORT
            //printf("REPORT PID:%d, PPID:%d\n", getpid(), getppid());
            fd3 = open_named_pipe(pipe3, O_RDONLY);

            int summe;
            float mittelwert;
            int minimum;
            int maximum;

            read_from_pipe(fd3, &summe, sizeof(int));
            read_from_pipe(fd3, &mittelwert, sizeof(float));
            read_from_pipe(fd3, &minimum, sizeof(int));
            read_from_pipe(fd3, &maximum, sizeof(int));

            close(fd3);
            printf("Mittelwert: %0.2f\n", mittelwert);
            printf("Summe: %i\n", summe);
            printf("Minimum: %d\n", minimum);
            printf("Maximum: %d\n", maximum);
            return 0;
        }
        else
        {   
            //printf("LOG PID:%d, PPID:%d\n", getpid(), getppid());
            int arrayLaenge;
            int array[15];

            fd2 = open_named_pipe(pipe2, O_RDONLY);

            read_from_pipe(fd2, &arrayLaenge, sizeof(int));
            read_from_pipe(fd2, array, sizeof(int) * arrayLaenge);

            close(fd2);

            FILE *file = fopen("konverterWerte.txt", "a");

            if (file == NULL)
            {
                printf("Fehler beim Erstellen der Datei\n");
            }
            for (int j = 0; j < arrayLaenge; j++)
            {
                fprintf(file, "%d,", array[j]);
            }
            fprintf(file, "\n");
            fclose(file);
            return 0;
        }
        printf("\n");
        sleep(2);

        //Pipes entbinden und schließen zur Sicherheit
        if (fd1 != -1) {
        unlink(pipe1);
        close(fd1);
        }
        if (fd2 != -1) {
        unlink(pipe2);
        close(fd2);
        }
        if (fd3 != -1) {
        unlink(pipe3);
        close(fd3);
        }
    }
    return 0;
}
