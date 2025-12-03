# C4Timer
Плагин для Counter-Strike 2, отображающий таймер бомбы с визуальным прогресс-баром в чате.

## Install
1. Download and install last Release
2. Download and install [cs2-menus](https://github.com/Pisex/cs2-menus/releases)
3. Customize the configuration file (addons/configs/C4Timer/core.ini)

## Features
- Real-time bomb timer display in chat
- Visual progress bar showing remaining time
- Bomb site detection (A/B)
- Multi-language support (Russian, English)

## Configuration
File: `addons/configs/C4Timer/core.ini`
```ini
"Settings"
{
    "ShowTimerFrom"     "40"
    "ShowSite"          "1"
    "TimerEnabled"      "1"
}

```

## Translations
Translation file: `addons/translations/C4Timer.phrases.txt`

Supported languages:
- Russian (ru)
- English (en)

You can add your own translations following the file format.

## Example Output
```
💣 Bomb at site A | Time left: 35 sec
[████████████░░░░░░░░] 70%
```

