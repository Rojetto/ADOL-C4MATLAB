@ECHO OFF
REM ****************************************************************************
REM *** Name:           BuildRunVC.bat                                       ***
REM ***                                                                      ***
REM *** Beschreibung:   Verarbeiten des Compileraufrufs aus MATLAB heraus    ***
REM ***                 (Setzen der Umgebungsvariablen f�r cl.exe)           ***
REM ***                 Ausf�hren der TapeFactory aus MATLAB heraus          ***
REM ***                 (Setzen des Pfades zu ADOL-C)                        ***
REM ***	                                                                     ***
REM *** Parameter:      %1 - Modusauswahl zw. �bersetzen und Ausf�hren       ***
REM ***                 %2 - vollst�ndiger Pfad zum Visual Studio Basis-     ***
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
REM *** R�ckgabewerte:  keine                                                ***
REM ***                                                                      ***
REM *** (c) 2011-2012                                                        ***
REM *** Carsten Friede, Jan Winkler                                          ***
REM *** Institut f�r Regelungs- und Steuerungstheorie, TU Dresden            ***
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

REM Auslesen der �bergebenen Argumente %1 und %3 an BuildVC.bat
REM Speichern in lokale Variablen und Entfernen der Anf�hrungszeichen in %3
REM andernfalls werden die Argumente f�r cl.exe nicht erkannt

if "%BITS%" == "64" ( set CC=%VCPATH%\VC\bin\amd64\cl.exe ) else ( set CC=%VCPATH%\VC\bin\cl.exe )
set CARGS=%3
set CARGS=%CARGS:~1,-1%

echo %CC%

REM Compileraufruf
%CC% %CARGS%

exit %ERRORLEVEL%

:Ausfuehren
REM Hinzuf�gen des Pfades zur Bibliothek AdolC.dll
set DLLPATH=%

set PATH=%PATH%;%DLLPATH%

%3

exit %ERRORLEVEL%