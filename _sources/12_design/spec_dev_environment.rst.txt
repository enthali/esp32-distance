Development Environment Design Specification
=============================================

This document defines the required content for ``docs/90_guides/devcontainer.rst``,
specifying which development environments are supported and what the guide must cover.

**Document Version**: 1.0
**Last Updated**: 2026-03-16

.. spec:: Supported Development Environments — Options
   :id: SPEC_DEV_ENV_OPTIONS
   :links: REQ_DEV_ENV_1, SPEC_ARCH_CODESPACES_1
   :status: approved
   :tags: developer, devcontainer, codespaces

   **Description:** The development options section of ``devcontainer.rst``
   SHALL list all supported and unsupported environments.

   **Supported environments:**

   - **GitHub Codespaces** (recommended) — zero-install cloud environment,
     works in any browser, no local Docker required
   - **VS Code Dev Container** (alternative) — requires Docker on
     Linux or macOS host

   **Explicitly not supported:**

   - **Native Windows** — ESP-IDF installs a large toolchain that conflicts
     with Windows PATH conventions and shell assumptions. All development
     SHALL use a container environment.

   **Required content in** ``devcontainer.rst`` **options section:**

   - Codespaces as the recommended path with one-click instructions
   - Local devcontainer as the alternative with Docker version requirement
   - Explicit "Windows not supported" notice with rationale

   **Traceability note:** If ``SPEC_ARCH_CODESPACES_1`` changes the
   devcontainer base image or toolchain version, this spec and the
   ``devcontainer.rst`` options section SHALL be reviewed.


.. spec:: Supported Development Environments — Setup Steps
   :id: SPEC_DEV_ENV_USAGE
   :links: REQ_DEV_ENV_1, SPEC_ARCH_CODESPACES_1
   :status: approved
   :tags: developer, devcontainer, codespaces

   **Description:** The usage section of ``devcontainer.rst`` SHALL provide
   complete setup instructions for each supported environment.

   **Required content for Codespaces path:**

   - Fork / use-template step
   - Open in Codespaces step (Code → Codespaces → Create)
   - Expected startup time (first build)
   - Verification step (``idf.py build`` succeeds)

   **Required content for local devcontainer path:**

   - Prerequisites (VS Code, Docker, Dev Containers extension)
   - Clone and open-in-container steps
   - Container rebuild instructions (when Dockerfile changes)

   **Required content — container features:**

   - Pre-installed toolchain version (ESP-IDF v5.4.1)
   - Available VS Code extensions
   - Build commands reference
