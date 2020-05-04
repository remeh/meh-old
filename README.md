# meh

`meh` is remeh's personal code editor.

## Features

    * C++/Qt5, tested on Linux and macOS (will most likely work on Windows as well)
    * Vim-like movements: `h`, `j`, `k`, `l`, `f`, `F`, `o`, `O`, `a`, `A`, `I`, `g`, `G`, ...
    * Insert, normal, replace, command, visual and visual-line mode are implemented (`i`, `Esc`, `R`, `:`, `v` and `V`)
    * Open multiple files
        * Ctrl-O (other) to go back to previous buffer
        * Ctrl-P to use the file opener
        * `:e <filepath>`
    * File opener
        * Fast lookup per directory
        * Filtering while typing
        * `Ctrl-n` next entry, `Ctrl-p` previous entry, `Return` open the file / move to the directory
    * Search in file with `/`, next occurrences with `n` and `N`. `,` to search for the word under the cursor
    * Search in current/all files with `ripgrep` integration
        * `:rg` search in all files with ripgrep for the word under the cursor
        * `:rg <pattern>` search in all files with ripgrep for the given pattern
        * `:rgf` search in current file with ripgrep for the word under the cursor
        * `:rgf <pattern>` search in current files with ripgrep for the given pattern
        * In results
            * `Ctrl-n` for next result, `Ctrl-p` for previous result, `Return` to open the file at the matching line
            * use `x` or `Backspace` to remove entries from the results to remove the noise
    * Auto-indentation (respect previous line indentation, automatically changes it depending on `{` and `}`)
    * Simple syntax highlighting (regexp based)
    * Highlight the selection / word under the cursor
    * Remember cursor position in files previously opened
    * Current-line visual emphasizing
    * 80 chars line indicator
    * Next/previous occurrence of a char on the line (`fa` go to next `a`, `F(` go to previous `(`)
    * Copy / paste
        * Visual mode to copy with `y` or cut with `x`
        * Paste with `Ctrl-V` in insert mode or `p` and `P` in normal mode
        * `yy` copy the current line in normal mode
        * `dd` cut the current line in normal mode
    * Undo `u` / Redo `U`
    * Page up (Ctrl-u) / Page down (Ctrl-d)
    * Go to line with `:<line number>`
    * Others: `J`, `C`, `D`, `ct` `cT` `cf` `cF`, `vf`, `vF`, `<`, `>`, ...

## Roadmap

    * Interface with an LSP server for "go to reference" feature / auto-completion
    * Fuzzy search in the file opener (it is just a `startsWith` for now)
    * Icons on macOS (missing in the file opener)
    * Syntax highlighting per file extension
    * May implement the file opener with FZF if this one annoys me, but so far I'm happy with it

## License

GNU General Public License v3.0