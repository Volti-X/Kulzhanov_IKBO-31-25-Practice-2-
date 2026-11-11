@echo off
cd /d "%~dp0.."

echo ========================================
echo TEST 1: Real Ubuntu package (accountsservice)
echo ========================================
echo.
"x64\Debug\dep_graph.exe" --package accountsservice --repo tests/test_packages.txt --version 22.07.5
echo.

echo ========================================
echo TEST 2: BFS with cycle and filter (A, B, C, D)
echo ========================================
echo.
"x64\Debug\dep_graph.exe" --package A --repo tests/test_graph.txt --filter D
echo.

echo ========================================
echo TEST 3: Reverse dependencies (Package D)
echo ========================================
echo.
"x64\Debug\dep_graph.exe" --package D --repo tests/test_graph.txt
echo.

pause
