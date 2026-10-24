find src include example -name '*.cpp' -or -name '*.h' -or -name '*.inl' | xargs clang-format -i
