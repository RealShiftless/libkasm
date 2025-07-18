# libkasm

**libkasm** is a modular tokenizer, parser, and assembler framework written in C.  
It’s designed for building custom instruction sets and bytecode formats — perfect for emulators, fantasy consoles or embedded projects.

---

## ✨ Features

- ⚙️ Manual tokenization with full debugging context
- 📜 Clean parser that builds an instruction/label list
- 🛠️ Directive + opcode system with target-specific support
- 🧠 Typed argument parsing: immediates, registers, addresses, labels
- 💾 Output-agnostic assembler backend — write bytecode your way

---

## 📦 Usage

```bash
kasm.exe -f path/to/program.kasm -t target_name
```

You can link libkasm directly into your emulator or compile the CLI tool standalone.