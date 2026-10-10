# doxit

[![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/23)
[![Build: Meson](https://img.shields.io/badge/build-Meson-39207B?logo=meson&logoColor=white)](https://mesonbuild.com/)
[![Built with trux](https://img.shields.io/badge/built%20with-trux-8A2BE2)](https://github.com/Jlesster/trux)
[![Platform: Linux](https://img.shields.io/badge/platform-Linux-FCC624?logo=linux&logoColor=black)](https://www.kernel.org/)
[![Viewer: w3m](https://img.shields.io/badge/viewer-w3m-informational)](https://w3m.sourceforge.net/)

A small terminal browser for the documentation installed on your system
(`/usr/share/doc`). Pick a package, pick a file, and read it in
[w3m](https://w3m.sourceforge.net/).

doxit is also a **small example application built on
[trux](https://github.com/Jlesster/trux)**, a C++23 terminal rendering and input library.
It is deliberately tiny so you can read all of it in a few minutes and see how
trux's pieces fit together.

## What it does

1. Lists every entry in `/usr/share/doc` in a scrollable, bordered menu.
2. Opens the selected entry and lists its contents in a second menu.
3. Hands the terminal to `w3m` to display the selected file, then returns you
   to the list exactly where you left off.

## Keys

| Key           | Package list     | File list            |
| ------------- | ---------------- | -------------------- |
| `j` / `Down`  | move down        | move down            |
| `k` / `Up`    | move up          | move up              |
| `l` / `Enter` | open the package | open the file in w3m |
| `h` / `Esc`   | n/a              | back to package list |
| `q`           | quit             | quit                 |

Navigation wraps at both ends. Inside w3m, the usual w3m keys apply; quit w3m
(`q`, then confirm) to return to doxit.

## Requirements

- A C++23 compiler (developed with GCC 16)
- [Meson](https://mesonbuild.com/) (>= 1.1) and Ninja
- `git`, so Meson can fetch trux (see below)
- `w3m` available in `$PATH`
- Linux (trux uses `eventfd` and POSIX terminal APIs)

[trux](https://github.com/Jlesster/trux) itself is **vendored through a Meson
wrap**, so you don't need to install it first.

## Build and run

```sh
git clone <this repo> doxit
cd doxit
meson setup builddir
meson compile -C builddir
./builddir/doxit
```

On first configure, Meson clones trux into `subprojects/trux/` using
`subprojects/trux.wrap` and links against its `trux_dep` dependency.

### How trux is vendored

`subprojects/trux.wrap`:

```ini
[wrap-git]
url = https://github.com/Jlesster/trux.git
revision = main
depth = 1

[provide]
trux = trux_dep
```

and in `meson.build`:

```meson
trux_dep = dependency('trux', fallback: ['trux', 'trux_dep'])
```

- **System install takes priority.** If trux is already installed (it ships a
  `trux.pc`), Meson uses that copy and never touches the wrap. To force the
  vendored copy, configure with `--force-fallback-for=trux`.
- **Pinning.** `revision = main` tracks the latest trux. For reproducible
  builds, replace it with a commit SHA.

## How it uses trux

The whole program is three files, and each one shows a different part of the
library.

| File           | What it shows                                                   |
| -------------- | --------------------------------------------------------------- |
| `src/main.cpp` | Terminal setup, the render loop, event handling, view switching |
| `src/docs.cpp` | Plain filesystem code that fills the data the menus display     |
| `src/docs.hpp` | The two small structs the menus are bound to                    |

### Components

`main.cpp` builds two `component::Menu`s and wraps each with
`component::BorderRounded` using trux's pipe syntax:

```cpp
auto documentation_dirs = component::Menu(docs.doc_dirs,
                                          doc_dirs_selection,
                                          doc_dirs_offset,
                                          &DocDirectory::title) |
                          component::BorderRounded;
documentation_dirs.selected_style = style::Style::Reverse;
```

A few things worth noticing:

- **Data is bound by reference.** A `Menu` doesn't own its items, selection or
  scroll offset. It reads your `std::vector`, `selected` and `scroll_offset`
  variables directly, so changing them changes what is drawn.
- **Projections choose what to display.** The fourth argument is a projection.
  `&DocDirectory::title` tells the menu which member to render, so the item
  type can be any struct. Each menu's projection must match its own item type.
- **Selection styling is per component.** `selected_style` is a text-attribute
  bitmask (`Reverse` here), set independently on each menu.

### The render loop

Each iteration redraws the frame and then waits for input:

```cpp
renderer.begin_draw();
renderer.push(documentation_files, files);   // or documentation_dirs
renderer.end_draw();
term.present(renderer);

if(auto event = input.poll(term)) { ... }
```

`Renderer::push` registers a component both for drawing and as an input
handler, so `input.poll(term)` already routes Up/Down/`j`/`k` to the focused
menu before your code sees the event.

### Handling input

The menu reports Enter as _consumed_ so that callers can detect an activation.
doxit therefore lets Enter through the usual "already handled" check:

```cpp
if(event.consumed && event.code != input::Key::Enter) continue;
```

After that, a `switch` on the current `View` maps keys to actions. Several keys
can share an action by stacking `case` labels (`case 'l': case
input::Key::Enter:`).

### Handing the terminal to another program

Opening a file suspends the UI, runs `w3m` in the real terminal, and restores
everything afterwards:

```cpp
term.shutdown();                       // leave alt screen, restore cooked mode
/* fork + execlp("w3m", ...) + waitpid */
term.init();
term.enable_raw_mode();
renderer = renderer::Renderer(term.size());   // fresh front buffer, full repaint
```

Replacing the renderer matters: `init()` clears the screen, so the old front
buffer would make trux think nothing changed and leave you with a blank
display.

## Limitations

- Entries are passed to `w3m` as-is, so what you can read depends on what w3m
  can open (HTML and plain text work well; compressed or binary files may not).
- Reads `/usr/share/doc` only; the path is the `DOC_PATH` constant in
  `src/docs.hpp`.
- Unreadable directories are not handled gracefully: the directory listing
  uses `std::filesystem::directory_iterator`, which throws on failure.

## License

Add your license here.
