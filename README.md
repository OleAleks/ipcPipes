# IPC Pipes - Dokumentation

Dieses Repository enthält ein Programm, das die Interprozesskommunikation (IPC) mit anonymen und benannten Pipes demonstriert. Das Programm ermöglicht die Kommunikation zwischen mehreren Prozessen, indem es Nachrichten über Pipes sendet, um einen korrekten Programmablauf sicherzustellen.

## Überblick

In diesem Projekt werden vier Prozesse mithilfe der `fork()`-Funktion erstellt. Diese Prozesse kommunizieren über anonyme und benannte Pipes. Die Hauptfunktion übernimmt die Prozesskreation, das Signalhandling und das Management der Pipes.

Die Prozesse im Programm sind:

- **CONV**: Generiert Zufallswerte und übergibt sie an die nächsten Prozesse.
- **STAT**: Liest Werte, berechnet Statistiken (Summe, Durchschnitt, Min, Max) und übergibt sie an den REPORT-Prozess.
- **REPORT**: Liest die berechneten Statistiken und gibt sie aus.
- **LOG**: Schreibt die generierten Werte in eine Datei.

## Hauptfunktion und Prozesskreation

Das Programm beginnt mit der Erstellung von vier Prozessen mithilfe von `fork()`. Die Hauptschleife läuft so lange, wie die Variable `programm_running` den Wert `true` hat. Das Programm behandelt auch Signalinterrupts (über Ctrl+C), sodass das Programm sicher gestoppt werden kann.

Die Hauptfunktion:

1. **Forks Prozesse**: Zwei Aufrufe von `fork()` erzeugen vier Prozesse.
2. **Signalbehandlung**: Wenn Ctrl+C gedrückt wird, wird der Signalhandler aufgerufen, um `programm_running` auf `false` zu setzen und die Schleife zu beenden.
3. **Erstellt Pipes**: Anonyme und benannte Pipes werden erstellt, um die Kommunikation zwischen den Prozessen zu ermöglichen.

## Pipe-Typen

### Anonyme Pipes

- **Beschreibung**: Anonyme Pipes ermöglichen die Kommunikation zwischen Prozessen, die eine Eltern-Kind-Beziehung haben.
- **Dateideskriptoren**: Die Dateideskriptoren `fd[0]` (zum Lesen) und `fd[1]` (zum Schreiben) werden verwendet, um mit den Pipes zu interagieren.
- **Funktionen**:
  - `create_pipe()`: Erstellt eine anonyme Pipe.
  - `write_to_pipe()`: Schreibt Daten in die Pipe.
  - `read_from_pipe()`: Liest Daten aus der Pipe.
- **Synchronisation**: Prozesse werden blockiert, wenn sie aus einer leeren Pipe lesen oder in eine volle Pipe schreiben wollen. Unbenutzte Enden der Pipes werden vor dem Zugriff geschlossen.
- **Bereinigung**: Die Pipes werden automatisch entfernt, nachdem der letzte Prozess sie nicht mehr benötigt.

### Benannte Pipes

- **Beschreibung**: Benannte Pipes (FIFOs) ermöglichen die Kommunikation zwischen nicht verwandten Prozessen.
- **Dateideskriptoren**: Sie werden außerhalb der Hauptfunktion mit dem Wert -1 initialisiert und müssen mit der `open()`-Funktion unter Angabe des Pipe-Namens und des Zugriffsmodus geöffnet werden.
- **Funktionen**:
  - `create_named_pipe()`: Erstellt eine benannte Pipe mit `mkfifo()` und führt eine Fehlerüberprüfung durch.
  - `unlink()`: Benannte Pipes müssen explizit nach der Nutzung entfernt werden, und diese Funktion sorgt dafür, dass sie nach der Kommunikation gelöscht werden.
- **Bereinigung**: Benannte Pipes müssen manuell mit `unlink()` entfernt werden, um zu verhindern, dass sie im Dateisystem verbleiben.

## Ablauf der Prozesse

### CONV-Prozess

- **Rolle**: Generiert Zufallswerte und übergibt sie über Pipes an die STAT- und LOG-Prozesse.
- **Aktionen**:
  - Generiert Zufallswerte und speichert sie in einem Array `konverterWerte`.
  - Schreibt die Anzahl der Werte und das Array in die Pipes `fd` und `fd2`.

### STAT-Prozess

- **Rolle**: Empfängt Werte vom CONV-Prozess, berechnet Statistiken und übergibt sie an den REPORT-Prozess.
- **Aktionen**:
  - Liest das Array und dessen Länge aus der Pipe.
  - Berechnet die Summe, den Durchschnitt, das Minimum und das Maximum der Werte.
  - Überträgt die Statistiken an den REPORT-Prozess über die Pipe `fd3`.

### REPORT-Prozess

- **Rolle**: Empfängt die Statistiken vom STAT-Prozess und gibt sie aus.
- **Aktionen**:
  - Liest die Statistiken aus der Pipe.
  - Gibt die Summe, den Durchschnitt, das Minimum und das Maximum auf der Konsole aus.

### LOG-Prozess

- **Rolle**: Empfängt die Werte vom CONV-Prozess und schreibt sie in eine Datei.
- **Aktionen**:
  - Liest die Werte aus der Pipe.
  - Schreibt die Werte in eine Datei mit dem Namen `konverterWerte.txt`.
