# meh

`meh` is remeh's personal code editor.

![Screenshot](https://raw.githubusercontent.com/remeh/meh/master/res/screenshot.png)

## Features

    * C++/Qt6, tested on Linux and macOS (will most likely work on Windows as well)
    * **Vim-like** movements: `h`, `j`, `k`, `l`, `f`, `F`, `o`, `O`, `a`, `A`, `I`, `g`, `G`, ...
    * Insert, normal, replace, command, visual and visual-line modes are implemented (`i`, `Esc`, `r`, `R`, `:`, `v` and `V`)
    * Open multiple files
        * Ctrl-O (other) to go back to previous buffer
        * Ctrl-P to use the file opener
        * Ctrl-Shift-P to use the file opener with opened buffers
        * `:e <filepath>`
    * **Instances mode** to open new files in existing instance
    * **Generic LSP client**, plugged for C++ (clangd), Go (gopls) and Zig (zls):
        * Go to definition with the `:def` command
        * Auto-completion with `Ctrl-Enter`
        * Display references to a variable / method / ... with `:ref` command
        * Get error of the current line with `err` or clicking on the red highlighted line number
        * Get functions/methods signatures and documentation with `:sig`
        * `:i` or `:info` to get infos on what's under the cursor
    * **Fast file opener**
        * Fast lookup per directory
        * Filtering while typing
        * `Ctrl-n` next entry, `Ctrl-p` previous entry, `Return` open the file / move to the directory
        * Use `:fd` to use `fd` to look for files in subdirs, and then filter in results
    * **Search** in current/all files with `ripgrep` integration
        * `:rg` search in all files with ripgrep for the word under the cursor
        * `:rg <pattern>` search in all files with ripgrep for the given pattern
        * `:rgf` search in current file with ripgrep for the word under the cursor
        * `:rgf <pattern>` search in current file with ripgrep for the given pattern
        * In results
            * `Ctrl-n` for next result, `Ctrl-p` for previous result, `Return` to open the file at the matching line
            * `j` and `k` for also works for next / previous result
            * `Ctrl-j` and `Ctrl-k` to expand / close results on a file
            * use `x` or `Backspace` to remove entries from the results to remove the noise
    * **Git support**:
        * Display lines git status (added, edited, removed)
        * `:gblame` opens the git blame of the current buffer and goes to current line
        * `:gshow [<checksum>]` show the commit of the given checksum or the one under the cursor if none provided
        * `:gdiff [--staged]` shows the current diff / staged diff
    * Search in file with `/`, next occurrences with `n` and `N`. `,` to search for the word under the cursor
    * Toggle `//` comments with Ctrl-M on selected lines (or current line if no selection), `#` with Ctrl-Shift-M
    * **Auto-complete** with words in opened buffers with `Ctrl-N`
    * **Auto-indentation** (respect previous line indentation, automatically changes it depending on `{`, `}`, and `:`)
    * **Simple syntax highlighting** (token parsing & regex based)
    * Execute a command and display the output in a buffer (with `:exec <command> <args>` or `:!<command> <args>`)
    * Highlight the selection / word under the cursor
    * Commands history
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
        * `dw` cut the word under the cursor
        * `D` cut the rest of the line, `C` cut the rest of the line and enter insert mode
    * Undo `u` / Redo `U`
    * Page up (Ctrl-u) / Page down (Ctrl-d)
    * Go to line with `:<line number>`
    * `s` stores a checkpoint, `S` goes back to previously saved checkpoint
    * Automatically runs `gofmt` on Go files (on save), `zig fmt` on Zig files.
    * Basic tasks manager (list of todo, done, cancelled tasks)
    * Confirm messagebox while re-opening a file already opened in another instance.
    * Others: `J`, `C`, `D`, `ct` `cT` `cf` `cF`, `vf`, `vF`, `<`, `>`, ...

## License

GNU General Public License v3.0
