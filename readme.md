
[![Join Discord](https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat)](https://discord.gg/WgAg9ymHbj)

[![Support Author](https://img.shields.io/badge/support-author-blue)](https://bit.ly/3sX2oMk) [![Help Ukraine](https://img.shields.io/badge/help-ukraine-yellow)](https://bit.ly/3afhuGm)

# EaglePatchAC1

EaglePatchAC1 is an ASI plugin that fixes a number of issues in Assassin's Creed: Director's Cut Edition

<img src="/assets/EaglePatchAC1.png" alt="EaglePatchAC1">

## Features
- Unlocked multisampling on high resolutions
- Added support of all XInput controllers
- Added detection of hotplugged controllers
- Removed support of DInput controllers
- Gamepad and kb/mouse controls work at the same time (read note below)
- Added ini setting for enabling PS3-like controls
- Added ini setting to skip intro videos (without having to rename files)
- [DX10 only] Fixed doubling of resolution modes in settings

Note: switching controls layout in game settings has no effect anymore, if you didn't use KeyboardMouse2 layout for keyboard, you need to change KeyboardLayout setting in EaglePatchAC1.ini

## Installation
1. Download Ultimate ASI Loader: https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip
2. Put dinput8.dll from Ultimate ASI Loader into your Assassin's Creed: Director's Cut Edition directory
3. In your Assassin's Creed: Director's Cut Edition directory create a folder named scripts
4. Put EaglePatchAC1.asi and EaglePatchAC1.ini into a newly created scripts folder
5. Done! You may want to check EaglePatchAC1.ini for any settings you want to enable or disable

## Confirmed supported exes
#### AssassinsCreed_Dx9.exe (GOG/Steam)

CRC32: E8936C99

MD5: 8E72C3333743780E43BC2C34BBF625F9

SHA-1: A9EA476E70F076709CCA59A802E26EC894432120


#### AssassinsCreed_Dx10.exe (GOG/Steam)

CRC32: 3AF8F9D0

MD5: CA87753255E2D14B1F18BB737C643792

SHA-1: 601475FD5EFA02BBAE80D1C06DFA25A0A1E7FD2F


# EaglePatchAC2

EaglePatchAC2 is an ASI plugin that fixes a number of issues in Assassin's Creed II

<img src="/assets/EaglePatchAC2.png" alt="EaglePatchAC2">

## Features
- Improved shadow map size from 1024 to 4096 (only when shadow quality is maxed out in game's settings; can be disabled in ini)
- Draw distance maxed out for buildings, peds and flags (can be disabled in ini)
- Enables UPlay bonuses - Auditore Crypt, Altair robes, extra knifes, bonus dye (can be disabled in ini)
- Added support of all XInput controllers
- Added detection of hotplugged controllers
- Removed support of DInput controllers
- Gamepad and kb/mouse controls work at the same time (read note below)
- Added ini setting for enabling PS3-like controls
- Added ini setting to skip intro videos (without having to rename files)

Note: switching controls layout in game settings has no effect anymore, if you didn't use KeyboardMouse2 layout for keyboard, you need to change KeyboardLayout setting in EaglePatchAC2.ini

## Installation
1. Download Ultimate ASI Loader: https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v4.68/Ultimate-ASI-Loader.zip
2. Put dinput8.dll from Ultimate ASI Loader into your Assassin's Creed II directory
3. In your Assassin's Creed II directory create a folder named scripts
4. Put EaglePatchAC2.asi and EaglePatchAC2.ini into a newly created scripts folder
5. Done! You may want to check EaglePatchAC2.ini for any settings you want to enable or disable

## Supported versions
- Digital UPlay
- Retail Akella v1.01

These plugins are using a lightweight win32 patcher by withmorten: https://github.com/withmorten/patcher