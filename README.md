Surveillance Camera Chip Diagnostic Tools

This project was developed during my internship at BigEndian Semiconductors.
It focuses on building a complete diagnostic framework for a surveillance camera SoC.

The system provides a full-stack solution: from kernel-level memory access to REST APIs and a Qt GUI.
It demonstrates real-world embedded system debugging and register control workflows.

Key components:

Linux Kernel Module

Maps DDR/physical memory using ioremap.

Supports 32-bit read/write operations.

Enforces non-overwrite protection.

Exposes IOCTLs for single and range access.

HTTPS Virtual Register Server (Python/Flask)

Simulates hardware registers in memory.

Provides secure REST endpoints for read, write, clear, and range operations.

Persists memory to disk (vreg.bin).

Performs alignment, range, and overwrite checks.

Qt GUI Diagnostic Tool (C++/Qt)

User-friendly interface for register access and monitoring.

Supports single and range operations with interactive feedback.

File import to load bulk values.

Integrates directly with the kernel module via IOCTL.

Features include:

32-bit enforced access with alignment checks.

Overwrite protection to ensure safety of register writes.

Persistent virtual memory for simulation.

REST API for automation and scripting.

This project demonstrates hardware-software interaction, system-level programming, and secure GUI/server integration.
It provides hands-on experience with Linux kernel development, embedded diagnostics, and full-stack system tools.
The work helped build real-world skills in kernel programming, network servers, and GUI design.
