cls
if not exist report mkdir report
ccache g++ testMain.cpp -g -o report/testMain.exe
cd report
testMain.exe
cd ..