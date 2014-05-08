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
�ffnet Tape f�r Nutzung in Matlab (Klartextnamen-
Tape wird dabei in f�r ADOL-C gebr�uchliche Nomenklatur umgewandelt)

madTapeClose -> 
Schlie�t das Tape wieder

madForward -> Wrapper f�r forward
madFunction -> Wrapper f�r function
madGradient -> Wrapper f�r gradient
madHessian -> Wrapper f�r hessian (voll ausgef�llt)
madInverse
madJacobian -> Wrapper f�r jacobian
madLagrange
madLieBracket
madLieCovector
madLieDeriv
madLieGemischt
madLieGradient
madLieScalar
madReverse -> Wrapper f�r reverse


Folgende Hilfsdateien sind in dem Paket enthalten und
d�rfen nicht gel�scht werden:

BuildRunVC.bat -> Um Microsoft-Compiler aus Matlab anzusteuern
TapingTemplate.cpp -> Template, das madTapeCreate ben�tigt
madTapingSettings -> Einstellungen f�r den Compilierprozess