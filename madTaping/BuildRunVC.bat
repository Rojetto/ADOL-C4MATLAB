@ECHO OFF
REM ****************************************************************************
REM *** Name:           BuildRunVC.bat                                       ***
REM ***                                                                      ***
REM *** Beschreibung:   Verarbeiten des Compileraufrufs aus MATLAB heraus    ***
REM ***                 (Setzen der Umgebungsvariablen für cl.exe)           ***
REM ***                 Ausführen der TapeFactory aus MATLAB heraus          ***
REM ***                 (Setzen des Pfades zu ADOL-C)                        ***
REM ***	                                                                     ***
REM *** Parameter:      %1 - Modusauswahl zw. Übersetzen und Ausführen       ***
REM ***                 %2 - vollständiger Pfad zum Visual Studio Basis-     ***
REM ***                      verzeichnis, z.B.                               ***
REM ***                      C:\Program Files\Microsoft Visual Studio 9.0    ***
REM ***                      (Modus c)                                       ***
REM ***                      Pfad zur Bibliothek AdolC.dll (Modus r)         ***
REM ***                 %3 - String mit allen Compiler- und Linkerargumenten ***
REM ***                      (Modus c)                                       ***
REM ***                      Name der erzeugten EXE-Datei aus TapeFactory    ***
REM ***                      (Modus r)                                       ***
REM ***                 %4 - Auswahl zw. 32 und 64 Bit                       ***
REM ***                      32 bzw. 64										 ***
REM ***	                                                                     ***
REM *** Rückgabewerte:  keine                                                ***
REM ***                                                                      ***
REM *** (c) 2011-2012                                                        ***
REM *** Carsten Friede, Jan Winkler                                          ***
REM *** Institut für Regelungs- und Steuerungstheorie, TU Dresden            ***
REM *** Jan.Winkler@tu-dresden.de                                            ***
REM ****************************************************************************

set MODE=%1
set VCPATH=%2
set BITS=%4

if "%MODE%" == "c" GOTO Uebersetzen
if "%MODE%" == "r" GOTO Ausfuehren


:Uebersetzen
REM Setzen der Umgebungsvariablen wie in Visual Studio 2008 Eingabeaufforderung
if "%BITS%" == "64" ( call %VCPATH%\VC\vcvarsall.bat amd64 ) else  ( call %VCPATH%\VC\vcvarsall.bat x86 )

REM Auslesen der übergebenen Argumente %1 und %3 an BuildVC.bat
REM Speichern in lokale Variablen und Entfernen der Anführungszeichen in %3
REM andernfalls werden die Argumente für cl.exe nicht erkannt

if "%BITS%" == "64" ( set CC=%VCPATH%\VC\bin\amd64\cl.exe ) else ( set CC=%VCPATH%\VC\bin\cl.exe )
set CARGS=%3
set CARGS=%CARGS:~1,-1%

echo %CC%

REM Compileraufruf
%CC% %CARGS%

exit %ERRORLEVEL%

:Ausfuehren
REM Hinzufügen des Pfades zur Bibliothek AdolC.dll
set DLLPATH=%

set PATH=%PATH%;%DLLPATH%

%3

exit %ERRORLEVEL%