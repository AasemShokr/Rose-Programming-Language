# 🌹 Rose Programming Language

> *“A petal of syntax, a stem of semantics—let algorithms bloom in verse.”*

Rose is a **general‑purpose, game‑friendly programming language** that marries readable, Python‑inspired control flow with instant‑run scripting and a splash of built‑in graphics power. It’s released under **Apache 2.0**, so studios, students, and skunkworks alike can bend its branches without fear of patent thorns.

---

## Table of Contents

1. [Why Rose?](#why-rose)
2. [Quick Start](#quick-start)
3. [Language Peek‑Sheet](#language-peek-sheet)
4. [Roadmap](#roadmap-)
5. [Contributing](#contributing)
6. [License](#license)

---

## Why Rose?

| 🌟  | Feature               | Why it Matters                                                                                             |
| --- | --------------------- | ---------------------------------------------------------------------------------------------------------- |
| ✏️  | **Expressive Syntax** | English‑like `for let … while … step … end` and `def … end` blocks keep code lyrical & legible.            |
| 🎮  | **`rgame` Module**    | Draw sprites, handle input, and animate pixels in minutes—perfect for rapid prototypes or teaching loops.  |
| 📦  | **Standard Library**  | Core libs: `math`, `containers`, plus an emerging package manager.                                         |
| 🔄  | **Running**        | `rose file.rose`.                                            |
| 🛡️ | **Safe & Open**       | Gradual typing, friendly errors, zero‑cost abstractions; Apache 2.0 for permissive & patent‑safe adoption. |

---

## Quick Start

### 1 · Grab Rose

Windows users: download the source code, build and add it to *PATH*.

### 2 · Say Hello

```rose
print("Hello, blossoming world!")
```

Run it:

```bash
rose hello.rose       # execute a file
rose                  # open the interactive REPL
```

`rose` launches a colourful prompt where you can type code live:

```text
rose> 1 + 1
2
rose> for let i = 0 while i < 3 step i = i + 1 do print(i) end
0
1
2
```

---

## Language Peek‑Sheet

| Rose                                        | English           |
| ------------------------------------------- | ----------------- |
| `def greet(name)`                           | Define a function |
| `array_add(arr, x)`                         | Push to array     |
| `for let i = 0 while i < 10 step i = i + 1` | Counted loop      |
| `if x == y do … end`                        | Conditional       |
| `import 'math'`                             | Module import     |
| `include 'script.rose'`                     | File include      |
| `class Vector`                              | Declare a class   |
| `let speed = 5`                             | Mutable binding   |
| `var hp = 100`                              | Mutable variable  |

More examples soon.

---

## Roadmap 🌱

* [ ] Package manager (will be improved)
* [ ] Debugger & profiler
* [ ] WebAssembly target
* [ ] Standard‑lib expansion (crypto, HTTP, JSON)

---

## Contributing

1. **Fork** ➜ `git clone` ➜ create branch ➜ commit.
2. Run `./scripts/test‑all.sh` and `reuse lint` before pushing.
3. Open a Pull Request—our CI bees will buzz.
4. Sign off your commits (`‑s`) to grant the patent peace promised by Apache 2.

*Small PRs are roses; big PRs are rose gardens.* 🌷

### Code of Conduct

Rose thrives in a welcoming garden. By participating you help making it better.

---

## License

Rose is released under the **Apache License 2.0**. See [`LICENSE`](LICENSE) for the precise legal poetry.

---

> *“May your loops be tight, your blossoms bright, and your segfaults forever out of sight.”*
> Aasem Shokr, a small humble computer scientist
