README:

Voraussetzungen:
- Installierter Microsoft Visual C++ Compiler
- ADOL-C Quellen
- Kompilierte adolc.dll

Schritt 1:
Anpassen der Pfade in der Datei ~\madTaping\madTapingSettings.m
(Hinweise siehe dort)

Schritt 2:
Erstellen des Funktionscodes der zu tapenden Funktion.
Zwei Beispiele sind in XSinXY.m und in Speelpenning.m
gegeben. Infos hierzu liefert auch "help madTapeCreate"

Schritt 3:
Wenn die Tapes erstellt sind, kann mit den Treibern auf
diesen gearbeitet werden.

In Example.m ist ein komplettes Beispiel gegeben.

Paket besteht derzeit aus folgenden Dateien:

madTapeCreate -> 
Erzeugt Tape aus Codesnippet, speichert es
mit Klartextnamen ab

madTapeOpen -> 
Öffnet Tape für Nutzung in Matlab (Klartextnamen-
Tape wird dabei in für ADOL-C gebräuchliche Nomenklatur umgewandelt)

madTapeClose -> 
Schließt das Tape wieder

madForward -> Wrapper für forward
madFunction -> Wrapper für function
madGradient -> Wrapper für gradient
madHessian -> Wrapper für hessian (voll ausgefüllt)
madInverse
madJacobian -> Wrapper für jacobian
madLagrange
madLieBracket
madLieCovector
madLieDeriv
madLieGemischt
madLieGradient
madLieScalar
madReverse -> Wrapper für reverse


Folgende Hilfsdateien sind in dem Paket enthalten und
dürfen nicht gelöscht werden:

BuildRunVC.bat -> Um Microsoft-Compiler aus Matlab anzusteuern
TapingTemplate.cpp -> Template, das madTapeCreate benötigt
madTapingSettings -> Einstellungen für den Compilierprozess