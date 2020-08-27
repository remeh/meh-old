# meh

`meh` is remeh's personal code editor.

## Features

    * C++/Qt5, tested on Linux and macOS (will most likely work on Windows as well)
    * Vim-like movements: `h`, `j`, `k`, `l`, `f`, `F`, `o`, `O`, `a`, `A`, `I`, `g`, `G`, ...
    * Insert, normal, replace, command, visual and visual-line mode are implemented (`i`, `Esc`, `r`, `R`, `:`, `v` and `V`)
    * Open multiple files
        * Ctrl-O (other) to go back to previous buffer
        * Ctrl-P to use the file opener
        * `:e <filepath>`
    * LSP client for C++ (clangd) and Go (gopls):
        * Go to definition with the `def` command
    * File opener
        * Fast lookup per directory
        * Filtering while typing
        * `Ctrl-n` next entry, `Ctrl-p` previous entry, `Return` open the file / move to the directory
    * Search in file with `/`, next occurrences with `n` and `N`. `,` to search for the word under the cursor
    * Toggle `//` comments with Ctrl-M on selected lines (or current line of no selection), `#` with Ctrl-Shift-M
    * Search in current/all files with `ripgrep` integration
        * `:rg` search in all files with ripgrep for the word under the cursor
        * `:rg <pattern>` search in all files with ripgrep for the given pattern
        * `:rgf` search in current file with ripgrep for the word under the cursor
        * `:rgf <pattern>` search in current file with ripgrep for the given pattern
        * In results
            * `Ctrl-n` for next result, `Ctrl-p` for previous result, `Return` to open the file at the matching line
            * use `x` or `Backspace` to remove entries from the results to remove the noise
    * Auto-complete with words in opened buffers with `Ctrl-Space`
    * Auto-indentation (respect previous line indentation, automatically changes it depending on `{`, `}`, and `:`)
    * Simple syntax highlighting (regexp based)
    * Highlight the selection / word under the cursor
    * Remember cursor position in previously opened files
    * Current-line visual emphasizing
    * 80 and 120 chars vertical lines indicator
    * Toggle // comments on selection with `Ctrl-m`, toggle # comments with `Ctrl-Shift-m`.
    * Next/previous occurrence of a char on the line (e.g. `fa` go to next `a`, `F(` go to previous `(`)
    * Copy / paste
        * Visual mode to copy with `y` or cut with `x`
        * Paste with `Ctrl-V` in insert mode or `p` and `P` in normal mode
        * `yy` copy the current line in normal mode
        * `dd` cut the current line in normal mode
    * Undo `u` / Redo `U`
    * Page up (Ctrl-u) / Page down (Ctrl-d)
    * Go to line with `:<line number>`
    * Automatically runs `gofmt` on Go files (on save), `zig fmt` on Zig files.
    * Basic tasks manager (list of todo, done, cancelled tasks)
    * Confirm messagebox while re-opening a file already opened in another instance.
    * Others: `J`, `C`, `D`, `ct` `cT` `cf` `cF`, `vf`, `vF`, `<`, `>`, ...

## Roadmap

    * Finish the LSP client with signatures infos, methods comments, type of objects and completion
    * Syntax highlighting per file extension
    * May implement the file opener with FZF if this one annoys me, but so far I'm happy with it

## License

GNU General Public License v3.0
