# Surveillance Camera Chip Diagnostic Tools

This repository contains the work completed during my internship at **BigEndian Semiconductors**, focused on developing **diagnostic tools** for a surveillance camera System-on-Chip (SoC).  
The project demonstrates a full-stack approach to hardware register diagnostics — from kernel-level memory access to a secure REST API and a Qt-based GUI for monitoring and control.

## Overview

- Complete diagnostic framework for a surveillance camera SoC.  
- Integrates Linux kernel module, HTTPS-based virtual register server, and Qt GUI tool.  
- Provides hands-on experience with system-level programming, embedded software, and toolchain integration.  
- Designed for safe and efficient read/write operations to physical and simulated memory regions.

## Key Components

### 1. Linux Kernel Module
- Maps DDR/physical memory using `ioremap()` for direct hardware access.  
- Supports **32-bit read/write operations**, enforcing proper alignment.  
- Implements **non-overwrite protection** to prevent accidental memory corruption.  
- Provides IOCTL interface for **single and range register operations**.  
- Designed with **robust error handling** for invalid addresses, alignment violations, and access protection.  
- Logs operations for debugging via `dmesg`.  

### 2. HTTPS Virtual Register Server (Python/Flask)
- Simulates hardware registers in a **virtual memory region**.  
- Provides secure REST API endpoints:
  - `/api/v1/read` & `/api/v1/write` for single register access  
  - `/api/v1/read_range` & `/api/v1/write_range` for bulk operations  
  - `/api/v1/clear`, `/api/v1/clear_range`, `/api/v1/clear_all` for memory management  
- Performs **alignment, range, and overwrite checks** for all operations.  
- Ensures **persistence** of register values via `vreg.bin`.  
- Implements **TLS/HTTPS** for secure communication and optional client authentication.  
- Returns structured **JSON responses** with detailed error reporting for automation.  

### 3. Qt GUI Diagnostic Tool (C++/Qt)
- Provides a **user-friendly interface** for monitoring and controlling registers.  
- Supports **single and range read/write operations**, including file-based batch writes.  
- Displays **interactive feedback** for errors, overwrite protection, and operation success.  
- Integrates directly with the Linux kernel module via IOCTL.  
- Includes input validation for addresses, widths, and values (hexadecimal support).  
- Enhances productivity for hardware/software debugging workflows.

## Features
- **32-bit enforced memory access** with strict alignment.  
- **Overwrite protection** to prevent accidental data loss.  
- **Persistent memory simulation** for virtual register server.  
- **Full-stack integration**: kernel ↔ REST server ↔ GUI.  
- **Secure API** with TLS/HTTPS support.  
- Designed for **automation and scripting** via REST API.  
- Comprehensive error handling and logging.

## Tech Stack
- **Linux Kernel Programming**: C, IOCTL, `/dev` interfaces  
- **Python 3**: Flask, SSL/TLS, JSON API  
- **C++/Qt**: Qt Widgets, GUI integration, user input validation  
- **JSON**: structured communication between server and GUI  

## Learning Outcomes
- Practical experience in **kernel-level Linux programming** and memory-mapped IO.  
- Developing **secure virtual hardware servers** with persistent memory.  
- Building **full-stack diagnostic tools** for SoC validation.  
- Understanding **hardware-software interaction** and safe system programming practices.  
- Exposure to **cross-layer integration**: kernel, server, and GUI.  

---


