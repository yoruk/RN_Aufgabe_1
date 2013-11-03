Starten des Demoservers: java -jar DemoServer.jar
Starten der Aufgabe: ./aufgabe_1 127.0.0.1 5000

Der Buffer und der Client sind soweit fertig.
In den Buffer kann mit jeweils einer Funktion 
gelesen und geschrieben werden.

Ich hoffe die beiden Methoden arbeiten soweit richtig 
und erzeugen intern keine Speicherzugriffsfehler.
Falls doch mal einer auftaucht muss genau
geprueft werden ob es wirklich der Buffer war,
der den Fehler erzeugt hat.
Evtl. schreibe ich noch eine Funktion die den
Buffer und die Array-Indexe ueberwacht.

Den Client kann man auch ohne den Server starten
und er macht dann eine vorgegeben Anzahl von
Verbindungsversuchen. Das Selbe ebenfalls
bei Verbindungsabbruch.

Der Server fehlt noch ganz. 
Bis jetzt gibt es nur einen Dummy-Server
damit auch mal was vom Buffer gelesen wird,
also ein wechselseitiger Schreib-/Lesezugriff
zustande kommt. Merkwuerdigerweise erzeugt
der Dummy-Server 100% CPU-Auslastung.
Deswegen ist er in der main.c erstmal auskommentiert.

Weiss nicht ob ich am Sonntag noch Zeit finde weiterzumachen,
ansonsten geht es Montag und Dienstag weiter.

Gruss, Michi