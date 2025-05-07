# Italian-Flavour-2.0

# It is a COMP3015 CW2 – Shader Feature Project

---

## Overview

This project demonstrates advanced shader-based visual effects using OpenGL. Key features include:

- Wireframe Rendering (Geometry Shader)
- Explosive Triangle Effect
- HDR Rendering with Tone Mapping
- Bloom Effect
- Edge Detection
- Shader Switching and Interactive Controls

---

## Controls

| Key | Function |
|-----|----------|
| `9` | Toggle wireframe geometry shader (ON) |
| `0` | Toggle wireframe geometry shader (OFF) |
| `4` | Toggle HDR |
| `5` | Toggle Bloom |
| `2` | Toggle Edge Detection |
| `1` | Toggle Phong only shading (kind of broken) |
| `3` | Toggle gaussian blur (broken) |
| `I` | Increase sigma2 value (only work when in HDR + bloom mode, kind of broken) |
| `K` | Decrease sigma2 value (only work when in HDR + bloom mode, kind of broken) |
| `O` | Increase gamma value (only work when in HDR + bloom mode, kind of broken) |
| `L` | Decrease gamma value (only work when in HDR + bloom mode, kind of broken) |
| `Esc` | Exit the program |

---

## Features Implemented

### 1. **Wireframe (Geometry Shader)**
A geometry shader dynamically emits triangle edge lines to render a real-time wireframe overlay. This feature can be toggled with the `9` and `0` keys.

### 2. **Explosive Geometry Effect**
This extends the wireframe geometry shader with outward displacement per triangle, creating a dramatic “explosion” animation.

### 3. **HDR + Tone Mapping**
The scene is rendered into a high dynamic range framebuffer, followed by tone mapping for exposure and luminance control.

### 4. **Bloom**
Bright fragments from the HDR render are blurred and combined with the base image to create a bloom effect, enhancing glowing areas.

### 5. **Edge Detection**
A post-processing Sobel filter highlights edges based on intensity differences between pixels, producing a stylized outline effect.

---

## Technical Notes

- All shaders use GLSL 460 Core
- Framebuffer-based postprocessing
- Effects can be toggled live via keyboard
- Geometry shader operates independently of postprocessing pipeline

---

## Challenges Faced

- Debugging fragment shaders and controlling primitive output, trying to find anything that caused fatal errors and didn't mentioned in the program itself
- Managing post-processing order for HDR and bloom
- Ensuring all toggles function reliably at runtime

---

## Video Demonstration

**YouTube Link**: https://youtu.be/D4MqL9_Pofc

---

## Submission

- Submitted via DLE zip and declared using the Feature Declaration Form
