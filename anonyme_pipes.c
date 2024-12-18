// Name:          BSRN Werkstück A - Alternative 1
// Beschreibung:  Interprozesskommunikation zwischen 4 mit fork() erzeugten Prozessen mittels anonymer Pipes
// Autor:         Aleksander Olewinski
// Datum:         25.06.2023
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

bool program_running = true;

void signal_handler(int num)
{
    signal(SIGINT, SIG_DFL);
    printf("\nProgramm wurde abgebrochen\n");
    program_running = false;
}

void create_pipe(int fd[2])
{
    if (pipe(fd) == -1)
    {
        perror("Fehler beim Öffnen der Pipe");
        exit(1);
    }
}

// size_t kann die maximale Größe möglichen Objekts eines beliebigen Typs (einschließlich Array) speichern
void write_to_pipe(int fd, void *werte, size_t size)
{
    if (write(fd, werte, size) < 0)
    {
        perror("Fehler beim Schreiben in die Pipe");
        exit(1);
    }
}

void read_from_pipe(int fd, void *buffer, size_t size)
{
    if (read(fd, buffer, size) < 0)
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
        // Pipes fd & fd2 (file descriptor) deklarieren [0] zum lesen & [1] zum schreiben
        int fd[2];
        int fd2[2];
        int fd3[2];

        create_pipe(fd);
        create_pipe(fd2);
        create_pipe(fd3);

        int prozess_id = fork();
        int prozess_id2 = fork();

        if (prozess_id > 0 && prozess_id2 > 0)
        {   // CONV
            int konverterWerte[15];
            int arrayLaenge = 15;
            srand(time(NULL));

            // Zufallszahlen zwischen 1 und 10 generieren und in konverterWerte[] speichern

            // printf("Werte: ");
            for (int i = 0; i < arrayLaenge; i++)
            {
                konverterWerte[i] = rand() % 100 + 1;
                // printf("%d ", konverterWerte[i]);
            }
            // printf("\n");
            // printf("Conv: PID: %i\n", getpid());
            // printf("Main PID: %d\n", getpid());
            //  Lesekanal der Pipes schließen
            close(fd[0]);
            close(fd2[0]);

            // Anzahl an Werten & danach Array in Pipes schreiben
            write_to_pipe(fd[1], &arrayLaenge, sizeof(int));
            write_to_pipe(fd[1], konverterWerte, sizeof(int) * arrayLaenge);
            write_to_pipe(fd2[1], &arrayLaenge, sizeof(int));
            write_to_pipe(fd2[1], konverterWerte, sizeof(int) * arrayLaenge);

            // Schreibkanal der Pipes schließen
            close(fd[1]);
        }
        else if (prozess_id == 0 && prozess_id2 > 0)
        { // STAT Prozess
            // printf("STAT Prozess\n");
            // printf("Stat PID: %i\n", getpid());
            // printf("Ich bin das Kind von PID: %i\n", getppid());

            int array[15];
            int arrayLaenge;
            int summe = 0;
            float mittelwert;
            int minimum = 100;
            int maximum = 0;

            close(fd[1]);
            // Länge & Werte des Arrays von CONV auslesen

            read_from_pipe(fd[0], &arrayLaenge, sizeof(int));
            read_from_pipe(fd[0], &array, sizeof(int) * arrayLaenge);

            // printf("Werte: ");
            for (int j = 0; j < arrayLaenge; j++)
            {
                // printf("%i ", array[j]);
            }
            // printf("\n");
            close(fd[0]);

            // Mittelwert, Summe, Min, Max der Werte berechnen
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

            // printf("Mittelwert: %f, Summe: %d\n", mittelwert, summe);

            close(fd3[0]);


            write_to_pipe(fd3[1], &summe, sizeof(int));
            write_to_pipe(fd3[1], &mittelwert, sizeof(float));
            write_to_pipe(fd3[1], &minimum, sizeof(float));
            write_to_pipe(fd3[1], &maximum, sizeof(float));
            
            // printf("-----\n");
            return 0;
            }
        else if (prozess_id == 0 && prozess_id2 == 0)
        { // REPORT
            // printf("REPORT Prozess\n");
            // printf("Report PID: %i\n", getpid());
            // printf("Ich bin das Kind von PID: %i\n", getppid());

            close(fd3[1]);

            int summe;
            float mittelwert;
            int minimum;
            int maximum;

            // Lesen der länge des Arrays & der Werte aus Pipe 3
            read_from_pipe(fd3[0], &summe, sizeof(int));
            read_from_pipe(fd3[0], &mittelwert, sizeof(float));
            read_from_pipe(fd3[0], &minimum, sizeof(float));
            read_from_pipe(fd3[0], &maximum, sizeof(float));

            close(fd3[0]);
            printf("Mittelwert: %0.2f\n", mittelwert);
            printf("Summe: %i\n", summe);
            printf("Minimum: %d\n", minimum);
            printf("Maximum: %d\n", maximum);
            // printf("-----\n");
            return 0;
        }
        else
        {
            // LOG
            // printf("LOG Prozess\n");
            // printf("LOG PID: %i\n", getpid());
            // printf("Ich bin das Kind von PID: %i\n", getppid());
            int arrayLaenge;
            int array[15];

            // Einlesen der Werte aus CONV
            close(fd2[1]);
            
            read_from_pipe(fd2[0], &arrayLaenge, sizeof(int));
            read_from_pipe(fd2[0], array, sizeof(int) * arrayLaenge);
            
            close(fd2[0]);

            // Datei erstellen & Conv-Werte reinschreiben
            FILE *file = fopen("konverterWerte.txt", "a");

            if (file == NULL)
            {
                printf("Fehler beim Ersteller der Datei\n");
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
    }
    return 0;
}
