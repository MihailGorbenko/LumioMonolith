Code style and comments

- Use `clang-format` with the supplied .clang-format file. Run `tools/format.ps1`.
- Indentation: 4 spaces, no tabs.
- Line endings: CRLF on Windows. Files will be normalized by `tools/cleanup_whitespace.ps1`.
- Comments: prefer English for public APIs and shared code. Short inline Russian comments are acceptable for private/local notes, but prefer English overall for consistency.
- Keep comments concise and placed above the code they describe using `//` single-line comments.
- Use descriptive variable names; avoid one-letter vars unless conventional (i,j).

Suggested workflow:
1. Run `powershell -ExecutionPolicy Bypass -File .\tools\cleanup_whitespace.ps1` to clean whitespace.
2. Install `clang-format` (LLVM) and run `powershell -ExecutionPolicy Bypass -File .\tools\format.ps1` to apply style.
3. Inspect changes and run `platformio run` to verify build.

If you want, I can proceed to run `platformio run` and then apply `clang-format` across the repo if you have `clang-format` installed. Otherwise I can continue converting remaining Russian comments to English gradually.