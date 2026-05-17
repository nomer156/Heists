@echo off
echo ========================================
echo   HEISTS - UnrealMCP Server Launcher
echo ========================================
echo.
echo Starting UnrealMCP Python server...
echo MCP Server will listen on port 55557
echo.
set PATH=C:\Users\sasha\.local\bin;%PATH%
uv --directory "C:\Users\sasha\unreal-mcp\Python" run unreal_mcp_server.py
pause
