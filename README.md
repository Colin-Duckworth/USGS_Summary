# USGS RF System

## Overview

This repository contains the documentation and source code for the USGS RF System developed during my summer. The project evolved through two primary implementations:

1. **AD8302-based system** (initial implementation)
2. **NanoVNA-based system** (updated implementation)

The repository is intended as a handoff package for future development and should provide enough information to understand the system architecture and continue the project.

---

## Repository Contents

### `Project.drawio`

The Draw.io file contains the primary documentation for the project and is organized into four tabs.
[Open the Draw.io Diagram]([[https://viewer.diagrams.net/?tags=%7B%7D&lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=USGS_Summary.drawio&dark=auto#Uhttps%3A%2F%2Fdrive.google.com%2Fuc%3Fid%3D1AIg-5LYU0VLGQAHxca7ommk_bvHUHpau%26export%3Ddownload](https://viewer.diagrams.net/?tags=%7B%7D&lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=USGS_RFchain.drawio&page-id=A0hF7V9FG_72OF4ILqJ3&dark=auto#Uhttps%3A%2F%2Fdrive.google.com%2Fuc%3Fid%3D1dTqwNJABriTpAbBzJGA4YtCAzyaBGVeL%26export%3Ddownload)](https://viewer.diagrams.net/?tags=%7B%7D&lightbox=1&highlight=0000ff&edit=_blank&layers=1&nav=1&title=USGS_Summary.drawio&dark=auto#Uhttps%3A%2F%2Fdrive.google.com%2Fuc%3Fid%3D1AIg-5LYU0VLGQAHxca7ommk_bvHUHpau%26export%3Ddownload#%7B%22pageId%22%3A%22Ll7vR38r0CmVPtSaqMqS%22%7D))

#### Tab 1 – RF Chain Diagrams
Contains the RF chain diagrams for both implementations:

- **AD8302 Version** (first)
- **NanoVNA Version** (second)

These diagrams show the RF signal path and overall system architecture for each version.

#### Tab 2 – Calibration Routine
Contains the calibration routine flowchart used by the system.

#### Tab 3 – Power & Digital Signal Flow
Contains the power distribution and digital signal flow diagram for the system electronics.

#### Tab 4 – Custom Filter Design
Contains the design information and documentation for the custom RF filter used in the project.

---

## Source Code

The attached folder also contains the complete codebase used during development of the project.

---

## Recommended Next Steps

### 1. Replace the RF Switch

The current SP4T/SP6T switch used during calibration introduces enough return loss to distort the reference plane. Replacing it with a higher-performance RF switch with improved return loss should significantly improve calibration accuracy.

### 2. Upgrade the Directional Coupler

For the AD8302 implementation, a directional coupler with higher directivity should be used in the calibration path. This will improve isolation between the forward and reflected signals, resulting in more accurate measurements.

### 3. Continue Development of the Cancellation Routine

I have a few ideas for improving the cancellation routine, but Dr. Gogineni has a much deeper understanding of the underlying theory and design considerations. I recommend using his guidance as the primary reference for any future development of the cancellation algorithm. Anyone continuing this project is welcome to contact me at clduckworth@crimson.ua.edu, and I can forward the documentation Dr. Gogineni shared regarding the cancellation routine. Alternatively, you can reach out to Dr. Gogineni directly at pgogineni@ua.edu for guidance.


## Final Notes

The Draw.io diagrams should be the main reference for understanding the hardware architecture, while the source code provides the current software implementation. Together, they should provide a solid foundation for continuing development of the project.
