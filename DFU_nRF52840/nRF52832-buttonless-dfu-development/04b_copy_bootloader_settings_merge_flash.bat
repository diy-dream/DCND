

@echo off

:: # Check to make sure nrfutil and mergehex and nrfjprog are installed before moving on
WHERE >nul 2>nul nrfutil
IF %ERRORLEVEL% NEQ 0 (
ECHO "nrfutil was not found in PATH, please install using pip install"
goto :end
)
WHERE >nul 2>nul nrfjprog
IF %ERRORLEVEL% NEQ 0 (
ECHO "nrfjprog was not found in PATH, please install using windows installer from nordicsemi.com"
goto :end
)
WHERE >nul 2>nul mergehex
IF %ERRORLEVEL% NEQ 0 (
ECHO "mergehex was not found in PATH, please install using windows installer from nordicsemi.com"
goto :end
)

SET APPLICATION_HEX=ble_app_dcnd_board_nrf52840_s140.hex

echo "## Copy the %APPLICATION_HEX% on the the folder"
copy "C:\Users\Alan\Documents\views\DCND\dcnd_nrf52840_board\s140\ses\Output\Debug\Exe\%APPLICATION_HEX%"

echo "## Looking to make sure %APPLICATION_HEX% is present in folder"
if not exist %APPLICATION_HEX% (
echo "#### app.hex file does not exist! Please copy a application .hex file into the folder, rename it, and try again!"
goto :end
)
echo.

echo "## Creating bootloader settings based on %APPLICATION_HEX%"
nrfutil settings generate --family NRF52 --application %APPLICATION_HEX% --application-version 1 --bootloader-version 1 --bl-settings-version 1 settings.hex
echo.

echo "## Merging bootloader settings and %APPLICATION_HEX% into merged.hex"
mergehex -m %APPLICATION_HEX% settings.hex -o merged.hex
echo.

nrfjprog --program merged.hex --family NRF52 --sectorerase

:end
pause