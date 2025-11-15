# Minimal UACMe Build Instructions

This document provides instructions on how to build the minimal version of the UACMe project, which contains only the `ucmQuickAssistMethod` UAC bypass.

## Prerequisites

* Microsoft Visual Studio 2022
* Windows SDK

## Build Steps

1. **Compile the `Naka` utility:**
   - Open the `uacme.sln` solution file in Visual Studio 2022.
   - Select the `Naka` project.
   - Build the project with the `Release` configuration for the `x64` platform.
   - This will generate `Naka.exe` in the `Bin/` directory.

2. **Compile the `Fubuki` DLL:**
   - In the same solution, select the `Fubuki` project.
   - Build the project with the `Release` configuration for the `x64` platform.
   - This will generate `Fubuki.dll` in the `Bin/` directory.

3. **Encrypt the `Fubuki` DLL:**
   - Open a command prompt and navigate to the root of the repository.
   - Run the following command:
     ```
     Bin/Naka.exe Bin/Fubuki.dll
     ```
   - This will create two files in the `Bin/` directory: `Fubuki.cd` (the encrypted payload) and `Fubuki.key` (the encryption key).

4. **Compile the `Akagi` executable:**
   - In the same solution, select the `Akagi` project.
   - Build the project with the `Release` configuration for the `x64` platform.
   - This will generate `Akagi.exe` in the `Bin/` directory.

You have now successfully built the minimal UACMe project.
