# 🚀 ESP32 Project Template

[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://enthali.github.io/esp32-template/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.4.1-orange)](https://github.com/espressif/esp-idf)

A production-ready ESP32 project template with GitHub Codespaces, QEMU emulation, and professional requirements engineering methodology.

## ✨ What's Included

- **Zero-setup development** - GitHub Codespaces with ESP-IDF pre-configured
- **Hardware optional** - QEMU emulation for testing without physical devices
- **Component-based architecture** - Modular structure following ESP-IDF best practices
- **Professional documentation** - Sphinx-Needs for requirements traceability
- **Example components** - Web server, config manager, network bridge

## 🎯 Quick Start

1. **Use this template** - Click "Use this template" at the top
2. **Open in Codespaces** - Click Code → Codespaces → Create codespace
3. **Customize your project** (Important!):
   - [ ] Change project name in `CMakeLists.txt` (line 12: `set(PROJECT_NAME "your-project-name")`)
   - [ ] Update binary path in `.vscode/launch.json` if using debugger (line 9)
   - [ ] (Optional) Customize `main/Kconfig.projbuild` menu name
   - [ ] (Optional) Update web UI titles in `main/components/web_server/www/` files
4. **Build and run**:

```bash
idf.py build                  # Build project
# Use VS Code task: "Run QEMU (No Debug)"
```

## 📚 Documentation

**👉 [Full Documentation on GitHub Pages](https://enthali.github.io/esp32-template/) 👈**

- [Getting Started Guide](https://enthali.github.io/esp32-template/90_guides/index.html)
- [Requirements & Design](https://enthali.github.io/esp32-template/11_requirements/index.html)
- [API Reference](https://enthali.github.io/esp32-template/21_api/index.html)
- [QEMU Emulation & Debugging](https://enthali.github.io/esp32-template/90_guides/index.html)

## 📁 Project Structure

```text
esp32-template/
├── main/               # Your application code (start here!)
├── docs/               # Sphinx documentation
├── tools/              # QEMU, network utilities, web flasher
└── .devcontainer/      # GitHub Codespaces configuration
```

## 🤖 GitHub Copilot Ready

This template includes comprehensive AI-assisted development instructions. Just ask Copilot for help with ESP32-specific patterns, component architecture, or memory optimization.

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

**Ready to build?** Fork this template and start coding! 🚀
