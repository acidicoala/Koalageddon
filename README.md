# 🐨 Koalageddon 💥
<font size="3">**Legit DLC Unlocker for Steam, Epic, Origin, EA Desktop & Uplay (R1)**</font> 

Welcome to the Koalageddon repository.
For user-friendly introduction or support, please check out the [official forum thread]. This document is meant for software developers.

# 🆕 Version 2

Check out the next major version of Koalageddon, currently in development, at [this repository](https://github.com/acidicoala/Koalageddon2#readme).

The informaion below is for version 1.

## 🗜 Solution Projects
#### 🧰 Common
This project is a static library that houses common functions of all other projects. For example, all projects need to access config file and logging utilites, so they are defined in this module.

#### 💉 Injector
This project is a simple DLL injector executable. The injector can be used as a command line utility that accepts 2 arguments: ID of the process which should be injected and DLL to inject.

#### 🔗 Integration
This project is a dynamic library that pretends to be `version.dll`. Nothing much going on here except for loading of the unlocker module.

#### 🧙🏼‍ Integration Wizard
This project is a trivial GUI utility that automatically installs the integration files and copies the original ones. The GUI is using [Task Dialog] available in Windows API.

#### 🔓 Unlocker
This project is a dynamic library which performs the main function of Koalageddon - DLC unlocking. It monitors DRM DLLs using undocumented WinAPI functions and suspends new processes before injection using undocumented functions as well. Once target DLLs have been identified, appropriate functions are hooked using the great PolyHook 2 library. A total of 4 hooking techniques are used in this project.

## 🛠 Dependencies
The solution uses a number of third party dependencies, which are available via [vcpkg].
Projects in the solution are configured to use static libraries instead of dynamic. If you wish to build the solution yourself, you would need to install the following libraries:

* [Boost preprocessor]
* [C++ Requests]
* [nlohmann JSON]
* [PolyHook 2.0]
* [spdlog]
* [TinyXML-2]
* [WinReg]

The solution includes the [install_vcpkg_dependencies.bat] script, which installs all of the above-mentioned dependencies with a single command.

You can verify installations via `vcpkg list`
## 🔢 Versioning

This project is following semantic versioning schema.

The version information is stored in the following files:
- [inno_setup.iss] - Used by the setup installer.
- [Integration.rc] - Used by Integration DLL.
- [constants.h] -  Used by Koalageddon binaries.

## 📄 License
This software is licensed under [Zero Clause BSD] license, terms of which are available in [LICENSE.txt]


[official forum thread]: https://cs.rin.ru/forum/viewtopic.php?f=10&t=112021
[Task Dialog]: https://docs.microsoft.com/en-us/windows/win32/controls/task-dialogs-overview#:~:text=A%20task%20dialog%20is%20a,features%20than%20a%20message%20box.
[vcpkg]: https://github.com/Microsoft/vcpkg#quick-start-windows
[spdlog]: https://github.com/gabime/spdlog
[nlohmann JSON]: https://github.com/nlohmann/json/
[PolyHook 2.0]: https://github.com/stevemk14ebr/PolyHook_2_0
[WinReg]: https://github.com/GiovanniDicanio/WinReg
[C++ Requests]: https://github.com/whoshuu/cpr
[TinyXML-2]: https://github.com/leethomason/tinyxml2
[Boost Preprocessor]: https://github.com/boostorg/preprocessor
[install_vcpkg_dependencies.bat]: ./install_vcpkg_dependencies.bat

[Zero Clause BSD]: https://choosealicense.com/licenses/0bsd/
[LICENSE.txt]: ./LICENSE.txt

[inno_setup.iss]: ./inno_setup.iss
[Integration.rc]: ./Integration/Integration.rc
[constants.h]: ./Common/src/constants.h
