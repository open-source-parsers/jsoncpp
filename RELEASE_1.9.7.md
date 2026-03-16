# jsoncpp 1.9.7 Release Work

Issues to fix before tagging 1.9.7, each in a separate CL.

---

## Done

- [x] **#1656** — Fix uninitialized CMake variable `JSONCPP_VERSION` in `version.in`
  → Change `@JSONCPP_VERSION@` to `@jsoncpp_VERSION@`

---

## To Do

### Security / Memory Safety

- [ ] **#1626** — MemorySanitizer: use-of-uninitialized-value in `Json::Value::resolveReference`
  → Uninitialized value detected by MSan in `json_value.cpp`. Need to identify and zero-initialize the offending member.

- [ ] **#1623** — Use-after-free: `Json::Reader::parse` stores raw pointers into input string
  → `Reader` stores `begin_`/`end_` pointers that dangle after the input `std::string` goes out of scope. `getFormattedErrorMessages()` then reads freed memory.
  → Fix: copy the input document internally, or clearly document the lifetime requirement (the simpler option given the old Reader API is deprecated).

### Correctness

- [x] **#1565** — Number parsing breaks when user sets a non-C locale (e.g. `de_DE`)
  → `istringstream`/`ostringstream` used for number parsing/writing inherit the global locale, which may use `,` as decimal separator instead of `.`.
  → Fix: imbue streams with `std::locale::classic()` in `json_reader.cpp` and `json_writer.cpp`.

- [ ] **#1546** — Control characters below 0x20 not rejected during parsing
  → JSON spec requires rejecting unescaped control characters. jsoncpp currently accepts them.

### Build / CMake

- [ ] **#1634** — `JSON_DLL_BUILD` compile definition applied globally instead of per-target
  → `add_compile_definitions` scopes it to all targets; should use `target_compile_definitions` scoped to the shared lib only.

- [x] **#1598** — CMake 3.31 deprecation warning about compatibility with CMake < 3.10
  → Update `cmake_minimum_required` to use `<min>...<max>` version range syntax, e.g. `cmake_minimum_required(VERSION 3.10...3.31)`.

- [x] **#1595** — Linker errors with `string_view` API when jsoncpp built as C++11 but consumer uses C++17
  → Root cause: `JSONCPP_HAS_STRING_VIEW` is not defined when building the library (forced C++11), but consumer with C++17 sees the `string_view` overloads in headers and tries to link them.
  → Fix options: (a) export `JSONCPP_HAS_STRING_VIEW` in the CMake config so consumers see the same value, or (b) drop `CMAKE_CXX_STANDARD` force and use `target_compile_features(cxx_std_11)` instead.

---

## Skipped (not bugs)

- **#1548** — "Memory leak" after parsing large files: confirmed to be normal allocator behavior (OS doesn't immediately reclaim heap). Not a library bug.
- **#1533** — `clear()` then adding values fails: `clear()` preserves the value type by design. Confirmed user error.
- **#1547** — Trailing commas/garbage not rejected: existing behavior, controllable via `strictMode()`. Not a regression.
