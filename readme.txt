Starten des Demoservers: java -jar DemoServer.jar
Starten der Aufgabe: ./aufgabe_1 127.0.0.1 5000

Der Server funktioniert wenn auch vorerst nur mit einem Client.
Das Ganze ist im Moment noch ein wenig mit der heissen Nadel gestrickt.
Fehler werden im Moment so gut wie garnicht abgefangen.

Lass den Server erstmal aussen vor, 
den mache ich noch bis Mittwoch fertig.

Schau dir aber bitte mal den Puffer und dessen Arbeitsweise an.
Es gibt keinen Nachlauf zum Server, also bei X Bildern im Puffer
laeuft der Client nicht X Bilder hinterher, was ich erwarten wuerde.
Das heisst, das auch bei Verbindungsproblemen der Client nicht noch
einige Zeit ohne den Server auskommt.

Ist meiner Meinung nach nichts kritisches, habe das auch schon
bei den Anderen gesehen. Das war aber ein Feature was ich
eigentlich eingeplant hatte und ich wuesste gerne warum das nicht laeuft.  

Gruss, Michi