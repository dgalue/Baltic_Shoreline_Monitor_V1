@echo off
echo ===== Baltic Meshtastic Upload Script =====
echo.
echo Instructions:
echo 1. Hold down the BOOT button on your XIAO ESP32-S3
echo 2. Press Enter to start upload
echo 3. Keep holding BOOT until you see "Connecting..."
echo 4. Release BOOT button when you see "Writing at..."
echo.
pause

echo Uploading Baltic Meshtastic firmware...
"C:\Users\Diego Galue\AppData\Roaming\Python\Python313\Scripts\pio.exe" run -e baltic_meshtastic --target upload --upload-port COM6

echo.
echo Upload completed! Starting serial monitor...
"C:\Users\Diego Galue\AppData\Roaming\Python\Python313\Scripts\pio.exe" device monitor --port COM6 --baud 115200

pause