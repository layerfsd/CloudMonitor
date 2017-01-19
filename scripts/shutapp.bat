REM shutapp.bat

@echo off

sc stop CloudMonitorService

taskkill /f /im CloudMonitor.exe
taskkill /f /im MonitorService.exe
taskkill /f /im MonitorService-64.exe

echo "Done"