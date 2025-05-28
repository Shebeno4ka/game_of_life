## Сборка и запуск

### Требования
- CMake >= 3.10
- Компилятор g++ с поддержкой C++17
- SDL3

### Сборка
```bash
mkdir build
cd build
cmake ..
make
```

### Запуск
Рекомендуемые параметры:
```bash
./game_of_life 50 40 8
```

Игра при запуске поставлена на паузу для настройки начального состояния клеток.
### Управление
- `Space` - Пауза/Продолжить
- `R` - Сброс игрового поля
- 'Esc' - Выход из игры

### Описание:
```
game_of_life - Game of Life cellular automaton simulation

Usage: game_of_life SIZE_X SIZE_Y [SPEED]

Game of Life simulates cells that live or die based on their neighbors.

Parameters:
  SIZE_X              integer, Width of the game grid
  SIZE_Y              integer, Height of the game grid
  SPEED               integer, Game simulation speed in updates/sec (optional, default: 1)

Options:
  -h, --help          Display this help and exit

Examples:
  game_of_life 50 40      Run with a 50x40 grid at default speed
  game_of_life 80 60 3    Run with an 80x60 grid at speed 3
```

### Дополнительно:

- Обновление игры не зависит от FPS 
- Игра может работать и на очень высоких скоростях:
```bash
./game_of_life 50 40 80
```