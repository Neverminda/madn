# Mensch Ärgere Dich Nicht / Ludo

Assignment for the Advanced Software Development course at University Pforzheim.

---

## 🛠️ Build & Run

### Requirements
- C++23 compliant compiler (i.e., Clang 17+)
- CMake 4.0+

### Build
```bash
cmake -B build -S .
cmake --build build
```

### Run
```bash
# Run interactive game
./build/madn

# Run benchmark (default: 1000 games)
./build/madn --benchmark

# Run benchmark with custom game count
./build/madn --benchmark 5000
```