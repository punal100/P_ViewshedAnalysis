@ECHO OFF
REM This script provides Windows CMD equivalents for common Git diffing tasks.
REM It generates two text files: UnDiffed_output.txt and Diffed_output.txt.

ECHO Generating Undiffed (BASE vs WORKSPACE) output...

REM === Command 1: Output BASE and WORKSPACE content for each changed file ===
(FOR /F "tokens=*" %%F IN ('git diff --name-only') DO (
    ECHO.
    ECHO === %%F ^(BASE^) Start ===
    git show HEAD:"%%F"
    ECHO === %%F ^(BASE^) End ===
    ECHO.
    ECHO === %%F ^(WORKSPACE^) Start ===
    IF EXIST "%%F" (
        type "%%F"
    ) ELSE (
        ECHO File deleted in workspace
    )
    ECHO === %%F ^(WORKSPACE^) End ===
    ECHO.
)) > UnDiffed_output.txt

ECHO Successfully created UnDiffed_output.txt
ECHO.
ECHO Generating Diffed output...

REM === Command 2: Output the 'git diff' for each changed file ===
(FOR /F "tokens=*" %%F IN ('git diff --name-only') DO (
    ECHO.
    ECHO === %%F Diff Start ===
    git diff -- "%%F"
    ECHO === %%F Diff End ===
    ECHO.
)) > Diffed_output.txt

ECHO Successfully created Diffed_output.txt
ECHO.
ECHO All tasks complete.