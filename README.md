# meh

`meh` is remeh's personal code editor.

## Features

    * C++/Qt5, tested on Linux and macOS (will most likely work on Windows as well)
    * Vim-like movements: `h`, `j`, `k`, `l`, `f`, `F`, `o`, `O`, `a`, `A`, `I`, `g`, `G`, ...
    * Insert, normal, replace, command, visual and visual-line mode are implemented (`i`, `Esc`, `r`, `R`, `:`, `v` and `V`)
    * Open multiple files
        * Ctrl-O (other) to go back to previous buffer
        * Ctrl-P to use the file opener
        * Ctrl-Shift-P to use the file opener with opened buffers
        * `:e <filepath>`
    * LSP client for C++ (clangd), Go (gopls) and Zig (zls):
        * Go to definition with the `:def` command
        * Auto-completion with `Ctrl-Enter`
        * Display references to a variable / method / ... with `:ref` command
        * Get functions/methods signatures and documentation with `:sig`
        * `:i` or `:info` to get infos on what's under the cursor
    * File opener
        * Fast lookup per directory
        * Filtering while typing
        * `Ctrl-n` next entry, `Ctrl-p` previous entry, `Return` open the file / move to the directory
    * Search in current/all files with `ripgrep` integration
        * `:rg` search in all files with ripgrep for the word under the cursor
        * `:rg <pattern>` search in all files with ripgrep for the given pattern
        * `:rgf` search in current file with ripgrep for the word under the cursor
        * `:rgf <pattern>` search in current file with ripgrep for the given pattern
        * In results
            * `Ctrl-n` for next result, `Ctrl-p` for previous result, `Return` to open the file at the matching line
            * `j` and `k` for also works for next / previous result
            * `Ctrl-j` and `Ctrl-k` to expand / close results on a file
            * use `x` or `Backspace` to remove entries from the results to remove the noise
    * Git support:
        * `:gblame` opens the git blame of the current buffer
        * `:gshow [<checksum>]` show the commit of the given checksum or the one under the cursor if none provided
        * `:gdiff [--staged]` shows the current diff / staged diff
    * Search in file with `/`, next occurrences with `n` and `N`. `,` to search for the word under the cursor
    * Toggle `//` comments with Ctrl-M on selected lines (or current line if no selection), `#` with Ctrl-Shift-M
    * Auto-complete with words in opened buffers with `Ctrl-N`
    * Auto-indentation (respect previous line indentation, automatically changes it depending on `{`, `}`, and `:`)
    * Simple syntax highlighting (regexp based)
    * Execute a command and display the output in a buffer (with `:exec <command> <args>` or `:!<command> <args>`)
    * Highlight the selection / word under the cursor
    * Remember cursor position in previously opened files
    * Current-line visual emphasizing
    * 80 and 120 chars vertical lines indicator
    * StatusBar with current mode / current buffer / current line
    * Next/previous occurrence of a char on the line (e.g. `fa` go to next `a`, `F(` go to previous `(`)
    * Copy / paste
        * Visual mode to copy with `y` or cut with `x`
        * Paste with `Ctrl-V` in insert mode or `p` and `P` in normal mode
        * `yy` copy the current line in normal mode
        * `dd` cut the current line in normal mode
    * Undo `u` / Redo `U`
    * Page up (Ctrl-u) / Page down (Ctrl-d)
    * Go to line with `:<line number>`
    * `s` stores a checkpoint, `S` goes back to previously saved checkpoint
    * Automatically runs `gofmt` on Go files (on save), `zig fmt` on Zig files.
    * Basic tasks manager (list of todo, done, cancelled tasks)
    * Confirm messagebox while re-opening a file already opened in another instance.
    * Others: `J`, `C`, `D`, `ct` `cT` `cf` `cF`, `vf`, `vF`, `<`, `>`, ...

## Roadmap

    * After having started `:gblame`, enter should load the file in the commit under the cursor
    * Syntax highlighting per file extension
    * May implement the file opener with FZF if this one annoys me, but so far I'm happy with it

## License

GNU General Public License v3.0
