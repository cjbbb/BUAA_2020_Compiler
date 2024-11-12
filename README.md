# BUAA 2020 Compiler Project

[English](README.md) | [中文](README.zh-CN.md)

A MIPS compiler implementation that translates simplified C code into MIPS assembly language. This project was developed as part of the Compiler course at Beihang University (BUAA) in 2022.

## Overview

This compiler implements the complete compilation process from source code to MIPS assembly, including:

- Lexical Analysis (Scanner)
- Syntax Analysis (Parser)
- Error Detection & Handling
- Intermediate Code Generation
- Code Optimization
- MIPS Assembly Generation

## Features

- Source Language: Simplified C
- Target Platform: MIPS Architecture
- Implementation Language: C++
- Key Components:
  - Token recognition and classification
  - Recursive descent parsing
  - Symbol table management
  - Intermediate code (IR) generation
  - Basic optimization techniques
  - MIPS instruction selection and register allocation

## Project Structure

compiler/
├── lex