<!--
Sync Impact Report
- Version change: 1.0.0 -> 1.0.0 (initial project-specific ratification)
- Modified principles: none; all five principles are newly defined for this project
- Added sections: Additional Constraints; Development Workflow
- Removed sections: none
- Follow-up TODOs: confirm the original ratification date and replace the provisional date if needed
-->
# Computer Graphics Final Project Constitution

## Core Principles

### I. Visual Correctness and Mathematical Clarity
Every graphics feature MUST state its geometric, mathematical, and visual behavior clearly. Implementations MUST use coordinate systems, transformations, projection, shading, and rasterization consistently with the feature specification. Numerical assumptions and tolerances MUST be documented when they affect visible results.

### II. Modular, Reproducible Implementation
Graphics functionality MUST be separated into components with clear responsibilities and interfaces. A contributor MUST be able to build and run the project from documented steps and declared dependencies. Changes MUST avoid hidden machine-specific paths, undeclared assets, and unexplained runtime configuration.

### III. Verification of Visible Behavior
Each feature MUST include a repeatable way to verify its required behavior. Tests MUST cover deterministic calculations and data transformations where practical; interactive or visual behavior MUST have explicit manual verification steps or reproducible examples. A change is not complete until its acceptance criteria have been checked.

### IV. Performance Proportional to the Workload
Implementations MUST avoid unnecessary per-frame work, allocations, and resource duplication in interactive rendering paths. Performance-sensitive changes MUST be evaluated against representative scene size and target hardware, and any material tradeoff between visual quality and responsiveness MUST be recorded.

### V. Readability and Explainable Complexity
Code and project documentation MUST make graphics algorithms and non-obvious design choices understandable to another student or reviewer. Contributors MUST prefer the simplest approach that satisfies the specified visual and technical requirements; added abstractions or dependencies require a concrete project need.

## Additional Constraints

The project is a computer graphics course deliverable. Source assets and dependencies MUST have clear provenance and compatible usage terms. Generated or third-party assets MUST be identified in project documentation. Build instructions MUST identify required tools and versions when version differences can affect the result.

## Development Workflow

Feature work MUST begin with written requirements and acceptance criteria. Plans and tasks MUST identify affected components, relevant graphics concepts, and verification steps. Changes MUST be reviewed against the constitution and feature requirements before being considered complete. Commits MUST describe their purpose, and the `main` branch MUST remain buildable to the extent supported by the project setup.

## Governance

This constitution governs project-specific engineering decisions. If another project document conflicts with it, contributors MUST resolve the conflict explicitly and amend the relevant document before implementation proceeds. Amendments MUST update this file, explain the rationale, and increment the version: MAJOR for incompatible governance changes, MINOR for new or materially expanded principles or sections, and PATCH for clarifications that do not change obligations. Every amendment MUST update the last-amended date.

Contributors MUST review each change against the applicable principles and record unmet requirements or justified exceptions in the feature plan or review notes. Exceptions MUST identify the affected principle, rationale, and scope. Compliance review is required before a change is accepted into `main`.

**Version**: 1.0.0 | **Ratified**: 2026-11-01 | **Last Amended**: 2026-11-01
