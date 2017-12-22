
set "Ymd=%date:~,4%%date:~5,2%%date:~8,2%"
set "pwd=D:\Keil5Projects\backup"
md %pwd%
"%ProgramFiles%\WinRAR"\rar a -pfeng2008 D:\Keil5Projects\backup\Coin%Ymd%.rar "D:\Keil5Projects\Coin"

dir %pwd%

pause