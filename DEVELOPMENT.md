<!-- pandoc -V papersize:a4 -f markdown-implicit_figures -o DEVELOPMENT.pdf DEVELOPMENT.md -->

# How to install on clean Ubuntu 22.xx

Open `Terminal` App

1. Update repository packages with command

   > `sudo apt update -y`

1. To install base development packages run:

   > `sudo apt install git build-essential cmake -y`

1. To install Qt6 run:

   > `sudo apt install libxkbcommon-dev qt6-base-dev qt6-base-private-dev qt6-base-dev-tools -y`

At this point you will be able to compile the project from terminal with:

   > `cmake -S . -B build`
   > `cmake --build build`

# Prepare development environment (QtCreator)

Open `QtCreator` App

1. Select "Open Project" button

   > ![Open Project](./screenshots/qtcreator-open-project.png)

1. Select "CMakeLists.txt" from "mqtt-qt" directory

   > ![Open CMakeLists.txt](./screenshots/qtcreator-open-cmakelists.png)

1. Select required configurations (usually "Debug" and "Release" enough)

   > ![Configure Project](./screenshots/qtcreator-configure.png)

1. Wait some time while till QtCreator will configure the project

   > ![Configure Done Log](./screenshots/qtcreator-configure-done-log.png)

1. Pass the config file path as argument to the programm

   > ![Run configuration](./screenshots/qtcreator-configure-done-log.png)

1. Select "Run" or "Debug" button on the left-bottom panel (it will also take a bit more the first time)

   > ![Run configuration](./screenshots/qtcreator-run-or-debug.png)