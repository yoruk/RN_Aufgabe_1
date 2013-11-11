Starten des Demoservers: java -jar DemoServer.jar
Starten der Aufgabe: ./aufgabe_1 127.0.0.1 <port fuer client> <port fuer server>

Das Programm ist jetzt komplett umgebaut.
Wie erklaere ich lieber persoenlich.

Der Server laueft noch nicht richtig.

Der Zugriff auf die Variablen: num_server und num_accesses 
in den Threads: client, server_handler, server und buffer_feeder
muss noch vernuenftig mit einem Mutex abgesichert werden.

Gruss, Michi