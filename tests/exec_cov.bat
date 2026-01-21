if not exist report mkdir report
ccache g++ testMain.cpp -O1 -g --coverage -o report/testMain.exe
start /wait report/testMain.exe
gcovr .. --html --html-details report/testMain.html -e doctest.h ^
	-r ../  --exclude-unreachable-branches --exclude-throw-branches --decisions
start report/testMain.html
