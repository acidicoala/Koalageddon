# 🐨 Koalageddon 💥
#### Legit DLC Unlocker for Steam, Epic & Origin
Welcome to the DreamAPI repository.
For user-friendly introduction or support, please check out the [official forum thread]. This document is meant for software developers.

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
Projects in the solution are configured to use static libraries instead of dynamic. If you wish to build the solution yourself, you would need to install following libraries:

* [PolyHook 2.0]:
	```
	vcpkg install polyhook2:x86-windows-static
	vcpkg install polyhook2:x64-windows-static
	```
* [WinReg]:
	```
	vcpkg install winreg:x86-windows-static
	vcpkg install winreg:x64-windows-static
	```
* [spdlog]:
	```
	vcpkg install spdlog:x86-windows-static
	vcpkg install spdlog:x64-windows-static
	```
* [nlohmann JSON]:
	```
	vcpkg install nlohmann-json:x86-windows-static
	vcpkg install nlohmann-json:x64-windows-static
	```
* [TinyXML-2]
	```
	vcpkg install tinyxml2:x86-windows-static
	vcpkg install tinyxml2:x64-windows-static
	```
* [C++ Requests]
	```
	vcpkg install cpr:x86-windows-static
	vcpkg install cpr:x64-windows-static
	```

You can verify installations via `vcpkg list`

## 📄 License
This software is licensed under [Zero Clause BSD] license, terms of which are available in [LICENSE.txt]

___

[official forum thread]: https://cs.rin.ru/forum/viewtopic.php?f=10&t=112021
[Task Dialog]: https://docs.microsoft.com/en-us/windows/win32/controls/task-dialogs-overview#:~:text=A%20task%20dialog%20is%20a,features%20than%20a%20message%20box.
[vcpkg]: https://github.com/Microsoft/vcpkg#quick-start-windows
[spdlog]: https://github.com/gabime/spdlog
[nlohmann JSON]: https://github.com/nlohmann/json/
[PolyHook 2.0]: https://github.com/stevemk14ebr/PolyHook_2_0
[WinReg]: https://github.com/GiovanniDicanio/WinReg
[C++ Requests]: https://github.com/whoshuu/cpr
[TinyXML-2]: https://github.com/leethomason/tinyxml2

[Zero Clause BSD]: https://choosealicense.com/licenses/0bsd/
[LICENSE.txt]: ./LICENSE.txt
