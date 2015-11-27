:start
REM HS485Demo.exe COM5 u 0x1029 hs485s_hw1_sw2_00.hex
REM HS485Demo.exe COM5 u 0x1029 ..\..\test-app1\ATmega_app_atmega8.hex
REM HS485Demo.exe COM5 u 0x1029 ..\..\test-app2\ATmega_app_atmega8.hex
HS485Demo.exe COM18 u 0x1001 ..\..\test-app2\ATmega_app_atmega8.hex
REM HS485Demo.exe COM5 u 0x102F ..\..\test-app\ATmega_app_atmega8.hex
REM HS485Demo.exe COM5  
pause
REM goto start

