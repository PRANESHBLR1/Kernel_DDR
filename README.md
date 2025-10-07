# Surveillance Camera Chip Diagnostic Tools

**Project Duration:** August – September 2025  
**Internship:** BigEndian Semiconductors  

This project was completed as part of my internship, focused on developing **diagnostic tools for a surveillance camera SoC**.  
The goal was to understand and implement **hardware-software interaction**, particularly for DDR/register access in surveillance systems, and to provide intuitive tools for testing and monitoring camera chips.

CCTV systems rely on precise memory-mapped register control for video capture, processing, and storage. This project provides **hands-on insight into SoC-level diagnostics** and simulates hardware registers for testing and validation.

---

## Overview

- Full-stack diagnostic framework for a surveillance camera SoC.  
- Combines **Linux kernel module**, **HTTPS-based virtual register server**, and **Qt GUI tool**.  
- Enables safe and efficient read/write operations to physical and simulated memory.  
- Designed to teach **practical embedded systems debugging** and register-level intuition.

---

## Key Components

### 1. Linux Kernel Module
- Maps DDR/physical memory using `ioremap()` for direct hardware access.  
- Supports **32-bit read/write operations** with alignment checks.  
- Implements **non-overwrite protection** to prevent accidental memory corruption.  
- IOCTL interface for **single and range register operations**.  
- Robust error handling for invalid addresses and misaligned accesses.  
- Logs operations for debugging via `dmesg`.  

### 2. HTTPS Virtual Register Server (Python/Flask)
- Simulates hardware registers in virtual memory.  
- REST API endpoints:
  - `/api/v1/read` & `/api/v1/write`  
  - `/api/v1/read_range` & `/api/v1/write_range`  
  - `/api/v1/clear`, `/api/v1/clear_range`, `/api/v1/clear_all`  
- Alignment, range, and overwrite checks enforced.  
- Persistent memory storage using `vreg.bin`.  
- TLS/HTTPS for secure communication.  
- Structured JSON responses for automation.

### 3. Qt GUI Diagnostic Tool (C++/Qt)
- User-friendly interface for **single and range register access**.  
- Interactive feedback for errors, overwrites, and successful operations.  
- File-based batch write support for testing multiple registers.  
- Connects to kernel module via IOCTL for direct hardware interaction.  
- Input validation for addresses, widths, and hexadecimal values.  

---

## Features

- **32-bit memory access enforcement** with alignment checks.  
- **Non-overwrite protection** to ensure register safety.  
- **Persistent memory simulation** for the virtual server.  
- **Full-stack integration**: kernel ↔ REST server ↔ GUI.  
- TLS/HTTPS secure API for remote testing and automation.  
- Error handling, logging, and debugging support.  
- Provides **CCTV register-level intuition** for video processing SoCs.

---

## Tech Stack

- **Linux Kernel Programming**: C, IOCTL, `/dev` interfaces  
- **Python 3**: Flask, TLS/HTTPS, JSON API  
- **C++/Qt**: Qt Widgets, GUI integration, input validation  
- **JSON**: structured communication between server and GUI  

---

## Learning Outcomes

- Hands-on experience with **kernel-level Linux programming** and memory-mapped IO.  
- Developing **secure virtual hardware servers** with persistent memory.  
- Building **full-stack diagnostic tools** for SoC validation.  
- Understanding **hardware-software interaction** in CCTV chips.  
- Exposure to cross-layer integration: kernel, server, and GUI.  

---

## Project Files

- **`kernel_ddr`**: Main project files and CLI commands.  
- **`qt_regtool`**: Qt-based diagnostic GUI tool.  
- **`web_servicing`**: Python/Flask HTTPS server.  
- **Screenshots**: Demonstrating project operations.  
- **Single read/write example files**: For reference and understanding.  
- **Backup files**: For safe restoration and testing.

---

