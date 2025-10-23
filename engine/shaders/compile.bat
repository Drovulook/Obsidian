@echo off
set GLSLC=C:\dev\VulkanSDK\1.4.313.2\Bin\glslc.exe
set OUTPUT_DIR=compiled

for %%f in (*.vert *.frag) do (
    %GLSLC% %%f -o %OUTPUT_DIR%\%%f.spv
)

for %%f in (*.comp) do (
    %GLSLC% %%f -o %OUTPUT_DIR%\%%~nf.spv
)
