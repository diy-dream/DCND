

@echo off

:: # Check to make sure nrfutil is installed before moving on
WHERE >nul 2>nul nrfutil
IF %ERRORLEVEL% NEQ 0 (
ECHO "nrfutil was not found in PATH, please install using pip install"
goto :end
)

SET APPLICATION_HEX=ble_app_dcnd_board_nrf52840_s140.hex
SET DEVICE_NAME=DCND
SET PORT_COM_NAME=COM17
SET PACKAGE_NAME=FW.zip

echo "## Copy the %APPLICATION_HEX% on the the folder"
copy "C:\Users\Alan\Documents\views\DCND\sparkfun_nrf52832\s132\ses\Output\Debug\Exe\%APPLICATION_HEX%"

echo "## Looking to make sure %APPLICATION_HEX% is present in folder"
if not exist %APPLICATION_HEX% (
echo "#### app.hex file does not exist! Please copy a application .hex file into the folder, rename it, and try again!"
goto :end
)
echo.

echo "## Creating a %PACKAGE_NAME% package that can be used to update the only application FW on the DK"
nrfutil pkg generate --application %APPLICATION_HEX% --application-version 1 --application-version-string "1.0.0" --hw-version 52 --sd-req 0xCA --key-file private.pem %PACKAGE_NAME%
echo.

echo "## Program the application over DFU"
nrfutil dfu ble -ic NRF52 -pkg %PACKAGE_NAME% -p %PORT_COM_NAME% -n "%DEVICE_NAME%"
echo.

:end
pause