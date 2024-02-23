Notes:
<!-- @ref -> https://community.platformio.org/t/raspberry-pi-pico-rp2040-will-not-upload-when-using-my-windows-pc-all-other-boards-work/29852 -->
1. To flash the firmware only using IDE, we need to install USB drivers for PICO:
[ZADIG](https://zadig.akeo.ie)

---

2. Installing Arduino-Pico core (earlephilhower core).
[REF](https://arduino-pico.readthedocs.io/en/latest/platformio.html#important-steps-for-windows-users-before-installing)

For windows users, do following

* Firs, call this command in a terminal:
    ```
    git config --system core.longpaths true
    ```

* Enable long paths in Windows --> Run Powershell as admin, and run the following command:
    ```
    New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
    -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
    ```

* After that restart the PC, and it should be possible to install the *earlephilhower* core.

* Because  earlephilhower core isn't yet fully spported in Platformio, the project needs to be created for the `raspberrypi` platform first. And after that, we need to modify `platformio.ini` file:
    ```
    [env:pico]
    platform = https://github.com/maxgerhardt/platform-raspberrypi.git
    board = pico
    framework = arduino
    board_build.core = earlephilhower
    ```

    After this, the project should build for Pico board with `earlephilhower` core.

---

3. Adding local libraries to platformio project:
[LINK](https://community.platformio.org/t/build-fails-when-using-symlink-to-link-libraries-of-a-depended-on-library-in-main-project/34701/3)

    * By default, platformio projects download all the used libraries into the project folder. This is not feasible for large libraries (Like Audiokit, etc.)

To solve this, we can do the following:

* Download the library to a local folder, outside of the PIO project.
* Add a `library.json` file to the folder that holds the libraries:
    ```json
    {
        "name": "Local_Library",
        "description": "Local library outside of the project folder",
        "build": {
            "unflags": "-std=gnu++11",
            "flags": "-std=gnu++17"
        },
        "version": "1.0.0"
    }
    ```
* In the project's `platformio.ini` file, add the following lines:
    ```
    lib_deps =
        symlink://C:\workspace\libs\arduino-audio-tools
    ```
* In this example `C:\workspace\lib` directory contains the `platformio.ini` file.
