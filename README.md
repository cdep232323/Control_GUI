# mqtt-qt

## Features

- [x] config.json for mqtt settings
- [x] gague widget
- [x] plot widget
- [ ] make UI more fancy
   - [x] add vibration to gague widget
   - [ ] make plot go forward in time
   - [ ] make plot non linear but curve

## Prerequisites

- Build tools
	+ `sudo apt install git build-essential cmake`
- Qt 6
	+ `sudo apt install libxcb-cursor0`
	+ `sudo apt install libxkbcommon-dev qt6-base-dev qt6-base-private-dev qt6-base-dev-tools`

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Configuration

Check [`config.json`](./config.json) it's simple

## Known issues

- https://bugreports.qt.io/browse/QTBUG-95391 - cmake Missing WrapVulkanHeaders
	+ https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html - how to install vulkan-sdk
# Control_GUI
# Control_GUI
